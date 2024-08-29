# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(pim_simulator VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(googletest)

# Add the main executable
add_executable(pim_simulator src/pim_simulator_main.cpp)

# Add compile options
target_compile_options(pim_simulator PRIVATE -O3)

# Specify the location of the mylib library
set(MYLIB_DIR ${PROJECT_SOURCE_DIR}/lib)

# Link the library
target_link_libraries(pim_simulator gtest gtest_main ${MYLIB_DIR}/liblp5pimsim.a)
