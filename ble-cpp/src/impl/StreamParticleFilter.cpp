/*******************************************************************************
 * Copyright (c) 2014, 2015  IBM Corporation and others
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *******************************************************************************/

#include <thread>
#include <queue>
#include <functional>

#include "StreamParticleFilter.hpp"
#include "StreamLocalizer.hpp"

#include "ObservationModel.hpp"

#include "SystemModel.hpp"

#include "Resampler.hpp"
#include "GridResampler.hpp"

#include "StatusInitializer.hpp"

#include "PoseRandomWalker.hpp"

#include "ArrayUtils.hpp"

#include "DataStore.hpp"
#include "DataLogger.hpp"
#include "BaseBeaconFilter.hpp"
#include "CleansingBeaconFilter.hpp"

namespace loc{
    
    class StreamParticleFilter::Impl{

    private:
        bool optVerbose = false;
        int mNumStates = 1000; //Defalt value
        double mAlphaWeaken = 1.0;
        int resetWaitingTimeMS = 100; // milliseconds
        
        //std::queue<Pose> posesForReset;
        std::queue<std::function<void()>> functionsForReset;
        
        std::shared_ptr<Status> status;
        
        std::shared_ptr<Pedometer> mPedometer;
        std::shared_ptr<OrientationMeter> mOrientationmeter;
        
        std::shared_ptr<SystemModel<State, PoseRandomWalkerInput>> mRandomWalker;
        
        std::shared_ptr<ObservationModel<State, Beacons>> mObservationModel;
        std::shared_ptr<Resampler<State>> mResampler;
        
        std::shared_ptr<StatusInitializer> mStatusInitializer;
        
        std::shared_ptr<BeaconFilter> mBeaconFilter;
        
        void (*mFunctionCalledAfterUpdate)(Status *);
        
        bool accelerationIsUpdated = false;
        bool attitudeIsUpdated = false;
        bool beaconsIsUpdated = false;
        
        CleansingBeaconFilter cleansingBeaconFilter;
        
    public:
        
        Impl() : status(new Status()),
                mBeaconFilter(new BaseBeaconFilter())
        { }
        
        ~Impl(){}
        
        void initializeStatusIfZero(){
            if(status->states()->size()==0) {
                initializeStatus();
            }
        }
        
        void putAcceleration(const Acceleration acceleration){
            initializeStatusIfZero();

            mPedometer->putAcceleration(acceleration);
            accelerationIsUpdated = mPedometer->isUpdated();
            
            // TODO (Tentative implementation)
            if(accelerationIsUpdated && attitudeIsUpdated){
                predictMotionState(acceleration.timestamp());
            }
            
        }
        
        void putAttitude(const Attitude attitude){
            initializeStatusIfZero();
            mOrientationmeter->putAttitude(attitude);
            attitudeIsUpdated = mOrientationmeter->isUpdated();
            
            processResetStatus();
        }
        
        void predictMotionState(long timestamp){
            initializeStatusIfZero();
            
            static long previousTimestamp = 0;
            if(previousTimestamp==0) previousTimestamp = timestamp;
            
            PoseRandomWalkerInput input;
            input.timestamp(timestamp);
            input.previousTimestamp(previousTimestamp);
            
            std::shared_ptr<States> states = status->states();
            States* statesPredicted = new States(mRandomWalker->predict(*states.get(), input));
            status->states(statesPredicted);
            
            if(optVerbose){
                std::cout << "prediction at t=" << timestamp << std::endl;
            }
            
            previousTimestamp = timestamp;
            
            mFunctionCalledAfterUpdate(status.get());
        }
        
        void logStates(const States& states, const std::string& filename){
            std::stringstream ss;
            for(State s: states){
                ss << s << std::endl;
            }
            if(DataLogger::getInstance()){
                DataLogger::getInstance()->log(filename, ss.str());
            }
        }
        
        void doFiltering(const Beacons& beacons){
            long timestamp = beacons.timestamp();
            std::shared_ptr<States> states = status->states();
            
            // Logging before likelihood computation
            logStates(*states, "before_likelihood_states_"+std::to_string(timestamp)+".csv");
            
            std::vector<double> vLogLLs = mObservationModel->computeLogLikelihood(*states, beacons);
            vLogLLs = weakenLogLikelihoods(vLogLLs, mAlphaWeaken);
            // Set negative log-likelihoods
            for(int i=0; i<vLogLLs.size(); i++){
                states->at(i).negativeLogLikelihood(-vLogLLs.at(i));
            }
            
            std::vector<double> weights = ArrayUtils::computeWeightsFromLogLikelihood(vLogLLs);
            double sumWeights = 0;
            // Multiply loglikelihood-based weights and particle weights.
            for(int i=0; i<weights.size(); i++){
                weights[i] = weights[i] * (states->at(i).weight());
                sumWeights += weights[i];
            }
            assert(sumWeights>0);
            // Renormalized
            for(int i=0; i<weights.size(); i++){
                weights[i] = weights[i]/sumWeights;
                states->at(i).weight(weights[i]);
            }
            
            // Logging after likelihood computation
            logStates(*states, "after_likelihood_states_"+std::to_string(timestamp)+".csv");
            
            States* statesNew = mResampler->resample(*states, &weights[0]);
            // Assign equal weights after resampling
            for(int i=0; i<weights.size(); i++){
                double weight = 1.0/(weights.size());
                statesNew->at(i).weight(weight);
            }
            status->states(statesNew);
            if(optVerbose){
                std::cout << "resampling at t=" << beacons.timestamp() << std::endl;
            }
            
            // Logging after resampling
            logStates(*statesNew, "resampled_states_"+std::to_string(timestamp)+".csv");
            
            double sumNegaLogLL = 0;
            for(State s: *statesNew){
                sumNegaLogLL += s.negativeLogLikelihood();
            }
            assert( sumNegaLogLL != 0.0);
            
        }
        
