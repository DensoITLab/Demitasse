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
 #ifndef __DEMITASSE_MNIST__
#define __DEMITASSE_MNIST__

#include <string>
#include <iostream>
#include <vector>

#include "blob.hpp"

namespace demitasse {

//
// MNIST data loader
//

// TODO: Should define generic data handling class as super class of MNIST.
class MNIST {

public:
  MNIST(const char* dir_path);
  ~MNIST();

  void shuffle_train_data();

  // DEBUG
  void export_image_to_file(int n, const char* filename);
  int train_label(int n);

  // slice one data
  Blob* get_train_image(int n);
  Blob* get_train_label(int n);

  Blob* get_varidate_images();
  Blob* get_varidate_labels();

  Blob* get_batch_train_images(int idx, int num);
  Blob* get_batch_train_labels(int idx, int num);

  Blob* get_test_images(int idx, int num);
  Blob* get_test_labels(int idx, int num);

  int train_size;
  int varidate_size;
  int test_size;

  std::vector<int> train_idx;

  Blob* train_images;   // (28 x 28 x 60000)
  Blob* train_labels;   // (60000)
  Blob* test_images;    // (28 x 28 x 10000)
  Blob* test_labels;    // (10000)

private:
  std::string get_file_path(const std::string& dir_path, const std::string& file);
  Blob* load_image_data(const std::string& dir_path, const std::string& file);
  Blob* load_label_data(const std::string& dir_path, const std::string& file);
};

}

#endif /* __DEMITASSE_MNIST__ */
