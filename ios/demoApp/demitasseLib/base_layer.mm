//
//  base_layer.mm
//  demitasseLib
//
//  Created by Satoshi Kondo on 2016/04/01.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

#import "base_layer.h"
#import "blob_protected.h"

#include "base_layer.hpp"

@implementation BaseLayer {
    bool                  hasOwnedObj_;
    demitasse::BaseLayer *obj_;
}


- (nullable id)init
{
    if (self = [super init]) {
        obj_         = nil;
        hasOwnedObj_ = true;
    }
    return self;
}

- (void)dealloc
{
    if (hasOwnedObj_ && obj_ != nil) {
        delete obj_;
    }
}

- (void)setConstLayer:(nonnull const demitasse::BaseLayer *)layer
{
    obj_         = (demitasse::BaseLayer*)layer;
    hasOwnedObj_ = false;
}


- (nonnull const NSString *)layerTypeName
{
    NSString *nsStr = @"unknown";
    
    if (obj_ != nil) {
        std::string name = obj_->layerTypeName();
        nsStr = [NSString stringWithUTF8String:name.c_str()];
    }

    return nsStr;
 }

- (void)setLayerName:(nullable const NSString *)name
{
    std::string newName = [name UTF8String];
    if (obj_ != nil) {
        obj_->setLayerName(newName);
    }
}

- (nullable const NSString *)layerName
{
    NSString *nsStr = nil;
    
    if (obj_ != nil) {
        std::string name = obj_->layerName();
        nsStr = [NSString stringWithUTF8String:name.c_str()];
    }

    return nsStr;
}

- (nullable const Blob *)feedForward:(nullable const Blob *)input
{
    if (input == nil) {
        return nil;
    }
    
    Blob *newObj = nil;
    
    if (obj_ != nil) {
        const demitasse::Blob *result = obj_->feedForward(input->obj_);
        if (result != nil) {
            newObj = [[Blob alloc] initWithConstBlob:result];
        }
    }

    return newObj;
}

@end