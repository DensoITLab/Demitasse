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

#include <chrono>
#include <random>

// ispc
#include "ispc_binary_gemv.hpp"

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

#include "binary_fully_connected_layer.hpp"
#include "debug_log.hpp"

using demitasse::BinaryFullyConnectedLayer;
using demitasse::Blob;

BinaryFullyConnectedLayer::BinaryFullyConnectedLayer(
    int in_maps,
    int out_maps,
    int num_basis,
    int num_basis_total,
    BinaryEncoder *encoder) {
  in_maps_         = in_maps;
  out_maps_        = out_maps;
  num_basis_       = num_basis;
  num_basis_total_ = num_basis_total;

  encoder_         = encoder;

  weight_ = new Blob(Blob::BlobType::Float, num_basis_total_, out_maps_);
  bias_   = new Blob(Blob::BlobType::Float, out_maps_);

  integer_basis_ = new Blob(Blob::BlobType::Ternary, in_maps_, num_basis_total_);
  basis_weight_  = new Blob(Blob::BlobType::Float, num_basis_);

  binary_ = nullptr;
  temp_   = nullptr;
  output_ = nullptr;
}

BinaryFullyConnectedLayer::~BinaryFullyConnectedLayer() {
  delete encoder_;

  delete weight_;
  delete bias_;

  delete integer_basis_;
  delete basis_weight_;

  delete binary_;
  delete temp_;
  delete output_;
}

const std::string BinaryFullyConnectedLayer::layerTypeName() {
  return std::string("<binary fully connected layer>");
}

const Blob* BinaryFullyConnectedLayer::weight() {
  return weight_;
};

const Blob* BinaryFullyConnectedLayer::bias() {
  return bias_;
};

const Blob* BinaryFullyConnectedLayer::integerBasis() {
  return integer_basis_;
};

const Blob* BinaryFullyConnectedLayer::basisWeight() {
  return basis_weight_;
};


void BinaryFullyConnectedLayer::reshapeOutputBlob(const Blob* input) {
  int input_dims = input->dimensions() - 1;
  int num_batch  = input->extent(input_dims);

  if (output_ == nullptr || output_->extent(0) != out_maps_ || output_->extent(1) != num_batch) {
    delete output_;
    output_ = new Blob(Blob::BlobType::Float, out_maps_, num_batch);

    delete binary_;

    int in_maps = 1;
    for (int d = 0; d < input->dimensions(); ++d) {
      in_maps *= input->extent(d);
    }
    binary_ = new Blob(Blob::BlobType::Binary, in_maps, num_basis_, 1 /* num_batch */);

    delete temp_;
    temp_ = new Blob(Blob::BlobType::Float, 1, num_basis_total_);
  }
};

const Blob* BinaryFullyConnectedLayer::feedForward(const Blob* input) {
  this->reshapeOutputBlob(input);

  int input_dims = input->dimensions() - 1;
  int num_batch  = input->extent(input_dims);

  const uint8_t* binary_ptr = binary_->rawData().Binary.data;    // input

  // binary/ternary basis matrix
  int basis_type = static_cast<int>(integer_basis_->blobType());
  const uint8_t*  basis_star_ptr   = integer_basis_->rawData().Ternary.star;
  const uint8_t*  basis_mask_ptr   = integer_basis_->rawData().Ternary.mask;
  const uint32_t* basis_nnz_ptr    = integer_basis_->rawData().Ternary.nnz;
  const float*    basis_weight_ptr = basis_weight_->rawData().RealSingle.data;

  float*          temp_ptr         = temp_->rawData().RealSingle.data;     // temporary output
  const float*    weight_ptr       = weight_->rawData().RealSingle.data;   // weight
  const float*    bias_ptr         = bias_->rawData().RealSingle.data;     // bias
  float*          output_ptr       = output_->rawData().RealSingle.data;   // output

  const int       binary_stride    = binary_->stride(0); // input  data stride for batch
  const int       output_stride    = output_->stride(0); // output data stride for batch

  for (int n = 0; n < num_batch; ++n) {
#if DEBUG_PROFILER
    auto t1 = std::chrono::high_resolution_clock::now();
#endif

    // binary encoding
    encoder_->encode_map(binary_, input, n);

#if DEBUG_PROFILER
    auto t2 = std::chrono::high_resolution_clock::now();
#endif

    // binary x basis
    // A[M N] x X[N]
    ispc::ispc_binary_gemv(
        num_basis_total_,      // M
        in_maps_,              // N
        num_basis_,            // basis
        basis_type,            // ternary
        basis_star_ptr,        // binary weight: A [M N]
        basis_mask_ptr,
        basis_nnz_ptr,
        basis_weight_ptr,      // basis_weight
        binary_ptr,            // X[N]
        temp_ptr);             // output:[M]


#if DEBUG_PROFILER
    auto t3 = std::chrono::high_resolution_clock::now();
#endif

#if USE_BLAS
    // output <- bias
    memcpy(output_ptr, bias_ptr, sizeof(float) * out_maps_);

    // A[M N] x X[N]
    cblas_sgemv(CblasRowMajor, CblasNoTrans,
        out_maps_,            // M
        num_basis_total_,     // N
        1.0f,                 // alpha
        weight_ptr,           // matrix A : weight
        num_basis_total_,     // lda (A: col size)
        temp_ptr,             // vector X : input
        1,                    // incX
        1.0f,                 // beta
        output_ptr,           // vector Y : output
        1);                   // incY
#else
    ispc::ispc_gemv(
        out_maps_,             // rows M
        num_basis_total_,      // cols N
        weight_ptr,            // A[M N]: weight
        bias_ptr,              // bias
        temp_ptr,              // X[N]: input
        output_ptr);           // C[M]: result
#endif

#if DEBUG_PROFILER
    auto t4 = std::chrono::high_resolution_clock::now();
#endif

#if DEBUG_PROFILER
    double dt_encode  = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1e6;
    double dt_bmul    = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count() / 1e6;
    double dt_blas    = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count() / 1e6;

    DebugLog("\n");
    DebugLog("binary encode: %f\n", dt_encode);
    DebugLog("binary mul:    %f [%d %d] x [%d %d]\n", dt_bmul, 1, in_maps_, in_maps_, num_basis_total_);
    DebugLog("BLAS:          %f [%d %d] x [%d %d]\n", dt_blas, 1, num_basis_total_, num_basis_total_, out_maps_);
#endif

    output_ptr += output_stride;
  }

  return output_;
}
