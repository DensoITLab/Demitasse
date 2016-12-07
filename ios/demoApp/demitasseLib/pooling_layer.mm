/*
Copyright (C) 2016 Denso IT Laboratory, Inc.
All Rights Reserved

Denso IT Laboratory, Inc. retains sole and exclusive ownership of all
intellectual property rights including copyrights and patents related to this
Software.

Permission is hereby granted, free of charge, to any person obtaining a copy
of the Software and accompanying documentation to use, copy, modify, merge,
publish, or distribute the Software or software derived from it for
non-commercial purposes, such as academic study, education and personal use,
subject to the following conditions:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
 */
#import <Foundation/Foundation.h>
#import "pooling_layer.h"
#import "base_layer_protected.h"

#include "pooling_function.hpp"
#include "pooling_param.hpp"
#include "pooling_layer.hpp"

@implementation PoolingLayer

- (nullable id)init:(PoolingFunc)type
              ksize:(unsigned int)ksize
             stride:(unsigned int)stride
            padding:(unsigned int)padding {
  if (self = [super init]) {
    demitasse::PoolingParam pooling_param;
    pooling_param.ksize = ksize;
    pooling_param.stride = stride;
    pooling_param.padding = padding;

    if (type == PoolingFuncMax) {
        obj_ = new demitasse::PoolingLayer<demitasse::max>(&pooling_param);
    } else if (type == PoolingFuncAvarage) {
        obj_ = new demitasse::PoolingLayer<demitasse::avarage>(&pooling_param);
    }

    if (obj_ == nil) {
      return nil;
    }
  }

  return self;
}

- (id)init:(PoolingFunc)type
     ksize:(unsigned int)ksize
    stride:(unsigned int)stride {
  return [self init:type ksize:ksize stride:stride padding:0];
}

- (id)init:(PoolingFunc)type {
    self = [super init];
    return self;
}


@end
