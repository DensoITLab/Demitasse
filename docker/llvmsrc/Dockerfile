FROM ubuntu:16.04

#ENV http_proxy http://10.81.247.8:8080
#ENV https_proxy http://10.81.247.8:8080

run apt-get update
run apt-get install -y --no-install-recommends \
    apt-utils \
    build-essential \
    cmake \
    curl \
    git \
    libprotobuf-dev \
    protobuf-compiler \
    bison \
    libpng15-dev \
    ca-certificates \
    python-dev \
    flex \
    libtinfo-dev \
    libncurses5-dev \
    libatlas-base-dev \
    libopenblas-dev \
    libc6-dev-i386 \
    ccache

RUN apt-get install -y wget

RUN wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
COPY apt/llvm.list /etc/apt/sources.list.d/llvm.list
COPY apt/llvm.pref /etc/apt/preferences.d/llvm
run echo 'deb http://us.archive.ubuntu.com/ubuntu precise main universe' > /etc/apt/sources.list

RUN apt-get update
RUN apt-get upgrade -y

run apt-get install -y --no-install-recommends \
    llvm-3.9-dev \
    clang-3.9 \
    libllvm3.9 \
    libclang-3.9-dev \
    llvm-3.9 \
    llvm-3.9-tools \
    zlib1g-dev \
    libncurses5-dev

run apt-get clean && rm -rf /var/cache/apt/archives/* /var/lib/apt/lists/*

WORKDIR /root
run git clone https://github.com/google/flatbuffers.git
run mkdir -p /root/flatbuffers/build
WORKDIR /root/flatbuffers/build
run cmake ..
run make install

run git clone http://llvm.org/git/llvm.git
run (cd llvm/tools/; git clone http://llvm.org/git/clang.git)
run (cd llvm/projects/; git clone http://llvm.org/git/compiler-rt.git)
run (cd llvm/projects/; git clone http://llvm.org/git/openmp.git)
run (cd llvm/projects/; git clone http://llvm.org/git/libcxx.git)
run (cd llvm/projects/; git clone http://llvm.org/git/libcxxabi.git)

run (cd llvm; git checkout -b release_39 origin/release_39)
run (cd llvm/tools/clang; git checkout -b release_39 origin/release_39)
run (cd llvm/projects/compiler-rt; git checkout -b release_39 origin/release_39)
run (cd llvm/projects/openmp; git checkout -b release_39 origin/release_39)
run (cd llvm/projects/libcxx; git checkout -b release_39 origin/release_39)
run (cd llvm/projects/libcxxabi; git checkout -b release_39 origin/release_39)

run export LLVM_HOME=/usr/local/

WORKDIR /root/
run git clone https://github.com/DensoITLab/ispc
run (cd ispc; make; cp ispc /usr/local/bin)
