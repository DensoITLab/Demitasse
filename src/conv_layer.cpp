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
 #include <string.h>
#include <stdlib.h>
#if DEBUG
#include <chrono>
#endif

#include "ispc_im2col.hpp"

#ifdef USE_BLAS
#ifdef USE_MKL
#include "mkl.h"
#else  // USE_MKL
#ifdef USE_ACCELERATE
#include <Accelerate/Accelerate.h>
#else  // USE_ACCELERATE
#include <cblas.h>
#endif // USE_ACCELERATE
#endif // USE_MKL
#else  // USE_BLAS
#include "ispc_gemv.hpp"
#endif // USE_BLAS

#include "debug_log.hpp"
#include "conv_layer.hpp"

using demitasse::ConvLayer;
using demitasse::ConvParam;
using demitasse::Blob;

ConvLayer::ConvLayer(const ConvParam* param) {
  param_.in_maps  = param->in_maps;
  param_.out_maps = param->out_maps;

  param_.ksize    = param->ksize;
  param_.stride   = param->stride;
  param_.padding  = param->padding;

  weight_   = new Blob(Blob::BlobType::Float, param_.in_maps, param_.ksize, param_.ksize, param_.out_maps);
  bias_     = new Blob(Blob::BlobType::Float, param_.out_maps);

  im2col_   = nullptr;
  output_   = nullptr;
}

ConvLayer::~ConvLayer() {
  delete weight_;
  delete bias_;

  delete im2col_;
  delete output_;
}

const std::string ConvLayer::layerTypeName() {
  return std::string("conv layer");
}

const Blob* ConvLayer::weight() {
  return weight_;
};

const Blob* ConvLayer::bias() {
  return bias_;
};

void ConvLayer::setWeight(const Blob* weight) {
  if (weight_->isEqualExtents(weight)) {
    memcpy(weight_->rawData().RealSingle.data, weight->rawData().RealSingle.data, weight->rawDataBytes());
  }
}

void ConvLayer::setBias(const Blob* bias) {
  if (bias_->isEqualExtents(bias)) {
    memcpy(bias_->rawData().RealSingle.data,   bias->rawData().RealSingle.data,   bias->rawDataBytes());
  }
}

bool ConvLayer::isNeedIm2Col(int in_x_nodes, int in_y_nodes) {
  bool need_im2col = true;
  if (in_x_nodes == param_.ksize && in_y_nodes == param_.ksize) {
    need_im2col = false;
  }

  return need_im2col;
}

void ConvLayer::reshapeOutputBlob(const Blob* input) {
  int in_maps     = input->extent(0);
  int in_x_nodes  = input->extent(1);
  int in_y_nodes  = input->extent(2);
  int num_batch   = input->extent(3);

  int out_x_nodes = param_.out_x_nodes(in_x_nodes);
  int out_y_nodes = param_.out_y_nodes(in_y_nodes);

  if (param_.in_maps != in_maps) {
    DebugLog("in_maps_ : %d != %d\n", param_.in_maps, in_maps);
    DebugLog("conv: invalid input data form\n");
    abort();
  }

  if (output_ == nullptr || output_->extent(0) != param_.out_maps || output_->extent(1) != out_x_nodes || output_->extent(2) != out_y_nodes || output_->extent(3) != num_batch) {
    delete output_;
    output_ = new Blob(Blob::BlobType::Float, param_.out_maps, out_x_nodes, out_y_nodes, num_batch);

    if (this->isNeedIm2Col(in_x_nodes, in_y_nodes)) {
      int filter_size = param_.in_maps * param_.ksize * param_.ksize;
      int col_nodes   = (filter_size % 16 > 0 ? ((filter_size / 16) + 1) * 16 : filter_size); // FIXME: 16 bytes align

      delete im2col_;
      im2col_ = new Blob(Blob::BlobType::Float, col_nodes, out_x_nodes * out_y_nodes, 1);
    }
  }
}

