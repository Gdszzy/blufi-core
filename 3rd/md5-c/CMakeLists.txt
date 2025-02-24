cmake_minimum_required(VERSION 3.12)

add_library(md5)
target_sources(md5 PRIVATE ${CMAKE_CURRENT_LIST_DIR}/md5.c)
target_include_directories(md5 PUBLIC ${CMAKE_CURRENT_LIST_DIR})