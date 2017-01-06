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
#include <math.h>

#include <memory>
#include <iostream>
#include <fstream>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>

#include "caffe.pb.h"

#include "flatbuffers/flatbuffers.h"
#include "network_model_generated.h"

#include "create_dirs.h"

using DataInputForm = struct {
        int w; // width
        int h; // height
        int c; // channel
      };

demitasse::serialize::LayerType
parse_activation_type(caffe::V1LayerParameter_LayerType activationType) {
  demitasse::serialize::LayerType activation_func_id = demitasse::serialize::LayerType_identity;

  switch (activationType) {
  case caffe::V1LayerParameter_LayerType_RELU:
    activation_func_id = demitasse::serialize::LayerType_ReLU;
    break;

  case caffe::V1LayerParameter_LayerType_TANH:
    activation_func_id = demitasse::serialize::LayerType_tanh;
    break;

  case caffe::V1LayerParameter_LayerType_SIGMOID:
    activation_func_id = demitasse::serialize::LayerType_sigmoid;
    break;

  case caffe::V1LayerParameter_LayerType_SOFTMAX:
    activation_func_id = demitasse::serialize::LayerType_softmax;
    break;

  default:
    // error
    break;
  }

  return activation_func_id;
}

flatbuffers::Offset<demitasse::serialize::Layer> parse_activation(
    int idx,
    caffe::V1LayerParameter& layer,
    flatbuffers::FlatBufferBuilder& builder) {

  const char* name_str = layer.name().c_str();
  const char* type_str = caffe::V1LayerParameter_LayerType_Name(layer.type()).c_str();

  printf("      [%.2d]: %s (%s)\n", idx, name_str, type_str);

  auto type    = parse_activation_type(layer.type());
  auto name    = builder.CreateString(name_str);

  auto fbLayer = demitasse::serialize::LayerBuilder(builder);

  fbLayer.add_type(type);
  fbLayer.add_name(name);

  auto data = fbLayer.Finish();

  return data;
}


