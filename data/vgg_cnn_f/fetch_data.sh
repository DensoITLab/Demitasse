#!/bin/bash

# downloading VGG-CNN-F model and prototxt

# prototxt
if [ ! -e VGG_CNN_F_deploy.prototxt ];
then
  curl -O https://gist.githubusercontent.com/ksimonyan/a32c9063ec8e1118221a/raw/6a3b8af023bae65669a4ceccd7331a5e7767aa4e/VGG_CNN_F_deploy.prototxt
fi

# caffemodel
if [ ! -e VGG_CNN_F.caffemodel ];
then
  curl -O http://www.robots.ox.ac.uk/~vgg/software/deep_eval/releases/bvlc/VGG_CNN_F.caffemodel
fi
