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

#ifndef StatusInitializer_hpp
#define StatusInitializer_hpp

#include <stdio.h>
#include "bleloc.h"

namespace loc{
    
    class StatusInitializer{
    public:
        virtual std::vector<Location> initializeLocations(int n) = 0;
        virtual std::vector<Pose> initializePoses(int n) = 0;
        virtual std::vector<State> initializeStates(int n) = 0;
        
        virtual std::vector<State> resetStates(int n, Pose pose, double orientationMeasured) = 0;
        virtual std::vector<State> resetStates(int n, Pose meanPose, Pose stdevPose, double orientationMeasured) = 0;
        virtual std::vector<State> resetStates(int n, const std::vector<Beacon>& beacons) = 0;
        
        virtual States initializeStatesFromLocations(const Locations& locations) = 0;
    };
    
}

#endif /* StatusInitializer_hpp */
