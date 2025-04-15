cmake_minimum_required(VERSION 3.13)
project(ISToolExecutor)
cmake_policy(SET CMP0079 NEW)

add_executable(test test.cpp)
target_link_libraries(test ${ALL_LIBS} ${ALL_LIBS} ${THIRDPARTY_LIBS})

add_executable(run_cegis run_cegis.cpp)
target_link_libraries(run_cegis ${ALL_LIBS} ${ALL_LIBS} ${THIRDPARTY_LIBS})

add_executable(run_merge run_merge.cpp)
target_link_libraries(run_merge ${ALL_LIBS} ${ALL_LIBS} ${THIRDPARTY_LIBS})