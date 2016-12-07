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
#include <iostream>
#include <random>
#include <array>
#include <algorithm>

#include "demitasse.hpp"
#include "benchmark.h"

// #include <omp.h>

int main(int argc, char** argv) {
  demitasse::ConvParam    conv_param;
  demitasse::PoolingParam max_param;

  // conv1 (1:28x28 -> 32:24x24) -> max pooling(32:24x24 -> 32:8x8)
  conv_param.in_maps  = 1;
  conv_param.out_maps = 32;
  conv_param.ksize    = 5;
  conv_param.stride   = 1;
  auto conv1 = new demitasse::ConvLayer(&conv_param /* 1, 32, 5, 1 */);
  auto act1  = new demitasse::ActivationLayer<demitasse::tanh>();

  max_param.ksize   = 3;
  max_param.stride  = 3;
  max_param.padding = 0;

  auto max1 = new demitasse::PoolingLayer<demitasse::max>(&max_param);

  auto w1   = (demitasse::Blob*)conv1->weight();
  auto b1   = (demitasse::Blob*)conv1->bias();

  w1->setRandamDistribution(0.0, 1.0);
  b1->setRandamDistribution(0.0, 1.0);

  // conv2 (32:8x8 -> 64:6x6) -> max pooling(64:6x6 -> 64:3x3)
  conv_param.in_maps  = 32;
  conv_param.out_maps = 64;
  conv_param.ksize    = 3;
  conv_param.stride   = 1;
  auto conv2 = new demitasse::ConvLayer(&conv_param /*32, 64, 3, 1*/);
  auto act2  = new demitasse::ActivationLayer<demitasse::tanh>();

  max_param.ksize   = 2;
  max_param.stride  = 2;
  max_param.padding = 0;

  auto max2 = new demitasse::PoolingLayer<demitasse::max>(&max_param);

  auto w2   = (demitasse::Blob*)conv2->weight();
  auto b2   = (demitasse::Blob*)conv2->bias();

  w2->setRandamDistribution(0.0, 1.0);
  b2->setRandamDistribution(0.0, 1.0);

  // MLP
  auto l1   = new demitasse::FullyConnectedLayer(64 * 3 * 3, 256);
  auto act3 = new demitasse::ActivationLayer<demitasse::tanh>();
  auto l2   = new demitasse::FullyConnectedLayer(256, 10);
  auto act4 = new demitasse::ActivationLayer<demitasse::softmax>();

  conv1->setLayerName("conv1");
  conv2->setLayerName("conv2");

  auto wl1 = (demitasse::Blob*)l1->weight();
  auto bl1 = (demitasse::Blob*)l1->bias();

  wl1->setRandamDistribution(0.0, 1.0);
  bl1->setRandamDistribution(0.0, 1.0);

  auto wl2 = (demitasse::Blob*)l2->weight();
  auto bl2 = (demitasse::Blob*)l2->bias();

  wl2->setRandamDistribution(0.0, 1.0);
  bl2->setRandamDistribution(0.0, 1.0);

  // construct layers at model initialization
  demitasse::Model* model = new demitasse::Model({
    conv1, act1, max1, conv2, act2, max2, l1, act3, l2, act4
  });

  demitasse::MNIST *data = new demitasse::MNIST("../data/mnist");

  int batch_size     = 1;

  demitasse::Blob* d     = data->get_batch_train_images(0, batch_size);

/*
        // add ch dimensions
        // demitasse::Blob *input = demitasse::Blob::wrapBlob(d->rawData(), 1, d->extent(0), d->extent(1), batch_size);
        demitasse::Blob *input = new demitasse::Blob(1, d->extent(0), d->extent(1), batch_size);
        float *p = d->rawData().RealSingle;
        float *x = input->rawData().RealSingle;
        memcpy(x, p, d->dataSize());
 */

  auto t = benchmark_median("./cnn_bench_halide.txt", 3, [&] () {
    const demitasse::Blob* out = model->feedForward(d);
  });

  printf("times: %f\n", t);

  return 0;
}
