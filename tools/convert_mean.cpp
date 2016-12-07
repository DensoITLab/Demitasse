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
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "caffe.pb.h"

void load_mean_blob(const char* blobfile, const char* outfile) {

  std::ifstream stream(blobfile, std::ios::binary);

  auto ifs = new google::protobuf::io::IstreamInputStream(&stream);
  google::protobuf::io::CodedInputStream input(ifs);

  caffe::BlobProto *blob = new caffe::BlobProto();

  blob->MergePartialFromCodedStream(&input);

  int size = blob->data_size();

  printf("\t blob size:%d\n", size);

  int num = 0;
  if (blob->has_num()) {
    num = blob->num();
    printf("\t blob num: %d\n", num);
  }

  int channels = 0;
  if (blob->has_channels()) {
    channels = blob->channels();
    printf("\t blob channels: %d\n", channels);
  }

  int height = 0;
  if (blob->has_height()) {
    height = blob->height();
    printf("\t blob height: %d\n", height);
  }

  int width = 0;
  if (blob->has_width()) {
    width = blob->width();
    printf("\t blob width: %d\n", width);
  }

  // output blob to file
  std::ofstream fs(outfile, std::ios::out | std::ios::binary | std::ios::trunc);

  size_t bufsize = channels * height * width;

  fs.write((char*)blob->data().data(), sizeof(float) * bufsize);
  fs.close();

  google::protobuf::ShutdownProtobufLibrary();
}

int main(int argc, char **argv) {

  if (argc < 2) {
    printf("Usage: %s <mean_blob> [<output>]\n", argv[0]);
    return 0;
  }

  const char* blobfile = argv[1];

  const char *outfile  = "./ilsvrc12_mean.blob";
  if (argc == 3) {
    outfile = argv[2];
  }

  load_mean_blob(blobfile, outfile);

  return 0;
}
