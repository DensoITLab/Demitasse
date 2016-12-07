#!/bin/bash

for i in {0..99}; do
  examples/vgg_bench ../data/vgg_16/vudnn.model >> vudnn_vgg_16.tsv
done
