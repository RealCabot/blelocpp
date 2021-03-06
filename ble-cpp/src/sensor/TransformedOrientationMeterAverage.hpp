/*******************************************************************************
 * Copyright (c) 2014, 2017  IBM Corporation and others
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

#ifndef TransformedOrientationMeterAverage_hpp
#define TransformedOrientationMeterAverage_hpp

#include <stdio.h>
#include <cmath>

#include "bleloc.h"
#include "OrientationMeter.hpp"
#include "OrientationMeterAverage.hpp"

namespace loc{
    
    using TransformedOrientationMeterAverageParameters = OrientationMeterAverageParameters;
    
    class TransformedOrientationMeterAverage : public OrientationMeter{
        
    private:
        long prevTimestamp = 0;
        double theta = 0;
        bool isUpdated_ = false;
        long count = 1;
        
        TransformedOrientationMeterAverageParameters parameters_;
        
        double getRatio();
        double getRatio(double dt);
        
    public:
        TransformedOrientationMeterAverage(){}
        TransformedOrientationMeterAverage(TransformedOrientationMeterAverageParameters parameters){
            parameters_ = parameters;
        }
        
        ~TransformedOrientationMeterAverage(){}
        
        OrientationMeter& putAttitude(Attitude attitude) override;
        bool isUpdated() override;
        double getYaw() override;
        void reset() override;
        
        static double transformOrientation(const Attitude& attitude);
    };
}


#endif /* TransformedOrientationMeterAverage_hpp */
