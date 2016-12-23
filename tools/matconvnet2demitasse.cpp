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

#include <fstream>

#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>

#include "mat.h"
#include "matrix.h"

#include "flatbuffers/flatbuffers.h"
#include "network_model_generated.h"

using DataInputForm = struct {
        int w; // width
        int h; // height
        int c; // channel
      };

demitasse::serialize::LayerType parse_activation_type(const std::string layer_type) {
  demitasse::serialize::LayerType activation_func_id = demitasse::serialize::LayerType_identity;

  if (layer_type == "relu") {
    activation_func_id = demitasse::serialize::LayerType_ReLU;
  } else if (layer_type == "tanh") {
    activation_func_id = demitasse::serialize::LayerType_tanh;
  } else if (layer_type == "sigmoid") {
    activation_func_id = demitasse::serialize::LayerType_sigmoid;
  } else if (layer_type == "softmax") {
    activation_func_id = demitasse::serialize::LayerType_softmax;
  } else {
    // error
  }

  return activation_func_id;
}

flatbuffers::Offset<demitasse::serialize::Layer> parse_activation(
    int idx,
    const mxArray *layer,
    flatbuffers::FlatBufferBuilder &builder) {

  char type_str[256];
  char name_str[256];

  mxGetString(mxGetField(layer, 0, "type"), type_str, 256);
  mxGetString(mxGetField(layer, 0, "name"), name_str, 256);

  const std::string layer_type = type_str;
  const std::string layer_name = name_str;

  printf("      [%.2d]: %s(%s)\n", idx, name_str, type_str);

  auto type    = parse_activation_type(layer_type);
  auto name    = builder.CreateString(name_str);

  auto fbLayer = demitasse::serialize::LayerBuilder(builder);

  fbLayer.add_type(type);
  fbLayer.add_name(name);

  auto data = fbLayer.Finish();

  return data;
}



flatbuffers::Offset<demitasse::serialize::Encoder> createEncoder(
    mxArray* mxEncoder,
    flatbuffers::FlatBufferBuilder &builder) {
  float coff1 = (float)mxGetScalar(mxGetField(mxEncoder, 0, "coff1"));
  float coff2 = (float)mxGetScalar(mxGetField(mxEncoder, 0, "coff2"));

  printf("\t coff1: %g, coff2: %f\n", coff1, coff2);

  mxArray *mxLUT      = mxGetField(mxEncoder, 0, "LUT");

  int      table_size = (int)mxGetM(mxLUT); // row
  int      num_basis  = (int)mxGetN(mxLUT); // col

  printf("\t table_size(row): %d, num_basis(col): %d\n", table_size, num_basis);

  size_t   data_size = (num_basis * table_size);
  int8_t * int_ptr   = (int8_t *)mxGetData(mxLUT);
  uint8_t *LUT       = (uint8_t *)malloc(sizeof(uint8_t) * data_size);

  // data layout is row -> col
/*
   printf("\t value{1, :}: ");
   for (int i = 0; i < 4; ++i) {
    printf("%d ", int_ptr[table_size * i]);
   }
   printf("\n");
 */

  printf("\t Tensor Size: %lu K bytes\n", (sizeof(uint8_t) * data_size) / 1024);

  // Transpose matrix [row col] to [col row]
  for (int n = 0; n < data_size; ++n) {
    // packing from {1, -1} to {1, 0}
    LUT[n] = int_ptr[n] > 0 ? 1 : 0;
  }

  auto fbLUT = builder.CreateVector((const uint8_t*)LUT, data_size);

  auto buildEncoder = demitasse::serialize::EncoderBuilder(builder);
  buildEncoder.add_num_basis(num_basis);
  buildEncoder.add_table_size(table_size);
  buildEncoder.add_lut(fbLUT);
  buildEncoder.add_coff1(coff1);
  buildEncoder.add_coff2(coff2);

  auto data = buildEncoder.Finish();

  return data;
}

