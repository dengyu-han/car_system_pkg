add_library(memory_pool_lib STATIC
    src/memory_pool.cpp
)
target_include_directories(memory_pool_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