        Beacons filterBeacons(const Beacons& beacons){
            size_t nBefore = beacons.size();
            const Beacons& beaconsCleansed = cleansingBeaconFilter.filter(beacons);
            Beacons beaconsFiltered = mBeaconFilter? mBeaconFilter->filter(beaconsCleansed) : beaconsCleansed;
            size_t nAfter = beaconsFiltered.size();
            if(optVerbose){
                if(nAfter!=nBefore){
                    std::cout << "BeaconFilter #beacons "<< nBefore << ">>" << nAfter <<std::endl;
                }
            }
            return beaconsFiltered;
        }
        
        void putBeacon(const Beacons & beacons){
            initializeStatusIfZero();
            
            const Beacons& beaconsFiltered = filterBeacons(beacons);

            doFiltering(beaconsFiltered);
            
            mFunctionCalledAfterUpdate(status.get());
        };
        
        static std::vector<double> weakenLogLikelihoods(std::vector<double> logLikelihoods, double alphaWeaken){
            size_t n = logLikelihoods.size();
            std::vector<double> weakenedLogLLs(n);
            for(int i=0; i<n; i++){
                weakenedLogLLs[i] = alphaWeaken*logLikelihoods[i];
            }
            return weakenedLogLLs;
        }
        
        void initializeStatus(){
            mPedometer->reset();
            mOrientationmeter->reset();
            States* states = new States(mStatusInitializer->initializeStates(mNumStates));
            updateStatus(states);
        }
        
        void updateStatus(States* states){
            Status *st = new Status();
            st->states(states);
            status.reset(st);
        }
        
        void updateHandler(void (*functionCalledAfterUpdate)(Status*)){
            mFunctionCalledAfterUpdate = functionCalledAfterUpdate;
        }
        
        Status* getStatus(){
            return status.get();
        };
        
        bool resetStatus(){
            initializeStatus();
            return true;
        }
        
        bool resetStatus(Pose pose){
            bool orientationWasUpdated = mOrientationmeter->isUpdated();
            if(orientationWasUpdated){
                std::cout << "Orientation is updated. Reset succeeded." << std::endl;
                double orientationMeasured = mOrientationmeter->getYaw();
                States* states = new States(mStatusInitializer->resetStates(mNumStates, pose, orientationMeasured));
                status->states(states);
                mFunctionCalledAfterUpdate(status.get());
                return true;
            }else{
                std::cout << "Orientation has not been updated. Reset input is cached to be processed later." << std::endl;
                //posesForReset.push(pose);
                std::function<void()> func = [this,pose](){
                    return resetStatus(pose);
                };
                functionsForReset.push(func);
                return false;
            }
        }
        
        bool resetStatus(Pose meanPose, Pose stdevPose){
            bool orientationWasUpdated = mOrientationmeter->isUpdated();
            if(orientationWasUpdated){
                std::cout << "Orientation is updated. Reset succeeded." << std::endl;
                double orientationMeasured = mOrientationmeter->getYaw();
                States* states = new States(mStatusInitializer->resetStates(mNumStates, meanPose, stdevPose, orientationMeasured));
                status->states(states);
                mFunctionCalledAfterUpdate(status.get());
                return true;
            }else{
                std::cout << "Orientation has not been updated. Reset input is cached to be processed later." << std::endl;
                std::function<void()> func = [this,meanPose,stdevPose](){
                    return resetStatus(meanPose, stdevPose);
                };
                functionsForReset.push(func);
                return false;
            }
        }
        
        bool refineStatus(const Beacons& beacons){
            // TODO
            initializeStatusIfZero();
            /*
            States* states = new States(mStatusInitializer->initializeStates(mNumStates));
            status->states(states);
            */
            const Beacons& beaconsFiltered = filterBeacons(beacons);
            doFiltering(beaconsFiltered);
            
            std::shared_ptr<States> statesTmp = status->states();
            std::vector<Location> locations(statesTmp->begin(), statesTmp->end());
            States* statesNew = new States(mStatusInitializer->initializeStatesFromLocations(locations));
            status->states(statesNew);
            
            return false;
        }
        
