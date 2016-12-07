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

#include "ispc_binary_im2col.hpp"
#include "ispc_binary_gemm.hpp"

#include "debug_log.hpp"

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
#include "binary_conv_layer.hpp"

using demitasse::BinaryConvLayer;
using demitasse::ConvParam;
using demitasse::Blob;

BinaryConvLayer::BinaryConvLayer(const ConvParam* param, int num_basis, int num_basis_total, BinaryEncoder* encoder) {
  param_.in_maps   = param->in_maps;
  param_.out_maps  = param->out_maps;

  param_.ksize     = param->ksize;
  param_.stride    = param->stride;
  param_.padding   = param->padding;

  num_basis_       = num_basis;
  num_basis_total_ = num_basis_total;

  encoder_ = encoder;

  // weight_   = new demitasse::Blob(Blob::BlobType::Float, param_.in_maps, param_.ksize, param_.ksize, param_.out_maps);
  weight_ = new Blob(Blob::BlobType::Float, num_basis_total_, param_.out_maps /* out_maps_ */);
  bias_   = new Blob(Blob::BlobType::Float, param_.out_maps);

  unsigned int total_in_maps = param_.ksize * param_.ksize * param_.in_maps;
  integer_basis_ = new Blob(Blob::BlobType::Ternary, total_in_maps, num_basis_total_);
  basis_weight_  = new Blob(Blob::BlobType::Float, num_basis_);

  binary_ = nullptr;
  temp_   = nullptr;

  im2col_ = nullptr;
  output_ = nullptr;
}

BinaryConvLayer::~BinaryConvLayer() {
  delete encoder_;

  delete weight_;
  delete bias_;

  delete integer_basis_;
  delete basis_weight_;

  delete binary_;
  delete temp_;

  delete im2col_;
  delete output_;
}

const std::string BinaryConvLayer::layerTypeName() {
  return std::string("binary conv layer");
}

const Blob* BinaryConvLayer::weight() {
  return weight_;
};

const Blob* BinaryConvLayer::bias() {
  return bias_;
};

const Blob* BinaryConvLayer::integerBasis() {
  return integer_basis_;
};

const Blob* BinaryConvLayer::basisWeight() {
  return basis_weight_;
};

/*
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
 */

bool BinaryConvLayer::isNeedIm2Col(int in_x_nodes, int in_y_nodes) {
  bool need_im2col = true;
  if (in_x_nodes == param_.ksize && in_y_nodes == param_.ksize) {
    need_im2col = false;
  }
  return need_im2col;
}

void BinaryConvLayer::reshapeOutputBlob(const Blob* input) {
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
      im2col_ = new Blob(Blob::BlobType::Binary, col_nodes, out_x_nodes * out_y_nodes);

      delete binary_;
      binary_ = new Blob(Blob::BlobType::Binary, col_nodes, num_basis_, out_x_nodes * out_y_nodes);
    } else {
      int total_in_maps = in_x_nodes * in_y_nodes * in_maps;
      binary_ = new Blob(Blob::BlobType::Binary, total_in_maps, num_basis_, 1);
    }

    delete temp_;
    temp_ = new Blob(Blob::BlobType::Float, num_basis_total_, out_x_nodes * out_y_nodes);
  }
}

