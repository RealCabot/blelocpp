//
//  Encoder.cpp
//  localization-library
//
//  Created by Yanda Huang on 10/22/17.
//  Copyright © 2017 HULOP. All rights reserved.
//

#ifndef Encoder_hpp
#define Encoder_hpp

#include <stdio.h>
#include "bleloc.h"

namespace loc
{
    class Encoder
    {
    public:
        using PtrE = std::shared_ptr<Encoder>;
        
        // virtual means it can be overriden by functions inheriting from it
        virtual ~Encoder(){}
        
        // & denotes reference instead of pointer to an object, guaranteed non-NULL
        virtual Encoder& putAcceleration(Acceleration acceleration) = 0;
        
        virtual bool isUpdated() = 0;
        virtual double getNCounts() = 0;
        virtual void reset() = 0;
    };
}

#endif /* Encoder_hpp */
