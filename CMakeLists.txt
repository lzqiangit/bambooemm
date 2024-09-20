cmake_minimum_required(VERSION 3.5)

project(bamboofilters)

set(CMAKE_CXX_FLAGS
    "-fno-strict-aliasing -O3 -std=c++11 -march=core-avx2 ${CMAKE_CXX_FLAGS} -fopenmp"
)
set(CMAKE_EXE_LINKER_FLAGS "-lpthread ${CMAKE_EXE_LINKER_FLAGS}")
set(CMAKE_CXX_FLAGS "-fpermissive ${CMAKE_CXX_FLAGS}")
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/lib)      # 设置库的生成地址

set(HEADPATH ${PROJECT_SOURCE_DIR}/include)

add_subdirectory(src)
add_subdirectory(test)
