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

#include <string>

#include "ispc_identity.hpp"
#include "ispc_relu.hpp"
#include "ispc_sigmoid.hpp"
#include "ispc_softmax.hpp"
#include "ispc_tanh.hpp"

#include "blob.hpp"
#include "activation_function.hpp"

using demitasse::Blob;
using demitasse::identity;

void identity::activation_f(const Blob* input, Blob* output) {
  float*       input_ptr  = input->rawData().RealSingle.data;
  float*       output_ptr = output->rawData().RealSingle.data;
  unsigned int num_data   = input->numOfData();

  ispc::ispc_identity(num_data, input_ptr, output_ptr);
}

void identity::activation_df(const Blob* input, Blob* output) {
  float*       input_ptr  = input->rawData().RealSingle.data;
  float*       output_ptr = output->rawData().RealSingle.data;
  unsigned int num_data   = input->numOfData();

  ispc::ispc_identity_df(num_data, input_ptr, output_ptr);
}

using demitasse::sigmoid;

void sigmoid::activation_f(const Blob* input, Blob* output) {
  float*       input_ptr  = input->rawData().RealSingle.data;
  float*       output_ptr = output->rawData().RealSingle.data;
  unsigned int num_data   = input->numOfData();

  ispc::ispc_sigmoid(num_data, input_ptr, output_ptr);
}

void sigmoid::activation_df(const Blob* input, Blob* output) {
  float*       input_ptr  = input->rawData().RealSingle.data;
  float*       output_ptr = output->rawData().RealSingle.data;
  unsigned int num_data   = input->numOfData();

  ispc::ispc_sigmoid_df(num_data, input_ptr, output_ptr);
}

using demitasse::tanh;

void tanh::activation_f(const Blob* input, Blob* output) {
  float*       input_ptr  = input->rawData().RealSingle.data;
  float*       output_ptr = output->rawData().RealSingle.data;
  unsigned int num_data   = input->numOfData();

  ispc::ispc_tanh(num_data, input_ptr, output_ptr);
}

void tanh::activation_df(const Blob* input, Blob* output) {
  float*       input_ptr  = input->rawData().RealSingle.data;
  float*       output_ptr = output->rawData().RealSingle.data;
  unsigned int num_data   = input->numOfData();

  ispc::ispc_tanh_df(num_data, input_ptr, output_ptr);
}

using demitasse::ReLU;

void ReLU::activation_f(const Blob* input, Blob* output) {
  float*       input_ptr  = input->rawData().RealSingle.data;
  float*       output_ptr = output->rawData().RealSingle.data;
  unsigned int num_data   = input->numOfData();

  ispc::ispc_relu(num_data, input_ptr, output_ptr);
}

void ReLU::activation_df(const Blob* input, Blob* output) {
  float*       input_ptr  = input->rawData().RealSingle.data;
  float*       output_ptr = output->rawData().RealSingle.data;
  unsigned int num_data   = input->numOfData();

  ispc::ispc_relu_df(num_data, input_ptr, output_ptr);
}

using demitasse::softmax;

void softmax::activation_f(const Blob* input, Blob* output) {
  float*       input_ptr  = input->rawData().RealSingle.data;
  float*       output_ptr = output->rawData().RealSingle.data;
  unsigned int num_data   = input->numOfData();

  ispc::ispc_softmax(num_data, input_ptr, output_ptr);
}

void softmax::activation_df(const Blob* input, Blob* output) {
  float*       input_ptr  = input->rawData().RealSingle.data;
  float*       output_ptr = output->rawData().RealSingle.data;
  unsigned int num_data   = input->numOfData();

  ispc::ispc_softmax_df(num_data, input_ptr, output_ptr);
}
