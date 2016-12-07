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
add_definitions("-std=c++11")

set (ENV{PROJECT_NAME}       demitasse)

if (USE_BLAS)
  if (USE_MKL)
    set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -DUSE_BLAS -DUSE_MKL")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_BLAS -DUSE_MKL")
  elseif(USE_ACCELERATE)
    set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -DUSE_BLAS -DUSE_ACCELERATE")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_BLAS -DUSE_ACCELERATE")
  else()
    set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -DUSE_BLAS")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_BLAS")
  endif()
endif()

# /usr/local/include
include_directories(/usr/local/include)

# vudnn library headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# add flatc generated header
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(
  OUTPUT network_model_generated.h
  COMMAND ${FLATBUFFERS_FLATC_EXECUTABLE} -c
    -o ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/network_model.fbs
)

set (ISPC_OPTIONS	"--opt=disable-assertions")
# set (ISPC_OPTIONS	"--opt=disable-assertions --opt=force-aligned-memory --opt=fast-math")

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_ARCH}/ispc_binary_encode.o
  COMMAND ${ISPC_EXECUTABLE} -O3
    --arch=${ISPC_ARCH}
    --target=${ISPC_TARGET}
    ${ISPC_OPTIONS}
    ${CMAKE_CURRENT_SOURCE_DIR}/ispc_func/ispc_binary_encode.ispc
    -o ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_ARCH}/ispc_binary_encode.o
    -h ${CMAKE_CURRENT_BINARY_DIR}/ispc_binary_encode.hpp
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_ARCH}/ispc_binary_im2col.o
  COMMAND ${ISPC_EXECUTABLE} -O3
    --arch=${ISPC_ARCH}
    --target=${ISPC_TARGET}
    ${ISPC_OPTIONS}
    ${CMAKE_CURRENT_SOURCE_DIR}/ispc_func/ispc_binary_im2col.ispc
    -o ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_ARCH}/ispc_binary_im2col.o
    -h ${CMAKE_CURRENT_BINARY_DIR}/ispc_binary_im2col.hpp
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_ARCH}/ispc_binary_gemm.o
  COMMAND ${ISPC_EXECUTABLE} -O3
  --arch=${ISPC_ARCH}
  --target=${ISPC_TARGET}
  ${ISPC_OPTIONS}
  ${CMAKE_CURRENT_SOURCE_DIR}/ispc_func/ispc_binary_gemm.ispc
  -o ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_ARCH}/ispc_binary_gemm.o
  -h ${CMAKE_CURRENT_BINARY_DIR}/ispc_binary_gemm.hpp
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_ARCH}/ispc_binary_gemv.o
  COMMAND ${ISPC_EXECUTABLE} -O3
    --arch=${ISPC_ARCH}
    --target=${ISPC_TARGET}
    ${ISPC_OPTIONS}
    ${CMAKE_CURRENT_SOURCE_DIR}/ispc_func/ispc_binary_gemv.ispc
    -o ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_ARCH}/ispc_binary_gemv.o
    -h ${CMAKE_CURRENT_BINARY_DIR}/ispc_binary_gemv.hpp
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_ARCH}/ispc_gemv.o
  COMMAND ${ISPC_EXECUTABLE} -O3
    --arch=${ISPC_ARCH}
    --target=${ISPC_TARGET}
    ${ISPC_OPTIONS}
    ${CMAKE_CURRENT_SOURCE_DIR}/ispc_func/ispc_gemv.ispc
    -o ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_ARCH}/ispc_gemv.o
    -h ${CMAKE_CURRENT_BINARY_DIR}/ispc_gemv.hpp
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_ARCH}/ispc_gemm.o
  COMMAND ${ISPC_EXECUTABLE} -O3
    --arch=${ISPC_ARCH}
    --target=${ISPC_TARGET}
    ${ISPC_OPTIONS}
    ${CMAKE_CURRENT_SOURCE_DIR}/ispc_func/ispc_gemm.ispc
    -o ${CMAKE_CURRENT_BINARY_DIR}/${CURRENT_ARCH}/ispc_gemm.o
    -h ${CMAKE_CURRENT_BINARY_DIR}/ispc_gemm.hpp
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ispc_im2col.o
  COMMAND ${ISPC_EXECUTABLE} -O3
    --arch=${ISPC_ARCH}
    --target=${ISPC_TARGET}
    ${ISPC_OPTIONS}
    ${CMAKE_CURRENT_SOURCE_DIR}/ispc_func/ispc_im2col.ispc
    -o ${CMAKE_CURRENT_BINARY_DIR}/ispc_im2col.o
    -h ${CMAKE_CURRENT_BINARY_DIR}/ispc_im2col.hpp
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ispc_lrn.o
  COMMAND ${ISPC_EXECUTABLE} -O3
    --arch=${ISPC_ARCH}
    --target=${ISPC_TARGET}
    ${ISPC_OPTIONS}
    ${CMAKE_CURRENT_SOURCE_DIR}/ispc_func/ispc_lrn.ispc
    -o ${CMAKE_CURRENT_BINARY_DIR}/ispc_lrn.o
    -h ${CMAKE_CURRENT_BINARY_DIR}/ispc_lrn.hpp
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ispc_avg2d.o
  COMMAND ${ISPC_EXECUTABLE} -O3
    --arch=${ISPC_ARCH}
    --target=${ISPC_TARGET}
    ${ISPC_OPTIONS}
    ${CMAKE_CURRENT_SOURCE_DIR}/ispc_func/ispc_avg2d.ispc
    -o ${CMAKE_CURRENT_BINARY_DIR}/ispc_avg2d.o
    -h ${CMAKE_CURRENT_BINARY_DIR}/ispc_avg2d.hpp
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ispc_max2d.o
  COMMAND ${ISPC_EXECUTABLE} -O3
    --arch=${ISPC_ARCH}
    --target=${ISPC_TARGET}
    ${ISPC_OPTIONS}
    ${CMAKE_CURRENT_SOURCE_DIR}/ispc_func/ispc_max2d.ispc
    -o ${CMAKE_CURRENT_BINARY_DIR}/ispc_max2d.o
    -h ${CMAKE_CURRENT_BINARY_DIR}/ispc_max2d.hpp
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ispc_conv2d.o
  COMMAND ${ISPC_EXECUTABLE} -O3
    --arch=${ISPC_ARCH}
    --target=${ISPC_TARGET}
    ${ISPC_OPTIONS}
    ${CMAKE_CURRENT_SOURCE_DIR}/ispc_func/ispc_conv2d.ispc
    -o ${CMAKE_CURRENT_BINARY_DIR}/ispc_conv2d.o
    -h ${CMAKE_CURRENT_BINARY_DIR}/ispc_conv2d.hpp
)

