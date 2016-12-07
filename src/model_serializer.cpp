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
#include <string>

#include "flatbuffers/flatbuffers.h"

#include "model_serializer.hpp"

#include "debug_log.hpp"
#include "blob.hpp"
#include "binary_encoder.hpp"
#include "activation_function.hpp"
#include "activation_layer.hpp"
#include "binary_conv_layer.hpp"
#include "binary_fully_connected_layer.hpp"
#include "fully_connected_layer.hpp"
#include "conv_param.hpp"
#include "conv_layer.hpp"
#include "pooling_param.hpp"
#include "pooling_function.hpp"
#include "pooling_layer.hpp"
#include "lrn_param.hpp"
#include "lrn_layer.hpp"

using demitasse::ModelSerializer;
using demitasse::BaseLayer;
using demitasse::Blob;
using demitasse::BinaryEncoder;

BaseLayer* ModelSerializer::createActivationLayer(const demitasse::serialize::Layer* param) {
  BaseLayer* base_layer = nullptr;

  switch (param->type()) {
  case demitasse::serialize::LayerType_identity: {
    auto new_layer = new ActivationLayer<identity>();
    base_layer = new_layer;
    break;
  }

  case demitasse::serialize::LayerType_sigmoid: {
    auto new_layer = new ActivationLayer<sigmoid>();
    base_layer = new_layer;
    break;
  }

  case demitasse::serialize::LayerType_tanh: {
    auto new_layer = new ActivationLayer<tanh>();
    base_layer = new_layer;
    break;
  }

  case demitasse::serialize::LayerType_ReLU: {
    auto new_layer = new ActivationLayer<ReLU>();
    base_layer = new_layer;
    break;
  }

  case demitasse::serialize::LayerType_softmax: {
    auto new_layer = new ActivationLayer<softmax>();
    base_layer = new_layer;
    break;
  }

  default:
    // error
    break;
  }

  return base_layer;
}

void ModelSerializer::loadConvLayerParameters(float* weight, float* bias,
    const demitasse::serialize::Layer* param) {
  int  in_maps      = param->in_maps();
  int  ksize        = param->ksize();
  int  wsize        = in_maps * ksize * ksize;
  int  wstride      = (wsize % 16 > 0) ? ((wsize / 16) + 1) * 16 : wsize;

  auto weight_param = param->weight();
  int  idx = 0;
  for (auto value: *weight_param) {
    weight[idx] = value;
    if (++idx >= wsize) {
      idx     = 0;
      weight += wstride;
    }
  }

  auto bias_param = param->bias();
  for (auto value: *bias_param) {
    *bias = value;
    ++bias;
  }
}

BaseLayer* ModelSerializer::createConvLayer(
    const demitasse::serialize::Layer* layer_param) {
  BaseLayer* base_layer = nullptr;
  ConvParam  conv_param;
  conv_param.in_maps  = layer_param->in_maps();
  conv_param.out_maps = layer_param->out_maps();
  conv_param.ksize    = layer_param->ksize();
  conv_param.stride   = layer_param->stride();
  conv_param.padding  = layer_param->padding();

  DebugLog("load conv layer (filters:%d, in_maps:%d, ksize:%d, stride:%d, padding:%d)\n",
      conv_param.out_maps, conv_param.in_maps, conv_param.ksize, conv_param.stride, conv_param.padding);

  auto   new_layer = new ConvLayer(&conv_param);

  float* w = new_layer->weight()->rawData().RealSingle.data;
  float* b = new_layer->bias()->rawData().RealSingle.data;

  ModelSerializer::loadConvLayerParameters(w, b, layer_param);

  base_layer = new_layer;

  return base_layer;
}


void ModelSerializer::loadFullyConnectedLayerParameters(float* weight, float* bias,
    const demitasse::serialize::Layer* layer_param) {
  auto weight_param = layer_param->weight();
  for (auto value: *weight_param) {
    *weight = value;
    ++weight;
  }

  auto bias_param = layer_param->bias();
  for (auto value: *bias_param) {
    *bias = value;
    ++bias;
  }
}

BaseLayer* ModelSerializer::createFullyConnectedLayer(
    const demitasse::serialize::Layer* layer_param) {
  BaseLayer *base_layer = nullptr;

  int in_maps  = layer_param->in_maps();
  int out_maps = layer_param->out_maps();

  DebugLog("load fc layer (in:%d, out:%d)\n", in_maps, out_maps);

  auto new_layer = new FullyConnectedLayer(in_maps, out_maps);

  auto weight    = new_layer->weight();
  auto bias      = new_layer->bias();

  ModelSerializer::loadFullyConnectedLayerParameters(weight->rawData().RealSingle.data, bias->rawData().RealSingle.data, layer_param);

  base_layer = new_layer;

  return base_layer;
}


