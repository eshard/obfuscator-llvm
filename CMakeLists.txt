cmake_minimum_required(VERSION 3.20)

project(llvm-obfuscator)

if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

add_definitions("-D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS")

add_library(LLVMObfuscator SHARED Plugin.cpp)

target_include_directories(LLVMObfuscator PRIVATE ${LLVM_INCLUDE_DIRS})
target_include_directories(LLVMObfuscator PRIVATE ${CMAKE_SOURCE_DIR})

add_subdirectory(utils)

add_subdirectory(bogus)
add_subdirectory(flattening)
add_subdirectory(split)
add_subdirectory(substitution)