flatbuffers::Offset<demitasse::serialize::Layer> parse_conv(
    int idx,
    DataInputForm& inputForm,
    caffe::V1LayerParameter& layer,
    flatbuffers::FlatBufferBuilder& builder) {

  if (layer.has_convolution_param() != true) {
    // error
    abort();
  }
  auto param = layer.convolution_param();

  int  pad   = 0; // default
  if (param.pad_size() > 0) {
    pad = (int)param.pad(0);
  }

  int stride = 1; // default
  if (param.stride_size() > 0) {
    stride = param.stride(0);
  }

  int ksize = 0;
  if (param.kernel_size_size() > 0) {
    ksize = param.kernel_size(0);
  }

  // number of output maps (num of filters)
  int num_output = 0;
  if (param.has_num_output()) {
    num_output = param.num_output();
  }

  const char* name_str = layer.name().c_str();
  const char* type_str = caffe::V1LayerParameter_LayerType_Name(layer.type()).c_str();

  printf("layer [%.2d]: %s (%s)\n", idx, name_str, type_str);

  printf("\t pad: %d\n",      pad);
  printf("\t stride: %d\n",   stride);
  printf("\t ksize: %d\n",    ksize);
  printf("\t filters: %d\n",  num_output);

  // read weight and bias from blobs
  float* weight  = nullptr;
  float* bias    = nullptr;

  int    in_maps = 0;

  for (int n = 0; n < layer.blobs_size(); ++n) {
    auto blob = layer.blobs(n);

    int  num  = 0;
    if (blob.has_num()) {
      num = blob.num();
      printf("\t blob num: %d\n", num);
    }

    int channels = 0;
    if (blob.has_channels()) {
      channels = blob.channels();
      printf("\t blob channels: %d\n", channels);
    }

    int height = 0;
    if (blob.has_height()) {
      height = blob.height();
      printf("\t blob height: %d\n", height);
    }

    int width = 0;
    if (blob.has_width()) {
      width = blob.width();
      printf("\t blob width: %d\n", width);
    }

    size_t blob_size = channels * width * height * num_output;

    if (n == 0) {
      printf("\t weight [%d %d %d %d]\n", width, height, channels, num);

      in_maps = channels;

      weight  = (float*)malloc(sizeof(float) * blob_size);
      // convert weight layout from WHCK -> CWHK

      for (int n = 0; n < blob_size; ++n) {
        weight[n] = 0.0;
      }

      int i = 0;
      for (int k = 0; k < num_output; ++k) {
        for (int c = 0; c < channels; ++c) {
          for (int y = 0; y < ksize; ++y) {
            for (int x = 0; x < ksize; ++x) {
              weight[(k * ksize * ksize * channels) + (y * ksize * channels) + (x * channels) + c] = blob.data(i);
              i++;
            }
          }
        }
      }
    } else if (n == 1) {

      printf("\t bias [%d %d %d %d]\n", width, height, channels, num);

      blob_size = num_output;
      bias      = (float*)malloc(sizeof(float) * blob_size);

      for (int n = 0; n < blob_size; ++n) {
        bias[n] = blob.data(n);
      }
    }
  }

  size_t rawWeightSize = (num_output * in_maps * ksize * ksize);
  size_t rawBiasSize   = num_output;

  auto   rawWeight     = builder.CreateVector((const float*)weight, rawWeightSize);
  auto   rawBias       = builder.CreateVector((const float*)bias, rawBiasSize);
  auto   name    = builder.CreateString(name_str);

  auto   fbLayer = demitasse::serialize::LayerBuilder(builder);

  fbLayer.add_type(demitasse::serialize::LayerType_Convolution);
  fbLayer.add_name(name);
  fbLayer.add_in_maps(in_maps);
  fbLayer.add_out_maps(num_output);
  fbLayer.add_ksize(ksize);
  fbLayer.add_stride(stride);
  fbLayer.add_padding(pad);
  fbLayer.add_weight(rawWeight);
  fbLayer.add_bias(rawBias);

  auto data = fbLayer.Finish();

  // calc output form from input
  inputForm.c = num_output;
  inputForm.w = (inputForm.w + (2 * pad) - ksize) / stride + 1;
  inputForm.h = (inputForm.h + (2 * pad) - ksize) / stride + 1;

  printf("          : (c:%d, w:%d, h:%d)\n", inputForm.c, inputForm.w, inputForm.h);

  return data;
}

flatbuffers::Offset<demitasse::serialize::Layer> parse_pool(
    int idx,
    DataInputForm& inputForm,
    caffe::V1LayerParameter& layer,
    flatbuffers::FlatBufferBuilder& builder) {

  if (layer.has_pooling_param() != true) {
    // error
    abort();
  }

  auto param = layer.pooling_param();

  int  pad   = 0;
  if (param.has_pad()) {
    pad = param.pad();
  }

  int stride = 0;
  if (param.has_stride()) {
    stride = param.stride();
  }

  int ksize = 0;
  if (param.kernel_size()) {
    ksize = param.kernel_size();
  }

  auto poolMethod = param.pool();
  auto layerType  = demitasse::serialize::LayerType_MaxPooling;

  switch (poolMethod) {
  case caffe::PoolingParameter_PoolMethod_MAX:
    layerType = demitasse::serialize::LayerType_MaxPooling;
    break;

  case caffe::PoolingParameter_PoolMethod_AVE:
    layerType = demitasse::serialize::LayerType_AvaragePooling;
    break;

  default:
    // error not support
    abort();
    break;
  }

  const char* name_str = layer.name().c_str();
  const char* type_str = caffe::PoolingParameter::PoolMethod_Name(poolMethod).c_str();

  printf("layer [%.2d]: %s (%s)\n", idx, name_str, type_str);

  printf("\t pad: %d\n",    pad);
  printf("\t stride: %d\n", stride);
  printf("\t ksize: %d\n",  ksize);


  // prepare flat buffers data
  auto name    = builder.CreateString(name_str);

  auto fbLayer = demitasse::serialize::LayerBuilder(builder);

  fbLayer.add_type(layerType);
  fbLayer.add_name(name);
  fbLayer.add_ksize(ksize);
  fbLayer.add_stride(stride);
  fbLayer.add_padding(pad);

  auto data = fbLayer.Finish();

  // calc output form from input
  inputForm.w = (int)ceilf((inputForm.w + (2 * pad) - ksize) / (float)stride) + 1;
  inputForm.h = (int)ceilf((inputForm.h + (2 * pad) - ksize) / (float)stride) + 1;

  return data;
}

