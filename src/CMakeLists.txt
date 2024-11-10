find_package(OpenSSL REQUIRED)

add_library(header INTERFACE)
target_include_directories(header INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# hash function
add_library(hash STATIC "common/BOBHash.cpp")
target_link_libraries(hash PUBLIC header)

add_library(utils STATIC "common/utils.cpp")
include_directories(utils PUBLIC ${HEADPATH} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(utils PUBLIC header mysqlclient ${OPENSSL_LIBRARIES})
message(OPENSSL_INCLUDE_DIR: ${OPENSSL_INCLUDE_DIR})
