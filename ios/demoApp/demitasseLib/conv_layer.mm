//
//  conv_layer.mm
//  demitasseLib
//
//  Created by Satoshi Kondo on 2016/04/01.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "conv_layer.h"

#import "blob_protected.h"
#import "base_layer_protected.h"

#include "conv_layer.hpp"

@implementation ConvLayer

- (nullable id)init:(unsigned int)in_maps
           out_maps:(unsigned int)out_maps
              ksize:(unsigned int)ksize
             stride:(unsigned int)stride
            padding:(unsigned int)padding {
  if (self = [super init]) {
    demitasse::ConvParam conv_param;
    conv_param.in_maps  = in_maps;
    conv_param.out_maps = out_maps;
    conv_param.ksize    = ksize;
    conv_param.stride   = stride;
    conv_param.padding  = padding;
        
    obj_ = new demitasse::ConvLayer(&conv_param);
    if (obj_ == nil) {
      return nil;
    }
  }

  return self;
}

- (nullable id)init:(unsigned int)in_maps
           out_maps:(unsigned int)out_maps
              ksize:(unsigned int)ksize
             stride:(unsigned int)stride
{
  return [self init:in_maps out_maps:out_maps ksize:ksize stride:stride padding:0];
}

- (nullable const Blob *)weight
{
  Blob *aWeight = nil;

  const demitasse::Blob *data = ((demitasse::ConvLayer *)obj_)->weight();
  if (data != nil) {
    aWeight = [[Blob alloc] initWithConstBlob:data];
  }

  return aWeight;
}

- (nullable const Blob *)bias
{
  Blob *aBias = nil;

  const demitasse::Blob *data = ((demitasse::ConvLayer *)obj_)->bias();
  if (data != nil) {
    aBias = [[Blob alloc] initWithConstBlob:data];
  }

  return aBias;
}


- (void)setWeight:(nonnull const Blob *)w
{
  ((demitasse::ConvLayer *)obj_)->setWeight((const demitasse::Blob *)w->obj_);
}

- (void)setBias:(nonnull const Blob *)b
{
  ((demitasse::ConvLayer *)obj_)->setBias((const demitasse::Blob *)b->obj_);
}

@end