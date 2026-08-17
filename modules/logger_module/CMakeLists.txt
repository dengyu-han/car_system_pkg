add_library(logger_lib STATIC
    src/logger.cpp
)
target_include_directories(logger_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

