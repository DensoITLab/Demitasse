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

#include <array>
#include <iostream>
#include <random>

#include "blob.hpp"

using demitasse::BlobRawData;
using demitasse::Blob;

Blob::Blob(BlobType type, unsigned int dims, std::array<unsigned int, 4>& extent) {
  type_ = type;
  dims_ = dims;
  for (int i = 0; i < 4; ++i) {
    extent_[i] = extent[i];
  }

  this->setStrides();
  this->allocRawMemory();
}

Blob::Blob(BlobType type, unsigned int ext0, unsigned int ext1, unsigned int ext2, unsigned int ext3) {
  type_      = type;
  dims_      = 4;
  extent_[0] = ext0;
  extent_[1] = ext1;
  extent_[2] = ext2;
  extent_[3] = ext3;

  this->setStrides();
  this->allocRawMemory();
}

Blob::Blob(BlobType type, unsigned int ext0, unsigned int ext1, unsigned int ext2) {
  type_      = type;
  dims_      = 3;
  extent_[0] = ext0;
  extent_[1] = ext1;
  extent_[2] = ext2;
  extent_[3] = 1;

  this->setStrides();
  this->allocRawMemory();
}

Blob::Blob(BlobType type, unsigned int ext0, unsigned int ext1) {
  type_      = type;
  dims_      = 2;
  extent_[0] = ext0;
  extent_[1] = ext1;
  extent_[2] = 1;
  extent_[3] = 1;

  this->setStrides();
  this->allocRawMemory();
}

Blob::Blob(BlobType type, unsigned int ext0) {
  type_      = type;
  dims_      = 1;
  extent_[0] = ext0;
  extent_[1] = 1;
  extent_[2] = 1;
  extent_[3] = 1;

  this->setStrides();
  this->allocRawMemory();
}

void Blob::setStrides() {
  for (int i = 0; i < 4; ++i) {
    stride_[i] = 0;
  }

  if (dims_ > 0) {
    // FIXME: if binary extent is smaller than 8, we need to add padding.
    stride_[0] = (type_ == BlobType::Float ? extent_[0] : extent_[0] / 8);
  }

  if (dims_ > 1) {
    stride_[1] = stride_[0] * extent_[1];
  }

  if (dims_ > 2) {
    stride_[2] = stride_[1] * extent_[2];
    if (stride_[2] % 8 > 0) {
      // 32 byte (8 float) align for num of batch (4D)
      stride_[2] = ((stride_[2] / 8) + 1) * 8;
    }
  }

  if (dims_ > 3) {
    stride_[3] = stride_[2] * extent_[3];
  }
}

void Blob::allocRawMemory() {

  is_allocated_ = true;

  if (type_ == Blob::BlobType::Float) {
    size_t bytes = this->rawDataBytes();

    if (posix_memalign(reinterpret_cast<void**>(&raw_data_.RealSingle.data), (size_t)32, bytes) != 0) {
      abort(); // error
    }
    size_t num_data = this->numOfData();
    for (int i = 0; i < num_data; ++i) {
      raw_data_.RealSingle.data[i] = 0.0f;
    }
  } else if (type_ == Blob::BlobType::Binary) {
    size_t bytes = this->rawDataBytes();

    if (posix_memalign(reinterpret_cast<void**>(&raw_data_.Binary.data), (size_t)32, bytes) != 0) {
      abort(); // error
    }
    memset(raw_data_.Binary.data, 0, bytes);
  } else if (type_ == Blob::BlobType::Ternary) {
    size_t bytes   = this->rawDataBytes();
    size_t num_nnz = this->numOfNNZ();

    if (posix_memalign(reinterpret_cast<void**>(&raw_data_.Ternary.star), (size_t)32, bytes) != 0) {
      abort(); // error
    }
    if (posix_memalign(reinterpret_cast<void**>(&raw_data_.Ternary.mask), (size_t)32, bytes) != 0) {
      abort(); // error
    }
    if (posix_memalign(reinterpret_cast<void**>(&raw_data_.Ternary.nnz), (size_t)32,
            num_nnz * sizeof(int)) != 0) {
      abort(); // error
    }
    memset(raw_data_.Ternary.star, 0, bytes);
    memset(raw_data_.Ternary.mask, 0, bytes);
    memset(raw_data_.Ternary.nnz,  0, num_nnz * sizeof(int));
  }
}

Blob* Blob::createSameExtents() const {
  std::array<unsigned int, 4> extent = { extent_[0], extent_[1], extent_[2], extent_[3]};
  auto new_blob = new Blob(type_, dims_, extent);

  return new_blob;
}

