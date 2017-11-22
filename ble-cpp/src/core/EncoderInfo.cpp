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
    EncoderInfo::EncoderInfo(long timestamp, float position, float velocity){
        timeStamp_ = timestamp;
        position_ = position;
        velocity_ = velocity;
    }
    
    /*EncoderInfo::EncoderInfo(long timestamp, float position, float velocityL, float velocityR){
        timeStamp_ = timestamp;
        position_ = position;
        velocityL_ = velocityL;
        velocityR_ = velocityR;
    }*/
    
    EncoderInfo::~EncoderInfo(){}
    
    EncoderInfo* EncoderInfo::setTimeStamp(long timeStamp){
        this->timeStamp_ = timeStamp;
        return this;
    }
    
    EncoderInfo* EncoderInfo::setPosition(float position){
        this->position_ = position;
        return this;
    }
    
    EncoderInfo* EncoderInfo::setVelocity(float velocity){
        this->velocity_ = velocity;
        return this;
    }
    
    long EncoderInfo::getTimeStamp() const{
        return timeStamp_;
    }
    
    float EncoderInfo::getPosition() const{
        return position_;
    }
    
    float EncoderInfo::getVelocity() const{
        return velocity_;
    }
}
