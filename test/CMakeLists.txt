project(test)
find_package(OpenSSL REQUIRED)

add_executable(example example.cpp)
target_link_libraries(example PRIVATE header hash)
target_compile_options(example PUBLIC "-mavx2")

add_executable(evaluation evaluation.cpp)
target_link_libraries(evaluation PRIVATE header hash)
target_compile_options(evaluation PUBLIC "-mavx2")

add_executable(loadkv loadkv.cpp)
target_include_directories(loadkv PRIVATE ${HEADPATH})
target_link_libraries(loadkv PRIVATE header utils)
target_compile_options(loadkv PUBLIC "-mavx2")

add_executable(encryption encryption.cpp)
target_include_directories(encryption PRIVATE ${HEADPATH})
target_link_libraries(encryption PRIVATE header utils)
target_compile_options(encryption PUBLIC "-mavx2")