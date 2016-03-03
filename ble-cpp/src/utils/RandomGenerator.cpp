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

#include <iostream>
#include "RandomGenerator.hpp"

namespace loc{
    
    int RandomGenerator::nextInt(int n){
        std::uniform_int_distribution<> uniIntDist(0,n);
        return uniIntDist(engine);
    }
    
    double RandomGenerator::nextDouble(){
        return uniformDistribution(engine);
    }
    
    double RandomGenerator::nextGaussian(){
        return normalDistribution(engine);
    }
    
    double RandomGenerator::nextTruncatedGaussian(double mean, double std, double min, double max){
        if(!(min<=mean && mean<= max)){
            std::cout << "mean=" << mean << ", min=" << min << ", max=" << max << std::endl;
            assert(min<=mean);
            assert(mean<= max);
        }
        double value = mean;
        for(int i=0;i<max_iteration; i++){
            value = mean + std*nextGaussian();
            if(min<=value && value<=max){
                break;
            }
        }
        return value;
    }
    
    std::vector<int> RandomGenerator::randomSet(int n, int k){
        assert(n>0);
        assert(k>0);
        
        std::vector<int> indices(n);
        for(int i=0; i<n; i++){
            indices[i] = i;
        }
        std::random_shuffle(indices.begin(), indices.end());
        
        std::vector<int> intSet(k);
        
        for(int i=0; i<k ; i++){
            int idx = i%n;
            intSet[i] = indices.at(idx);
        }
        return intSet;
    }
}