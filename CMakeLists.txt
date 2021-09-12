# minimum required CMAKE version
CMAKE_MINIMUM_REQUIRED(VERSION 3.7)

project(remniw)

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

# compile for debug if no build other type is set
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# compiler must be 17 with LLVM
SET(CMAKE_CXX_STANDARD 17)
# add_compile_options(-Wall -Wextra -pedantic)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

######## ANTLR4 ###########
# required if linking to static library
add_definitions(-DANTLR4CPP_STATIC)

# using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR4_WITH_STATIC_CRT OFF)
# add external build for antlrcpp
include(ExternalAntlr4Cpp)
# add antrl4cpp artifacts to project environment
include_directories(${ANTLR4_INCLUDE_DIRS})

# set variable pointing to the antlr tool that supports C++
# this is not required if the jar file can be found under PATH environment
set(ANTLR_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/antlr/antlr-4.9.3-complete.jar)
# add macros to generate ANTLR Cpp code from grammar
find_package(ANTLR REQUIRED)

######## LLVM ###########
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs Core ExecutionEngine Passes native Support)

######## Build remniw ###########
add_subdirectory(thirdparty/olive)
add_subdirectory(src)
add_subdirectory(test)