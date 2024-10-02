find_package(OpenSSL REQUIRED)

add_library(header INTERFACE)
target_include_directories(header INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# hash function
add_library(hash STATIC "common/BOBHash.cpp")
target_link_libraries(hash PUBLIC header)

add_library(utils STATIC "common/utils.cpp")
include_directories(utils PUBLIC ${HEADPATH})
target_link_libraries(utils PUBLIC header mysqlclient ${OPENSSL_LIBRARIES})      # ÐèÒªÁ´½Ó  mysqlclient
message(OPENSSL_LIBRARIES: ${OPENSSL_LIBRARIES})

