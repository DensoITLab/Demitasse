FROM sonsongithub/ispc:ditlab

WORKDIR /root/
run git clone https://github.com/sonsongithub/Demitasse
run (cd Demitasse; git checkout docker_support)
run (cd Demitasse; sed -i -e "s/set (BLAS \"atlas\")/set (BLAS \"open\")/" CMakeLists.txt;)
run mkdir -p build_Demitasse
run (cd build_Demitasse; cmake ../Demitasse; make;)
