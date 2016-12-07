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
 #ifndef __DEMITASSE_MODEL_SERIALIZER__
#define __DEMITASSE_MODEL_SERIALIZER__

#include "base_layer.hpp"
#include "binary_encoder.hpp"
#include "network_model_generated.h"

namespace demitasse {

class ModelSerializer {

public:
  static BaseLayer* loadLayer(const demitasse::serialize::Layer *param);

protected:
  static void loadConvLayerParameters(
      float* weight,
      float* bias,
      const demitasse::serialize::Layer* param);

  static void loadFullyConnectedLayerParameters(
      float* weight,
      float* bias,
      const demitasse::serialize::Layer* param);

  static BinaryEncoder* createBinaryEncoder(const demitasse::serialize::Encoder* param);

  static BaseLayer* createActivationLayer(const demitasse::serialize::Layer* param);
  static BaseLayer* createConvLayer(const demitasse::serialize::Layer* param);
  static BaseLayer* createFullyConnectedLayer(const demitasse::serialize::Layer* param);
  static BaseLayer* createBinaryConvLayer(const demitasse::serialize::Layer* param);
  static BaseLayer* createBinaryFullyConnectedLayer(const demitasse::serialize::Layer* param);

};

}

#endif // __DEMITASSE_MODEL_SERIALIZER__
