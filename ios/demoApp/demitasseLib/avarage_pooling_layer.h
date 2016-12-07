//
//  avarage_pooling_layer.h
//  vudnn
//
//  Created by Satoshi Kondo on 2016/04/01.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

#ifndef avarage_pooling_layer_h
#define avarage_pooling_layer_h

#import "base_layer.h"

@interface AvaragePoolingLayer : BaseLayer

- (nullable id)init:(int)ksize stride:(int)stride padding:(int)padding;
- (nullable id)init:(int)ksize stride:(int)stride;

@end

#endif /* avarage_pooling_layer_h */