BinaryEncoder * ModelSerializer::createBinaryEncoder(
    const demitasse::serialize::Encoder *encoder_param) {
  int      lut_size = encoder_param->table_size() * encoder_param->num_basis();
  uint32_t* lut      = new uint32_t[lut_size]();

  DebugLog("lut table_size:%d, num_basis:%d\n", encoder_param->table_size(), encoder_param->num_basis());

  auto     lut_param = encoder_param->lut();
  uint32_t* p = lut;

  for (auto value: *lut_param) {
    *p = value;
    ++p;
  }

  auto encoder = new BinaryEncoder(
      encoder_param->coff1(),
      encoder_param->coff2(),
      encoder_param->num_basis(),
      encoder_param->table_size(),
      lut
      );

  delete[] lut;

  return encoder;
}



BaseLayer* ModelSerializer::createBinaryFullyConnectedLayer(
    const demitasse::serialize::Layer* layer_param) {
  int  in_maps         = layer_param->in_maps();
  int  out_maps        = layer_param->out_maps();
  int  num_basis       = layer_param->num_basis();
  int  num_basis_total = layer_param->num_basis_total();

  auto encoder         = ModelSerializer::createBinaryEncoder(layer_param->encoder());

  DebugLog("load binary fc layer (in:%d, out:%d)\n", in_maps, out_maps);

  auto   new_layer     = new BinaryFullyConnectedLayer(in_maps, out_maps, num_basis, num_basis_total, encoder);

  auto   weight        = new_layer->weight();
  auto   bias          = new_layer->bias();
  auto   integer_basis = new_layer->integerBasis();
  auto   basis_weight  = new_layer->basisWeight();

  float *p = nullptr;

  // weight
  p = weight->rawData().RealSingle.data;
  auto weight_param = layer_param->weight();
  for (auto value: *weight_param) {
    *p = value;
    ++p;
  }

  // bias
  p = bias->rawData().RealSingle.data;
  auto bias_param = layer_param->bias();
  for (auto value: *bias_param) {
    *p = value;
    ++p;
  }

  // integer basis
  if (integer_basis->blobType() == Blob::BlobType::Ternary) {
    // ternary

    // FIXME: check dimensions
    auto     tensor       = layer_param->integer_basis();
    auto     ternaryValue = (demitasse::serialize::TernaryValue *)tensor->value();

    uint8_t *b = nullptr;

    b = integer_basis->rawData().Ternary.star;
    auto star_param = ternaryValue->star();
    for (auto value: *star_param) {
      *b = value;
      ++b;
    }

    b = integer_basis->rawData().Ternary.mask;
    auto mask_param = ternaryValue->mask();
    for (auto value: *mask_param) {
      *b = value;
      ++b;
    }

    uint32_t *uint_p    = integer_basis->rawData().Ternary.nnz;
    auto      nnz_param = ternaryValue->nnz();
    for (auto value: *nnz_param) {
      *uint_p = value;
      ++uint_p;
    }
  } else {
    // binary

    // FIXME: check dimensions
    auto     tensor      = layer_param->integer_basis();
    auto     binaryValue = (demitasse::serialize::BinaryValue *)tensor->value();

    uint8_t *b = nullptr;

    b = integer_basis->rawData().Binary.data;
    auto data_param = binaryValue->data();
    for (auto value: *data_param) {
      *b = value;
      ++b;
    }
  }

  // basis weight
  p = basis_weight->rawData().RealSingle.data;
  auto basis_weight_param = layer_param->basis_weight();
  for (auto value: *basis_weight_param) {
    *p = value;
    ++p;
  }

  return (BaseLayer *)new_layer;
}

