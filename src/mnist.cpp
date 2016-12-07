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
#include <netinet/in.h>

#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <random>

#include "mnist.hpp"

static const char* TRAIN_IMAGE_FILE = "train-images.idx3-ubyte";
static const char* TRAIN_LABEL_FILE = "train-labels.idx1-ubyte";

static const char* TEST_IMAGE_FILE  = "t10k-images.idx3-ubyte";
static const char* TEST_LABEL_FILE  = "t10k-labels.idx1-ubyte";

using demitasse::MNIST;
using demitasse::Blob;

std::string MNIST::get_file_path(const std::string& dir_path, const std::string& file) {

  std::string path = dir_path;

  size_t      pos  = path.rfind('/');
  if (pos == std::string::npos || pos < path.size() - 1) {
    path += "/";
  }

  path += file;

  return path;
}

//
// MNIST image data loader
//
Blob * MNIST::load_image_data(const std::string& dir_path, const std::string& file) {

  std::string   path = get_file_path(dir_path, file);

  std::ifstream fs(path, std::ios_base::in | std::ios_base::binary);

  // assert error
  // assert(fs);

  // read file header
  int32_t magic = 0;
  int32_t size  = 0;
  int32_t row   = 0;
  int32_t col   = 0;

  fs.read((char*)&magic,   sizeof(int32_t));
  fs.read((char*)&size,   sizeof(int32_t));
  fs.read((char*)&row,     sizeof(int32_t));
  fs.read((char*)&col,     sizeof(int32_t));

  magic = ntohl(magic);
  size  = ntohl(size);
  row   = ntohl(row);
  col   = ntohl(col);

  // check  file format
  // assert(magic == 2051);

  // read data
  // TODO: should use memory mapped file to access chunk of data (specified by index range) from whole data set
  int     data_size = row * col;
  uint8_t buff[data_size];

  Blob*   data = new Blob(Blob::BlobType::Float, 1 /* ch */, row, col, size);
  float*  p    = data->rawData().RealSingle.data;

  for (int i = 0; i < size; ++i) {
    fs.read((char *)buff, sizeof(uint8_t) * data_size);

    // convert uint8_t (0 ~ 255) to float (0.0 ~ 1.0)
    for (int n = 0; n < data_size; ++n) {
      uint8_t v = buff[n];
      *p = static_cast<float>(v) / 255.0f;
      p++;
    }
  }

  return data;
}


Blob * MNIST::load_label_data(const std::string& dir_path, const std::string& file) {
  std::string   path = get_file_path(dir_path, file);

  std::ifstream fs(path, std::ios_base::in | std::ios_base::binary);

  // assert error
  // assert(fs);

  // read file header
  int32_t magic = 0;
  int32_t size  = 0;

  fs.read((char*)&magic,   sizeof(int32_t));
  fs.read((char*)&size,   sizeof(int32_t));

  magic = ntohl(magic);
  size  = ntohl(size);

  // check  file format
  // assert(magic == 2049);

  Blob*  data = new Blob(Blob::BlobType::Float, 10, size);
  float* p    = (float *)data->rawData().RealSingle.data;

  for (int i = 0; i < size; ++i) {
    uint8_t v;
    fs.read((char *)&v, sizeof(uint8_t));

    for (int n = 0; n < 10; ++n) {
      if (v == n) {
        *p = 1.0f;
      } else {
        *p = 0.0f;
      }
      p++;
    }
  }

  return data;
}

MNIST::MNIST(const char* dir_path) {
  train_size    = 0;
  varidate_size = 0;
  test_size     = 0;

  train_images  = this->load_image_data(dir_path, TRAIN_IMAGE_FILE);
  train_labels  = this->load_label_data(dir_path, TRAIN_LABEL_FILE);
  test_images   = this->load_image_data(dir_path, TEST_IMAGE_FILE);
  test_labels   = this->load_label_data(dir_path, TEST_LABEL_FILE);

  train_size    = train_images->extent(3) - 5000;
  varidate_size = 5000;
  test_size     = test_images->extent(3);

  train_idx     = std::vector<int>(55000);
  std::iota(train_idx.begin(), train_idx.end(), 0);
}

MNIST::~MNIST() {
  delete train_images;
  delete train_labels;
  delete test_images;
  delete test_labels;
}

void MNIST::shuffle_train_data() {
  std::mt19937 mt(0);

  std::shuffle(train_idx.begin(), train_idx.end(), mt);
}


Blob * MNIST::get_train_image(int n) {
  Blob* v = train_images->copyAt(train_idx[n]);

  return v;
}


Blob* MNIST::get_train_label(int n) {
  Blob* v = train_labels->copyAt(train_idx[n]);

  return v;
}


Blob* MNIST::get_varidate_images() {
  Blob* v = train_images->copyAt(train_size, varidate_size);

  return v;
}


Blob* MNIST::get_varidate_labels() {
  Blob* v = train_labels->copyAt(train_size, varidate_size);

  return v;
}


Blob* MNIST::get_batch_train_labels(int idx, int num) {
  Blob*  v    = new Blob(Blob::BlobType::Float, train_labels->extent(0), num);
  float* p    = v->rawData().RealSingle.data;

  int    size = train_labels->extent(0);

  for (int n = 0; n < num; ++n) {
    float* src = train_labels->rawData().RealSingle.data;
    src += size * train_idx[idx + n];

    memcpy((char*)p, (char*)src, size * sizeof(float));

    p += size;
  }

  return v;
}


Blob* MNIST::get_batch_train_images(int idx, int num) {
  Blob*  v    = new Blob(Blob::BlobType::Float, train_images->extent(0), train_images->extent(1), train_images->extent(2), num);

  float *p    = v->rawData().RealSingle.data;
  int    size = train_images->extent(0) * train_images->extent(1) * train_images->extent(2);
  for (int n = 0; n < num; ++n) {
    float *src = train_images->rawData().RealSingle.data;
    src += size * train_idx[idx + n];

    memcpy((char*)p, (char*)src, size * sizeof(float));

    p += size;
  }

  return v;
}


Blob* MNIST::get_test_images(int idx, int num) {
  Blob* v = test_images->copyAt(idx, num);

  return v;
}


Blob* MNIST::get_test_labels(int idx, int num) {
  Blob *v = test_labels->copyAt(idx, num);

  return v;
}

// DEBUG:
// export map to gray scale (8 bit) image with normalization.
void MNIST::export_image_to_file(int n, const char* filename) {
  Blob*  v    = this->get_train_image(n);
  int    size = v->extent(0) * v->extent(1) * v->extent(2);

  float* p    = v->rawData().RealSingle.data;

  // convert to 8 bit gray scale
  uint8_t* img = static_cast<uint8_t*>(malloc(sizeof(uint8_t) * size));

  for (int i = 0; i < size; ++i) {
    img[i] = (uint8_t)(*p * 255);
  }

  // Halide::Tools::save_image(image, filename);

  free(img);
}


int MNIST::train_label(int n) {
  for (int i = 0; i < 10; ++i) {
    float v = train_labels->getValue(i, n);
    if (v != 0) {
      return i;
    }
  }

  return -1;
}
