cmake_minimum_required(VERSION 3.13)
project(ISToolSyGuSTheory)

aux_source_directory(basic/clia BASIC_CLIA_SRC)
aux_source_directory(basic/string BASIC_STRING_SRC)
aux_source_directory(basic/bv BASIC_BV_SRC)
set(BASIC_SRC ${BASIC_CLIA_SRC} ${BASIC_STRING_SRC} ${BASIC_BV_SRC} basic/theory_semantics.cpp)
aux_source_directory(z3/clia Z3_CLIA_SRC)
aux_source_directory(z3/bv Z3_BV_SRC)
set(Z3_SRC ${Z3_CLIA_SRC} ${Z3_BV_SRC} z3/theory_z3_semantics.cpp)

add_library(sygus_theory_lib STATIC ${BASIC_SRC} ${Z3_SRC} theory.cpp)
target_link_libraries(sygus_theory_lib glog::glog)