flatbuffers::Offset<demitasse::serialize::Layer> parse_lrn(
    int idx,
    caffe::V1LayerParameter& layer,
    flatbuffers::FlatBufferBuilder& builder) {

  if (layer.has_lrn_param() != true) {
    // error
    abort();
  }

  auto  param = layer.lrn_param();

  float k     = 1;
  if (param.has_k()) {
    k = param.k();
  }

  float alpha = 1;
  if (param.has_alpha()) {
    alpha = param.alpha();
  }

  float beta = 5;
  if (param.has_beta()) {
    beta = param.beta();
  }

  int local_size = 5;
  if (param.has_local_size()) {
    local_size = param.local_size();
  }

  const char* name_str = layer.name().c_str();
  const char* type_str = caffe::V1LayerParameter_LayerType_Name(layer.type()).c_str();

  printf("layer [%.2d]: %s (%s)\n", idx, name_str, type_str);

  printf("\t local_size: %d\n", local_size);
  printf("\t k:     %f\n", k);
  printf("\t alpha: %f\n", alpha);
  printf("\t beta:  %f\n", beta);

  // prepare flat buffers data
  auto name    = builder.CreateString(name_str);

  auto fbLayer = demitasse::serialize::LayerBuilder(builder);

  fbLayer.add_type(demitasse::serialize::LayerType_LRN);
  fbLayer.add_name(name);
  fbLayer.add_ksize(local_size);
  fbLayer.add_k(k);
  fbLayer.add_alpha(alpha);
  fbLayer.add_beta(beta);

  auto data = fbLayer.Finish();

  return data;
}

