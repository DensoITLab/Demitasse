FROM densoitlab/demitassebase:1.0

# build llvm locally
run mkdir -p /root/build_llvm
WORKDIR /root/build_llvm
run (cmake ../llvm -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD="X86;AArch64"; make -j 4)
run (make install)

# build ispc
WORKDIR /root/
run export LLVM_HOME=/usr/local/
run git clone https://github.com/DensoITLab/ispc
run (cd ispc; make; cp ispc /usr/local/bin)

# build Demitasse
WORKDIR /root/
run git clone https://github.com/DensoITLab/Demitasse
run (cd Demitasse; sed -i -e "s/set (BLAS \"atlas\")/set (BLAS \"open\")/" CMakeLists.txt;)
run mkdir -p build_Demitasse
run (cd build_Demitasse; cmake ../Demitasse; make; make test)

# copy test script
add https://gist.githubusercontent.com/sonsongithub/879d7ec298004ba41382806ffb518cd3/raw/9177462c9c8224033b7e46b09068b296e8743079/Demitasse_run_example.sh /root/Demitasse_run_example.sh