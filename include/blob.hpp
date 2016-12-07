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
#ifndef __DEMITASSE_BLOB__
#define __DEMITASSE_BLOB__

#include <cstdint>
#include <cstddef>
#include <array>

namespace demitasse {

//
// unit size:
//    float type  : 4 byte    float   []
//    binary type : 1 byte    uint8_t []
// aligned by 128bit (16bytes)
//
union BlobRawData {
  struct {
    float* data;
  } RealSingle;
  struct {
    uint8_t* data;
  } Binary;
  struct {
    uint8_t*  star;
    uint8_t*  mask;
    uint32_t* nnz;      // num of non zero bits
  } Ternary;
};


class Blob {
public:
  enum class BlobType {
    Float   = 0,  // real (single)
    Binary  = 1,  // binary
    Ternary = 2   // ternary
  };

  Blob(BlobType type, unsigned int ext0,
      unsigned int ext1,
      unsigned int ext2,
      unsigned int ext3);                  // 4D
  Blob(BlobType type, unsigned int ext0,
      unsigned int ext1,
      unsigned int ext2);                  // 3D
  Blob(BlobType type, unsigned int ext0,
      unsigned int ext1);                  // 2D
  Blob(BlobType type, unsigned int ext0);  // 1D

  ~Blob();

  Blob* createSameExtents() const;
  Blob* copy() const;

  BlobRawData& rawData() const;

  unsigned int dimensions() const;
  unsigned int extent(unsigned int dim) const;
  unsigned int stride(unsigned int dim) const;

  unsigned int numOfData() const;
  size_t rawDataBytes() const;

  float getValue(unsigned int idx0, unsigned int idx1 = 0, unsigned int idx2 = 0, unsigned int idx3 = 0) const;
  void  setValue(float value, unsigned int idx0, unsigned int idx1 = 0, unsigned int idx2 = 0, unsigned int idx3 = 0) const;

  bool isEqualExtents(const Blob* base);

  Blob* copyAt(int index, unsigned int num = 1);

  BlobType blobType() const;

  void setRandamDistribution(float min, float max);

private:
  Blob(BlobType type, unsigned int dims, std::array<unsigned int, 4>& extent);
  void setStrides();
  void allocRawMemory();

  size_t numOfNNZ() const;

  BlobType     type_;
  bool         is_allocated_;
  BlobRawData  raw_data_;

  unsigned int dims_;

  std::array<uint32_t, 4> extent_;
  std::array<uint32_t, 4> stride_;
};

}

#endif /* __DEMITASSE_BLOB__ */
