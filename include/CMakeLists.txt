cmake_minimum_required(VERSION 3.20)

add_library(cds INTERFACE)
target_include_directories(cds INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(cds INTERFACE cxx_std_20)