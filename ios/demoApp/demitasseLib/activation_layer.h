//
//  activation_layer.h
//  demitasseLib
//
//  Created by Satoshi Kondo on 2016/04/01.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

#ifndef __DEMITASSE_IOS_ACTIVATION_LAYER__
#define __DEMITASSE_IOS_ACTIVATION_LAYER__

#import "base_layer.h"

typedef enum ActivationFunc : NSUInteger
{
    ActivationFuncIdentity = 0, // default
    ActivationFuncSigmoid,
    ActivationFuncTanh,
    ActivationFuncReLU,
    ActivationFuncSoftMax
} ActivationFunc;


@interface ActivationLayer : BaseLayer

- (nullable id)init:(ActivationFunc)type;

@end

#endif /* __DEMITASSE_IOS_ACTIVATION_LAYER__ */
