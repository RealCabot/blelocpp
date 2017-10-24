//
//  EncoderWalkingState.hpp
//  localization-library
//
//  Created by Yanda Huang on 10/22/17.
//  Copyright © 2017 HULOP. All rights reserved.
//

#ifndef EncoderWalkingState_hpp
#define EncoderWalkingState_hpp

#include <stdio.h>
#include <deque>
#include <mutex>
#include "Encoder.hpp"

namespace loc
{
    class EncoderWalkingStateParameters
    {
        double updatePeriod_ = 0.1; // second
        
        public:
            EncoderWalkingStateParameters& updatePeriod(double updatePeriod) { // just a parameter you can set
                updatePeriod_ = updatePeriod;
                return *this;
            }
        
            double walkDetectStdevWindow() const {
                return walkDetectStdevWindow_;
            }
        
            double updatePeriod() const{
                return updatePeriod_;
            }
    };
    
    class EncoderWalkingState : public Encoder
    {
        private:
            long prevUpdateTime = 0;
            bool isUpdated_ = false;
            double nSteps = 0;
            double nStepsConst = 0.1;
        
            EncodererWalkingStateParameters mParameters;
        
        public:
            EncoderWalkingState(){}
            EncoderWalkingState(EncoderWalkingStateParameters parameters);
            ~EncodererWalkingState(){}
        
            EncoderWalkingState& putAcceleration(Acceleration acceleration) override;
            bool isUpdated() override;
            double getNSteps() override;
            void reset() override;
    };
