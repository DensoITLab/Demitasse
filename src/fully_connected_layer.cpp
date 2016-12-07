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
#include <memory.h>
#include <stdlib.h>
#if DEBUG
#include <chrono>
#endif

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

#include "fully_connected_layer.hpp"
#include "debug_log.hpp"

using demitasse::FullyConnectedLayer;
using demitasse::Blob;

FullyConnectedLayer::FullyConnectedLayer(int in_maps, int out_maps) {
  in_maps_  = in_maps;
  out_maps_ = out_maps;

  weight_   = new Blob(Blob::BlobType::Float, in_maps, out_maps);
  bias_     = new Blob(Blob::BlobType::Float, out_maps);
  output_   = nullptr;
}

FullyConnectedLayer::~FullyConnectedLayer() {
  delete weight_;
  delete bias_;
  delete output_;
}

const std::string FullyConnectedLayer::layerTypeName() {
  return std::string("fully connected layer");
}

const Blob* FullyConnectedLayer::weight() {
  return weight_;
};

void FullyConnectedLayer::setWeight(const Blob* weight) {
  if (weight_->isEqualExtents(weight)) {
    memcpy(weight_->rawData().RealSingle.data, weight->rawData().RealSingle.data, weight->rawDataBytes());
  }
}

const Blob* FullyConnectedLayer::bias() {
  return bias_;
};

void FullyConnectedLayer::setBias(const Blob* bias) {
  if (bias_->isEqualExtents(bias)) {
    memcpy(bias_->rawData().RealSingle.data, bias->rawData().RealSingle.data, bias->rawDataBytes());
  }
}

void FullyConnectedLayer::reshapeOutputBlob(const Blob* input) {
  int input_dims = input->dimensions() - 1;
  int num_batch  = input->extent(input_dims);

  if (output_ == nullptr || output_->extent(0) != out_maps_ || output_->extent(1) != num_batch) {
    delete output_;
    output_ = new Blob(Blob::BlobType::Float, out_maps_, num_batch);
  }
}

const Blob* FullyConnectedLayer::feedForward(const Blob* input) {
  this->reshapeOutputBlob(input);

  int input_dims = input->dimensions() - 1;
  int num_batch  = input->extent(input_dims);

  const float* input_ptr     = input->rawData().RealSingle.data; // input
  const float* weight_ptr    = weight_->rawData().RealSingle.data; // weight
  const float* bias_ptr      = bias_->rawData().RealSingle.data; // bias
  float*       output_ptr    = output_->rawData().RealSingle.data; // output

  const int    input_stride  = (input_dims > 0 ? input->stride(input_dims - 1) : input->stride(0));
  const int    output_stride = output_->stride(0); // output data stride for batch

  for (int n = 0; n < num_batch; ++n) {
#if DEBUG_PROFILER
    auto t1 = std::chrono::high_resolution_clock::now();
#endif

#if USE_BLAS
    // output <- bias
    memcpy(output_ptr, bias_ptr, sizeof(float) * out_maps_);

    // weight
    // A [M N] x X [N]
    cblas_sgemv(CblasRowMajor, CblasNoTrans,
        out_maps_,            // M
        in_maps_,             // N
        1.0f,                 // alpha
        weight_ptr,           // matrix A : weight
        in_maps_,             // lda (A: col size)
        input_ptr,            // vector X : input
        1,                    // incX
        1.0f,                 // beta
        output_ptr,           // vector Y : output
        1);                   // incY
#else
    ispc::ispc_gemv(
        out_maps_,             // rows M
        in_maps_,              // cols N
        weight_ptr,            // weight
        bias_ptr,              // bias
        input_ptr,             // col layout data
        output_ptr);           // result
#endif

#if DEBUG_PROFILER
    auto t2 = std::chrono::high_resolution_clock::now();
#endif

#if DEBUG_PROFILER
    double dt_blas   = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1e6;

    DebugLog("\n");
    DebugLog("BLAS:          %f [%d %d] x [%d %d]\n", dt_blas, 1, in_maps_, in_maps_, out_maps_);
#endif

    input_ptr  += input_stride;
    output_ptr += output_stride;
  }

  return output_;
}
