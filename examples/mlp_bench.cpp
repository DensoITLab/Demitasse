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

int main(int argc, char** argv) {
  auto l1   = new demitasse::FullyConnectedLayer(784, 256);
  auto act1 = new demitasse::ActivationLayer<demitasse::tanh>();
  auto l2   = new demitasse::FullyConnectedLayer(256, 10);
  auto act2 = new demitasse::ActivationLayer<demitasse::softmax>();

  auto wl1  = (demitasse::Blob*)l1->weight();
  auto bl1  = (demitasse::Blob*)l1->bias();

  wl1->setRandamDistribution(0.0, 1.0);
  bl1->setRandamDistribution(0.0, 1.0);

  auto wl2 = (demitasse::Blob*)l2->weight();
  auto bl2 = (demitasse::Blob*)l2->bias();

  wl2->setRandamDistribution(0.0, 1.0);
  bl2->setRandamDistribution(0.0, 1.0);

  // construct layers at model initialization
  demitasse::Model* model = new demitasse::Model({
    l1, act1, l2, act2
  });

  demitasse::MNIST *data  = new demitasse::MNIST("../data/mnist");

  int batch_size      = 1;

  demitasse::Blob* d      = data->get_batch_train_images(0, batch_size);
  demitasse::Blob* result = nullptr;
  auto t = benchmark_median("./mlp_bench_ispc.txt", 1000, [&] () {
    model->feedForward(d);
  });

  printf("times: %f\n", t);

  return 0;
}
