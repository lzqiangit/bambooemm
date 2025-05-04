project(test)
find_package(OpenSSL REQUIRED)

# 将test文件夹下的所有cpp文件创建创建为可执行程序


add_executable(example example.cpp ${COMMON_CPP})
target_include_directories(example PRIVATE ${HEADPATH} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(example PUBLIC mysqlclient ${OPENSSL_LIBRARIES} ${XXHASH_LIB})
target_compile_options(example PUBLIC "-mavx2")

add_executable(demo demo.cpp ${COMMON_CPP})
message(COMMON_CPP: ${COMMON_CPP})
target_include_directories(demo PUBLIC ${HEADPATH} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(demo PUBLIC mysqlclient ${OPENSSL_LIBRARIES} ${XXHASH_LIB})
target_compile_options(demo PUBLIC "-mavx2")

add_executable(time time.cpp ${COMMON_CPP})
target_include_directories(time PUBLIC ${HEADPATH} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(time PUBLIC mysqlclient ${OPENSSL_LIBRARIES} ${XXHASH_LIB})
target_compile_options(time PUBLIC "-mavx2")

add_executable(twoch twoch.cpp ${COMMON_CPP})
target_include_directories(twoch PUBLIC ${HEADPATH} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(twoch PUBLIC mysqlclient ${OPENSSL_LIBRARIES} ${XXHASH_LIB})
target_compile_options(twoch PUBLIC "-mavx2")

add_executable(testutils testutils.cpp ${COMMON_CPP})
target_include_directories(testutils PUBLIC ${HEADPATH} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(testutils PUBLIC mysqlclient ${OPENSSL_LIBRARIES} ${XXHASH_LIB})
target_compile_options(testutils PUBLIC "-mavx2")