flatbuffers::Offset<demitasse::serialize::Layer> parse_inner_product(
    int idx,
    DataInputForm& inputForm,
    caffe::V1LayerParameter& layer,
    flatbuffers::FlatBufferBuilder& builder) {

  if (layer.has_inner_product_param() != true) {
    // error
    abort();
  }
  auto param      = layer.inner_product_param();

  int  num_output = 0; // default
  if (param.has_num_output()) {
    num_output = param.num_output();
  }

  const char* name_str = layer.name().c_str();
  const char* type_str = caffe::V1LayerParameter_LayerType_Name(layer.type()).c_str();

  printf("layer [%.2d]: %s (%s)\n", idx, name_str, type_str);

  // read weight and bias from blobs
  float *weight = nullptr;
  float *bias   = nullptr;

  for (int n = 0; n < layer.blobs_size(); ++n) {
    auto blob = layer.blobs(n);

    int  num  = 0;
    if (blob.has_num()) {
      num = blob.num();
      printf("\t blob num: %d\n", num);
    }

    int channels = 0;
    if (blob.has_channels()) {
      channels = blob.channels();
      printf("\t blob channels: %d\n", channels);
    }

    int width = 0;
    if (blob.has_width()) {
      width = blob.width();
      printf("\t blob width: %d\n", width);
    }

    int height = 0;
    if (blob.has_height()) {
      height = blob.height();
      printf("\t blob height: %d\n", height);
    }

    printf("\t blob: [%d %d %d %d]\n", width, height, channels, num);

    size_t blob_size = num * channels * width * height;

    if (n == 0) {
      weight = (float*)malloc(sizeof(float) * blob_size);

      for (int n = 0; n < blob_size; ++n) {
        weight[n] = 0.0;
      }

      for (int n = 0; n < blob_size; ++n) {
        weight[n] = blob.data(n);
      }
    } else if (n == 1) {
      bias = (float*)malloc(sizeof(float) * blob_size);

      for (int n = 0; n < blob_size; ++n) {
        bias[n] = blob.data(n);
      }
    }
  }

  // prepare flat buffers data
  int    total_maps    = inputForm.c * inputForm.w * inputForm.h;
  size_t rawWeightSize = total_maps * num_output;
  size_t rawBiasSize   = num_output;

  auto   rawWeight     = builder.CreateVector((const float*)weight, rawWeightSize);
  auto   rawBias       = builder.CreateVector((const float*)bias, rawBiasSize);
  auto   name    = builder.CreateString(name_str);

  auto   fbLayer = demitasse::serialize::LayerBuilder(builder);

  fbLayer.add_type(demitasse::serialize::LayerType_FullyConnected);
  fbLayer.add_name(name);
  fbLayer.add_in_maps(total_maps);
  fbLayer.add_out_maps(num_output);
  fbLayer.add_weight(rawWeight);
  fbLayer.add_bias(rawBias);

  auto data = fbLayer.Finish();

  // calc output form from input
  inputForm.w = 1;
  inputForm.h = 1;
  inputForm.c = num_output;

  // in = num_output;

  printf("          : (num:%d)\n", inputForm.c /* in */);

  return data;
}

flatbuffers::Offset<demitasse::serialize::Layer> parse_inner_product2(
    int idx,
    DataInputForm& inputForm,
    caffe::V1LayerParameter& layer,
    flatbuffers::FlatBufferBuilder& builder) {

  if (layer.has_inner_product_param() != true) {
    // error
    abort();
  }
  auto param      = layer.inner_product_param();

  int  num_output = 0; // default
  if (param.has_num_output()) {
    num_output = param.num_output();
  }

  const char* name_str = layer.name().c_str();
  const char* type_str = caffe::V1LayerParameter_LayerType_Name(layer.type()).c_str();

  printf("layer [%.2d]: %s (%s)\n", idx, name_str, type_str);


  // read weight and bias from blobs
  float* weight = nullptr;
  float* bias   = nullptr;

  for (int n = 0; n < layer.blobs_size(); ++n) {
    auto blob = layer.blobs(n);

    int  size = blob.data_size();

    int  num  = 0;
    if (blob.has_num()) {
      num = blob.num();
      printf("\t blob num: %d\n", num);
    }

    int channels = 0;
    if (blob.has_channels()) {
      channels = blob.channels();
      printf("\t blob channels: %d\n", channels);
    }

    int width = 0;
    if (blob.has_width()) {
      width = blob.width();
      printf("\t blob width: %d\n", width);
    }

    int height = 0;
    if (blob.has_height()) {
      height = blob.height();
      printf("\t blob height: %d\n", height);
    }

    printf("\t blob: [%d %d %d %d]\n", width, height, channels, num);

    size_t blob_size = num * channels * width * height;
    // printf("\t blob data size: %zu\n", blob_size);

    if (n == 0) {
      weight = (float*)malloc(sizeof(float) * blob_size);

      size_t s = inputForm.c * inputForm.w * inputForm.h;
      printf("\t C:%d, W:%d H:%d : %zu\n", inputForm.c, inputForm.w, inputForm.h, s);

      for (int n = 0; n < blob_size; ++n) {
        weight[n] = 0.0;
      }

      int i = 0;
      for (int k = 0; k < num_output; ++k) {
        for (int c = 0; c < inputForm.c; ++c) {
          for (int y = 0; y < inputForm.h; ++y) {
            for (int x = 0; x < inputForm.w; ++x) {
              weight[(k * inputForm.h * inputForm.w * inputForm.c) + (y * inputForm.w * inputForm.c) + (x * inputForm.c) + c] = blob.data(i);
              i++;
            }
          }
        }
      }
    } else if (n == 1) {
      bias = (float*)malloc(sizeof(float) * blob_size);

      for (int n = 0; n < blob_size; ++n) {
        bias[n] = blob.data(n);
      }
    }
  }

  // prepare flat buffers data
  int    total_maps    = inputForm.c * inputForm.w * inputForm.h;
  size_t rawWeightSize = total_maps * num_output;
  size_t rawBiasSize   = num_output;
  auto   rawWeight     = builder.CreateVector((const float*)weight, rawWeightSize);
  auto   rawBias       = builder.CreateVector((const float*)bias, rawBiasSize);
  auto   name    = builder.CreateString(name_str);

  auto   fbLayer = demitasse::serialize::LayerBuilder(builder);

  fbLayer.add_type(demitasse::serialize::LayerType_FullyConnected);
  fbLayer.add_name(name);
  fbLayer.add_in_maps(total_maps);
  fbLayer.add_out_maps(num_output);
  fbLayer.add_weight(rawWeight);
  fbLayer.add_bias(rawBias);

  auto data = fbLayer.Finish();

  // calc output form from input
  // in = num_output;
  inputForm.c = num_output;
  inputForm.w = 1;
  inputForm.h = 1;

  printf("          : (num:%d)\n", inputForm.c);

  return data;
}

