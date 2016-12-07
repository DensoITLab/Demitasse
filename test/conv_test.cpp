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
#include <gtest/gtest.h>
#include "demitasse.hpp"

demitasse::Blob* init_input_data() {
  /*  [input data: 9 x 9]
                  0 3 0 2 4 9 1 0 3
                  1 5 7 2 3 6 2 1 5
                  0 4 8 1 5 0 2 0 8
                  2 1 2 0 7 4 0 0 2
                  3 1 5 1 1 0 9 3 7
                  1 6 2 3 6 2 1 0 6
                  0 4 0 7 0 2 5 8 4
                  7 7 2 5 0 4 3 1 9
                  3 1 0 6 2 1 3 5 4
   */

  float d[9][9] = {
    { 0.0f, 3.0f, 0.0f, 2.0f, 4.0f, 9.0f, 1.0f, 0.0f, 3.0f },
    { 1.0f, 5.0f, 7.0f, 2.0f, 3.0f, 6.0f, 2.0f, 1.0f, 5.0f },
    { 0.0f, 4.0f, 8.0f, 1.0f, 5.0f, 0.0f, 2.0f, 0.0f, 8.0f },
    { 2.0f, 1.0f, 2.0f, 0.0f, 7.0f, 4.0f, 0.0f, 0.0f, 2.0f },
    { 3.0f, 1.0f, 5.0f, 1.0f, 1.0f, 0.0f, 9.0f, 3.0f, 7.0f },
    { 1.0f, 6.0f, 2.0f, 3.0f, 6.0f, 2.0f, 1.0f, 0.0f, 6.0f },
    { 0.0f, 4.0f, 0.0f, 7.0f, 0.0f, 2.0f, 5.0f, 8.0f, 4.0f },
    { 7.0f, 7.0f, 2.0f, 5.0f, 0.0f, 4.0f, 3.0f, 1.0f, 9.0f },
    { 3.0f, 1.0f, 0.0f, 6.0f, 2.0f, 1.0f, 3.0f, 5.0f, 4.0f },
  };

  // prepare input data
  auto data = new demitasse::Blob(demitasse::Blob::BlobType::Float, 1, 9, 9, 1);

  for (int j = 0; j < 9; ++j) {
    for (int i = 0; i < 9; ++i) {
      data->setValue(d[j][i], 0, i, j, 0);
    }
  }

  return data;
}

demitasse::Blob* init_filters(int n) {
  // init 3 x 3 conv filter
  auto filters = new demitasse::Blob(demitasse::Blob::BlobType::Float, 1, 3, 3, n);

  float d[3][3] = {
    { 0.0f,  1.0f, 0.0f },
    { 1.0f, -4.0f, 1.0f },
    { 0.0f,  1.0f, 0.0f },
  };

  for (int j = 0; j < 3; ++j) {
    for (int i = 0; i < 3; ++i) {
      for (int f = 0; f < n; ++f) {
        filters->setValue(d[j][i], 0, i, j, f);
      }
    }
  }

  return filters;
}

demitasse::Blob* init_bias(int n) {
  auto bias = new demitasse::Blob(demitasse::Blob::BlobType::Float, n);

  for (int i = 0; i < n; ++i) {
    bias->setValue(0.0f, i);
  }

  return bias;
}

bool check_conv_stride_1(const demitasse::Blob* out) {
  /*  [input data: 9 x 9]
                  0 3 0 2 4 9 1 0 3
                  1 5 7 2 3 6 2 1 5
                  0 4 8 1 5 0 2 0 8
                  2 1 2 0 7 4 0 0 2
                  3 1 5 1 1 0 9 3 7
                  1 6 2 3 6 2 1 0 6
                  0 4 0 7 0 2 5 8 4
                  7 7 2 5 0 4 3 1 9
                  3 1 0 6 2 1 3 5 4

                  [output]
                    -5 -13   5   5 -10   2   3
                    -2 -18  11  -9  17  -6  11
                     5   6  11 -18  -9  15   5
                    11 -14   5  10  16 -32   4
                   -16   6   4 -18   1  12  18
                    -3  15 -20  15   3  -6 -22
                   -14   4  -5  11 -10   1  21
   */

  float d[7][7] = {
    { -5.0f, -13.0f,   5.0f,   5.0f, -10.0f,   2.0f,   3.0f },
    { -2.0f, -18.0f,  11.0f, -9.0f,  17.0f, -6.0f,  11.0f },
    {   5.0f,   6.0f,  11.0f, -18.0f, -9.0f,  15.0f,   5.0f },
    {  11.0f, -14.0f,   5.0f,  10.0f,  16.0f, -32.0f,   4.0f },
    { -16.0f,   6.0f,   4.0f, -18.0f,   1.0f,  12.0f,  18.0f },
    { -3.0f,  15.0f, -20.0f,  15.0f,   3.0f, -6.0f, -22.0f },
    { -14.0f,   4.0f, -5.0f,  11.0f, -10.0f,   1.0f,  21.0f },
  };

  for (int j = 0; j < 7; ++j) {
    for (int i = 0; i < 7; ++i) {
      if (d[j][i] != out->getValue(0, i, j, 0)) {
        return false;
      }
    }
  }

  return true;
}

