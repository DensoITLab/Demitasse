//
//  pooling_layer.h
//  demitasseLib
//
//  Created by Satoshi Kondo on 2016/04/01.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

#ifndef __DEMITASSE_IOS_POOLING_LAYER__
#define __DEMITASSE_IOS_POOLING_LAYER__

#import "base_layer.h"

typedef enum PoolingFunc : NSUInteger
{
    PoolingFuncMax = 0, // default
    PoolingFuncAvarage
} PoolingFunc;

@interface PoolingLayer : BaseLayer

- (nullable id)init:(PoolingFunc)type
              ksize:(unsigned int)ksize
             stride:(unsigned int)stride
            padding:(unsigned int)padding;

- (nullable id)init:(PoolingFunc)type
              ksize:(unsigned int)ksize
             stride:(unsigned int)stride;

- (nullable id)init:(PoolingFunc)type; // called by model

@end

#endif /* __DEMITASSE_IOS_POOLING_LAYER__ */
