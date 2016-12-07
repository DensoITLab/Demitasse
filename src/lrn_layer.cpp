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

#include "ispc_lrn.hpp"
#include "lrn_layer.hpp"

using demitasse::LRNLayer;
using demitasse::Blob;

LRNLayer::LRNLayer(const LRNParam *param) {
  param_.mode       = param->mode;
  param_.local_size = param->local_size;
  param_.k          = param->k;
  param_.alpha      = param->alpha;
  param_.beta       = param->beta;

  output_ = nullptr;
}


LRNLayer::~LRNLayer() {
  delete output_;
}

const std::string LRNLayer::layerTypeName() {
  if (param_.mode == 0) {
    return "lrn <across_channel>";
  } else {
    return "lrn <within_channel>";
  }
}

void LRNLayer::reshapeOutputBlob(const Blob* input) {
  if (output_ == nullptr || !output_->isEqualExtents(input)) {
    delete output_;
    output_ = input->createSameExtents();
  }
}

const Blob * LRNLayer::feedForward(const Blob* input) {
  this->reshapeOutputBlob(input);

  int in_maps   = input->extent(0);
  int in_node_x = input->extent(1);
  int in_node_y = input->extent(2);
  int num_batch = input->extent(3);

  const float* input_ptr     = input->rawData().RealSingle.data;
  float*       output_ptr    = output_->rawData().RealSingle.data;
  const int    input_stride  = input->stride(2);
  const int    output_stride = output_->stride(2);

  for (int n = 0; n < num_batch; ++n) {
    ispc::ispc_lrn(in_node_x, in_node_y, in_maps, param_.local_size, param_.k, param_.alpha, param_.beta, input_ptr, output_ptr);
    input_ptr  += input_stride;
    output_ptr += output_stride;
  }

  return output_;
}
