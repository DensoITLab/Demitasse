#!/bin/bash

for i in {0..99}; do
  th vgg_bench.lua
  cat dump.tsv >> torch7_vgg_16.tsv
done
