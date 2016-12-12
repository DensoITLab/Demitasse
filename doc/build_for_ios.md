How to build Demitasse
-----------------------

## Preparation on host machine

This document shows the procedure for installing various compilers and tools necessary for building an application that uses Demitasse
Build the iOS application on the host machine (macOS).
On the macOS host machine, the following tools are required.

### Commandline tools for XCode

Install [Commandline tools for XCode](https://developer.apple.com/download/more/) beforehand.

### Homebrew

We use the Homebrew package management tool to install various tools.
Please setup Homebrew on the host machine.

### Other

To build demitasse, you need cmake, git, etc. Install it on the host machine with the following brew command.

```
$ brew install cmake git
```

## Installation of clang/LLVM

For building ispc, the latest LLVM is required.
You can obtain and install a binary package from the LLVM official site, or install it from the package management system for each platform.
Since cross-compiling settings for ARM and AArch64 targets are required when you build ispc for a target other than host, such as for iPhone. Here, we explain how to build from source.

### Clone source code

```
$ git clone http://llvm.org/git/llvm.git
$ cd llvm
$ (cd tools/; git clone http://llvm.org/git/clang.git)
$ (cd projects/; git clone http://llvm.org/git/compiler-rt.git)
$ (cd projects/; git clone http://llvm.org/git/openmp.git)
$ (cd projects/; git clone http://llvm.org/git/libcxx.git)
$ (cd projects/; git clone http://llvm.org/git/libcxxabi.git)
```

### Check out the latest branch of each repositories

To use the latest release version 3.9.0 at the present time (2016.10.12), check out the branch as follows.

```
$ git checkout -b release_39 origin/release_39
$ (cd tools/clang; git checkout -b release_39 origin/release_39)
$ (cd projects/compiler-rt; git checkout -b release_39 origin/release_39)
$ (cd projects/openmp; git checkout -b release_39 origin/release_39)
$ (cd projects/libcxx; git checkout -b release_39 origin/release_39)
$ (cd projects/libcxxabi; git checkout -b release_39 origin/release_39)
```

### Switch to ld.gold(for Linux)

When setting up the environment on Linux, switch ld to another one.
You do not have to execute the following operations in macOS.

```
$ sudo update-alternatives --install "/usr/bin/ld" "ld" "/usr/bin/ld.gold" 20
$ sudo update-alternatives --install "/usr/bin/ld" "ld" "/usr/bin/ld.bfd" 10
$ sudo update-alternatives --set ld /usr/bin/ld.gold
```

### Build clang/LLVM

Build it using cmake, build Clang / LLVM in a directory different from the repository.
You need to designated their building targets as ARM, AArch64.

Specify X86, ARM, AArch64 as `LLVM_TARGETS_TO_BULD` when executing the cmake command.

```
% cd ..
$ mkdir build_llvm
$ cd build_llvm
$ cmake ../llvm -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD="X86;ARM;AArch64"
$ make -j 4
```

### Install clang/LLVM

```
$ sudo make install
```

By default it will be installed under `/usr/local`, but if you want to install them to other directory paths, please set the following environment variable `LLVM_HOME `appropriately.
Add the following lines to .bash_profile etc and set environment variables.

```
export LLVM_HOME=<llvm install path>
```

## Installation of ispc

Support for the aarch64 architecture is required to build for iOS.
We will use what we forked until aarch64 correspondence is merged into the original repository. Clone the following repository and switch to aarch64 branch.

```
$ git clone https://github.com/DensoITLab/ispc
$ cd ispc
$ git checkout aarch64
```

### Install bison

In macOS, installed version of bison is old. Install latest bison with brew command.

```
$ brew install bison
$ brew link --force bison
```

After doing the work, set `unlink` to restore the existing one.

```
$ brew unlink --force bison
```

### Set `LLVM_HOME`

In order to explicitly specify the LLVM used for building ispc, set the following environment variables.

```
LLVM_HOME=/usr/local
```

### Edit Makefile

In order to build for iOS, you need to enable ARM target.
Edit `ARM_ENABLED` in Makefile as follows.

```
ARM_ENABLED=1
```

### Build ispc

Build ispc with the make command.
Since there is no install target in the Makefile, we will manually copy ispc.

```
$ make -j 4
$ sudo cp ispc /usr/local/bin
```

## Build Demitasse

### Install dependent modules

Install follows using brew command.

```
$ brew install protobuf flatbuffers libpng
```

### Clone repository

```
$ git clone https://github.com/DensoITLab/Demitasse
```

### Build for macOS

Build Demitasse with a directory different from the repository because we use cmake.
To build, "Command Line Tools" installation is required. Please download from [Apple] (https://developer.apple.com/download/more/).

```
$ mkdir build_Demitasse
$ cd build_Demitasse
$ cmake ../Demitasse
$ make -j 4
```

### Build for iOS

You can build a sample application or Demitasse library for iOS using Xcode. 

Open `./demitasse/ios/DemoApp/Demitasse.xcodeproj` using Xcode.