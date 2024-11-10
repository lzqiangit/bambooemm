project(test)
find_package(OpenSSL REQUIRED)

add_executable(example example.cpp)
target_include_directories(example PRIVATE ${HEADPATH} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(example PRIVATE header utils hash ${OPENSSL_LIBRARIES})
target_compile_options(example PUBLIC "-mavx2")

add_executable(evaluation evaluation.cpp)
target_include_directories(evaluation PRIVATE ${HEADPATH} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(evaluation PRIVATE header utils hash ${OPENSSL_LIBRARIES})
target_compile_options(evaluation PUBLIC "-mavx2")

add_executable(loadkv loadkv.cpp)
target_include_directories(loadkv PRIVATE ${HEADPATH} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(loadkv PRIVATE header utils ${OPENSSL_LIBRARIES})
target_compile_options(loadkv PUBLIC "-mavx2")

add_executable(encryption encryption.cpp)
target_include_directories(encryption PRIVATE ${HEADPATH} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(encryption PRIVATE header utils ${OPENSSL_LIBRARIES})
target_compile_options(encryption PUBLIC "-mavx2")

add_executable(bambooemm bambooemm.cpp)
target_include_directories(bambooemm PRIVATE ${HEADPATH} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(bambooemm PRIVATE header utils hash ${OPENSSL_LIBRARIES})
target_compile_options(bambooemm PUBLIC "-mavx2")

add_executable(simd simd.cpp)
target_include_directories(simd PUBLIC ${HEADPATH} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(simd PUBLIC header utils hash ${OPENSSL_LIBRARIES})
target_compile_options(simd PUBLIC "-mavx2")

add_executable(segment segment.cpp)
target_include_directories(segment PUBLIC ${HEADPATH} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(segment PUBLIC header utils hash ${OPENSSL_LIBRARIES})
target_compile_options(segment PUBLIC "-mavx2")

add_executable(client client.cpp)
target_include_directories(client PUBLIC ${HEADPATH} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(client PUBLIC header utils hash ${OPENSSL_LIBRARIES})
target_compile_options(client PUBLIC "-mavx2")

add_executable(emmu emmu.cpp)
target_include_directories(emmu PUBLIC ${HEADPATH} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(emmu PUBLIC header utils hash ${OPENSSL_LIBRARIES})
target_compile_options(emmu PUBLIC "-mavx2")