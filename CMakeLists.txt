cmake_minimum_required(VERSION 3.16)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 17)
set(PROJECT_NAME timer)
project(${PROJECT_NAME} VERSION 0.1.0)

if ( CMAKE_COMPILER_IS_GNUCC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

set(SOURCE src/timer.cpp)

add_executable(${PROJECT_NAME} ${SOURCE})
