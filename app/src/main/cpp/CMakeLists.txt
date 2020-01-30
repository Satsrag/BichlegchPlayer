cmake_minimum_required(VERSION 3.4.1)

file(GLOB bichlegch *.cpp)

add_library(
        BichlegchPlayer
        SHARED
        ${bichlegch})

include_directories(${CMAKE_SOURCE_DIR}/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/lib/${CMAKE_ANDROID_ARCH_ABI}")

target_link_libraries(
        BichlegchPlayer
        log
        -Wl,--start-group
        avformat avcodec avfilter avutil swresample swscale
        -Wl,--end-group
        z
        rtmp
        android
        OpenSLES
)