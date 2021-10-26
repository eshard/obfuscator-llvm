cmake_minimum_required(VERSION 3.13)

project(llvm-obfuscator)

find_package(LLVM PATHS "${LLVM_DIR}" REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})
link_directories(${LLVM_LIBRARY_DIRS})

add_definitions("-D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS")

# Depends on how clang was compiled, by default it's set to C++14
# https://llvm.org/docs/CMake.html#rarely-used-cmake-variables
set(CMAKE_CXX_STANDARD 14)

if (LLVM_ENABLE_LIBCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

add_library(LLVMObfuscator SHARED Plugin.cpp)

target_include_directories(LLVMObfuscator PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(LLVMObfuscator PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

option(BUILD_DUMMY "Build dummy plugin" OFF)
if(BUILD_DUMMY)
  add_library(LLVMDummy SHARED Dummy.cpp)
endif(BUILD_DUMMY)

add_subdirectory(utils)

add_subdirectory(bogus)
add_subdirectory(flattening)
add_subdirectory(split)
add_subdirectory(substitution)
