cmake_minimum_required(VERSION 3.5)
project(bamboofilters)

set(CMAKE_CXX_FLAGS
    "-fno-strict-aliasing -O0 -std=c++11 -march=core-avx2 ${CMAKE_CXX_FLAGS} -fopenmp"
)
set(CMAKE_EXE_LINKER_FLAGS "-lpthread ${CMAKE_EXE_LINKER_FLAGS}")
set(CMAKE_CXX_FLAGS "-fpermissive ${CMAKE_CXX_FLAGS}")      # -fsanitize=address -fno-omit-frame-pointer
set(CMAKE_BUILD_TYPE "Debug")
# set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/lib)      # ���ÿ�����ɵ�ַ
set(XXHASH_LIB "/usr/lib/x86_64-linux-gnu/libxxhash.a")

set(HEADPATH ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src/bamboofilter ${PROJECT_SOURCE_DIR}/src/bambooemm)
# 将common 目录下的cpp文件加入COMMON_CPP 变量中
file(GLOB COMMON_CPP ${PROJECT_SOURCE_DIR}/src/common/*.cpp)



add_subdirectory(src)
add_subdirectory(test)
 