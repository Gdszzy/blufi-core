cmake_minimum_required(VERSION 3.13)
project(blufi)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF)

set(FLAGS_COMMON "-Wall -Os -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS_COMMON}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS_COMMON}")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rd/uaes)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rd/md5-c)

# blufi
add_library(core msg.cpp dh.cpp blufi.cpp)
target_include_directories(core PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
)
target_include_directories(core PRIVATE 
    3rd/multiprecision-Boost_1_86_0/include
)
target_link_libraries(core PRIVATE uaes md5)

# wasm
add_executable(wasm wasm.cpp)
set_target_properties(wasm PROPERTIES OUTPUT_NAME "blufi")
target_include_directories(wasm PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
)
target_link_libraries(wasm PRIVATE core)
target_link_options(
    wasm 
    PRIVATE 
    -lembind 
    -Os 
    --post-js=${CMAKE_CURRENT_SOURCE_DIR}/mixin.js 
    --minify 0 
    -flto 
    -sDYNAMIC_EXECUTION=0 
    -sEXPORT_ES6=1 
    -sENVIRONMENT=webview 
    -sINCOMING_MODULE_JS_API=[instantiateWasm,locateFile,postRun]
    -s EXPORTED_FUNCTIONS=[_free,_malloc]
)

# android
add_library(android SHARED android.cpp)
set_target_properties(android PROPERTIES OUTPUT_NAME "blufi")
target_include_directories(android PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
)
target_link_libraries(android PRIVATE core)
add_custom_command(TARGET android POST_BUILD
    COMMAND "$ENV{STRIP}" "$<TARGET_FILE:android>"
    COMMENT "Stripping symbols from shared library"
)