//
// create Tensor from integer basis matrix
//
flatbuffers::Offset<demitasse::serialize::Tensor> createBinaryTensor(
    bool isTernary,
    mxArray* mxData,
    flatbuffers::FlatBufferBuilder& builder) {
  // FIXME: input data dimensions must be 2
  int M = mxGetM(mxData); // input_map (row)
  int N = mxGetN(mxData); // num_total_basis (col)

  printf("binary row: %d, col: %d\n", M, N);

  int dims[2];
  dims[0] = N; // switch col -> row
  dims[1] = M; // switch row -> col

  size_t    data_size = (M * N) / 8;
  size_t    nnz_size  = N;

  int8_t*   int_ptr   = (int8_t*)mxGetData(mxData);
  uint8_t*  star      = (uint8_t*)malloc(sizeof(uint8_t) * data_size);
  uint8_t*  mask      = nullptr;
  uint32_t* nnz       = nullptr;

  if (isTernary) {
    printf("nnz size: %d\n", (int)N);
    mask = (uint8_t*)malloc(sizeof(uint8_t) * data_size);
    nnz  = (uint32_t*)malloc(sizeof(uint32_t) * nnz_size);
  }

  int idx = 0;
  for (int n = 0; n < N; ++n) {
    for (int m = 0; m < M; m += 8) {
      // {-1, 0} -> 0, {1} -> 1
      star[idx]  = ( (int_ptr[0])  > 0 ) ? 0x01 : 0;
      star[idx] |= ( (int_ptr[1])  > 0 ) ? 0x02 : 0;
      star[idx] |= ( (int_ptr[2])  > 0 ) ? 0x04 : 0;
      star[idx] |= ( (int_ptr[3])  > 0 ) ? 0x08 : 0;
      star[idx] |= ( (int_ptr[4])  > 0 ) ? 0x10 : 0;
      star[idx] |= ( (int_ptr[5])  > 0 ) ? 0x20 : 0;
      star[idx] |= ( (int_ptr[6])  > 0 ) ? 0x40 : 0;
      star[idx] |= ( (int_ptr[7])  > 0 ) ? 0x80 : 0;

      if (isTernary) {
        // bit mask
        // {0} -> 0, {-1, 1} -> 1
        mask[idx]  = ( (int_ptr[0]) == 0 ) ? 0 : 0x01;
        mask[idx] |= ( (int_ptr[1]) == 0 ) ? 0 : 0x02;
        mask[idx] |= ( (int_ptr[2]) == 0 ) ? 0 : 0x04;
        mask[idx] |= ( (int_ptr[3]) == 0 ) ? 0 : 0x08;
        mask[idx] |= ( (int_ptr[4]) == 0 ) ? 0 : 0x10;
        mask[idx] |= ( (int_ptr[5]) == 0 ) ? 0 : 0x20;
        mask[idx] |= ( (int_ptr[6]) == 0 ) ? 0 : 0x40;
        mask[idx] |= ( (int_ptr[7]) == 0 ) ? 0 : 0x80;
      }

      int_ptr += 8;
      idx++;
    }
  }

  if (isTernary) {
    // number of non zero
    int_ptr = (int8_t*)mxGetData(mxData);
    for (int n = 0; n < nnz_size; ++n) {
      nnz[n] = 0;
      for (int m = 0; m < M; ++m) {
        if (*int_ptr != 0) {
          nnz[n] += 1;
        }
        int_ptr++;
      }
    }
  }

  if (isTernary) {
    printf("\t Tensor Size: %lu K bytes\n", ((data_size * 3) / 1024));

    auto fbDims       = builder.CreateVector((const int*)dims, 2);
    auto fbStar       = builder.CreateVector((const uint8_t*)star, data_size);
    auto fbMask       = builder.CreateVector((const uint8_t*)mask, data_size);
    auto fbNNZ        = builder.CreateVector((const uint32_t*)nnz, nnz_size);

    auto valueBuilder = demitasse::serialize::TernaryValueBuilder(builder);
    valueBuilder.add_star(fbStar);
    valueBuilder.add_mask(fbMask);
    valueBuilder.add_nnz(fbNNZ);
    auto fbValue       = valueBuilder.Finish();

    auto tensorBuilder = demitasse::serialize::TensorBuilder(builder);
    tensorBuilder.add_type(demitasse::serialize::TensorType_Ternary);
    tensorBuilder.add_extent(fbDims);
    tensorBuilder.add_value(fbValue.Union());

    auto data = tensorBuilder.Finish();

    return data;
  } else {
    printf("\t Tensor Size: %lu K bytes\n", (data_size / 1024));

    auto fbDims        = builder.CreateVector((const int*)dims, 2);
    auto fbStar        = builder.CreateVector((const uint8_t*)star, data_size);

    auto valueBuilder  = demitasse::serialize::BinaryValueBuilder(builder);
    valueBuilder.add_data(fbStar);
    auto fbValue       = valueBuilder.Finish();

    auto tensorBuilder = demitasse::serialize::TensorBuilder(builder);
    tensorBuilder.add_type(demitasse::serialize::TensorType_Binary);
    tensorBuilder.add_extent(fbDims);
    tensorBuilder.add_value(fbValue.Union());

    auto data = tensorBuilder.Finish();

    return data;
  }
}

