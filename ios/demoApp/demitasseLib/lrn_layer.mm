//
//  lrn_layer.mm
//  demitasseLib
//
//  Created by Satoshi Kondo on 2016/04/01.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "lrn_layer.h"
#import "base_layer_protected.h"

#include "lrn_layer.hpp"

@implementation LRNLayer

- (nullable id)init:(int)mode local_size:(int)local_size k:(float)k alpha:(float)alpha beta:(float)beta
{
    if (self = [super init]) {
        demitasse::LRNParam lrn_param;
        lrn_param.mode       = mode;
        lrn_param.local_size = local_size;
        lrn_param.k          = k;
        lrn_param.alpha      = alpha;
        lrn_param.beta       = beta;
        obj_ = new demitasse::LRNLayer(&lrn_param);
        if (obj_ == nil) {
            return nil;
        }
    }

    return self;
}


@end