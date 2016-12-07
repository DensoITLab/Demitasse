//
//  fully_connected_layer.h
//  demitasseLib
//
//  Created by Satoshi Kondo on 2016/04/01.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

#ifndef __DEMITASSE_IOS_FULLY_CONNECTED_LAYER__
#define __DEMITASSE_IOS_FULLY_CONNECTED_LAYER__

#import "base_layer.h"

@interface FullyConnectedLayer : BaseLayer

- (nullable id)init:(int)in_nodes out_nodes:(int)out_nodes;

- (nullable const Blob *)weight;
- (nullable const Blob *)bias;
- (void)setWeight:(nonnull const Blob *)w;
- (void)setBias:(nonnull const Blob *)b;

@end

#endif /* __DEMITASSE_IOS_FULLY_CONNECTED_LAYER__ */
