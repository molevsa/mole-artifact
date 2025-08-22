cmake_minimum_required(VERSION 3.13)
project(ISToolExecutor)
cmake_policy(SET CMP0079 NEW)

add_executable(solver solver.cpp)
target_link_libraries(solver ${ALL_LIBS} ${ALL_LIBS} ${THIRDPARTY_LIBS})