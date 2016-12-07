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
if(NOT APPLE)
  return()
endif()

message(STATUS "OSX System root path: ${CMAKE_OSX_SYSROOT}")

find_path(AccelerateFramework_INCLUDE_DIR Accelerate.h
  PATHS ${CMAKE_OSX_SYSROOT}/System/Library/Frameworks/Accelerate.framework/Versions/Current/Headers
  NO_DEFAULT_PATH)

find_library(AccelerateFramework_LIBRARIES
  NAMES Accelerate
  PATHS ${CMAKE_OSX_SYSROOT}/System/Library
              PATH_SUFFIXES Frameworks
              NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AccelerateFramework DEFAULT_MSG
  AccelerateFramework_INCLUDE_DIR
  AccelerateFramework_LIBRARIES)

# set(AccelerateFramework_LINKER_LIBS -lcblas "-framework Accelerate")
