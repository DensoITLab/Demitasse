//
//  lrn_layer.h
//  demitasseLib
//
//  Created by Satoshi Kondo on 2016/04/01.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

#ifndef __DEMITASSE_IOS_LRN_LAYER__
#define __DEMITASSE_IOS_LRN_LAYER__

#import "base_layer.h"

@interface LRNLayer : BaseLayer

- (nullable id)init:(int)mode local_size:(int)local_size k:(float)k alpha:(float)alpha beta:(float)beta;

@end


#endif /* __DEMITASSE_IOS_LRN_LAYER__ */
