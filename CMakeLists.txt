cmake_minimum_required(VERSION 3.20)
project(ups_server)

set(CMAKE_CXX_STANDARD 14)
set(GCC_COVERAGE_COMPILE_FLAGS "-pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )

add_executable(ups_server main.cpp Constants.cpp Constants.h ServerManager.cpp ServerManager.h Player.cpp Player.h Game.cpp Game.h main.h ValidationUtils.cpp ValidationUtils.h StringUtils.cpp StringUtils.h SendUtils.cpp SendUtils.h)