        void processResetStatus(){
            /*
            if(posesForReset.size()>0){
                bool orientationWasUpdated = mOrientationmeter->isUpdated();
                if(orientationWasUpdated){
                    Pose pose = posesForReset.back();
                    posesForReset.pop();
                    resetStatus(pose);
                }
            }
            */
            if(functionsForReset.size()>0){
                bool orientationWasUpdated = mOrientationmeter->isUpdated();
                if(orientationWasUpdated){
                    std::function<void()> func = functionsForReset.back();
                    func();
                    functionsForReset.pop();
                    std::cout << "Stored reset request was processed." << std::endl;
                }
            }
        }
        
        void numStates(int numStates){
            mNumStates = numStates;
        }
        
        void alphaWeaken(double alphaWeaken){
            mAlphaWeaken = alphaWeaken;
        }
        
        void pedometer(std::shared_ptr<Pedometer> pedometer){
            mPedometer=pedometer;
        }
        
        void orientationmeter(std::shared_ptr<OrientationMeter> orientationMeter){
            mOrientationmeter = orientationMeter;
        }
        
        void systemModel(std::shared_ptr<SystemModel<State, PoseRandomWalkerInput>> randomWalker){
            mRandomWalker = randomWalker;
        }
        
        void observationModel(std::shared_ptr<ObservationModel<State, Beacons>> observationModel){
            mObservationModel = observationModel;
        }
        
        void resampler(std::shared_ptr<Resampler<State>> resampler){
            mResampler = resampler;
        }
        
        void statusInitializer(std::shared_ptr<StatusInitializer> statusInitializer){
            mStatusInitializer = statusInitializer;
        }
        
        void beaconFilter(std::shared_ptr<BeaconFilter> beaconFilter){
            mBeaconFilter = beaconFilter;
        }
        
    };
    
    
    // Delegation to StreamParticleFilter::Impl class
    
    StreamParticleFilter::StreamParticleFilter() : impl(new StreamParticleFilter::Impl()) {}
    
    StreamParticleFilter::~StreamParticleFilter(){}
    
    StreamParticleFilter& StreamParticleFilter::putAcceleration(const Acceleration acceleration){
        impl->putAcceleration(acceleration);
        return *this;
    }
    
    StreamParticleFilter& StreamParticleFilter::putAttitude(const Attitude attitude) {
        impl->putAttitude(attitude);
        return *this;
    }
    
    StreamParticleFilter& StreamParticleFilter::putBeacons(const Beacons beacons) {
        impl->putBeacon(beacons);
        return *this;
    }
    
    StreamParticleFilter& StreamParticleFilter::updateHandler(void (*functionCalledAfterUpdate)(Status*)) {
        impl->updateHandler(functionCalledAfterUpdate);
        return *this;
    }
    
    Status* StreamParticleFilter::getStatus() {
        return impl->getStatus();
    }
    
    
    bool StreamParticleFilter::resetStatus(){
        return impl->resetStatus();
    }
    
    bool StreamParticleFilter::resetStatus(Pose pose){
        return impl->resetStatus(pose);
    }
    
    bool StreamParticleFilter::resetStatus(Pose meanPose, Pose stdevPose){
        return impl->resetStatus(meanPose, stdevPose);
    }
    
    bool StreamParticleFilter::refineStatus(const Beacons& beacons){
        return impl->refineStatus(beacons);
    }
    
    
    StreamParticleFilter& StreamParticleFilter::numStates(int numStates){
        impl->numStates(numStates);
        return *this;
    }
    
    StreamParticleFilter& StreamParticleFilter::alphaWeaken(double alphaWeaken){
        impl->alphaWeaken(alphaWeaken);
        return *this;
    }
    
    StreamParticleFilter& StreamParticleFilter::pedometer(std::shared_ptr<Pedometer> pedometer){
        impl->pedometer(pedometer);
        return *this;
    }
    StreamParticleFilter& StreamParticleFilter::orientationMeter(std::shared_ptr<OrientationMeter> orientationMeter){
        impl->orientationmeter(orientationMeter);
        return *this;
    }
    
    StreamParticleFilter& StreamParticleFilter::systemModel(std::shared_ptr<SystemModel<State, PoseRandomWalkerInput>> randomWalker){
        impl->systemModel(randomWalker);
        return *this;
    }
    
    StreamParticleFilter& StreamParticleFilter::observationModel(std::shared_ptr<ObservationModel<State, Beacons>> observationModel){
        impl->observationModel(observationModel);
        return *this;
    }
    
    StreamParticleFilter& StreamParticleFilter::resampler(std::shared_ptr<Resampler<State>> resampler){
        impl->resampler(resampler);
        return *this;
    }
    
    StreamParticleFilter& StreamParticleFilter::statusInitializer(std::shared_ptr<StatusInitializer> statusInitializer){
        impl->statusInitializer(statusInitializer);
        return *this;
    }
    
    StreamParticleFilter& StreamParticleFilter::beaconFilter(std::shared_ptr<BeaconFilter> beaconFilter){
        impl->beaconFilter(beaconFilter);
        return *this;
    }
    
}