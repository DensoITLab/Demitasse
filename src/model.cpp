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
 #include <iostream>
#include <fstream>
#include <chrono>

#include "flatbuffers/flatbuffers.h"
#include "network_model_generated.h"

#include "debug_log.hpp"
#include "model.hpp"
#include "model_serializer.hpp"

using demitasse::Model;
using demitasse::BaseLayer;
using demitasse::Blob;
using demitasse::ModelSerializer;

Model::Model() {
  layers_ = std::vector<BaseLayer*>();
}

Model::Model(std::vector<BaseLayer*> layers) {
  // TODO: validation check for layers input and output size
  layers_ = layers;
}

Model::~Model() {
  for (auto layer: layers_) {
    delete layer;
  }
}

void Model::addLayer(BaseLayer* layer) {
  // TODO: validation check for layers input and output size
  layers_.push_back(layer);
}

BaseLayer* Model::getLayer(int atIndex) {
  return layers_[atIndex];
}

unsigned int Model::numOfLayers() {
  return static_cast<unsigned int>(layers_.size());
}

/*
   void Model::save(const char* filename) {

   flatbuffers::FlatBufferBuilder builder;

   int numOfLayers = _layers.size();

   auto layers = new flatbuffers::Offset<demitasse::serializer::Layer>[numOfLayers];

   int idx = 0;
   for (auto layer = _layers.begin(); layer != _layers.end(); ++layer) {
    layers[idx] = (*layer)->save(builder);
    idx++;
   }

   auto mlocLayers = builder.CreateVector(layers, numOfLayers);

   // root entity
   auto model = demitasse::serializer::ModelBuilder(builder);

   model.add_layers(mlocLayers);

   auto mloc = model.Finish();

   demitasse::serializer::FinishModelBuffer(builder, mloc);

   // write to the file
   auto buffer = builder.GetBufferPointer();
   auto size   = builder.GetSize();

   std::ofstream fs(filename, std::ios::out | std::ios::binary | std::ios::trunc);

   fs.write((const char *)buffer, size);
   fs.close();

   builder.ReleaseBufferPointer();

   delete[] layers;
   }
 */


void Model::load(const std::string& filename) {
  std::ifstream fs(filename, std::ios::in | std::ios::binary);

  // get file size
  auto begin = fs.tellg();
  fs.seekg(0, fs.end);
  auto end   = fs.tellg();
  auto size  = end - begin;
  fs.clear();
  fs.seekg(0, fs.beg);

  // load into memory
  char* buffer = new char[size];
  fs.read(buffer, (std::streamsize)size);
  fs.close();

  DebugLog("load model to memory\n");

  auto model  = demitasse::serialize::GetModel(buffer);

  auto layers = model->layers();

  for (auto layer: *layers) {
    auto newLayer = ModelSerializer::loadLayer(layer);
    if (newLayer != nullptr) {
      layers_.push_back(newLayer);
    }
  }

  delete[] buffer;
}


const Blob* Model::feedForward(Blob* input) {
  unsigned long num = layers_.size() + 1;

  std::vector<const Blob*> z(num);

  int idx = 0;
  z[idx] = input;

  auto t = std::vector<double>();

  for (auto layer = layers_.begin(); layer != layers_.end(); ++layer) {

    auto t1 = std::chrono::high_resolution_clock::now();

    z[idx + 1] = (*layer)->feedForward(z[idx]);

    auto t2 = std::chrono::high_resolution_clock::now();

    // convers usec -> sec
    double dt = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1e6;

    t.push_back(dt);

    idx++;
  }


  std::string header = "";
  std::string data   = "";

  double total = 0.0;
  int    i     = 0;
  for (auto layer = layers_.begin(); layer != layers_.end(); ++layer) {
    header += (*layer)->layerTypeName() + "\t";
    data   += std::to_string(t[i]) + "\t";
    total += t[i];
    // DebugLog("%s [%.2d] : %f\n", (*layer)->layerTypeName().c_str(), i, t[i]);
    ++i;
  }

  header += "total\n";
  data   += std::to_string(total) + "\n";

  printf("%s", header.c_str());
  printf("%s", data.c_str());

  // DebugLog("total: %f\n\n", total);

  return z[idx];
}