# activation functions
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ispc_identity.o
  COMMAND ${ISPC_EXECUTABLE} -O3
    --arch=${ISPC_ARCH}
    --target=${ISPC_TARGET}
    ${ISPC_OPTIONS}
    ${CMAKE_CURRENT_SOURCE_DIR}/ispc_func/ispc_identity.ispc
    -o ${CMAKE_CURRENT_BINARY_DIR}/ispc_identity.o
    -h ${CMAKE_CURRENT_BINARY_DIR}/ispc_identity.hpp
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ispc_relu.o
  COMMAND ${ISPC_EXECUTABLE} -O3
    --arch=${ISPC_ARCH}
    --target=${ISPC_TARGET}
    ${ISPC_OPTIONS}
    ${CMAKE_CURRENT_SOURCE_DIR}/ispc_func/ispc_relu.ispc
    -o ${CMAKE_CURRENT_BINARY_DIR}/ispc_relu.o
    -h ${CMAKE_CURRENT_BINARY_DIR}/ispc_relu.hpp
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ispc_sigmoid.o
  COMMAND ${ISPC_EXECUTABLE} -O3
    --arch=${ISPC_ARCH}
    --target=${ISPC_TARGET}
    ${ISPC_OPTIONS}
    ${CMAKE_CURRENT_SOURCE_DIR}/ispc_func/ispc_sigmoid.ispc
    -o ${CMAKE_CURRENT_BINARY_DIR}/ispc_sigmoid.o
    -h ${CMAKE_CURRENT_BINARY_DIR}/ispc_sigmoid.hpp
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ispc_softmax.o
  COMMAND ${ISPC_EXECUTABLE} -O3
    --arch=${ISPC_ARCH}
    --target=${ISPC_TARGET}
    ${ISPC_OPTIONS}
    ${CMAKE_CURRENT_SOURCE_DIR}/ispc_func/ispc_softmax.ispc
    -o ${CMAKE_CURRENT_BINARY_DIR}/ispc_softmax.o
    -h ${CMAKE_CURRENT_BINARY_DIR}/ispc_softmax.hpp
)
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ispc_tanh.o
  COMMAND ${ISPC_EXECUTABLE} -O3
    --arch=${ISPC_ARCH}
    --target=${ISPC_TARGET}
    ${ISPC_OPTIONS}
    ${CMAKE_CURRENT_SOURCE_DIR}/ispc_func/ispc_tanh.ispc
    -o ${CMAKE_CURRENT_BINARY_DIR}/ispc_tanh.o
    -h ${CMAKE_CURRENT_BINARY_DIR}/ispc_tanh.hpp
)

add_library(demitasse STATIC
  network_model_generated.h
  ${CMAKE_CURRENT_BINARY_DIR}/ispc_binary_encode.o
  ${CMAKE_CURRENT_BINARY_DIR}/ispc_binary_im2col.o
  ${CMAKE_CURRENT_BINARY_DIR}/ispc_binary_gemm.o
  ${CMAKE_CURRENT_BINARY_DIR}/ispc_binary_gemv.o
  ${CMAKE_CURRENT_BINARY_DIR}/ispc_gemv.o
  ${CMAKE_CURRENT_BINARY_DIR}/ispc_gemm.o
  ${CMAKE_CURRENT_BINARY_DIR}/ispc_im2col.o
  ${CMAKE_CURRENT_BINARY_DIR}/ispc_lrn.o
  ${CMAKE_CURRENT_BINARY_DIR}/ispc_avg2d.o
  ${CMAKE_CURRENT_BINARY_DIR}/ispc_max2d.o
  ${CMAKE_CURRENT_BINARY_DIR}/ispc_conv2d.o
  ${CMAKE_CURRENT_BINARY_DIR}/ispc_identity.o
  ${CMAKE_CURRENT_BINARY_DIR}/ispc_relu.o
  ${CMAKE_CURRENT_BINARY_DIR}/ispc_sigmoid.o
  ${CMAKE_CURRENT_BINARY_DIR}/ispc_softmax.o
  ${CMAKE_CURRENT_BINARY_DIR}/ispc_tanh.o
  binary_encoder.cpp
  blob.cpp
  activation_function.cpp
  activation_layer.cpp
  binary_conv_layer.cpp
  binary_fully_connected_layer.cpp
  conv_layer.cpp
  fully_connected_layer.cpp
  lrn_layer.cpp
  pooling_function.cpp
  pooling_layer.cpp
  model.cpp
  model_serializer.cpp
  mnist.cpp
  utils.cpp
  tasksys.cpp	# from ispc example
)

target_link_libraries(demitasse
  ${demitasse_LINKER_LIBS}
)
