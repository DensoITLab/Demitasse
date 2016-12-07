#!/bin/bash

# downloading ilsvrc aux data set files

# imagenet_mean.binaryproto and synset_words.txt
if [ ! -e imagenet_mean.binaryproto ];
then
  curl -O http://dl.caffe.berkeleyvision.org/caffe_ilsvrc12.tar.gz
  tar zxf caffe_ilsvrc12.tar.gz
  rm -f caffe_ilsvrc12.tar.gz
fi

# image mean file for Torch7
if [ ! -e ilsvrc_2012_mean.t7 ];
then
  # https://www.dropbox.com/s/p33rheie3xjx6eu/ilsvrc_2012_mean.t7
  curl -o ilsvrc_2012_mean.t7 https://dl.dropboxusercontent.com/content_link/27uvCcvAvNBV346aXliSdFhhp4xrZ25Mb820rrZRNBM0k6FAOGeAbrRg73FFf9JA/file
fi
