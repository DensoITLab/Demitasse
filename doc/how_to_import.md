How to import model data.
-----------------------

Suppose you build Demitasse at `../build_Demitasse` from Demitasse source code path. You have to build Demitasse before the following instructions. We explain how to convert VGG-16 and make the sample code load it.

## Download VGG-16 model data

You have to download pre-trained model file(.caffemodel) and model definition file(.prototxt).
Here is downloading script in `./data/vgg_16/fetch_data.sh` or `./data/vgg_cnn_f/fetch_data.sh`.

```
$ cd ./data/vgg_16
$ ./fetch_data.sh
```

These file will be downloaded.

```
VGG_ILSVRC_16_layers.caffemodel
VGG_ILSVRC_16_layers_deploy.prototxt
```

## Convert caffemodel file

To convert Caffe model data file to Demitasse model file, use `caffe2demitasse` command.

```
$ caffe2demitasse <model .protobuf file> <model file .caffemodel> <output_file>
```

So, you can convert the downloaded files by the following commands.

```
$ cd ./data/vgg_16
$ ../../../build_Demitasse/tools/caffe2demitasse \
./VGG_ILSVRC_16_layers_deploy.prototxt \
./VGG_ILSVRC_16_layers.caffemodel \
./converted.bin
```

## Run `vgg_bench`

`vgg_bench` is a sample code to use the model that is converted from caffe model data. `vgg_bench` needs `../data/ilsvrc/synset_words.txt`. Here is downloading script in `./data/ilsvrc/fetch_data.sh`.

```
$ cd ./data/ilsvrc
$ ./fetch_data.sh
```

These file will be downloaded.

```
det_synset_words.txt
ilsvrc_2012_mean.t7
imagenet_mean.binaryproto
synsets.txt
train.txt
imagenet.bet.pickle
synset_words.txt
test.txt
val.txt
```

You try to run `vgg_bench` by the following commands.

```
$ cd ../build_Demitasse/example
$ ./vgg_bench ../../Demitasse/data/vgg_16/converted.bin
preprocessed img01
conv layer	activation <ReLU>	conv layer	activation <ReLU>	pooling <max>	conv layer	activation <ReLU>	conv layer	activation <ReLU>	pooling <max>	conv layer	activation <ReLU>	conv layer	activation <ReLU>	conv layer	activation <ReLU>	pooling <max>	conv layer	activation <ReLU>	conv layer	activation <ReLU>	conv layer	activation <ReLU>	pooling <max>	conv layer	activation <ReLU>	conv layer	activation <ReLU>	conv layer	activation <ReLU>	pooling <max>	fully connected layer	activation <ReLU>	fully connected layer	activation <ReLU>	fully connected layer	activation <softmax>	total
0.021230	0.010857	0.165129	0.010502	0.003906	0.045169	0.002995	0.082287	0.003180	0.001455	0.029893	0.002620	0.055428	0.004034	0.063542	0.004235	0.001891	0.026688	0.000308	0.045121	0.000627	0.046989	0.000630	0.000223	0.012551	0.000080	0.011580	0.000333	0.011967	0.000322	0.000165	0.029056	0.000055	0.004866	0.000045	0.001224	0.000021	0.701204
00 : [0207] golden retriever (0.983486)
01 : [0213] Irish setter, red setter (0.009660)
02 : [0176] Saluki, gazelle hound (0.002492)
03 : [0852] tennis ball (0.001743)
04 : [0219] cocker spaniel, English cocker spaniel, cocker (0.000987)
```