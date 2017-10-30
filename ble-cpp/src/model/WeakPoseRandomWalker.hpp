/*******************************************************************************
 * Copyright (c) 2014, 2016  IBM Corporation and others
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

#ifndef WeakRandomWalker_hpp
#define WeakRandomWalker_hpp

#include <stdio.h>
#include <memory>
#include "SystemModel.hpp"
#include "RandomWalkerMotion.hpp"
#include "PoseRandomWalker.hpp"
#include "PosteriorResampler.hpp"

#include "EncoderInfo.hpp"

namespace loc {
    
    class WeakPoseRandomWalkerProperty{
        double pOrientationBiasJump = 0.1; // defines probability of contamination of uniformly-distributed noise to orientation bias
        double pOrientationJump = 0.0; // defines probability of contamination of uniformly-distributed noise to orientation
        double mPoseRandomWalkRate = 0.5; // defines mixture rate of orientation-based prediction
        double mRandomWalkRate = 0.5; // defines mixture rate of random-walk-based prediction
        
        double pBackwardMove = 0.0;
        
        double mMaxLongTimestep = 1.2;
        
        
    public:
        using Ptr = std::shared_ptr<WeakPoseRandomWalkerProperty>;
        
        void probabilityOrientationBiasJump(double probabilityOrientationBiasJump_){
            pOrientationBiasJump = probabilityOrientationBiasJump_;
        }
        double probabilityOrientationBiasJump() const{
            return pOrientationBiasJump;
        }
        
        void probabilityOrientationJump(double probabilityOrientationJump_){
            pOrientationJump = probabilityOrientationJump_;
        }
        double probabilityOrientationJump() const{
            return pOrientationJump;
        }
        
        void poseRandomWalkRate(double poseRandomWalkRate_){
            mPoseRandomWalkRate = poseRandomWalkRate_;
        }
        
        double poseRandomWalkRate() const{
            return mPoseRandomWalkRate;
        }
        
        void randomWalkRate(double randomWalkRate_){
            mRandomWalkRate = randomWalkRate_;
        }
        
        double randomWalkRate() const{
            return mRandomWalkRate;
        }
        
        void probabilityBackwardMove(double probabilityBackwardMove){
            pBackwardMove = probabilityBackwardMove;
        }
        
        double probabilityBackwardMove() const{
            return pBackwardMove;
        }
        
        void maxLongTimestep(double timestep){
            mMaxLongTimestep = timestep;
        }
        
        double maxLongTimestep() const{
            return mMaxLongTimestep;
        }
    };
    
    
    // Tentative name
    template<class Ts = State, class Tin = SystemModelInput, class Tin2 = EncoderInfo>
    class WeakPoseRandomWalker: public RandomWalkerMotion<Ts, Tin, Tin2>{
        using RandomWalkerMotion<Ts, Tin, Tin2>::currentTimestamp;
        using RandomWalkerMotion<Ts, Tin, Tin2>::currentYaw;
        using RandomWalkerMotion<Ts, Tin, Tin2>::wasYawUpdated;
        using RandomWalkerMotion<Ts, Tin, Tin2>::turningVelocityRate;
        
        PoseProperty::Ptr mPoseProperty;
        StateProperty::Ptr mStateProperty;
        WeakPoseRandomWalkerProperty::Ptr wPRWProperty;
        
        bool enabledPredictions = false;
        bool wasFiltered = false;
        long previousTimestampResample = 0;
        
    public:
        using Ptr = std::shared_ptr<WeakPoseRandomWalker<Ts, Tin, Tin2>>;
        using RandomWalker<Ts, Tin, Tin2>::predict;
        using RandomWalkerMotion<Ts, Tin, Tin2>::predict;
        using RandomWalkerMotion<Ts, Tin, Tin2>::velocityRate;
        using RandomWalkerMotion<Ts, Tin, Tin2>::setProperty;
        using RandomWalkerMotion<Ts, Tin, Tin2>::relativeVelocity;
        
        WeakPoseRandomWalker() : wPRWProperty(new WeakPoseRandomWalkerProperty) {
            // pass
        }
        
        virtual ~WeakPoseRandomWalker() = default;
        virtual Ts predict(Ts state, Tin input, Tin2 encoderInfo) override;
        virtual void startPredictions(const std::vector<Ts>& states, const Tin&) override;
        virtual void endPredictions(const std::vector<Ts>& states, const Tin&) override;
        virtual void notifyObservationUpdated() override;
        
        virtual void setWeakPoseRandomWalkerProperty(WeakPoseRandomWalkerProperty::Ptr wPRWProperty){
            this->wPRWProperty = wPRWProperty;
        }
        
        void setPoseProperty(PoseProperty::Ptr poseProperty){
            mPoseProperty = poseProperty;
        }
        void setStateProperty(StateProperty::Ptr stateProperty){
            mStateProperty = stateProperty;
        }
        
    };
}

#endif /* PoseRandomWalkerResampling_hpp */