int countLayers(caffe::NetParameter* netParam) {
  int layers = 0;

  int i      = 0;
  while (i < netParam->layers_size()) {
    auto layer     = netParam->layers(i);

    auto layerType = layer.type();
    switch (layerType) {
    case caffe::V1LayerParameter_LayerType_RELU:
      layers++;
      break;

    case caffe::V1LayerParameter_LayerType_TANH:
      layers++;
      break;

    case caffe::V1LayerParameter_LayerType_SIGMOID:
      layers++;
      break;

    case caffe::V1LayerParameter_LayerType_SOFTMAX:
      layers++;
      break;

    case caffe::V1LayerParameter_LayerType_CONVOLUTION:
      layers++;
      break;

    case caffe::V1LayerParameter_LayerType_LRN:
      layers++;
      break;

    case caffe::V1LayerParameter_LayerType_POOLING:
      layers++;
      break;

    case caffe::V1LayerParameter_LayerType_INNER_PRODUCT:
      layers++;
      break;

    default:
      break;
    }
    i++;
  }

  return layers;
}

void write_model_to_file(flatbuffers::FlatBufferBuilder& builder, const std::string& file) {

  auto buffer = builder.GetBufferPointer();
  auto size   = builder.GetSize();

  std::ofstream fs(file, std::ios::out | std::ios::binary | std::ios::trunc);

  if (fs.is_open()) {
    fs.write((const char *)buffer, size);
    fs.close();
  } else {
    std::cerr << "Failed to open file :" << errno << std::endl;
  }

  builder.ReleaseBufferPointer();
}


bool read_proto_model(const std::string& filename, google::protobuf::Message* proto) {
  std::ifstream stream(filename, std::ios::in);
  auto ifs     = new google::protobuf::io::IstreamInputStream(&stream);

  bool success = google::protobuf::TextFormat::Parse(ifs, proto);

  return success;
}

bool read_caffe_model_binary(const std::string& filename, google::protobuf::Message* proto) {

  std::ifstream stream(filename, std::ios::binary);

  auto ifs = new google::protobuf::io::IstreamInputStream(&stream);
  google::protobuf::io::CodedInputStream input(ifs);

  input.SetTotalBytesLimit(640 * 1024 * 1024, 640 * 1024 * 1024);

  // bool success = proto->MergePartialFromCodedStream(&input);
  bool success = proto->ParseFromCodedStream(&input);

  return success;
}

