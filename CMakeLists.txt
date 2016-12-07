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

# add cmake module path
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

#
# compiler options
#
set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -O3")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

add_definitions("-std=c++11")

if (APPLE)
  message( STATUS "build for OSX")
  set (CMAKE_MACOSX_RPATH ON)

  # on OS X, use Clang and libc++
  set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -stdlib=libc++")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
else()
  message( STATUS "build for Linux")
endif()

if (DEBUG)
  set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -DDEBUG")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG")
endif()

#
# check required sub modules
#
set(demitasse_LINKER_LIBS "")

find_package(ispc REQUIRED)
find_package(flatbuffers REQUIRED)
find_package(PNG)

#
# libpng
#
if (PNG_FOUND)
  set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -DUSE_LIBPNG")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_LIBPNG")
  include_directories(${PNG_INCLUDE_DIR})
  link_directories(${PNG_LIBRARY_DIR})
  list(APPEND demitasse_LINKER_LIBS "z")
  list(APPEND demitasse_LINKER_LIBS "png")
endif()

#
# thread library
#
list(APPEND demitasse_LINKER_LIBS "pthread")

#
# BLAS library
# atlas, open, mkl
#
set (BLAS "atlas")
set (USE_BLAS 1)

if(USE_BLAS)
  if(NOT APPLE)
    if(BLAS STREQUAL "Atlas" OR BLAS STREQUAL "atlas")
      find_package(Atlas REQUIRED)
      include_directories(${Atlas_CBLAS_INCLUDE_DIR})
      link_directories(${Atlas_LIBRARY_DIR})
      list(APPEND demitasse_LINKER_LIBS "atlas")
    elseif(BLAS STREQUAL "Open" OR BLAS STREQUAL "open")
      find_package(OpenBLAS REQUIRED)
      include_directories(${OpenBLAS_CBLAS_INCLUDE_DIR})
      link_directories(${OpenBLAS_LIBRARY_DIR})
      list(APPEND demitasse_LINKER_LIBS "openblas")
    elseif(BLAS STREQUAL "MKL" OR BLAS STREQUAL "mkl")
      find_package(MKL REQUIRED)
      set (USE_MKL 1)
      include_directories(${MKL_INCLUDE_DIR})
      link_directories(${MKL_LIBRARY_DIR})
      list(APPEND demitasse_LINKER_LIBS "mkl_rt")
    endif()
  else()
     find_package(AccelerateFramework REQUIRED)
     set (USE_ACCELERATE 1)
     list(APPEND demitasse_LINKER_LIBS "-framework Accelerate")
  endif()
endif()

# build target
add_subdirectory(src)
add_subdirectory(examples)

# model convert tools
if(NOT without-tools)
  add_subdirectory(tools)
endif()

# tests
if(NOT without-test)
  enable_testing()
  include(cmake/gtest.cmake)
  add_subdirectory(test)
endif()
