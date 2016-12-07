#!/bin/bash

for i in {0..99}; do
  examples/vgg_bench ../data/vgg_cnn_f/vudnn.model >> vudnn_vgg_cnn_f.tsv
done
