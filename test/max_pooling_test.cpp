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
  demitasse::Blob* data = new demitasse::Blob(demitasse::Blob::BlobType::Float, 1, 9, 9, 1);

  // float *p = data->rawData().RealSingle;

  for (int j = 0; j < 9; ++j) {
    for (int i = 0; i < 9; ++i) {
      // p[(j * 9) + i] = d[j][i];
      data->setValue(d[j][i], 0, i, j, 0);
    }
  }

  return data;
}

bool check_stride1(const demitasse::Blob* out) {
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
                  8 8 8 9 9 9 8
                  8 8 8 7 7 6 8
                  8 8 8 7 9 9 9
                  6 6 7 7 9 9 9
                  6 7 7 7 9 9 9
                  7 7 7 7 6 8 9
                  7 7 7 7 5 8 9
   */

  float d[7][7] = {
    { 8.0f, 8.0f, 8.0f, 9.0f, 9.0f, 9.0f, 8.0f },
    { 8.0f, 8.0f, 8.0f, 7.0f, 7.0f, 6.0f, 8.0f },
    { 8.0f, 8.0f, 8.0f, 7.0f, 9.0f, 9.0f, 9.0f },
    { 6.0f, 6.0f, 7.0f, 7.0f, 9.0f, 9.0f, 9.0f },
    { 6.0f, 7.0f, 7.0f, 7.0f, 9.0f, 9.0f, 9.0f },
    { 7.0f, 7.0f, 7.0f, 7.0f, 6.0f, 8.0f, 9.0f },
    { 7.0f, 7.0f, 7.0f, 7.0f, 5.0f, 8.0f, 9.0f },
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

bool check_stride2(const demitasse::Blob* out) {
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
                  8 8 9 8
                  8 8 9 9
                  6 7 9 9
                  7 7 5 9
   */

  float d[4][4] = {
    { 8.0f, 8.0f, 9.0f, 8.0f },
    { 8.0f, 8.0f, 9.0f, 9.0f },
    { 6.0f, 7.0f, 9.0f, 9.0f },
    { 7.0f, 7.0f, 5.0f, 9.0f },
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

bool check_stride3(const demitasse::Blob *out) {
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
                  8 9 8
                  6 7 9
                  7 7 9
   */

  float d[3][3] = {
    { 8.0f, 9.0f, 8.0f },
    { 6.0f, 7.0f, 9.0f },
    { 7.0f, 7.0f, 9.0f },
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


bool check_stride1_pad1(const demitasse::Blob* out) {
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
                  5 7 7 7 9 9 9 5 5
                  5 8 8 8 9 9 9 8 8
                  5 8 8 8 7 7 6 8 8
                  4 8 8 8 7 9 9 9 8
                  6 6 6 7 7 9 9 9 7
                  6 6 7 7 7 9 9 9 8
                  7 7 7 7 7 6 8 9 9
                  7 7 7 7 7 5 8 9 9
                  7 7 7 6 6 4 5 9 9
   */

  float d[9][9] = {
    { 5.0f, 7.0f, 7.0f, 7.0f, 9.0f, 9.0f, 9.0f, 5.0f, 5.0f },
    { 5.0f, 8.0f, 8.0f, 8.0f, 9.0f, 9.0f, 9.0f, 8.0f, 8.0f },
    { 5.0f, 8.0f, 8.0f, 8.0f, 7.0f, 7.0f, 6.0f, 8.0f, 8.0f },
    { 4.0f, 8.0f, 8.0f, 8.0f, 7.0f, 9.0f, 9.0f, 9.0f, 8.0f },
    { 6.0f, 6.0f, 6.0f, 7.0f, 7.0f, 9.0f, 9.0f, 9.0f, 7.0f },
    { 6.0f, 6.0f, 7.0f, 7.0f, 7.0f, 9.0f, 9.0f, 9.0f, 8.0f },
    { 7.0f, 7.0f, 7.0f, 7.0f, 7.0f, 6.0f, 8.0f, 9.0f, 9.0f },
    { 7.0f, 7.0f, 7.0f, 7.0f, 7.0f, 5.0f, 8.0f, 9.0f, 9.0f },
    { 7.0f, 7.0f, 7.0f, 6.0f, 6.0f, 4.0f, 5.0f, 9.0f, 9.0f },
  };

  for (int j = 0; j < 9; ++j) {
    for (int i = 0; i < 9; ++i) {
      if (d[j][i] != out->getValue(0, i, j, 0)) {
        return false;
      }
    }
  }

  return true;
}

bool check_stride2_pad1(const demitasse::Blob* out) {
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
                  5 7 9 9 5
                  5 8 7 6 8
                  6 6 7 9 7
                  7 7 7 8 9
                  7 7 6 5 9
   */

  float d[5][5] = {
    { 5.0f, 7.0f, 9.0f, 9.0f, 5.0f },
    { 5.0f, 8.0f, 7.0f, 6.0f, 8.0f },
    { 6.0f, 6.0f, 7.0f, 9.0f, 7.0f },
    { 7.0f, 7.0f, 7.0f, 8.0f, 9.0f },
    { 7.0f, 7.0f, 6.0f, 5.0f, 9.0f },
  };

  for (int j = 0; j < 5; ++j) {
    for (int i = 0; i < 5; ++i) {
      if (d[j][i] != out->getValue(0, i, j, 0)) {
        return false;
      }
    }
  }

  return true;
}

bool check_stride3_pad1(const demitasse::Blob* out) {
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
                  5 7 9
                  4 8 9
                  7 7 8
   */

  float d[3][3] = {
    { 5.0f, 7.0f, 9.0f },
    { 4.0f, 8.0f, 9.0f },
    { 7.0f, 7.0f, 8.0f },
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


demitasse::Blob* init_delta(int size) {
  demitasse::Blob *delta = new demitasse::Blob(demitasse::Blob::BlobType::Float, size, size, 1, 1);
  float *      p     = delta->rawData().RealSingle.data;

  for (int j = 0; j < size; ++j) {
    for (int i = 0; i < size; ++i) {
      p[(j * size) + i] = 1.0f;
    }
  }

  return delta;
}

bool check_backprop1(const demitasse::Blob* delta) {
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
                  8 8 8 9 9 9 8
                  8 8 8 7 7 6 8
                  8 8 8 7 9 9 9
                  6 6 7 7 9 9 9
                  6 7 7 7 9 9 9
                  7 7 7 7 6 8 9
                  7 7 7 7 5 8 9

                  [delta]
                  0 0 0 0 0 3 0 0 0
                  0 0 0 0 0 1 0 0 0
                  0 0 9 0 0 0 0 0 2
                  0 0 0 0 5 0 0 0 0
                  0 0 0 0 0 0 9 0 0
                  0 3 0 0 1 0 0 0 0
                  0 0 0 9 0 0 1 2 0
                  2 0 0 0 0 0 0 0 2
                  0 0 0 0 0 0 0 0 0
   */

  float  d[9][9] = {
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 9.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f },
    { 0.0f, 0.0f, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 9.0f, 0.0f, 0.0f },
    { 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f, 9.0f, 0.0f, 0.0f, 1.0f, 2.0f, 0.0f },
    { 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f },
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
  };

  float* p = delta->rawData().RealSingle.data;
  for (int j = 0; j < 9; ++j) {
    for (int i = 0; i < 9; ++i) {
      if (d[j][i] != p[(j * 9) + i]) {
        return false;
      }
    }
  }

  return true;
}

bool check_backprop2(const demitasse::Blob* delta) {
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
                  8 8 9 8
                  8 8 9 9
                  6 7 9 9
                  7 7 5 9

                  [delta]
                  0 0 0 0 0 1 0 0 0
                  0 0 0 0 0 0 0 0 0
                  0 0 4 0 0 0 0 0 1
                  0 0 0 0 0 0 0 0 0
                  0 0 0 0 0 0 4 0 0
                  0 1 0 0 0 0 0 0 0
                  0 0 0 2 0 0 1 0 0
                  1 0 0 0 0 0 0 0 1
                  0 0 0 0 0 0 0 0 0
   */

  float  d[9][9] = {
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 4.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 4.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },
    { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
  };

  float* p = delta->rawData().RealSingle.data;
  for (int j = 0; j < 9; ++j) {
    for (int i = 0; i < 9; ++i) {
      if (d[j][i] != p[(j * 9) + i]) {
        return false;
      }
    }
  }

  return true;
}

bool check_backprop3(demitasse::Blob* delta) {
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
                  8 9 8
                  6 7 9
                  7 7 9

                  [delta]
                  0 0 0 0 0 1 0 0 0
                  0 0 0 0 0 0 0 0 0
                  0 0 1 0 0 0 0 0 1
                  0 0 0 0 1 0 0 0 0
                  0 0 0 0 0 0 1 0 0
                  0 1 0 0 0 0 0 0 0
                  0 0 0 1 0 0 0 0 0
                  1 0 0 0 0 0 0 0 1
                  0 0 0 0 0 0 0 0 0
   */

  float  d[9][9] = {
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }
  };

  float* p = delta->rawData().RealSingle.data;
  for (int j = 0; j < 9; ++j) {
    for (int i = 0; i < 9; ++i) {
      if (d[j][i] != p[(j * 9) + i]) {
        return false;
      }
    }
  }

  return true;
}


TEST(PoolingLayerTest, MaxPoolingStride1) {
  demitasse::Blob* data = init_input_data();

  demitasse::PoolingParam maxParam;
  maxParam.ksize   = 3;
  maxParam.stride  = 1;
  maxParam.padding = 0;

  auto layer  = new demitasse::PoolingLayer<demitasse::max>(&maxParam);
  auto out    = layer->feedForward(data);

  bool result = check_stride1(out);

  ASSERT_EQ(result, true);
}


TEST(PoolingLayerTest, MaxPoolingStride2) {
  demitasse::Blob* data = init_input_data();

  demitasse::PoolingParam maxParam;
  maxParam.ksize   = 3;
  maxParam.stride  = 2;
  maxParam.padding = 0;

  auto layer  = new demitasse::PoolingLayer<demitasse::max>(&maxParam);
  auto out    = layer->feedForward(data);

  bool result = check_stride2(out);
  ASSERT_EQ(result, true);
}


TEST(PoolingLayerTest, MaxPoolingStride3) {
  demitasse::Blob* data = init_input_data();

  demitasse::PoolingParam maxParam;
  maxParam.ksize   = 3;
  maxParam.stride  = 3;
  maxParam.padding = 0;

  auto layer  = new demitasse::PoolingLayer<demitasse::max>(&maxParam);
  auto out    = layer->feedForward(data);

  bool result = check_stride3(out);

  ASSERT_EQ(result, true);
}

TEST(PoolingLayerTest, MaxPoolingStride1Pad1) {
  demitasse::Blob* data = init_input_data();

  demitasse::PoolingParam maxParam;
  maxParam.ksize   = 3;
  maxParam.stride  = 1;
  maxParam.padding = 1;

  auto layer  = new demitasse::PoolingLayer<demitasse::max>(&maxParam);
  auto out    = layer->feedForward(data);

  bool result = check_stride1_pad1(out);

  ASSERT_EQ(result, true);
}

TEST(PoolingLayerTest, MaxPoolingStride2Pad1) {
  demitasse::Blob* data = init_input_data();

  demitasse::PoolingParam maxParam;
  maxParam.ksize   = 3;
  maxParam.stride  = 2;
  maxParam.padding = 1;

  auto layer  = new demitasse::PoolingLayer<demitasse::max>(&maxParam);
  auto out    = layer->feedForward(data);

  bool result = check_stride2_pad1(out);
  ASSERT_EQ(result, true);
}

TEST(PoolingLayerTest, MaxPoolingStride3Pad1) {
  demitasse::Blob* data = init_input_data();

  demitasse::PoolingParam maxParam;
  maxParam.ksize   = 3;
  maxParam.stride  = 3;
  maxParam.padding = 1;

  auto layer  = new demitasse::PoolingLayer<demitasse::max>(&maxParam);
  auto out    = layer->feedForward(data);

  bool result = check_stride3_pad1(out);
  ASSERT_EQ(result, true);
}

/*
   TEST(PoolingLayerTest, MaxPoolingBackProp1) {

        demitasse::Blob* data  = init_input_data();
        demitasse::Blob* delta = init_delta(7);

   demitasse::PoolingParam maxParam;
   maxParam.ksize    = 3;
   maxParam.stride   = 1;
   maxParam.padding  = 0;

        auto layer = new demitasse::PoolingLayer<demitasse::max>(&maxParam);
        auto out   = layer->delta(data, delta);

        for (int j = 0; j < out->extent(1); ++j) {
                for (int i = 0; i < out->extent(0); ++i) {
                        printf("%d ", (int)(*out)(i, j));
                }
                printf("\n");
        }

        bool result = check_backprop1(out);

        ASSERT_EQ(result, true);
   }

TEST(PoolingLayerTest, MaxPoolingBackProp2) {

   demitasse::Blob* data  = init_input_data();
   demitasse::Blob* delta = init_delta(4);

   demitasse::PoolingParam maxParam;
   maxParam.ksize    = 3;
   maxParam.stride   = 2;
   maxParam.padding  = 0;

   auto layer = new demitasse::PoolingLayer<demitasse::max>(&maxParam);
   auto out   = layer->delta(data, delta);

        for (int j = 0; j < out->extent(1); ++j) {
                for (int i = 0; i < out->extent(0); ++i) {
                        printf("%d ", (int)(*out)(i, j));
                }
                printf("\n");
        }

        bool result = check_backprop2(out);

        ASSERT_EQ(result, true);
   }

TEST(PoolingLayerTest, MaxPoolingBackProp3) {

   demitasse::Blob* data  = init_input_data();
   demitasse::Blob* delta = init_delta(3);

   demitasse::PoolingParam maxParam;
   maxParam.ksize    = 3;
   maxParam.stride   = 3;
   maxParam.padding  = 0;

   demitasse::PoolingLayer* layer = new demitasse::PoolingLayer<demitasse::max>(&maxParam);
   demitasse::Blob* out = layer->delta(data, delta);

        for (int j = 0; j < out->extent(1); ++j) {
                for (int i = 0; i < out->extent(0); ++i) {
                        printf("%d ", (int)(*out)(i, j));
                }
                printf("\n");
        }

        bool result = check_backprop3(out);

        ASSERT_EQ(result, true);
   }
 */
