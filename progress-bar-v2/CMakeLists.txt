cmake_minimum_required(VERSION 3.10)

project(progress-bar)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE "Release")

add_subdirectory(${CMAKE_SOURCE_DIR}/src)

set(CMAKE_INSTALL_PREFIX "/usr/local")

install(FILES ${CMAKE_SOURCE_DIR}/src/include/progressbar-v2.hpp DESTINATION "include/artwork/progress-bar")