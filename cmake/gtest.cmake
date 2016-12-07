cmake_minimum_required(VERSION 3.5)

include(ExternalProject)

ExternalProject_Add(
	GoogleTest-master
	GIT_REPOSITORY https://github.com/google/googletest.git
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/lib
	LOG_DOWNLOAD ON
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
)

# Build gtest from existing sources
ExternalProject_Add(
	googletest
	DOWNLOAD_COMMAND "" # No download required
  SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib/src/GoogleTest-master/googletest" # Use specific source dir
  PREFIX "${CMAKE_CURRENT_BINARY_DIR}/lib/GoogleTest-master" # But use prefix for compute other dirs
  INSTALL_COMMAND ""
  LOG_CONFIGURE ON
  LOG_BUILD ON
)

# gtest should be build after being downloaded
add_dependencies(googletest GoogleTest-master)

ExternalProject_Get_Property(googletest source_dir binary_dir)
include_directories(${source_dir}/include)

add_library(gtest STATIC IMPORTED)
set_property(
	TARGET gtest
	PROPERTY IMPORTED_LOCATION ${binary_dir}/libgtest.a
)
add_library(gtest_main STATIC IMPORTED)
set_property(
	TARGET gtest_main
	PROPERTY IMPORTED_LOCATION ${binary_dir}/libgtest_main.a
)
