//
//  model.mm
//  demitasseLib
//
//  Created by Satoshi Kondo on 2016/04/04.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "model.h"
#import "blob_protected.h"
#import "base_layer_protected.h"

#include "activation_layer.h"
#include "conv_layer.h"
#include "fully_connected_layer.h"
#include "lrn_layer.h"
#include "pooling_layer.h"

#include "model.hpp"

@implementation Model {
    demitasse::Model *obj_;
}

- (nullable id)init
{
    if (self = [super init]) {
        obj_ = new demitasse::Model();
        if (obj_ == nil) {
            return nil;
        }
    }

    return self;
}

- (nullable id)initWithLayers:(nonnull const NSArray *)layers
{
    if (self = [super init]) {
        std::vector<demitasse::BaseLayer*> aLayers;
    
        for (BaseLayer* layer in layers) {
            aLayers.push_back(layer->obj_);
        }
    
        obj_ = new demitasse::Model(aLayers);
    }

    return self;
}

- (void)dealloc {
    if (obj_ != nil) {
        delete obj_;
    }
}

- (void)addLayer:(nonnull BaseLayer *)layer {
    obj_->addLayer(layer->obj_);
}

- (nullable const BaseLayer *)getLayerAt:(int)atIndex {
    BaseLayer* aLayer = nil;
    demitasse::BaseLayer *layer = obj_->getLayer(atIndex);
    
    if (layer->layerTypeName() == "pooling <avarage>") {
        aLayer = [[PoolingLayer alloc] init:PoolingFuncAvarage];
    } else if (layer->layerTypeName() == "pooling <max>") {
        aLayer = [[PoolingLayer alloc] init:PoolingFuncMax];
    } else if (layer->layerTypeName() == "conv layer") {
        aLayer = [[ConvLayer alloc] init];
    } else if (layer->layerTypeName() == "fully connected layer") {
        aLayer = [[FullyConnectedLayer alloc] init];
    } else if (layer->layerTypeName() == "lrn <across_channel>") {
        aLayer = [[LRNLayer alloc] init];
    } else if (layer->layerTypeName() ==  "activation <identity>") {
        aLayer = [[ActivationLayer alloc] init:ActivationFuncIdentity];
    } else if (layer->layerTypeName() ==  "activation <sigmoid>") {
        aLayer = [[ActivationLayer alloc] init:ActivationFuncSigmoid];
    } else if (layer->layerTypeName() ==  "activation <tanh>") {
        aLayer = [[ActivationLayer alloc] init:ActivationFuncTanh];
    } else if (layer->layerTypeName() ==  "activation <ReLU>") {
        aLayer = [[ActivationLayer alloc] init:ActivationFuncReLU];
    } else if (layer->layerTypeName() ==  "activation <softmax>") {
        aLayer = [[ActivationLayer alloc] init:ActivationFuncSoftMax];
    }
    
    if (aLayer != nil) {
        [aLayer setConstLayer:layer];
    }
    
    return aLayer;
}

- (void)load:(nonnull const NSString*)filename {
    std::string name = [filename UTF8String];
    obj_->load(name);
}

- (nullable Blob *)feedForward:(nullable const Blob *)input
{
    if (input == nil) {
        return nil;
    }
    
    Blob *aResult = nil;

    if (obj_ != nil) {
        const demitasse::Blob *blob = obj_->feedForward(input->obj_);
        if (blob != nil) {
            aResult = [[Blob alloc] initWithConstBlob:blob];
        }
    }

    return aResult;
}

@end

