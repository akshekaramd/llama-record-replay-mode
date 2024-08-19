# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(testProject VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

include_directories(/home/rocm-user/googletest/googletest/include)

# Compile into a c++ library 
#add_library(lpddr_sim_lib STATIC src/lpddr_sim_wrapper.cpp)

# Add the main executable
add_executable(testProject src/main_2.cpp)

# Add compile options
target_compile_options(testProject PRIVATE -O3)

# Specify the location of the mylib library
set(MYLIB_DIR ${PROJECT_SOURCE_DIR}/lib)

# Link the library
#target_link_libraries(testProject ${MYLIB_DIR}/liblp5pimsim.a m lpddr_sim_lib)
target_link_libraries(testProject ${MYLIB_DIR}/liblp5pimsim.a)
