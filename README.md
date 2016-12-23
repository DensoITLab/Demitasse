Demitasse
=====

Demitasse is the Deep Neural Network Library for mobile devices(non GPGPU).

## How to build for macOS and Linux

* [See documentation.](https://github.com/DensoITLab/Demitasse/blob/master/doc/build_for_ios.md)
* [日本語ドキュメントもあります．](https://github.com/DensoITLab/Demitasse/blob/master/doc/build_for_ios_ja.md)
* [Here](https://gist.github.com/sonsongithub/b836d8cf7d81b600e7f0fe18fa5f261a) is building script for macOS.

## How to convert parameters from caffe

Demitasse supports to import pre-trained model data from [Caffe Model Zoo.](https://github.com/BVLC/caffe/wiki/Model-Zoo)

At this moment, we tested VGG-16 and VGG-CNN-F pre-trained model data for Demitasse.

### Model data download

First, you have to download pre-trained model file(.caffemodel) and model definition file(.prototxt).
Here is downloading script in data/vgg_16/fetch_data.sh and data/vgg_cnn_f/fetch_data.sh.

```
$ cd data/vgg-16
$ ./fetch_data.sh
```
### Model data conversion

To convert Caffe model data file to Demitasse model file, use caffe2demitasse command.

```
$ caffe2demitasse <model .protobuf file> <model file .caffemodel> <output_file>
```

## Reference

Please cite Caffe in your publications if it helps your research:

```
@inproceedings{kondo2016,
author = {Satoshi Kondo},
booktitle = {ML Systems Workshop NIPS 2016},
title = {{Demitasse: SPMD Programing Implementation of Deep Neural Network Library for Mobile Devices}},
year = {2016}
}
```

## License

This software is released under the License, see LICENSE.txt.
