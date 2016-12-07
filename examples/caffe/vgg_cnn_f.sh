#!/bin/bash

CAFFE_HOME=~/workspace/caffe
CAFFE_CMD=$CAFFE_HOME/build/examples/cpp_classification/classification.bin

MODEL_HOME=~/workspace/vudnn/data
PROTO_FILE=$MODEL_HOME/vgg_cnn_f/VGG_CNN_F_deploy.prototxt
MODEL_FILE=$MODEL_HOME/vgg_cnn_f/VGG_CNN_F.caffemodel
MEAN_FILE=$MODEL_HOME/ilsvrc/imagenet_mean.binaryproto
LABEL_FILE=$MODEL_HOME/ilsvrc/synset_words.txt

IMG_FILE=$MODEL_HOME/images/sample01.png

for i in {0..99}; do
  $CAFFE_CMD $PROTO_FILE $MODEL_FILE $MEAN_FILE $LABEL_FILE $IMG_FILE >> caffe_vgg_cnn_f.tsv
done
