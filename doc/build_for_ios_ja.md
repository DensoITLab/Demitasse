demitasseのビルド環境設定
-----------------------

# はじめに

以下、iOSデモアプリをビルドするために必要な手順をまとめます。

ここでは、アプリのビルドに必要な各種コンパイラ、ツールのインストール手順を示します。
iOSアプリのビルドは、ホストマシン(macOS)上で行います。

## ホストマシンでの事前準備

macOSホストマシンでは、以下のツールが必要となります。

### Commandline tools for XCode

あらかじめCommandline tools for XCodeをインストールしてください。

### Homebrew

各種依存ツールをインストールするためにHomebrewパッケージ管理ツールを利用します。
ホストマシン上であらかじめbrewコマンドが利用できるように設定してください。

### その他のツール

demitasseのビルドには、cmake, gitなどが必要です。brewコマンドでホストマシン上にインストールします。

```
$ brew install cmake git
```

# Clang/LLVMコンパイラのインストール

ispcのビルドのためには、最新のLLVMが必要となる。
LLVM公式ページから、バイナリパッケージを入手してインストールするか、各プラットフォームごとにパッケージ管理システムからインストールする方法があります。

iPhone向けなど、ホスト以外のターゲット向けのビルドが必要な場合には、ARM, AArch64ターゲット向けのクロスコンパイル設定が必要なため、ここでは、ソースからビルドする方法を説明します。

## ソースリポジトリの取得

```
$ git clone http://llvm.org/git/llvm.git
$ cd llvm
$ (cd tools/; git clone http://llvm.org/git/clang.git)
$ (cd projects/; git clone http://llvm.org/git/compiler-rt.git)
$ (cd projects/; git clone http://llvm.org/git/openmp.git)
$ (cd projects/; git clone http://llvm.org/git/libcxx.git)
$ (cd projects/; git clone http://llvm.org/git/libcxxabi.git)
```

## 最新バージョンのブランチに切り替え

現時点（2016.10.12）での最新版のリリースバージョン3.9.0を利用する場合には、以下のようにブランチをチェックアウトする。

```
$ git checkout -b release_39 origin/release_39
$ (cd tools/clang; git checkout -b release_39 origin/release_39)
$ (cd projects/compiler-rt; git checkout -b release_39 origin/release_39)
$ (cd projects/openmp; git checkout -b release_39 origin/release_39)
$ (cd projects/libcxx; git checkout -b release_39 origin/release_39)
$ (cd projects/libcxxabi; git checkout -b release_39 origin/release_39)
```

## ld.goldに切り替え

Linux上で環境構築する際に、ldを別のものに切り替えます。
macOSでは以下の操作は不要です。

```
$ sudo update-alternatives --install "/usr/bin/ld" "ld" "/usr/bin/ld.gold" 20
$ sudo update-alternatives --install "/usr/bin/ld" "ld" "/usr/bin/ld.bfd" 10
$ sudo update-alternatives --set ld /usr/bin/ld.gold
```

## Clang/LLVMのビルド

リポジトリとは別のディレクトリでClang/LLVMをビルドします。
iOS向けにもビルドする必要があるため、ARM, AArch64ターゲットの指定が必要です。

cmakeコマンド実行時の引数`LLVM_TARGETS_TO_BULD`にターゲットプラットフォームとしてX86, ARM, AArch64を指定します。


```
% cd ..
$ mkdir build_llvm
$ cd build_llvm
$ cmake ../llvm -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD="X86;ARM;AArch64"
$ make -j 4
```

## Clang/LLVMのインストール

ビルドしたClang/LLVMをインストールします。

```
$ sudo make install
```

デフォルトでは/usr/local以下にインストールされますが、他のディレクトリパスにインストールする場合には、以下の環境変数LLVM_HOMEを適切に設定してください。
.bash_profileなどに以下の行を追加して、環境変数を設定します。

```
export LLVM_HOME=<llvm install path>
```

# ispcのインストール

iOS向けにビルドするため aarch64 アーキテクチャのサポートが必要になります。
オリジナルのリポジトリにaarch64対応がmergeされるまでforkしたものを利用します。以下のリポジトリをcloneし、aarch64ブランチに切り替えます。

```
$ git clone https://github.com/DensoITLab/ispc
$ cd ispc
$ git checkout aarch64
```

## bisonのインストール

macOSでは、インストール済みのbisonのバージョンが古いため、brewコマンドにより最新バージョンのbisonをインストールする。

```
$ brew install bison
$ brew link --force bison
```

作業が終わったら，インストール済みのものに戻すために```unlink```しておく．

```
$ brew unlink --force bison
```

## 環境変数の設定

ispcにて利用するLLVMを明示的に指定するため、以下の環境変数を設定します。

```
LLVM_HOME=/usr/local
```

## Makefileの設定

iOS向けのビルドを行うために、ARMターゲットを有効にする必要があります。
ARMターゲットを追加するため、Makefile中のARM_ENABLEDを以下のように設定します。

```
ARM_ENABLED=1
```

## ispcのビルド

makeコマンドでビルドします。
Makefileにinstallターゲットがないため、手動でispcをコピーします。

```
$ make -j 4
$ sudo cp ispc /usr/local/bin
```

# demitasseのビルド

## 依存モジュールのインストール

```
$ brew install protobuf flatbuffers libpng
```

## リポジトリの取得

GitHub Enterpriseのリポジトリからビルド環境にクローンします。
（※　公開前なので、今はhttp://10.81.247.114/skondo/demitasseを利用する）

```
$ git clone https://github.com/DensoITLab/Demitasse
```

## macOS向けビルド

ビルド用のディレクトリ上で、ビルドします。
ビルドには，"Command Line Tools"のインストールが必要です．[Apple](https://developer.apple.com/download/more/)からダウンロードしてください．

```
$ mkdir build_Demitasse
$ cd build_Demitasse
$ cmake ../Demitasse
$ make -j 4
```

## iOS向けビルド

iOS向けデモアプリ、ライブラリビルドでは、Xcodeを利用します。
./demitasse/ios/DemoApp/Demitasse.xcodeprojファイルを開き、アプリおよびライブラリのビルドを行います。
`