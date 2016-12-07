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
 #ifndef __DEMITASSE_CONV_LAYER__
#define __DEMITASSE_CONV_LAYER__

#include "blob.hpp"
#include "base_layer.hpp"
#include "conv_param.hpp"

namespace demitasse {

class ConvLayer : public BaseLayer {

public:
  //
  // constructor & destructor
  //
  ConvLayer(const ConvParam* param);
  ~ConvLayer();

  // layer type name string for serialization
  const std::string layerTypeName() override;

  //
  // parameter accessors
  //
  const Blob* weight();
  const Blob* bias();

  void setWeight(const Blob* weight);
  void setBias(const Blob* bias);

  //
  // forward
  //
  const Blob* feedForward(const Blob* input) override;

protected:
  bool isNeedIm2Col(int in_x_nodes, int in_y_nodes);
  void reshapeOutputBlob(const Blob *input);

  ConvParam param_;

  Blob* weight_;                                         // filters
  Blob* bias_;                                           // bias

  Blob* im2col_;                                         // im2col buffer
  Blob* output_;                                         // output
};

}

#endif /* __DEMITASSE_CONV_LAYER__ */
