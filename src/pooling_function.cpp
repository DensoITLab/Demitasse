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

#include "ispc_avg2d.hpp"
#include "ispc_max2d.hpp"

#include "blob.hpp"
#include "pooling_param.hpp"
#include "pooling_function.hpp"

using demitasse::avarage;
using demitasse::max;

void avarage::activation_f(
    const PoolingParam* param,
    const Blob* input,
    Blob* output) {
  unsigned int in_maps     = input->extent(0);
  unsigned int in_x_nodes  = input->extent(1);
  unsigned int in_y_nodes  = input->extent(2);
  unsigned int num_batch   = input->extent(3);
  unsigned int out_x_nodes = param->out_x_nodes(in_x_nodes);
  unsigned int out_y_nodes = param->out_y_nodes(in_y_nodes);
  const float* input_ptr   = input->rawData().RealSingle.data;
  float*       output_ptr  = output->rawData().RealSingle.data;
  const int    in_stride   = input->stride(2);
  const int    out_stride  = output->stride(2);

  for (unsigned int n = 0; n < num_batch; ++n) {
    ispc::ispc_avg2d(in_x_nodes, in_y_nodes, in_maps,
        param->ksize, param->stride, param->padding,
        out_x_nodes, out_y_nodes, input_ptr, output_ptr);
    input_ptr  += in_stride;
    output_ptr += out_stride;
  }
}

void max::activation_f(
    const PoolingParam* param,
    const Blob* input,
    Blob* output) {
  unsigned int in_maps     = input->extent(0);
  unsigned int in_x_nodes  = input->extent(1);
  unsigned int in_y_nodes  = input->extent(2);
  unsigned int num_batch   = input->extent(3);
  unsigned int out_x_nodes = param->out_x_nodes(in_x_nodes);
  unsigned int out_y_nodes = param->out_y_nodes(in_y_nodes);
  const float* input_ptr   = input->rawData().RealSingle.data;
  float*       output_ptr  = output->rawData().RealSingle.data;
  const int    in_stride   = input->stride(2);
  const int    out_stride  = output->stride(2);

  for (unsigned int n = 0; n < num_batch; ++n) {
    ispc::ispc_max2d(in_x_nodes, in_y_nodes, in_maps,
        param->ksize, param->stride, param->padding,
        out_x_nodes, out_y_nodes, input_ptr, output_ptr);
    input_ptr  += in_stride;
    output_ptr += out_stride;
  }
}
