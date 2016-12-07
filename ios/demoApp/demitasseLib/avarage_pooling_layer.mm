//
//  avarage_pooling_layer.mm
//  vudnn
//
//  Created by Satoshi Kondo on 2016/04/01.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "avarage_pooling_layer.h"
#import "base_layer_protected.h"

#include "avarage_pooling_layer.hpp"

@implementation AvaragePoolingLayer

- (nullable id)init:(int)ksize stride:(int)stride padding:(int)padding {
    if (self = [super init]) {
        obj_ = new vudnn::AvaragePoolingLayer(ksize, stride, padding);
        if (obj_ == nil) {
            return nil;
        }
    }

    return self;
}

- (nullable id)init:(int)ksize stride:(int)stride {
    return [self init:ksize stride:stride padding:0];
}


@end