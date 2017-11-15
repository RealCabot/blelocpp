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

#ifndef EncoderInfo_hpp
#define EncoderInfo_hpp

#include <iostream>
#include <vector>

namespace loc
{
    class EncoderInfo
    {
        private:
            long timeStamp_;
            float position_;
            float velocityL_ = 1.0;  // m/s
            float velocityR_ = 1.0;  // m/s
        
        public:
            EncoderInfo(long timestamp, float position, float velocityL, float velocityR);
            ~EncoderInfo();
        
            long  getTimeStamp() const;
            float getPosition() const;
            float getVelocityL() const;
            float getVelocityR() const;

            EncoderInfo* setTimeStamp(long timestamp);
            EncoderInfo* setPosition(float position);
            EncoderInfo* setVelocityL(float velocity);
            EncoderInfo* setVelocityR(float velocity);
        
            //friend std::ostream& operator<<(std::ostream&os, const Acceleration& acc);
    };
    typedef std::vector<EncoderInfo> EncoderInfos;    
}
#endif /* Acceleration_hpp */

