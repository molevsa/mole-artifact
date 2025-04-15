cmake_minimum_required(VERSION 3.13)
project(ISToolBasic)

file(GLOB basic_src  "*.cpp" "*.h")
add_library(basic_lib STATIC ${basic_src})
target_link_libraries(basic_lib glog::glog)