bool check_conv_stride_2(const demitasse::Blob* out) {
  /*  [input data: 9 x 9]
                  0 3 0 2 4 9 1 0 3
                  1 5 7 2 3 6 2 1 5
                  0 4 8 1 5 0 2 0 8
                  2 1 2 0 7 4 0 0 2
                  3 1 5 1 1 0 9 3 7
                  1 6 2 3 6 2 1 0 6
                  0 4 0 7 0 2 5 8 4
                  7 7 2 5 0 4 3 1 9
                  3 1 0 6 2 1 3 5 4

                  [output]
                   -5   5 -10  3
                    5  11  -9  5
                  -16   4   1 18
                  -14  -5 -10 21
   */

  float d[4][4] = {
    { -5.0f,   5.0f, -10.0f,   3.0f },
    {   5.0f,  11.0f, -9.0f,   5.0f },
    { -16.0f,   4.0f,   1.0f,  18.0f },
    { -14.0f, -5.0f, -10.0f,  21.0f },
  };

  for (int j = 0; j < 4; ++j) {
    for (int i = 0; i < 4; ++i) {
      if (d[j][i] != out->getValue(0, i, j, 0)) {
        return false;
      }
    }
  }

  return true;
}

bool check_conv_stride_3(const demitasse::Blob *out) {
  /*  [input data: 9 x 9]
                  0 3 0 2 4 9 1 0 3
                  1 5 7 2 3 6 2 1 5
                  0 4 8 1 5 0 2 0 8
                  2 1 2 0 7 4 0 0 2
                  3 1 5 1 1 0 9 3 7
                  1 6 2 3 6 2 1 0 6
                  0 4 0 7 0 2 5 8 4
                  7 7 2 5 0 4 3 1 9
                  3 1 0 6 2 1 3 5 4

                  [output]
                    -5   5   3
                    11  10   4
                   -14  11  21
   */

  float d[3][3] = {
    {  -5.0f,   5.0f,   3.0f },
    {  11.0f,  10.0f,   4.0f },
    { -14.0f,  11.0f,  21.0f },
  };

  for (int j = 0; j < 3; ++j) {
    for (int i = 0; i < 3; ++i) {
      if (d[j][i] != out->getValue(0, i, j, 0)) {
        return false;
      }
    }
  }

  return true;
}

bool check_conv_filter_2_stride_1(const demitasse::Blob* out) {
  /*  [input data: 9 x 9]
                  0 3 0 2 4 9 1 0 3
                  1 5 7 2 3 6 2 1 5
                  0 4 8 1 5 0 2 0 8
                  2 1 2 0 7 4 0 0 2
                  3 1 5 1 1 0 9 3 7
                  1 6 2 3 6 2 1 0 6
                  0 4 0 7 0 2 5 8 4
                  7 7 2 5 0 4 3 1 9
                  3 1 0 6 2 1 3 5 4

                  [output]
                    -5 -13   5   5 -10   2   3
                    -2 -18  11  -9  17  -6  11
                     5   6  11 -18  -9  15   5
                    11 -14   5  10  16 -32   4
                   -16   6   4 -18   1  12  18
                    -3  15 -20  15   3  -6 -22
                   -14   4  -5  11 -10   1  21
   */

  float d[7][7] = {
    { -5.0f, -13.0f,   5.0f,   5.0f, -10.0f,   2.0f,   3.0f },
    { -2.0f, -18.0f,  11.0f, -9.0f,  17.0f, -6.0f,  11.0f },
    {   5.0f,   6.0f,  11.0f, -18.0f, -9.0f,  15.0f,   5.0f },
    {  11.0f, -14.0f,   5.0f,  10.0f,  16.0f, -32.0f,   4.0f },
    { -16.0f,   6.0f,   4.0f, -18.0f,   1.0f,  12.0f,  18.0f },
    { -3.0f,  15.0f, -20.0f,  15.0f,   3.0f, -6.0f, -22.0f },
    { -14.0f,   4.0f, -5.0f,  11.0f, -10.0f,   1.0f,  21.0f },
  };

  for (int n = 0; n < 2; ++n) {
    for (int j = 0; j < 7; ++j) {
      for (int i = 0; i < 7; ++i) {
        if (d[j][i] != out->getValue(n, i, j, 0)) {
          printf("error filter:%d\n", n);
          return false;
        }
      }
    }
  }

  return true;
}

