#!/bin/bash

# downloading VGG-16 model and prototxt

# prototxt
if [ ! -e VGG_ILSVRC_16_layers_deploy.prototxt ];
then
  curl -O https://gist.githubusercontent.com/ksimonyan/211839e770f7b538e2d8/raw/c3ba00e272d9f48594acef1f67e5fd12aff7a806/VGG_ILSVRC_16_layers_deploy.prototxt
fi

# caffemodel
if [ ! -e VGG_ILSVRC_16_layers.caffemodel ];
then
  curl -O http://www.robots.ox.ac.uk/~vgg/software/very_deep/caffe/VGG_ILSVRC_16_layers.caffemodel
fi
