//
//  activation_layer.mm
//  demitasse
//
//  Created by Satoshi Kondo on 2016/04/01.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "activation_layer.h"
#import "base_layer_protected.h"

#include "activation_layer.hpp"
#include "activation_function.hpp"

@implementation ActivationLayer

- (nullable id)init:(ActivationFunc)type {
    if (self = [super init]) {
        switch (type) {
            case ActivationFuncIdentity:
                self->obj_ = new demitasse::ActivationLayer<demitasse::identity>();
                break;
            case ActivationFuncSigmoid:
                self->obj_ = new demitasse::ActivationLayer<demitasse::sigmoid>();
                break;
            case ActivationFuncTanh:
                self->obj_ = new demitasse::ActivationLayer<demitasse::tanh>();
                break;
            case ActivationFuncReLU:
                self->obj_ = new demitasse::ActivationLayer<demitasse::ReLU>();
                break;
            case ActivationFuncSoftMax:
                self->obj_ = new demitasse::ActivationLayer<demitasse::softmax>();
                break;
        }
    }

    return self;
}


@end