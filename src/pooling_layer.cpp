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

#include "pooling_param.hpp"
#include "pooling_function.hpp"
#include "pooling_layer.hpp"

using demitasse::PoolingLayer;
using demitasse::PoolingParam;
using demitasse::Blob;

template <typename T>
PoolingLayer<T>::PoolingLayer(const PoolingParam* param) {
  param_.ksize   = param->ksize;
  param_.stride  = param->stride;
  param_.padding = param->padding;
  output_ = nullptr;
}

template <typename T>
PoolingLayer<T>::~PoolingLayer() {
  delete output_;
}

template <typename T>
const std::string PoolingLayer<T>::layerTypeName() {
  return std::string("pooling <") + pooling_.name() + std::string(">");
}

template <typename T>
void PoolingLayer<T>::reshapeOutputBlob(const Blob* input) {
  unsigned int in_maps     = input->extent(0);
  unsigned int in_x_nodes  = input->extent(1);
  unsigned int in_y_nodes  = input->extent(2);
  unsigned int num_batch   = input->extent(3);

  unsigned int out_x_nodes = param_.out_x_nodes(in_x_nodes);
  unsigned int out_y_nodes = param_.out_y_nodes(in_y_nodes);

  if (output_ == nullptr || output_->extent(0) != in_maps || output_->extent(1) != out_x_nodes || output_->extent(2) != out_y_nodes || output_->extent(3) != num_batch) {
    delete output_;
    output_ = new Blob(Blob::BlobType::Float, in_maps, out_x_nodes, out_y_nodes, num_batch);
  }
}

template <typename T>
const Blob * PoolingLayer<T>::feedForward(const Blob* input) {
  this->reshapeOutputBlob(input);

  pooling_.activation_f(&param_, input, output_);

  return output_;
}

namespace demitasse {
template class PoolingLayer<avarage>;
template class PoolingLayer<max>;
}
