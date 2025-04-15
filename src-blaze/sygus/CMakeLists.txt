cmake_minimum_required(VERSION 3.13)
project(ISToolSyGuS)

add_subdirectory(theory)
aux_source_directory(parser sygus_parser_src)
add_library(sygus_parser_lib ${sygus_parser_src} sygus.cpp)
target_link_libraries(sygus_parser_lib glog::glog)
set(SyGuS_LIBS sygus_parser_lib sygus_theory_lib PARENT_SCOPE)
