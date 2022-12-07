cmake_minimum_required(VERSION 3.2)

add_library(libgb libgb.c)

target_include_directories(libgb PRIVATE ${LIBGB_INCLUDE_DIRECTORY})