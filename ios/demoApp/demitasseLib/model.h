//
//  model.h
//  demitasseLib
//
//  Created by Satoshi Kondo on 2016/04/04.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

#ifndef __DEMITASSE_IOS_MODEL__
#define __DEMITASSE_IOS_MODEL__

#import "base_layer.h"

@interface Model : NSObject

- (nullable id)init;
- (nullable id)initWithLayers:(nonnull const NSArray *)layers;
- (void)dealloc;

- (void)addLayer:(nonnull BaseLayer *)layer;
- (nullable const BaseLayer *)getLayerAt:(int)atIndex;

- (void)load:(nonnull const NSString*)filename;

- (nullable Blob *)feedForward:(nullable const Blob *)input;

@end


#endif /* __DEMITASSE_IOS_MODEL__ */