//
// create Tensor from Float matrix
//
flatbuffers::Offset<demitasse::serialize::Tensor> createFloatTensor4D(
    mxArray* mxData,
    flatbuffers::FlatBufferBuilder& builder) {
  int     dims[4];
  int     dimensions = (int)mxGetNumberOfDimensions(mxData);
  mwSize* dataDims   = (mwSize*)mxGetDimensions(mxData);

  if (dimensions != 4) {
    // errr
  }

  // reshape
  dims[0] = dataDims[2];  // C
  dims[1] = dataDims[1];  // W
  dims[2] = dataDims[0];  // H
  dims[3] = dataDims[3];  // M

  int data_size = dims[0];

  for (int d = 1; d < dimensions; ++d) {
    data_size *= dims[d];
  }

  float* f_ptr = (float*)mxGetData(mxData);
  float* data  = (float*)malloc(sizeof(float) * data_size);

  int    idx   = 0;
  // [H W C M] -> [C W H M]
  for (int m = 0; m < dims[3]; ++m) {
    for (int c = 0; c < dims[0]; ++c) {
      for (int w = 0; w < dims[1]; ++w) {
        for (int h = 0; h < dims[2]; ++h) {
          int offset = (m * dims[2] * dims[1] * dims[0]) + (h * dims[1] * dims[0]) + (w * dims[0]) + c;
          data[offset] = *f_ptr;
          f_ptr++;
        }
      }
    }
  }

  auto fbDims        = builder.CreateVector((const int*)dims, dimensions);
  auto fbData        = builder.CreateVector((const float*)data, data_size);

  auto valueBuilder  = demitasse::serialize::FloatValueBuilder(builder);
  valueBuilder.add_data(fbData);
  auto fbValue       = valueBuilder.Finish();

  auto tensorBuilder = demitasse::serialize::TensorBuilder(builder);
  tensorBuilder.add_type(demitasse::serialize::TensorType_Float);
  tensorBuilder.add_extent(fbDims);
  tensorBuilder.add_value(fbValue.Union());

  auto fbTensor = tensorBuilder.Finish();

  return fbTensor;
}

flatbuffers::Offset<demitasse::serialize::Tensor> createFloatTensor2D(
    mxArray* mxData,
    flatbuffers::FlatBufferBuilder& builder) {
  int     dims[2];
  int     dimensions = (int)mxGetNumberOfDimensions(mxData);
  mwSize* dataDims   = (mwSize*)mxGetDimensions(mxData);

  if (dimensions != 2) {
    // errr
  }

  // reshape
  dims[0] = dataDims[1];  // W
  dims[1] = dataDims[0];  // H

  int    data_size = dims[0] * dims[1];

  float* f_ptr     = (float*)mxGetData(mxData);
  float* data      = (float*)malloc(sizeof(float) * data_size);

  int    idx       = 0;
  for (int h = 0; h < dims[1]; ++h) {
    for (int w = 0; w < dims[0]; ++w) {
      int offset = (h * dims[0]) + w;
      data[offset] = *f_ptr;
      f_ptr++;
    }
  }

  auto fbDims        = builder.CreateVector((const int*)dims, dimensions);
  auto fbData        = builder.CreateVector((const float*)data, data_size);

  auto valueBuilder  = demitasse::serialize::FloatValueBuilder(builder);
  valueBuilder.add_data(fbData);
  auto fbValue       = valueBuilder.Finish();

  auto tensorBuilder = demitasse::serialize::TensorBuilder(builder);
  tensorBuilder.add_type(demitasse::serialize::TensorType_Float);
  tensorBuilder.add_extent(fbDims);
  tensorBuilder.add_value(fbValue.Union());

  auto fbTensor = tensorBuilder.Finish();

  return fbTensor;
}


