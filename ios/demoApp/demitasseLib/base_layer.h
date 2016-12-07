//
//  base_layer.h
//  demitasseLib
//
//  Created by Satoshi Kondo on 2016/04/01.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

#ifndef __DEMITASSE_IOS_BASE_LAYER__
#define __DEMITASSE_IOS_BASE_LAYER__

#import <Foundation/Foundation.h>
#import "Blob.h"

@interface BaseLayer : NSObject

- (nullable id)init;
- (void)dealloc;

- (nonnull const NSString *)layerTypeName;

- (void)setLayerName:(nullable const NSString *)name;
- (nullable const NSString *)layerName;

- (nullable const Blob *)feedForward:(nullable const Blob *)input;

@end

#endif /* __DEMITASSE_IOS_BASE_LAYER__ */
