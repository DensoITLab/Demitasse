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
#include <fstream>
#include <iostream>
#include <random>
#include <algorithm>
#include <map>

#include "demitasse.hpp"
#include "benchmark.h"

#include "vgg_util.h"

/*
                load synset word label
 */
std::vector<std::string>* load_synset(const std::string& filename) {
  std::ifstream ifs(filename);

  if (ifs.fail()) {
    return nullptr;
  }

  auto labels = new std::vector<std::string>();
  std::string str;

  while (getline(ifs, str)) {
    auto label = str.substr(10);
    labels->push_back(label);
  }

  return labels;
}

/*
                show top 5 prediction resluts with synset word label
 */
void show_top5(const demitasse::Blob* result, std::vector<std::string>& labels) {
  int dims = result->dimensions();
  int num  = 1;

  for (int i = 0; i < dims; ++i) {
    num *= result->extent(i);
  }

  // sort result
  std::vector<std::pair<int, float> > pairs(num);
  float* values = result->rawData().RealSingle.data;

  for (int i = 0; i < num; ++i) {
    pairs[i] = std::make_pair(i, values[i]);
  }

  std::sort(pairs.begin(), pairs.end(), [] (const std::pair<int, float> &left, const std::pair<int, float> &right) {
    return left.second > right.second;
  });

  for (int i = 0; i < 5; ++i) {
    fprintf(stderr, "%.2d : [%.4d] %s (%f)\n", i, pairs[i].first, labels[pairs[i].first].c_str(), pairs[i].second);
  }
}


/*
                main
 */
int main(int argc, char** argv) {
  if (argc != 2) {
    printf("usage : vgg_bench <model_file>\n");
    return -1;
  }

  const char* model_file = argv[1];       // "../data/vgg_cnn_f/vudnn.model"

  auto labels = load_synset("../../Demitasse/data/ilsvrc/synset_words.txt");

  demitasse::Model* model = new demitasse::Model();
  model->load(model_file);

  auto input01 = load_preprocessed_image("../../Demitasse/data/ilsvrc/sample.png");
  fprintf(stderr, "preprocessed img01\n");

/*
        auto input02 = load_preprocessed_image("../data/images/sample02.png");
        printf("preprocessed img02\n");

        demitasse::Blob *out01 = model->feedForward(input01);

        show_top5(out01, *labels);

        const demitasse::Blob *out02 = nullptr;
 */
  const demitasse::Blob *out01 = nullptr;

  // auto t = benchmark_median("./vgg_bench_ispc.txt", 10, [&] () {
    out01 = model->feedForward(input01);
  // });

  // printf("times: %f\n", t);

  show_top5(out01, *labels);

  return 0;
}