//
//
flatbuffers::Offset<demitasse::serialize::Layer> parse_conv(
    int idx,
    DataInputForm& inputForm,
    mxArray*       layer,
    flatbuffers::FlatBufferBuilder& builder) {
  char type_str[256];
  char name_str[256];

  mxGetString(mxGetField(layer, 0, "type"),   type_str,   256);
  mxGetString(mxGetField(layer, 0, "name"),   name_str,   256);

  double* size_ptr = (double*)mxGetData(mxGetField(layer, 0, "size"));

  int ksize = 0;
  ksize = (int)size_ptr[0]; // size_ptr[0]: w, size_ptr[1]: h

  int channels = 0;
  channels = (int)size_ptr[2];

  // number of output maps (num of filters)
  int num_output = 0;
  num_output = (int)size_ptr[3];

  double* pad_ptr = (double*)mxGetData(mxGetField(layer, 0, "pad"));

  int     pad     = 0; // default
  pad = (int)pad_ptr[0];

  double* stride_ptr = (double*)mxGetData(mxGetField(layer, 0, "stride"));

  int     stride     = 1; // default
  stride = (int)stride_ptr[0];

  // weight
  bool     isConv           = true;

  mxArray* mxWeight         = mxGetCell(mxGetField(layer, 0, "weights"), 0);
  int      weightDimensions = (int)mxGetNumberOfDimensions(mxWeight);
  mwSize*  weightDims       = (mwSize*)mxGetDimensions(mxWeight);

  size_t weight_size = weightDims[0] * weightDims[1] * weightDims[2] * weightDims[3]; // channels * ksize * ksize * num_output;
  float* weight_ptr  = (float*)mxGetData(mxWeight);
  float* weight      = (float*)malloc(sizeof(float) * weight_size);

  // if imput data dim and filter dim is same, it is fully connected layer
  if (weightDims[0] == inputForm.w
      && weightDims[1] == inputForm.h
      && weightDims[2] == inputForm.c
      && stride == 1 && pad == 0) {
    isConv = false;
    strcpy(type_str, "fc");
  }

  // DEBUG
  printf("layer [%.2d]: %s (%s)\n", idx, name_str, type_str);

  printf("\t size [%d %d %d %d]\n", (int)size_ptr[0], (int)size_ptr[1], (int)size_ptr[2], (int)size_ptr[3]);
  printf("\t in   [%d %d %d]\n", inputForm.w, inputForm.h, inputForm.c);

  printf("\t pad: %d\n",      pad);
  printf("\t stride: %d\n",   stride);
  printf("\t ksize: %d\n",    ksize);
  printf("\t filters: %d\n",  num_output);

  printf("\t weight(%d) [%d %d %d %d]\n", weightDimensions, (int)weightDims[0], (int)weightDims[1], (int)weightDims[2], (int)weightDims[3]);
  printf("\t Tensor Size: %lu K bytes\n", (sizeof(float) * weight_size) / 1024);

  for (int n = 0; n < weight_size; ++n) {
    weight[n] = 0.0;
  }

  // load column major array (Matlab) -> row major array (C/C++)
  // copy weights value [H W C K] -> [C W H K]
  for (int k = 0; k < weightDims[3]; ++k) {     // 3: M
    for (int c = 0; c < weightDims[2]; ++c) {   // 2: C
      int cc = c;
      if (idx == 0) {
        // reverse channel order from RGB to BGR.
        cc = weightDims[2] - 1 - c;
      }

      for (int x = 0; x < weightDims[0]; ++x) {       // 0: W
        for (int y = 0; y <  weightDims[1]; ++y) {    // 1: H
          int offset = (k * weightDims[0] * weightDims[1] * weightDims[2]) + (y * weightDims[0] * weightDims[2]) + (x * weightDims[2]) + cc;
          weight[offset] = *weight_ptr;
          weight_ptr++;
        }
      }
    }
  }

  // debug
/*
   for (int n = 0; n < channels * ksize * ksize; ++n) {
    if (n % (channels * ksize) == 0) {
      printf("\n");
    }
    printf("%f ", weight[n]);
    if (n % channels == (channels - 1)) {
      printf("\n");
    }
   }
   printf("\n");
 */

  // bias
  mxArray* mxBias = mxGetCell(mxGetField(layer, 0, "weights"), 1);
  int      biasDimensions = (int)mxGetNumberOfDimensions(mxBias);

  mwSize*  biasDims       = (mwSize*)mxGetDimensions(mxBias);
  size_t   bias_size      = biasDims[0] * biasDims[1];
  float*   bias = (float*)malloc(sizeof(float) * bias_size);
  float*   bias_ptr       = (float*)mxGetData(mxBias);

  printf("\t bias(%d)   [%d %d]\n", biasDimensions, (int)biasDims[0], (int)biasDims[1]);
  printf("\t Tensor Size: %lu K bytes\n", (sizeof(float) * bias_size) / 1024);

  for (int n = 0; n < bias_size; ++n) {
    bias[n] = 0.0;
  }

  // copy bias values
  for (int k = 0; k < bias_size; ++k) {
    bias[k] = (float)*bias_ptr;
    bias_ptr++;
  }

  auto fbWeight = builder.CreateVector((const float*)weight, weight_size);
  auto fbBias   = builder.CreateVector((const float*)bias, bias_size);
  auto name     = builder.CreateString(name_str);

  auto fbLayer  = demitasse::serialize::LayerBuilder(builder);

  if (isConv) {
    fbLayer.add_type(demitasse::serialize::LayerType_Convolution);
    fbLayer.add_name(name);
    fbLayer.add_in_maps(channels);
    fbLayer.add_out_maps(num_output);
    fbLayer.add_ksize(ksize);
    fbLayer.add_stride(stride);
    fbLayer.add_padding(pad);

    // calc output form from input
    inputForm.c = num_output;
    inputForm.w = (inputForm.w + (2 * pad) - ksize) / stride + 1;
    inputForm.h = (inputForm.h + (2 * pad) - ksize) / stride + 1;
  } else {
    int total_maps = inputForm.w * inputForm.h * inputForm.c;
    fbLayer.add_type(demitasse::serialize::LayerType_FullyConnected);
    fbLayer.add_name(name);
    fbLayer.add_in_maps(total_maps);
    fbLayer.add_out_maps(num_output);

    // calc output form from input
    inputForm.c = num_output; // total_maps;
    inputForm.w = 1;
    inputForm.h = 1;
  }

  fbLayer.add_weight(fbWeight);
  fbLayer.add_bias(fbBias);

  auto data = fbLayer.Finish();

  printf("          : (c:%d, w:%d, h:%d)\n", inputForm.c, inputForm.w, inputForm.h);

  return data;
}


