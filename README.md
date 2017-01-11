Demitasse
=====

Demitasse is the **fast Deep Neural Network** Library for mobile devices(non GPGPU). You can check the detail of Demitasse from this white paper.

## How to build

* [for macOS](https://github.com/DensoITLab/Demitasse/blob/master/doc/build_for_ios.md)
* [for Linux(ubuntu)](https://github.com/DensoITLab/Demitasse/blob/master/doc/build_for_linux.md)
* [for macOS(日本語)](https://github.com/DensoITLab/Demitasse/blob/master/doc/build_for_ios_ja.md)

### Build script
* [for macOS](https://gist.github.com/sonsongithub/b836d8cf7d81b600e7f0fe18fa5f261a)
* [for Linux](https://gist.github.com/sonsongithub/6e0ee5ee675339baca43f0d44f890e4e)

## How to convert parameters from caffe

It is very boring to make DNN's model data yourself.
Demitasse supports to import pre-trained model data from [Caffe Model Zoo.](https://github.com/BVLC/caffe/wiki/Model-Zoo) At this moment, we tested VGG-16 and VGG-CNN-F pre-trained model data for Demitasse. 
Demitasse import tool places at `./tools/`. You have to build Demitasse binaries in order to convert pre-trained datas from the other libraries' models into Demitasse's one. How to use the tools is [here](https://github.com/DensoITLab/Demitasse/blob/master/doc/how_to_import.md).

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

* This software is released under the License, see [LICENSE.txt](https://github.com/DensoITLab/Demitasse/blob/master/LICENSE.txt).
* The sample query image is created from a [public domain content](http://free-photos.gatag.net/2015/02/06/020000.html).