const Blob * ConvLayer::feedForward(const Blob* input) {
  this->reshapeOutputBlob(input);

  int  in_maps               = input->extent(0);
  int  in_x_nodes            = input->extent(1);
  int  in_y_nodes            = input->extent(2);
  int  num_batch             = input->extent(3);

  int  out_x_nodes           = param_.out_x_nodes(in_x_nodes);
  int  out_y_nodes           = param_.out_y_nodes(in_y_nodes);

  bool use_im2col            = this->isNeedIm2Col(in_x_nodes, in_y_nodes);
  int  filter_size           = param_.in_maps * param_.ksize * param_.ksize;
  int  col_nodes             = (filter_size % 16 > 0 ? ((filter_size / 16) + 1) * 16 : filter_size); // 16 bytes alignment padding

  float*       input_ptr     = input->rawData().RealSingle.data;      // input data
  const float* weight_ptr    = weight_->rawData().RealSingle.data;    // weight
  const float* bias_ptr      = bias_->rawData().RealSingle.data;      // bias
  float*       output_ptr    = output_->rawData().RealSingle.data;    // output
  float*       im2col_ptr    = nullptr;

  const int    input_stride  = input->stride(2);
  const int    output_stride = output_->stride(2);

  for (int n = 0; n < num_batch; ++n) {

#if DEBUG_PROFILER
    auto t1 = std::chrono::high_resolution_clock::now();
#endif

    if (use_im2col) {
      im2col_ptr = im2col_->rawData().RealSingle.data;        // im2col output

      ispc::ispc_im2col(in_x_nodes,
          in_y_nodes,
          param_.in_maps,
          param_.ksize,
          param_.stride,
          param_.padding,
          out_x_nodes,                // output nodes x
          out_y_nodes,                // output nodes y
          input_ptr,                  // input data
          im2col_ptr);                // col layout data
    } else {
      im2col_ptr = input_ptr;
    }

#if DEBUG_PROFILER
    auto t2 = std::chrono::high_resolution_clock::now();
#endif

#if USE_BLAS
    // output <- bias
    // if conv as fc, out_x_nodes and out_y_nodes = 1
    for (int i = 0; i < out_x_nodes * out_y_nodes; ++i) {
      memcpy(&output_ptr[param_.out_maps * i], bias_ptr, sizeof(float) * param_.out_maps);
    }

    // weight
    // C = alpha AB + beta C
    // M, N, K: A[M × K], B[K × N], C[M × N]
    // * [row x col]
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasTrans,
        out_x_nodes * out_y_nodes,  // M
        param_.out_maps,            // N
        col_nodes,                  // K
        1.0f,                       // alpha
        im2col_ptr,                 // matrix A
        col_nodes,                  // lda
        weight_ptr,                 // matrix B
        col_nodes,                  // ldb
        1.0f,                       // beta
        output_ptr,                 // matrix C
        param_.out_maps);           // ldc
#else
    ispc::ispc_gemm(
        out_x_nodes * out_y_nodes,  // M
        param_.out_maps,            // N
        col_nodes,                  // K
        im2col_ptr,                 // A[M K] col layout data
        weight_ptr,                 // B[K N] weight
        bias_ptr,                   // Cp[N]  bias
        output_ptr);                // C[M N] result
#endif

#if DEBUG_PROFILER
    auto t3 = std::chrono::high_resolution_clock::now();

    double dt_im2col = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1e6;
    double dt_blas   = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count() / 1e6;

    DebugLog("\n");
    DebugLog("im2col:        %f\n", dt_im2col);
    DebugLog("BLAS:          %f [%d %d] x [%d %d]\n", dt_blas, out_x_nodes * out_y_nodes, col_nodes, col_nodes, param_.out_maps);
#endif

    input_ptr  += input_stride;
    output_ptr += output_stride;
  }

  return output_;
}
