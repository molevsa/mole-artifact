cmake_minimum_required(VERSION 3.13)
project(ISToolOBE)

add_library(obe_lib STATIC obe.cpp)
target_link_libraries(obe_lib glog::glog)