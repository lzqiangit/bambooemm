add_library(header INTERFACE)
target_include_directories(header INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# hash function
add_library(hash STATIC "common/BOBHash.cpp")
target_link_libraries(hash PUBLIC header)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/client/KV.hpp)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/utils UTILSSRC)
add_library(utils STATIC ${UTILSSRC})