const Blob * BinaryConvLayer::feedForward(const Blob* input) {
  this->reshapeOutputBlob(input);

  int  in_maps     = input->extent(0);
  int  in_x_nodes  = input->extent(1);
  int  in_y_nodes  = input->extent(2);
  int  num_batch   = input->extent(3);

  int  out_x_nodes = param_.out_x_nodes(in_x_nodes);
  int  out_y_nodes = param_.out_y_nodes(in_y_nodes);

  bool use_im2col  = this->isNeedIm2Col(in_x_nodes, in_y_nodes);
  int  filter_size = param_.in_maps * param_.ksize * param_.ksize;
  int  col_nodes   = (filter_size % 16 > 0 ? ((filter_size / 16) + 1) * 16 : filter_size);           // 16 bytes alignment padding

  float*         input_ptr  = input->rawData().RealSingle.data;       // input data
  float*         temp_ptr   = temp_->rawData().RealSingle.data;       // temporary output

  const float*   weight_ptr = weight_->rawData().RealSingle.data;     // weight
  const float*   bias_ptr   = bias_->rawData().RealSingle.data;       // bias
  float*         output_ptr = output_->rawData().RealSingle.data;     // output

  uint8_t* im2col_ptr = nullptr;
  uint8_t* binary_ptr = binary_->rawData().Binary.data;         // input

  // binary/ternary basis matrix
  int basis_type = static_cast<int>(integer_basis_->blobType());
  const uint8_t*  basis_star_ptr   = integer_basis_->rawData().Ternary.star;
  const uint8_t*  basis_mask_ptr   = integer_basis_->rawData().Ternary.mask;
  const uint32_t* basis_nnz_ptr    = integer_basis_->rawData().Ternary.nnz;
  const float*    basis_weight_ptr = basis_weight_->rawData().RealSingle.data;

  const int       input_stride     = input->stride(2);
  const int       output_stride    = output_->stride(2);

  for (int n = 0; n < num_batch; ++n) {

#if DEBUG_PROFILER
    auto t1 = std::chrono::high_resolution_clock::now();
#endif

    // convert to binary
    encoder_->encode_map(binary_, input, n);

#if DEBUG_PROFILER
    auto t2 = std::chrono::high_resolution_clock::now();
#endif

    // im2col
    binary_ptr = binary_->rawData().Binary.data;
    if (use_im2col) {
      im2col_ptr = im2col_->rawData().Binary.data;        // im2col output
      ispc::ispc_binary_im2col(in_x_nodes,
          in_y_nodes,
          (param_.in_maps / 8),       // binary packed size
          num_basis_,                 // basis: N
          param_.ksize,
          param_.stride,
          param_.padding,
          out_x_nodes,                // output nodes x
          out_y_nodes,                // output nodes y
          binary_ptr,                 // input data
          im2col_ptr);                // col layout data
    } else {
      im2col_ptr = binary_ptr;
    }

#if DEBUG_PROFILER
    auto t3 = std::chrono::high_resolution_clock::now();
#endif

    // binary x basis
    // M, N, K: A[M × K], B[K × N], C[M × N]
    ispc::ispc_binary_gemm(
        out_x_nodes * out_y_nodes,  // M
        num_basis_total_,           // N
        col_nodes,                  // K
        num_basis_,                 // basis
        im2col_ptr,                 // A[M K]
        basis_type,                 // ternary
        basis_star_ptr,             // B[K N]^T: binary weight
        basis_mask_ptr,
        basis_nnz_ptr,
        basis_weight_ptr,           // basis_weight
        temp_ptr);                  // C:[M N]

#if DEBUG_PROFILER
    auto t4 = std::chrono::high_resolution_clock::now();
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
        out_x_nodes * out_y_nodes, // M
        param_.out_maps,           // N
        num_basis_total_,          // K
        1.0f,                      // alpha
        temp_ptr,                  // matrix A
        num_basis_total_,          // lda
        weight_ptr,                // matrix B
        num_basis_total_,          // ldb
        1.0f,                      // beta
        output_ptr,                // matrix C
        param_.out_maps);          // ldc
#else
    ispc::ispc_gemm(
        out_x_nodes * out_y_nodes, // M
        param_.out_maps,           // N
        num_basis_total_,          // K
        temp_ptr,                  // A:  col layout data
        weight_ptr,                // B:  weight
        bias_ptr,                  // C': bias
        output_ptr);               // C:  result
#endif

#if DEBUG_PROFILER
    auto t5 = std::chrono::high_resolution_clock::now();

    double dt_encode  = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1e6;
    double dt_bim2col = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count() / 1e6;
    double dt_bmul    = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count() / 1e6;
    double dt_blas    = std::chrono::duration_cast<std::chrono::microseconds>(t5 - t4).count() / 1e6;

    DebugLog("\n");
    DebugLog("binary encode: %f\n", dt_encode);
    DebugLog("binary im2col: %f\n", dt_bim2col);
    DebugLog("binary mul:    %f [%d %d] x [%d %d]\n", dt_bmul, out_x_nodes * out_y_nodes, col_nodes, col_nodes, num_basis_total_);
    DebugLog("BLAS:          %f [%d %d] x [%d %d]\n", dt_blas, out_x_nodes * out_y_nodes, num_basis_total_, num_basis_total_, param_.out_maps);
#endif

    input_ptr  += input_stride;
    output_ptr += output_stride;
  }

  return output_;
}
