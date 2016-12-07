#
# Copyright (C) 2016 Denso IT Laboratory, Inc.
# All Rights Reserved
#
# Denso IT Laboratory, Inc. retains sole and exclusive ownership of all
# intellectual property rights including copyrights and patents related to this
# Software.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of the Software and accompanying documentation to use, copy, modify, merge,
# publish, or distribute the Software or software derived from it for
# non-commercial purposes, such as academic study, education and personal use,
# subject to the following conditions:
#
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
cmake_minimum_required(VERSION 3.5)

include(FindProtobuf)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})

#
# for Caffe protocol buffer file
#
# download caffe.proto
#
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/caffe.proto
  COMMAND curl https://raw.githubusercontent.com/BVLC/caffe/master/src/caffe/proto/caffe.proto -o ${CMAKE_CURRENT_SOURCE_DIR}/caffe.proto
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/$ENV{PROJECT_NAME}.dir/caffe.pb.cc
  COMMAND echo `pwd`
  COMMAND protoc --proto_path=${CMAKE_CURRENT_SOURCE_DIR} --cpp_out=${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/$ENV{PROJECT_NAME}.dir ${CMAKE_CURRENT_SOURCE_DIR}/caffe.proto
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/caffe.proto
)

#
# for Flatbuffers
#
add_custom_command(
  OUTPUT network_model_generated.h
  COMMAND ${FLATBUFFERS_FLATC_EXECUTABLE} -c -o ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/$ENV{PROJECT_NAME}.dir ${CMAKE_CURRENT_SOURCE_DIR}/../src/network_model.fbs
)
# add flatc generated header
include_directories(${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/$ENV{PROJECT_NAME}.dir)

#
# Caffe Tools
#
add_executable(caffe2demitasse
  caffe2demitasse.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/$ENV{PROJECT_NAME}.dir/caffe.pb.cc
  network_model_generated.h
)

target_link_libraries(caffe2demitasse
  pthread
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${PROTOBUF_LIBRARY}
)

add_executable(convert_mean
  convert_mean.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/$ENV{PROJECT_NAME}.dir/caffe.pb.cc
  network_model_generated.h
)

target_link_libraries(convert_mean
  pthread
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${PROTOBUF_LIBRARY}
)

#
# MatConvNet Tools
#
find_package(Matlab)
if (Matlab_FOUND)
  get_filename_component(Matlab_LIBRARY_DIRS ${Matlab_LIBRARIES} DIRECTORY)
  include_directories(${Matlab_INCLUDE_DIRS})
  link_directories(${Matlab_LIBRARY_DIRS})

  add_executable(matconvnet2demitasse
    matconvnet2demitasse.cpp
    network_model_generated.h
  )

  target_link_libraries(matconvnet2demitasse
    mat
    mx
  )
endif()
