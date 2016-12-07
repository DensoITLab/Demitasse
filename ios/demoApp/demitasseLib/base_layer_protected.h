//
//  base_layer_protected.h
//  demitasseLib
//
//  Created by Satoshi Kondo on 2016/04/01.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

#ifndef __DEMITASSE_IOS_BASE_LAYER_PROTECTED__
#define __DEMITASSE_IOS_BASE_LAYER_PROTECTED__

#import "base_layer.h"

#include "base_layer.hpp"

@interface BaseLayer() {
  @public demitasse::BaseLayer *obj_;
}

- (void)setConstLayer:(nonnull const demitasse::BaseLayer *)layer;

@end

#endif /* __DEMITASSE_IOS_BASE_LAYER_PROTECTED__ */
