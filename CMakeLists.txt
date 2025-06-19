cmake_minimum_required(VERSION 3.13)
project(blufi)

set(CMAKE_C_COMPILER emcc)
set(CMAKE_CXX_COMPILER em++)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_EXECUTABLE_SUFFIX ".js")

set(FLAGS_COMMON "-Wall -Os")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS_COMMON}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS_COMMON}")

add_subdirectory(${CMAKE_SOURCE_DIR}/3rd/uaes)
add_subdirectory(${CMAKE_SOURCE_DIR}/3rd/md5-c)

# blufi
add_library(core msg.cpp dh.cpp blufi.cpp data_chan.cpp)
target_include_directories(core PUBLIC 
    ${CMAKE_SOURCE_DIR}/include 
    3rd/multiprecision-Boost_1_86_0/include
)
target_link_libraries(core PRIVATE uaes md5)

# wasm
add_executable(blufi wasm.cpp)
target_include_directories(blufi PUBLIC 
    ${CMAKE_SOURCE_DIR}/include 
)
target_link_libraries(blufi PRIVATE core)
target_link_options(
    blufi 
    PRIVATE 
    -lembind 
    -Os 
    --post-js=${CMAKE_SOURCE_DIR}/mixin.js 
    --minify 0 
    -flto 
    -sDYNAMIC_EXECUTION=0 
    -sEXPORT_ES6=1 
    -sENVIRONMENT=webview 
    -sINCOMING_MODULE_JS_API=[instantiateWasm,locateFile,postRun]
    -s EXPORTED_FUNCTIONS=[_free,_malloc]
)