demitasse::Blob* init_delta(int size) {
  auto delta = new demitasse::Blob(demitasse::Blob::BlobType::Float, 1, size, size, 1);

  float*       p     = delta->rawData().RealSingle.data;

  for (int j = 0; j < size; ++j) {
    for (int i = 0; i < size; ++i) {
      p[(j * size) + i] = 1.0f;
    }
  }

  return delta;
}


TEST(ConvLayerTest, ConvolutionStride1) {
  demitasse::Blob*     data   = init_input_data();
  demitasse::Blob*     weight = init_filters(1);
  demitasse::Blob*     bias   = init_bias(1);

  demitasse::ConvParam conv_param;
  conv_param.in_maps  = 1;
  conv_param.out_maps = 1;
  conv_param.ksize    = 3;
  conv_param.stride   = 1;

  auto conv = new demitasse::ConvLayer(&conv_param /* 1, 1, 3, 1 */);
  conv->setWeight(weight);
  conv->setBias(bias);

  auto out = conv->feedForward(data);
  bool result      = check_conv_stride_1(out);

  ASSERT_EQ(result, true);

  delete data;
  delete weight;
  delete bias;
}


TEST(ConvLayerTest, ConvolutionStride2) {
  demitasse::Blob*     data   = init_input_data();
  demitasse::Blob*     weight = init_filters(1);
  demitasse::Blob*     bias   = init_bias(1);

  demitasse::ConvParam conv_param;
  conv_param.in_maps  = 1;
  conv_param.out_maps = 1;
  conv_param.ksize    = 3;
  conv_param.stride   = 2;

  auto *conv = new demitasse::ConvLayer(&conv_param /*1, 1, 3, 2 */);
  conv->setWeight(weight);
  conv->setBias(bias);

  auto out = conv->feedForward(data);

  bool result      = check_conv_stride_2(out);
  ASSERT_EQ(result, true);

  delete data;
  delete weight;
  delete bias;
}


TEST(ConvLayerTest, ConvolutionStride3) {
  demitasse::Blob*     data   = init_input_data();
  demitasse::Blob*     weight = init_filters(1);
  demitasse::Blob*     bias   = init_bias(1);

  demitasse::ConvParam conv_param;
  conv_param.in_maps  = 1;
  conv_param.out_maps = 1;
  conv_param.ksize    = 3;
  conv_param.stride   = 3;

  auto conv = new demitasse::ConvLayer(&conv_param /*1, 1, 3, 3*/);
  conv->setWeight(weight);
  conv->setBias(bias);

  auto out = conv->feedForward(data);

  bool result      = check_conv_stride_3(out);

  ASSERT_EQ(result, true);

  delete data;
  delete weight;
  delete bias;
}

TEST(ConvLayerTest, ConvolutionFilter2Stride1) {
  demitasse::Blob*     data   = init_input_data();
  demitasse::Blob*     weight = init_filters(2);
  demitasse::Blob*     bias   = init_bias(2);

  demitasse::ConvParam conv_param;
  conv_param.in_maps  = 1;
  conv_param.out_maps = 2;
  conv_param.ksize    = 3;
  conv_param.stride   = 1;

  auto conv = new demitasse::ConvLayer(&conv_param /*1, 2, 3, 1*/);
  conv->setWeight(weight);
  conv->setBias(bias);

  auto out = conv->feedForward(data);
/*
        printf("out ch : %d, (%d, %d)\n", out->extent(0), out->extent(1), out->extent(2));
        for (int n = 0; n < out->extent(0); ++n) {
                for (int j = 0; j < out->extent(2); ++j) {
                        for (int i = 0; i < out->extent(1); ++i) {
                                printf("%d ", (int)(*out)(n, i, j, 0));
                        }
                        printf("\n");
                }
        }
 */
  bool result      = check_conv_filter_2_stride_1(out);

  ASSERT_EQ(result, true);

  delete data;
  delete weight;
  delete bias;
}