void load_caffemodel(const std::string& proto, const std::string& model, const std::string& outfile) {

  caffe::NetParameter *netParam = new caffe::NetParameter();

  read_proto_model(proto, netParam);

  // get input data info from prototxt
  int dims = netParam->input_dim_size();
  printf("input dim: %d\n", dims);
  for (int n = 0; n < dims; ++n) {
    printf("input [%d]: %d\n", n, netParam->input_dim(n));
  }

  DataInputForm inputForm;
  inputForm.c = netParam->input_dim(1);
  inputForm.w = netParam->input_dim(3);
  inputForm.h = netParam->input_dim(2);

  int in = 0;

  // read model parameter from caffe model binary file
  read_caffe_model_binary(model, netParam);

  auto layers = netParam->layers_size();
  printf("layers : %d\n", layers);

  int fbNumOfLayers = countLayers(netParam);

  // prepare output flatbuffer layers
  flatbuffers::FlatBufferBuilder builder;

  auto fbLayers = new flatbuffers::Offset<demitasse::serialize::Layer>[fbNumOfLayers];

  int  i   = 0;
  int  idx = 0;
  while (i < layers) {
    auto layer     = netParam->layers(i);

    auto layerType = layer.type();
    switch (layerType) {
    case caffe::V1LayerParameter_LayerType_RELU:
    case caffe::V1LayerParameter_LayerType_TANH:
    case caffe::V1LayerParameter_LayerType_SIGMOID:
    case caffe::V1LayerParameter_LayerType_SOFTMAX:
      fbLayers[idx] = parse_activation(i, layer, builder);
      idx++;
      break;

    case caffe::V1LayerParameter_LayerType_CONVOLUTION: {
      fbLayers[idx] = parse_conv(i, inputForm, /* ch, w, h, */ layer, builder);
      idx++;
      break;
    }

    case caffe::V1LayerParameter_LayerType_POOLING: {
      fbLayers[idx] = parse_pool(i, inputForm, /* ch, w, h, */ layer, builder);
      idx++;
      break;
    }

    case caffe::V1LayerParameter_LayerType_LRN: {
      fbLayers[idx] = parse_lrn(i, layer, builder);
      idx++;
      break;
    }

    case caffe::V1LayerParameter_LayerType_INNER_PRODUCT: {
      if (in == 0) {
        // serialize input dimension, remap and reshape dimensions to 1d here
        in = inputForm.c * inputForm.w * inputForm.h;
        printf("          : (num:%d)\n", in);
        fbLayers[idx] = parse_inner_product2(i, inputForm, layer, builder);
      } else {
        fbLayers[idx] = parse_inner_product(i, inputForm, layer, builder);
      }
      idx++;
      break;
    }

    default:
      printf("layer [%.2d]: %s(%s)\n", i, layer.name().c_str(), caffe::V1LayerParameter_LayerType_Name(layer.type()).c_str());
      break;
    }

    i++;
  }

  // write layers to the model
  auto mlocLayers = builder.CreateVector(fbLayers, fbNumOfLayers);

  // root entity
  auto fbModel = demitasse::serialize::ModelBuilder(builder);
  fbModel.add_layers(mlocLayers);

  auto mloc = fbModel.Finish();
  demitasse::serialize::FinishModelBuffer(builder, mloc);

  // write to the file
  std::cerr << "output file:" << outfile << std::endl;
  write_model_to_file(builder, outfile);

  google::protobuf::ShutdownProtobufLibrary();
}


int main(int argc, char** argv) {

  if (argc < 3) {
    printf("Usage: %s <proto> <model> [<output>]\n", argv[0]);
    return 0;
  }

  const char* proto   = argv[1];
  const char* model   = argv[2];

  const char* outfile = "./vudnn.model";
  if (argc == 4) {
    // prepare directory
    create_dirs(argv[3]);
    outfile = argv[3];
  }

  load_caffemodel(proto, model, outfile);

  return 0;
}
