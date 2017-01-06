FROM llvmsrc:3.9

run mkdir -p /root/build_llvm
WORKDIR /root/build_llvm
run (cmake ../llvm -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD="X86;AArch64"; make -j 2)
run (make install)
