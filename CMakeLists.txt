cmake_minimum_required(VERSION 3.16)
project(LogX VERSION 1.0.0 LANGUAGES C)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# --- Shared library ---
add_library(logx SHARED
    src/logx.c
    src/logx_config_keys.c
)

# --- Static library ---
add_library(logx_static STATIC
    src/logx.c
    src/logx_config_keys.c
)

# Include dirs
target_include_directories(logx PUBLIC include)
target_include_directories(logx_static PUBLIC include)

# Link dependencies
target_link_libraries(logx PRIVATE yaml cjson)
# Static version usually doesn't link runtime deps; add if needed:
target_link_libraries(logx_static PRIVATE yaml cjson)

# Properties for shared library
set_target_properties(logx PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER
        "include/logx/logx.h;include/logx/logx_config_keys.h;include/logx/logx_defaults.h;include/logx/version.h"
)

# --- Installation paths ---
include(GNUInstallDirs)

# Shared library install
install(TARGETS logx
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/logx
)

# Static library install
install(TARGETS logx_static
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/logx
)

# --- Packaging configuration ---
set(CPACK_PACKAGE_NAME "liblogx")
set(CPACK_PACKAGE_VENDOR "Kulasekaran")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "LogX - Production-grade Logger Library for C")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

# --- Debian-specific settings ---
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Kulasekaran <kulasekaranslrk@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_SECTION "libs")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
set(CPACK_GENERATOR "DEB")

# Control the output file name exactly
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")

# Include CPack last
include(CPack)

