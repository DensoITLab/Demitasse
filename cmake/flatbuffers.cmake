cmake_minimum_required(VERSION 2.8)

include(ExternalProject)

ExternalProject_Add(
	FlatBuffers
	GIT_REPOSITORY https://github.com/google/flatbuffers.git
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/lib
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(FlatBuffers source_dir binary_dir)
include_directories(${source_dir}/include)

# set flat buffers tool path
set(ENV{FlatBuffers_BIN_DIR}        ${binary_dir})
set(ENV{FlatBuffers_INCLUDE_DIRS}   ${${source_dir}/include})
message(STATUS "Flatbuffers path: ${FlatBuffers_INCLUDE_DIRS}")

add_library(flatbuffers STATIC IMPORTED)
set_property(
	TARGET flatbuffers
  PROPERTY IMPORTED_LOCATION ${binary_dir}/libflatbuffers.a
)
