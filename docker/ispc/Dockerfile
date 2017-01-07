FROM llvm:3.9

run export LLVM_HOME=/usr/local/

WORKDIR /root/
run git clone https://github.com/DensoITLab/ispc
run (cd ispc; make; cp ispc /usr/local/bin)
