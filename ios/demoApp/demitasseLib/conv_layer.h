//
//  conv_layer.h
//  demitasseLib
//
//  Created by Satoshi Kondo on 2016/04/01.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

#ifndef __DEMITASSE_IOS_CONV_LAYER__
#define __DEMITASSE_IOS_CONV_LAYER__

#import "base_layer.h"

@interface ConvLayer : BaseLayer

- (nullable id)init:(unsigned int)in_maps
           out_maps:(unsigned int)out_maps
              ksize:(unsigned int)ksize
             stride:(unsigned int)stride
            padding:(unsigned int)padding;

- (nullable id)init:(unsigned int)in_maps
           out_maps:(unsigned int)out_maps
              ksize:(unsigned int)ksize
             stride:(unsigned int)stride;

- (nullable const Blob *)weight;
- (nullable const Blob *)bias;
- (void)setWeight:(nonnull const Blob *)w;
- (void)setBias:(nonnull const Blob *)b;

@end


#endif /* __DEMITASSE_IOS_CONV_LAYER__ */
