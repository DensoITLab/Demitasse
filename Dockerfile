FROM 1of0/llvm:latest

# move
WORKDIR /root/

# indepent packages
run apt-get update && apt-get install -y --no-install-recommends \
    apt-utils \
    build-essential \
    curl \
    git \
    libprotobuf-dev \
    protobuf-compiler \
    bison \
    libpng12-dev \
    ca-certificates \
    python-dev \
    flex \
    libtinfo-dev \
    libncurses5-dev \
    libatlas-base-dev \
    libopenblas-dev \
    libc6-dev-i386 \
    ccache \
    && apt-get clean && rm -rf /var/cache/apt/archives/* /var/lib/apt/lists/*

# latest cmake
run (curl -O https://cmake.org/files/v3.7/cmake-3.7.1.tar.gz; tar zxf cmake-3.7.1.tar.gz; cd cmake-3.7.1; ./configure; make install;)

# flatbuffers
run (git clone https://github.com/google/flatbuffers.git; mkdir -p /root/build_flatbuffers; cd build_flatbuffers; cmake ../flatbuffers; make install)

# ispc
run (git clone https://github.com/DensoITLab/ispc; cd ispc; make; cp ispc /usr/local/bin)

# Demitasse
run git clone https://github.com/DensoITLab/Demitasse
run (cd Demitasse; sed -i -e "s/set (BLAS \"atlas\")/set (BLAS \"open\")/" CMakeLists.txt;)
run mkdir -p /root/build_Demitasse
run (cd build_Demitasse; cmake ../Demitasse; make;)