//
//
flatbuffers::Offset<demitasse::serialize::Layer> parse_intconv(
    int idx,
    DataInputForm &inputForm,
    mxArray       *mxLayer,
    flatbuffers::FlatBufferBuilder &builder) {
  char type_str[256];
  char name_str[256];

  mxGetString(mxGetField(mxLayer, 0, "type"),   type_str,   256);
  mxGetString(mxGetField(mxLayer, 0, "name"),   name_str,   256);

  auto name          = builder.CreateString(name_str);

  // size
  double* size_ptr   = (double*)mxGetData(mxGetField(mxLayer, 0, "size"));

  int     ksize      = (int)size_ptr[0]; // size_ptr[0]: w, size_ptr[1]: h
  int     channels   = (int)size_ptr[2];
  int     num_output = (int)size_ptr[3];

  double* pad_ptr    = (double*)mxGetData(mxGetField(mxLayer, 0, "pad"));
  int     pad        = (int)pad_ptr[0];

  double* stride_ptr = (double*)mxGetData(mxGetField(mxLayer, 0, "stride"));
  int     stride     = (int)stride_ptr[0];

  // intconv stores float weight and bias in decomp_filter
  mxArray* mxDecompFilter  = mxGetField(mxLayer, 0, "decomp_filter");
  mxArray* mxIntegerBasis  = mxGetField(mxDecompFilter, 0, "integer_basis");
  mwSize*  basisDims       = (mwSize*)mxGetDimensions(mxIntegerBasis);
  int      total_in_maps   = (int)basisDims[0]; // input_maps (row)
  int      num_basis_total = (int)basisDims[1]; // num_basis_total (col)

  // weight
  bool isConv = true;

  // if imput data dim and filter dim is same, it is fully connected layer
  if (total_in_maps == inputForm.w * inputForm.h * inputForm.c && stride == 1 && pad == 0) {
    isConv = false;
    strcpy(type_str, "intfc");
  }

  // read weight and bias from blobs
  // weight
  mxArray* mxWeight         = mxGetField(mxDecompFilter, 0, "weight");
  int      weightDimensions = (int)mxGetNumberOfDimensions(mxWeight);
  mwSize*  weightDims       = (mwSize*)mxGetDimensions(mxWeight);
  size_t   weight_size      = weightDims[0] * weightDims[1]; // num_basis_total * num_output;
  float*   weight_ptr       = (float *)mxGetData(mxWeight);
  float*   weight           = (float*)malloc(sizeof(float) * weight_size);

  // DEBUG
  printf("layer [%.2d]: %s (%s)\n", idx, name_str, type_str);

  printf("\t size [%d %d %d %d]\n", (int)size_ptr[0], (int)size_ptr[1], (int)size_ptr[2], (int)size_ptr[3]);
  printf("\t in   [%d %d %d]\n", inputForm.w, inputForm.h, inputForm.c);

  printf("\t pad: %d\n",      pad);
  printf("\t stride: %d\n",   stride);
  printf("\t ksize: %d\n",    ksize);
  printf("\t filters: %d\n",  num_output);

  printf("\t weight(%d) [%d %d]\n", weightDimensions, (int)weightDims[0], (int)weightDims[1]);
  printf("\t Tensor Size: %lu K bytes\n", (sizeof(float) * weight_size) / 1024);

  // clear
  // load column major array (Matlab) -> row major array (C/C++)
  for (int n = 0; n < weight_size; ++n) {
    weight[n] = *weight_ptr;
    weight_ptr++;
  }

  // bias
  mxArray* mxBias         = mxGetCell(mxGetField(mxLayer, 0, "weights"), 2);
  mwSize*  biasDims       = (mwSize*)mxGetDimensions(mxBias);

  int      biasDimensions = (int)mxGetNumberOfDimensions(mxBias);
  size_t   bias_size      = biasDims[0];

  float*   bias_ptr       = (float*)mxGetData(mxBias);
  float*   bias           = (float*)malloc(sizeof(float) * bias_size);

  printf("\t bias(%d)   [%d %d]\n", biasDimensions, (int)biasDims[0], (int)biasDims[1]);
  printf("\t Tensor Size: %lu K bytes\n", (sizeof(float) * bias_size) / 1024);

  // clear
  // copy bias values to 1D Tensor
  for (int n = 0; n < bias_size; ++n) {
    bias[n] = (float)*bias_ptr;
    bias_ptr++;
  }

  auto fbWeight = builder.CreateVector((const float*)weight, weight_size);
  auto fbBias   = builder.CreateVector((const float*)bias,   bias_size);

  // integer basis
  int  binary_type = (int)mxGetScalar(mxGetField(mxDecompFilter, 0, "binaly_or_ternaly"));
  bool isTernary   = (binary_type == 3 ? true : false);
  if (isTernary) {
    printf("\t integer_basis type: ternary\n");
  } else {
    printf("\t integer_basis type: binary\n");
  }


  auto     fbIntegerBasis  = createBinaryTensor(isTernary, mxIntegerBasis, builder);

  // decomp_map
  mxArray* mxDecompMap = mxGetField(mxLayer, 0, "decomp_map");

  // basis weight
  mxArray* mxBasisWeight = mxGetField(mxDecompMap, 0, "weight");
  mwSize*  dims          = (mwSize*)mxGetDimensions(mxBasisWeight);

  printf("map weight [%lu %lu]\n", dims[0], dims[1]);

  size_t num_basis       = (int)dims[0];
  float* rawBasisWeight  = (float *)malloc(sizeof(float) * num_basis);
  float* basis_w_ptr     = (float *)mxGetData(mxBasisWeight);
  for (int i = 0; i < num_basis; ++i) {
    rawBasisWeight[i] = basis_w_ptr[i];
  }

  printf("\t basis weight [%lu]\n",     num_basis);
  printf("\t Tensor Size: %lu bytes\n", (sizeof(float) * num_basis));

  auto fbBasisWeight = builder.CreateVector((const float*)rawBasisWeight, num_basis);

  // binary encoder
  mxArray* mxEncoder = mxGetField(mxDecompMap, 0, "encoder");
  auto     fbEncoder = createEncoder(mxEncoder, builder);

  // create layer
  auto fbLayer = demitasse::serialize::LayerBuilder(builder);

  if (isConv) {
    fbLayer.add_type(demitasse::serialize::LayerType_BinaryConvolution);
    fbLayer.add_name(name);
    fbLayer.add_in_maps(channels);
    fbLayer.add_out_maps(num_output);
    fbLayer.add_ksize(ksize);
    fbLayer.add_stride(stride);
    fbLayer.add_padding(pad);

    // calc output form from input
    inputForm.c = num_output;
    inputForm.w = (inputForm.w + (2 * pad) - ksize) / stride + 1;
    inputForm.h = (inputForm.h + (2 * pad) - ksize) / stride + 1;
  } else {
    fbLayer.add_type(demitasse::serialize::LayerType_BinaryFullyConnected);
    fbLayer.add_name(name);
    fbLayer.add_in_maps(total_in_maps);
    fbLayer.add_out_maps(num_output);

    // calc output form from input
    inputForm.c = num_output;
    inputForm.w = 1;
    inputForm.h = 1;
  }

  fbLayer.add_weight(fbWeight);
  fbLayer.add_bias(fbBias);
  fbLayer.add_num_basis(num_basis);
  fbLayer.add_num_basis_total(num_basis_total);
  fbLayer.add_integer_basis(fbIntegerBasis);
  fbLayer.add_basis_weight(fbBasisWeight);
  fbLayer.add_encoder(fbEncoder);

  auto data = fbLayer.Finish();

  printf("          : (c:%d, w:%d, h:%d)\n", inputForm.c, inputForm.w, inputForm.h);

  return data;
}

