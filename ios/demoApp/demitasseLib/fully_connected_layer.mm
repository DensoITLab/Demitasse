//
//  fully_connected_layer.mm
//  demitasseLib
//
//  Created by Satoshi Kondo on 2016/04/01.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "fully_connected_layer.h"

#import "blob_protected.h"
#import "base_layer_protected.h"

#include "fully_connected_layer.hpp"

@implementation FullyConnectedLayer

- (id)init:(int)in_nodes out_nodes:(int)out_nodes
{
    if (self = [super init]) {
        obj_ = new demitasse::FullyConnectedLayer(in_nodes, out_nodes);
        if (obj_ == nil) {
            return nil;
        }
    }

    return self;
}

- (nullable const Blob *)weight
{
    Blob *aWeight = nil;
    
    const demitasse::Blob *data = ((demitasse::FullyConnectedLayer *)obj_)->weight();
    if (data != nil) {
        aWeight = [[Blob alloc] initWithConstBlob:data];
    }

    return aWeight;
}

- (nullable const Blob *)bias
{
    Blob *aBias = nil;
    
    const demitasse::Blob *data = ((demitasse::FullyConnectedLayer *)obj_)->bias();
    if (data != nil) {
        aBias = [[Blob alloc] initWithConstBlob:data];
    }

    return aBias;
}


- (void)setWeight:(nonnull const Blob *)w {
    ((demitasse::FullyConnectedLayer *)obj_)->setWeight((const demitasse::Blob *)w->obj_);
}

- (void)setBias:(nonnull const Blob *)b {
    ((demitasse::FullyConnectedLayer *)obj_)->setBias((const demitasse::Blob *)b->obj_);
}


@end