cmake_minimum_required(VERSION 3.13)
project(blufi)

# set(CMAKE_C_COMPILER emcc)
# set(CMAKE_CXX_COMPILER em++)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_EXECUTABLE_SUFFIX ".js")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -lembind -sASYNCIFY --post-js=${CMAKE_SOURCE_DIR}/mixin.js")

add_subdirectory(${CMAKE_SOURCE_DIR}/3rd/uaes)
add_subdirectory(${CMAKE_SOURCE_DIR}/3rd/md5-c)

# blufi
add_executable(blufi wasm.cpp msg.cpp dh.cpp blufi.cpp)
set_target_properties(blufi PROPERTIES
    LINK_FLAGS "-O3"
)
target_include_directories(blufi PRIVATE 
    ${CMAKE_SOURCE_DIR}/include 
    3rd/multiprecision-Boost_1_86_0/include
)
target_link_libraries(blufi PRIVATE uaes md5)