FROM ubuntu:16.04

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
    libpng16-dev \
    ca-certificates \
    python-dev \
    flex \
    libtinfo-dev \
    libncurses5-dev \
    libatlas-base-dev \
    libopenblas-dev \
    libc6-dev-i386 \
    ccache
run apt-get clean && rm -rf /var/cache/apt/archives/* /var/lib/apt/lists/*

WORKDIR /root
run git clone https://github.com/google/flatbuffers.git
run mkdir -p /root/build_flatbuffers
run (cd /root/build_flatbuffers; cmake ../flatbuffers; make install)

WORKDIR /root
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
