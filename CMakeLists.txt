cmake_minimum_required(VERSION 3.2)

project(libgb)

set(CMAKE_C_STANDARD 11)
set(LIBGB_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(src)
