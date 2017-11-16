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

#include "PoseRandomWalker.hpp"


namespace loc{
    
    PoseRandomWalker& PoseRandomWalker::setProperty(PoseRandomWalkerProperty::Ptr property){
        mProperty = property;
        return *this;
    }
    
    PoseRandomWalker& PoseRandomWalker::setPoseProperty(loc::PoseProperty::Ptr poseProperty){
        this->poseProperty = poseProperty;
        return *this;
    }
    
    PoseRandomWalker& PoseRandomWalker::setStateProperty(StateProperty::Ptr stateProperty){
        this->stateProperty = stateProperty;
        return *this;
    }
    
    std::vector<State> PoseRandomWalker::predict(std::vector<State> states, SystemModelInput input, EncoderInfo encoderInfo){
        std::vector<State> statesPredicted(states.size());
        for(int i=0; i<states.size(); i++){
            statesPredicted[i]= predict(states[i], input, encoderInfo);
        }
        return statesPredicted;
    }
    
    ////////////////////
    //                //
    //  Motion Model  //
    //                //
    ////////////////////
    State PoseRandomWalker::predict(State state, SystemModelInput input, EncoderInfo encoderInfo){
        //long timestamp = input.timestamp;
        //long previousTimestamp = input.previousTimestamp;
        
        double dTime = (input.timestamp()-input.previousTimestamp())/(1000.0); //[s] Difference in time
        double movLevel = movingLevel();
        double nSteps = mProperty->pedometer()->getNSteps();
        double yaw = mProperty->orientationMeter()->getYaw();
        
        float vL = encoderInfo.getVelocityL();
        float vR = encoderInfo.getVelocityR();
        
        // Perturb variables in State
        if(nSteps > 0 || mProperty->doesUpdateWhenStopping() ){
            state.orientationBias(state.orientationBias() + stateProperty->diffusionOrientationBias()*randomGenerator.nextGaussian()*dTime );
            state.rssiBias(randomGenerator.nextTruncatedGaussian(state.rssiBias(),
                                                                 stateProperty->diffusionRssiBias()*dTime,
                                                                 stateProperty->minRssiBias(),
                                                                 stateProperty->maxRssiBias()));
        }
        
        // Update orientation
        double previousOrientation = state.orientation();
        double orientationActual = yaw - state.orientationBias();
        // orientationActual += poseProperty->stdOrientation()*randomGenerator.nextGaussian()*dTime;   // commented out by Chris
        orientationActual = Pose::normalizeOrientaion(orientationActual);
        state.orientation(orientationActual);
        
        // Reduce velocity when turning
        double oriDiff = Pose::computeOrientationDifference(previousOrientation, orientationActual);
        double angularVelocityLimit = mProperty->angularVelocityLimit();
        double turningVelocityRate = std::sqrt(1.0 - std::min(1.0, std::pow(oriDiff/angularVelocityLimit,2)));
        
        // Perturb variables in Pose
        double v = 0.0;
        double nV = state.normalVelocity();
        if(nSteps >0 || mProperty->doesUpdateWhenStopping()){
            nV = randomGenerator.nextTruncatedGaussian(encoderInfo.getVelocityL(),  // used to be state.normalVelocity()
                                                       poseProperty->diffusionVelocity()*dTime,
                                                       poseProperty->minVelocity(),
                                                       poseProperty->maxVelocity());
            state.normalVelocity(nV);
        }
        
        // Update velocity at the moment
        if(nSteps > 0){
            v = nV * velocityRate() * turningVelocityRate;
        }
        
        if(relativeVelocity()>0){  // commented out by Chris
            /*v += randomGenerator.nextTruncatedGaussian(relativeVelocity(),
                                                       poseProperty->diffusionVelocity()*dTime,
                                                       poseProperty->minVelocity(),
                                                       poseProperty->maxVelocity());*/
            
            state.velocity(v);
        }
        // commented out by Chris
        // state.velocity(v);
        // Update in-plane coordinate. OG
        // double x = state.x() + state.vx() * dTime;
        // double y = state.y() + state.vy() * dTime;
        
        /////////////////
        //  propagate  //
        /////////////////
        float l = 0.0125;
        double x = 0;
        double y = 0;
        double the = 0;
        // case 1
        if (vR == vL) {
            x = state.x()+vR*cos(orientationActual)*dTime;
            y = state.y()+vL*cos(orientationActual)*dTime;
            // the = state.orientation();
        }
        // case 2
        else if ( vR == -vL) {
            x = state.x();
            y = state.y();
            // the = state.orientation() + 2*vR*dTime/l;
        }
        // case 3
        else {
            float R = l/2*(vL+vR)/(vR-vL);
            float wdt = (orientationActual - state.orientation())/l;
            float ICCx = state.x() - R*sin(orientationActual);
            float ICCy = state.y() + R*cos(orientationActual);
            
            x = cos(wdt)*(state.x()-ICCx) - sin(wdt)*(y-ICCy) + ICCx;
            y = sin(wdt)*(state.x()-ICCx) + cos(wdt)*(y-ICCy) + ICCy;
            // the = state.orientation() + wdt;
        }
        
        //////////////
        //  Update  //
        //////////////
        State statePred(state);
        statePred.x(x);
        statePred.y(y);
        statePred.orientation(orientationActual); // save previous orientation to find delta later for motion model
        
        return statePred;
    }

    double PoseRandomWalker::movingLevel(){
        if(isUnderControll){
            return mMovement;
        }
        else{
            return mProperty->pedometer()->getNSteps();
        }
    }
}
