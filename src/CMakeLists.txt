cmake_minimum_required(VERSION 3.13)
project(ISTool)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/)
set(THREADS_PREFER_PTHREAD_FLAG ON)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++2a -Ofast")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++2a -O0")
if(DEBUG)
    add_definitions(-DDEBUG)
    message(STATUS "DEBUG mode is ON")
else()
    message(STATUS "DEBUG mode is OFF")
endif()

INCLUDE(FindPkgConfig)
include_directories(include)
find_package(Threads)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    #jsoncpp
    find_package(Jsoncpp)
    set(Jsoncpp_INCLUDE_DIR /usr/include/jsoncpp)
    include_directories(${Jsoncpp_INCLUDE_DIR})

    #glog
    find_package(glog 0.4.0 REQUIRED)

    #Z3
    set(Z3_PATH Z3PATH)
    link_directories(${Z3_PATH}/build)
    set(Z3_FILE libz3.so)
    include_directories(${Z3_PATH}/src/api/c++)
    include_directories(${Z3_PATH}/src/api)

    set(THIRDPARTY_LIBS glog::glog gflags ${Jsoncpp_LIBRARY} ${Z3_FILE} ${CMAKE_THREAD_LIBS_INIT})
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    #jsoncpp
    set(Jsoncpp_LIBRARY /usr/local/lib/libjsoncpp.dylib)
    include_directories(/usr/local/include)

    #glog
    include_directories(/usr/local/opt/glog/include/)
    set(GLOG_FILE /usr/local/opt/glog/lib/libglog.dylib)

    #z3
    set(Z3_PATH Z3PATH)
    set(Z3_FILE /usr/local/lib/libz3.dylib)
    include_directories(${Z3_PATH}/src/api/c++)
    include_directories(${Z3_PATH}/src/api)

    # libs
    set(THIRDPARTY_LIBS ${Jsoncpp_LIBRARY} ${GLOG_FILE} ${Z3_FILE}  ${CMAKE_THREAD_LIBS_INIT})
else()
    message(FATAL_ERROR "Unexceptional system ${CMAKE_SYSTEM_NAME}")
endif()

# subdic
add_subdirectory(basic)
add_subdirectory(sygus)
add_subdirectory(ext)
add_subdirectory(fta)

set(ALL_LIBS ${SyGuS_LIBS} basic_lib ext_lib fta_lib)

add_subdirectory(main)