flatbuffers::Offset<demitasse::serialize::Layer> parse_pool(
    int idx,
    DataInputForm &inputForm,
    mxArray* layer,
    flatbuffers::FlatBufferBuilder &builder) {
  char method_str[256];
  char name_str[256];

  mxGetString(mxGetField(layer, 0, "method"), method_str, 256);
  mxGetString(mxGetField(layer, 0, "name"),   name_str,   256);

  const std::string pool_method = method_str;
  const std::string layer_name  = name_str;

  auto layerType = demitasse::serialize::LayerType_MaxPooling;

  if (pool_method == "max") {
    layerType = demitasse::serialize::LayerType_MaxPooling;
  } else if (pool_method == "avarage") {
    layerType = demitasse::serialize::LayerType_AvaragePooling;
  } else {
    // error not support
    abort();
  }

  int     pad     = 0;
  double* pad_ptr = (double*)mxGetData(mxGetField(layer, 0, "pad"));
  pad = (int)pad_ptr[0]; // top bottom left right

  int     stride     = 0;
  double* stride_ptr = (double*)mxGetData(mxGetField(layer, 0, "stride"));
  stride = (int)stride_ptr[0];

  int     ksize    = 0;
  double* pool_ptr = (double*)mxGetData(mxGetField(layer, 0, "pool"));
  ksize = (int)pool_ptr[0];

  printf("layer [%.2d]: %s (%s)\n", idx, name_str, method_str);

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
    int i,
    mxArray* layer,
    flatbuffers::FlatBufferBuilder &builder) {
  char type_str[256];
  char name_str[256];

  mxGetString(mxGetField(layer, 0, "type"),   type_str,   256);
  mxGetString(mxGetField(layer, 0, "name"),   name_str,   256);

  const std::string layer_name = name_str;

  double* param_ptr = (double*)mxGetData(mxGetField(layer, 0, "param"));
  // to equal with Caffe, alpha is N * ALPHA
  int     n     = (int)param_ptr[0];    // default: 5
  float   k     = param_ptr[1];         // default: 1
  float   alpha = param_ptr[2] * param_ptr[0];    // default: 1
  float   beta  = param_ptr[3];         // default: 5

  printf("layer [%.2d]: %s (%s)\n", i, name_str, type_str);

  printf("\t n:     %d\n", n);
  printf("\t k:     %f\n", k);
  printf("\t alpha: %f\n", alpha);
  printf("\t beta:  %f\n", beta);

  // prepare flat buffers data
  auto name    = builder.CreateString(name_str);

  auto fbLayer = demitasse::serialize::LayerBuilder(builder);

  fbLayer.add_type(demitasse::serialize::LayerType_LRN);
  fbLayer.add_name(name);
  fbLayer.add_ksize(n);
  fbLayer.add_k(k);
  fbLayer.add_alpha(alpha);
  fbLayer.add_beta(beta);

  auto data = fbLayer.Finish();

  return data;
}


