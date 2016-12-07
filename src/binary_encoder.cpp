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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ispc func
#include "ispc_binary_encode.hpp"

#include "binary_encoder.hpp"

using demitasse::BinaryEncoder;
using demitasse::Blob;

BinaryEncoder::BinaryEncoder(
    float coff1,
    float coff2,
    int num_basis,
    int table_size,
    const uint32_t* table) {
  coff1_      = coff1;
  coff2_      = coff2;
  num_basis_  = num_basis;
  table_size_ = table_size;

  if (posix_memalign((void **)&table_, (size_t)32, sizeof(uint32_t) * table_size * num_basis) != 0) {
    abort(); // error
  }
  memcpy(table_, table, sizeof(uint32_t) * table_size * num_basis);
};

BinaryEncoder::~BinaryEncoder() {
  free(table_);
}

void BinaryEncoder::encode_map(Blob* output, const Blob* input) {
  float*   input_ptr  = input->rawData().RealSingle.data;
  uint8_t* output_ptr = output->rawData().Binary.data;

  int      in_dims    = input->dimensions();
  int      num_batch  = input->extent(in_dims - 1);
  int      data_size  = 1;

  for (int d = 0; d < in_dims; ++d) {
    data_size *= input->extent(d);
  }
  // printf("encoder data_size:%d, num_batch:%d\n", data_size, num_batch);

  const int in_stride  = input->stride(in_dims - 1 - 1); // input  data stride for batch
  const int out_stride = output->stride(1);              // output data stride for batch

  // loop for batch input
  for (int n = 0; n < num_batch; ++n) {
    ispc::ispc_binary_encode(
        output_ptr,
        input_ptr,
        data_size,
        coff1_,
        coff2_,
        num_basis_,
        table_size_,
        table_);
    input_ptr  += in_stride;
    output_ptr += out_stride;
  }
}

void BinaryEncoder::encode_map(Blob* output, const Blob* input, unsigned int index) {

  float*   input_ptr  = input->rawData().RealSingle.data;
  uint8_t* output_ptr = output->rawData().Binary.data;
  int      in_dims    = input->dimensions();
  int      num_batch  = input->extent(in_dims - 1);

  const int in_stride  = input->stride(in_dims - 1 - 1); // input  data stride for batch

  input_ptr += in_stride * index;

  ispc::ispc_binary_encode(
      output_ptr,
      input_ptr,
      in_stride,
      coff1_,
      coff2_,
      num_basis_,
      table_size_,
      table_);
}