Blob* Blob::copy() const {
  std::array<unsigned int, 4> extent = { extent_[0], extent_[1], extent_[2], extent_[3]};
  auto   new_blob       = new Blob(type_, dims_, extent);

  size_t raw_data_bytes = this->rawDataBytes();
  if (type_ == Blob::BlobType::Float) {
    memcpy(new_blob->raw_data_.RealSingle.data,
        raw_data_.RealSingle.data,
        raw_data_bytes);
  } else if (type_ == Blob::BlobType::Binary) {
    memcpy(new_blob->raw_data_.Binary.data,  raw_data_.Binary.data, raw_data_bytes);
  } else if (type_ == Blob::BlobType::Ternary) {
    memcpy(new_blob->raw_data_.Ternary.star, raw_data_.Ternary.star, raw_data_bytes);
    memcpy(new_blob->raw_data_.Ternary.mask, raw_data_.Ternary.mask, raw_data_bytes);
    memcpy(new_blob->raw_data_.Ternary.nnz,  raw_data_.Ternary.nnz,  this->numOfNNZ());
  }
  return new_blob;
}

Blob::BlobType Blob::blobType() const {
  return this->type_;
}

Blob::~Blob() {
  if (is_allocated_) {
    if (type_ == Blob::BlobType::Float) {
      free(raw_data_.RealSingle.data);
    } else if (type_ == Blob::BlobType::Binary) {
      free(raw_data_.Binary.data);
    } else if (type_ == Blob::BlobType::Ternary) {
      free(raw_data_.Ternary.star);
      free(raw_data_.Ternary.mask);
      free(raw_data_.Ternary.nnz);
    }
  }
}

BlobRawData& Blob::rawData() const {
  return (BlobRawData&)raw_data_;
}

unsigned int Blob::dimensions() const {
  return dims_;
}

unsigned int Blob::extent(unsigned int dim) const {
  if (dim > 3) {
    return 0;
  }
  return extent_[dim];
}

unsigned int Blob::stride(unsigned int dim) const {
  if (dim > 3) {
    return 0;
  }
  return stride_[dim];
}

size_t Blob::rawDataBytes() const {
  size_t raw_data_bytes = 0;
  if (dims_ > 0) {
    if (type_ == Blob::BlobType::Float) {
      raw_data_bytes = stride_[dims_ - 1] * sizeof(float);
    } else if (type_ == Blob::BlobType::Binary) {
      raw_data_bytes = stride_[dims_ - 1] * sizeof(uint8_t);
    } else if (type_ == Blob::BlobType::Ternary) {
      raw_data_bytes = stride_[dims_ - 1] * sizeof(uint8_t);
    }
  }
  return raw_data_bytes;
}

unsigned int Blob::numOfData() const {
  unsigned int num_of_data = 0;
  if (dims_ > 0) {
    if (type_ == Blob::BlobType::Float) {
      num_of_data = stride_[dims_ - 1];
    } else if (type_ == Blob::BlobType::Binary) {
      num_of_data = stride_[dims_ - 1] * sizeof(uint8_t);
    } else if (type_ == Blob::BlobType::Ternary) {
      num_of_data = stride_[dims_ - 1] * sizeof(uint8_t);
    }
  }
  return num_of_data;
}

size_t Blob::numOfNNZ() const {
  size_t num_of_nnz = 1;
  for (int d = 1; d < dims_; ++d) {
    num_of_nnz *= extent_[d];
  }
  return num_of_nnz;
}

float Blob::getValue(
    unsigned int idx0,
    unsigned int idx1,
    unsigned int idx2,
    unsigned int idx3) const {
  // Float
  int offset = (idx3 * stride_[2]) + (idx2 * stride_[1]) + (idx1 * stride_[0]) + idx0;
  return raw_data_.RealSingle.data[offset];
}

void Blob::setValue(
    float value,
    unsigned int idx0,
    unsigned int idx1,
    unsigned int idx2,
    unsigned int idx3) const {
  // Float
  int offset = (idx3 * stride_[2]) + (idx2 * stride_[1]) + (idx1 * stride_[0]) + idx0;
  raw_data_.RealSingle.data[offset] = value;
}

bool Blob::isEqualExtents(const Blob* base) {
  if (base == nullptr) {
    return false;
  }

  if (dims_ != base->dims_) {
    return false;
  }

  for (int d = 0; d < dims_; ++d) {
    if (extent_[d] != base->extent_[d]) {
      return false;
    }
  }

  return true;
}

Blob* Blob::copyAt(int index, unsigned int num) {
  if (dims_ < 4 || index >= extent_[3] || num > extent_[3] - index) {
    return nullptr; // error
  }

  std::array<unsigned int, 4> extent;

  for (int i = 0; i < 3; ++i) {
    extent[i] = extent_[i];
  }
  extent[3] = num;

  Blob* new_blob = new Blob(type_, dims_, extent);
  if (type_ == Blob::BlobType::Float) {
    float* p = raw_data_.RealSingle.data + (stride_[2] * index);
    memcpy(new_blob->raw_data_.RealSingle.data, p, stride_[2] * num * sizeof(float));
  }
  // FIXME: binary, ternary

  return new_blob;
}

void Blob::setRandamDistribution(float min, float max) {
  if (type_ != Blob::BlobType::Float) {
    // binary/ternary type is not supported
    return;
  }

  std::mt19937 mt(0);
  std::uniform_real_distribution<float> rnd(min, max);

  float* data_ptr = raw_data_.RealSingle.data;
  size_t num_data = this->numOfData();
  for (int i = 0; i < num_data; ++i) {
    *data_ptr = rnd(mt);
    ++data_ptr;
  }
}
