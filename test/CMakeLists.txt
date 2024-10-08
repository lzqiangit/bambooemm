project(test)
find_package(OpenSSL REQUIRED)

add_executable(example example.cpp)
target_include_directories(example PRIVATE ${HEADPATH})
target_link_libraries(example PRIVATE header hash)
target_compile_options(example PUBLIC "-mavx2")

add_executable(evaluation evaluation.cpp)
target_include_directories(evaluation PRIVATE ${HEADPATH})
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

add_executable(bambooemm bambooemm.cpp)
target_include_directories(bambooemm PRIVATE ${HEADPATH})
target_link_libraries(bambooemm PRIVATE header utils hash)
target_compile_options(bambooemm PUBLIC "-mavx2")

add_executable(simd simd.cpp)
target_include_directories(simd PUBLIC ${HEADPATH})
target_link_libraries(simd PUBLIC header utils hash)
target_compile_options(simd PUBLIC "-mavx2")

add_executable(segment segment.cpp)
target_include_directories(segment PUBLIC ${HEADPATH})
target_link_libraries(segment PUBLIC header utils hash)
target_compile_options(segment PUBLIC "-mavx2")