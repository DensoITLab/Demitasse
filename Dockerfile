FROM sonsongithub/llvmsrc:3.9

# install tools
run apt-get update
run apt-get install -y --no-install-recommends emacs
run apt-get clean && rm -rf /var/cache/apt/archives/* /var/lib/apt/lists/*

# build llvm locally
run mkdir -p /root/build_llvm
WORKDIR /root/build_llvm
run (cmake ../llvm -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD="X86;AArch64"; make -j 2)
run (make install)

# build ispc
WORKDIR /root/
run export LLVM_HOME=/usr/local/
run git clone https://github.com/DensoITLab/ispc
run (cd ispc; make; cp ispc /usr/local/bin)

# build Demitasse
WORKDIR /root/
run git clone https://github.com/sonsongithub/Demitasse
run (cd Demitasse; git checkout docker_support)
run (cd Demitasse; sed -i -e "s/set (BLAS \"atlas\")/set (BLAS \"open\")/" CMakeLists.txt;)
run mkdir -p build_Demitasse
run (cd build_Demitasse; cmake ../Demitasse; make; make test)
