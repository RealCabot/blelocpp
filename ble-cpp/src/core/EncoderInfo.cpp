//
//  EncoderInfo.cpp
//  localization-library
//
//  Created by Yanda Huang on 10/22/17.
//  Copyright © 2017 HULOP. All rights reserved.
//

#include "EncoderInfo.hpp"

namespace loc
{
    EncoderInfo::EncoderInfo(long timestamp, float position, float velocityL, float velocityR){
        timeStamp_ = timestamp;
        position_ = position;
        velocityL_ = velocityL;
        velocityR_ = velocityR;
    }
    
    EncoderInfo::~EncoderInfo(){}
    
    EncoderInfo* EncoderInfo::setTimeStamp(long timeStamp){
        this->timeStamp_ = timeStamp;
        return this;
    }
    
    EncoderInfo* EncoderInfo::setPosition(float position){
        this->position_ = position;
        return this;
    }
    
    EncoderInfo* EncoderInfo::setVelocityL(float velocity){
        this->velocityL_ = velocity;
        return this;
    }
    
    EncoderInfo* EncoderInfo::setVelocityR(float velocity){
        this->velocityR_ = velocity;
        return this;
    }
    
    long EncoderInfo::getTimeStamp() const{
        return timeStamp_;
    }
    
    float EncoderInfo::getPosition() const{
        return position_;
    }
    
    float EncoderInfo::getVelocityL() const{
        return velocityL_;
    }
    
    float EncoderInfo::getVelocityR() const{
        return velocityR_;
    }
}