void write_model_to_file(flatbuffers::FlatBufferBuilder& builder, const std::string& file) {
  auto buffer = builder.GetBufferPointer();
  auto size   = builder.GetSize();

  std::ofstream fs(file, std::ios::out | std::ios::binary | std::ios::trunc);

  fs.write((const char*)buffer, size);
  fs.close();

  builder.ReleaseBufferPointer();
}

void load_matconvnet_model(const std::string& model, const std::string& outfile) {
  MATFile* mfp       = matOpen(model.c_str(), "r");

  mxArray* matLayers = matGetVariable(mfp, "layers");
  if (matLayers == nullptr) {
    // error
    matClose(mfp);
    return;
  }

  int num_layers = mxGetNumberOfElements(matLayers);

  printf("layers : %d\n", num_layers);

  // These parameters required to calculate weight array layout conversion at conv -> fc
  DataInputForm inputForm;

  inputForm.w = 224;
  inputForm.h = 224;
  inputForm.c = 3;

  // prepare output flatbuffer layers
  flatbuffers::FlatBufferBuilder builder;

  auto fbLayers = new flatbuffers::Offset<demitasse::serialize::Layer>[num_layers];
  int  idx      = 0;

  for (int i = 0; i < num_layers; ++i) {
    mxArray* layer = mxGetCell(matLayers, i);
    char     type_str[256];
    mxGetString(mxGetField(layer, 0, "type"), type_str, 256);

    std::string layer_type = type_str;

    if (layer_type == "relu") {
      fbLayers[idx] = parse_activation(i, layer, builder);
      idx++;
    } else if (layer_type == "tanh") {
      fbLayers[idx] = parse_activation(i, layer, builder);
      idx++;
    } else if (layer_type == "sigmoid") {
      fbLayers[idx] = parse_activation(i, layer, builder);
      idx++;
    } else if (layer_type == "softmax") {
      fbLayers[idx] = parse_activation(i, layer, builder);
      idx++;
    } else if (layer_type == "conv") {
      // check it is fully connected layer or convolution layer
      fbLayers[idx] = parse_conv(i, inputForm, layer, builder);
      idx++;
    } else if (layer_type == "intconv") {
      fbLayers[idx] = parse_intconv(i, inputForm, layer, builder);
      idx++;
    } else if (layer_type == "pool") {
      fbLayers[idx] = parse_pool(i, inputForm, layer, builder);
      idx++;
    } else if (layer_type == "lrn") {
      fbLayers[idx] = parse_lrn(i, layer, builder);
      idx++;
    } else {
      // error
      printf("unsupported layer type \'%s\' in .mat file\n", type_str);

      matClose(mfp);
      exit(-1);
    }
  }

  matClose(mfp);

  // write layers to the model
  auto mlocLayers = builder.CreateVector(fbLayers, idx);

  // root entity
  auto fbModel = demitasse::serialize::ModelBuilder(builder);
  fbModel.add_layers(mlocLayers);

  auto mloc = fbModel.Finish();
  demitasse::serialize::FinishModelBuffer(builder, mloc);

  // write to the file
  write_model_to_file(builder, outfile);
}

void create_dirs(const char* path) {
  const char *dir_name = dirname((char *)path);

  if (strcmp(dir_name, ".") != 0 && strcmp(dir_name, "/") != 0) {
    mkdir(dir_name, 0777);
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("Usage: %s <model> [<output>]\n", argv[0]);
    return 0;
  }

  const char* model   = argv[1];
  const char* outfile = "./vudnn.model";
  if (argc == 3) {
    // prepare directory
    create_dirs(argv[2]);
    outfile = argv[2];
  }

  load_matconvnet_model(model, outfile);

  return 0;
}