BaseLayer* ModelSerializer::createBinaryConvLayer(
    const demitasse::serialize::Layer* layer_param) {
  ConvParam  conv_param;
  conv_param.in_maps  = layer_param->in_maps();
  conv_param.out_maps = layer_param->out_maps();
  conv_param.ksize    = layer_param->ksize();
  conv_param.stride   = layer_param->stride();
  conv_param.padding  = layer_param->padding();

  int  num_basis       = layer_param->num_basis();
  int  num_basis_total = layer_param->num_basis_total();
  auto encoder         = ModelSerializer::createBinaryEncoder(layer_param->encoder());

  DebugLog("load binary conv layer (filters:%d, in_maps:%d, ksize:%d, stride:%d, padding:%d)\n",
          conv_param.out_maps, conv_param.in_maps, conv_param.ksize, conv_param.stride, conv_param.padding);

  auto   new_layer     = new BinaryConvLayer(&conv_param, num_basis, num_basis_total, encoder);

  auto   weight        = new_layer->weight();
  auto   bias          = new_layer->bias();
  auto   integer_basis = new_layer->integerBasis();
  auto   basis_weight  = new_layer->basisWeight();

  DebugLog("weight size [%d %d]\n", weight->extent(0), weight->extent(1));

  float *p = nullptr;

  // weight
  p = weight->rawData().RealSingle.data;
  auto weight_param = layer_param->weight();
  for (auto value: *weight_param) {
    *p = value;
    ++p;
  }

  // bias
  p = bias->rawData().RealSingle.data;
  auto bias_param = layer_param->bias();
  for (auto value: *bias_param) {
    *p = value;
    ++p;
  }

  // integer basis
  if (integer_basis->blobType() == Blob::BlobType::Ternary) {
    // ternary

    // FIXME: check dimensions
    auto     tensor       = layer_param->integer_basis();
    auto     ternaryValue = (demitasse::serialize::TernaryValue *)tensor->value();

    uint8_t *b = nullptr;

    b = integer_basis->rawData().Ternary.star;
    auto star_param = ternaryValue->star();
    for (auto value: *star_param) {
      *b = value;
      ++b;
    }

    b = integer_basis->rawData().Ternary.mask;
    auto mask_param = ternaryValue->mask();
    for (auto value: *mask_param) {
      *b = value;
      ++b;
    }

    uint32_t *uint_p    = integer_basis->rawData().Ternary.nnz;
    auto      nnz_param = ternaryValue->nnz();
    for (auto value: *nnz_param) {
      *uint_p = value;
      ++uint_p;
    }
  } else {
    // binary

    // FIXME: check dimensions
    auto     tensor      = layer_param->integer_basis();
    auto     binaryValue = (demitasse::serialize::BinaryValue *)tensor->value();

    uint8_t *b = nullptr;

    b = integer_basis->rawData().Binary.data;
    auto data_param = binaryValue->data();
    for (auto value: *data_param) {
      *b = value;
      ++b;
    }
  }

  // basis weight
  p = basis_weight->rawData().RealSingle.data;
  auto basis_weight_param = layer_param->basis_weight();
  for (auto value: *basis_weight_param) {
    *p = value;
    ++p;
  }

  return (BaseLayer *)new_layer;
}


BaseLayer* ModelSerializer::loadLayer(const demitasse::serialize::Layer* layer) {
  BaseLayer* newLayer = nullptr;

  switch (layer->type()) {
  case demitasse::serialize::LayerType_sigmoid:
  case demitasse::serialize::LayerType_tanh:
  case demitasse::serialize::LayerType_ReLU:
  case demitasse::serialize::LayerType_softmax:
    newLayer = ModelSerializer::createActivationLayer(layer);
    break;

  case demitasse::serialize::LayerType_Convolution:
    newLayer = ModelSerializer::createConvLayer(layer);
    break;

  case demitasse::serialize::LayerType_MaxPooling: {
    PoolingParam pooling_param;
    pooling_param.ksize   = layer->ksize();
    pooling_param.stride  = layer->stride();
    pooling_param.padding = layer->padding();

    DebugLog("load max pool layer (ksize:%d, stride:%d, padding:%d)\n",
        pooling_param.ksize, pooling_param.stride, pooling_param.padding);
    newLayer = new PoolingLayer<max>(&pooling_param);
    break;
  }

  case demitasse::serialize::LayerType_AvaragePooling: {
    PoolingParam pooling_param;
    pooling_param.ksize   = layer->ksize();
    pooling_param.stride  = layer->stride();
    pooling_param.padding = layer->padding();

    DebugLog("load avarage pool layer (ksize:%d, stride:%d, padding:%d)\n",
        pooling_param.ksize, pooling_param.stride, pooling_param.padding);
    newLayer = new PoolingLayer<avarage>(&pooling_param);
    break;
  }

  case demitasse::serialize::LayerType_LRN: {
    LRNParam lrn_param;
    lrn_param.mode       = 0;
    lrn_param.local_size = layer->ksize();
    lrn_param.k          = layer->k();
    lrn_param.alpha      = layer->alpha();
    lrn_param.beta       = layer->beta();

    DebugLog("load LRN layer (local_size:%d, k:%f, alpha:%f, beta:%f)\n",
        lrn_param.local_size, lrn_param.k, lrn_param.alpha, lrn_param.beta);
    newLayer = new LRNLayer(&lrn_param);
    break;
  }

  case demitasse::serialize::LayerType_FullyConnected:
    newLayer = ModelSerializer::createFullyConnectedLayer(layer);
    break;

  case demitasse::serialize::LayerType_BinaryFullyConnected:
    newLayer = ModelSerializer::createBinaryFullyConnectedLayer(layer);
    break;

  case demitasse::serialize::LayerType_BinaryConvolution:
    newLayer = ModelSerializer::createBinaryConvLayer(layer);
    break;

  default:
    // error
    break;
  }

  return newLayer;
}
