#!/bin/bash

for i in {0..99}; do
  th vgg_cnn_f.lua
  cat dump.tsv >> torch7_vgg_cnn_f.tsv
done
