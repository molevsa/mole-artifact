cmake_minimum_required(VERSION 3.13)
project(ISToolFTA)


file(GLOB fta_src "*.cpp" "*.h")
add_library(fta_lib STATIC ${fta_src})
target_link_libraries(fta_lib glog::glog)
