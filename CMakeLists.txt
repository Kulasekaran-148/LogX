cmake_minimum_required(VERSION 3.16)

# Path to version header
set(VERSION_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/logx/version.h")

# Read version header file
file(READ "${VERSION_HEADER}" VERSION_CONTENTS)

# Extract major, minor, patch using regex
string(REGEX MATCH "#define[ \t]+LOGX_MAJOR_VERSION[ \t]+([0-9]+)" _ "${VERSION_CONTENTS}")
set(LOGX_MAJOR_VERSION "${CMAKE_MATCH_1}")

string(REGEX MATCH "#define[ \t]+LOGX_MINOR_VERSION[ \t]+([0-9]+)" _ "${VERSION_CONTENTS}")
set(LOGX_MINOR_VERSION "${CMAKE_MATCH_1}")

string(REGEX MATCH "#define[ \t]+LOGX_PATCH_VERSION[ \t]+([0-9]+)" _ "${VERSION_CONTENTS}")
set(LOGX_PATCH_VERSION "${CMAKE_MATCH_1}")

# Combine into full version string
set(LOGX_VERSION "${LOGX_MAJOR_VERSION}.${LOGX_MINOR_VERSION}.${LOGX_PATCH_VERSION}")

# Use extracted version
project(LogX VERSION "${LOGX_VERSION}" LANGUAGES C)

#project(LogX VERSION 1.0.2 LANGUAGES C)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# --- Auto-collect .c and .h files ---
file(GLOB LOGX_SOURCES "${PROJECT_SOURCE_DIR}/src/logx/*.c")
file(GLOB LOGX_PUBLIC_HEADERS "${PROJECT_SOURCE_DIR}/include/logx/*.h")

# --- Shared library ---
add_library(logx SHARED ${LOGX_SOURCES})

# --- Static library ---
add_library(logx_static STATIC ${LOGX_SOURCES})

# Includes
target_include_directories(logx PUBLIC include)
target_include_directories(logx_static PUBLIC include)

# Dependencies
target_link_libraries(logx PRIVATE yaml cjson)
target_link_libraries(logx_static PRIVATE yaml cjson)

# --- LogX Daemon executable ---
add_executable(logxd
    src/logxd/logxd.c
)

target_include_directories(logxd PRIVATE include)
target_link_libraries(logxd PRIVATE logx pthread)

# --- LogX CLI executable ---
add_executable(logx-cli
    src/logxd/logx_cli.c
)

target_include_directories(logx-cli PRIVATE include)
target_link_libraries(logx-cli PRIVATE logx pthread)

# --- Properties for shared library ---
set_target_properties(logx PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "${LOGX_PUBLIC_HEADERS}"
)

# --- Install paths ---
include(GNUInstallDirs)

install(TARGETS logx
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/logx
)

install(TARGETS logx_static
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/logx
)

install(TARGETS logxd
    RUNTIME DESTINATION sbin
)

install(TARGETS logx-cli
    RUNTIME DESTINATION bin
)

install(PROGRAMS
    src/logxd/logxd.sh
    DESTINATION lib/logx
)

install(
    FILES src/logxd/logxd.service
    DESTINATION lib/systemd/system
)

# --- Packaging configuration ---
set(CPACK_PACKAGE_NAME "liblogx")
set(CPACK_PACKAGE_VENDOR "Kulasekaran")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "LogX - Production-grade Logger Library in C for Linux")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

# --- Debian-specific settings ---
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Kulasekaran <kulasekaranslrk@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libyaml-dev, libcjson-dev")
set(CPACK_DEBIAN_PACKAGE_SECTION "libs")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
set(CPACK_GENERATOR "DEB")

# Control the output file name exactly
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")

set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${CMAKE_SOURCE_DIR}/scripts/logxd_postinst"
    "${CMAKE_SOURCE_DIR}/scripts/logxd_prerm"
    "${CMAKE_SOURCE_DIR}/scripts/logxd_postrm"
)

# Include CPack last
include(CPack)

