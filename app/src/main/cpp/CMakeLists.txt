
cmake_minimum_required(VERSION 3.4.1)

message("当前cmakel路径: ${CMAKE_SOURCE_DIR} \n cpu架构：${CMAKE_ANDROID_ARCH_ABI}")

# 将 cpp 文件夹下所有的源文件定义成了 SOURCE（后面的源文件使用相对路径）
file(GLOB SOURCE ./*.cpp )

add_library( # Sets the name of the library.
        native-lib
        # Sets the library as a shared library.
        SHARED
        # 引入 SOURCE 下的所有源文件
        ${SOURCE}
        )

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# 引入 FFmpeg 的头文件
include_directories(${CMAKE_SOURCE_DIR}/include)

# 引入外部静态库，直接给 cmake 在添加一个查找路径，在这个路径下可以找到所以的静态库
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/${CMAKE_ANDROID_ARCH_ABI}")

target_link_libraries( # Specifies the target library.
        native-lib
        # 添加依赖的静态库
        avformat
        avcodec
        avfilter
        avutil
        swresample
        swscale

        ${log-lib}
        z
        android
        OpenSLES)
