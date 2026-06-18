cmake_minimum_required(VERSION 3.16)

# =====================================
# Read version from version.h
# =====================================

set(VERSION_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/logx/version.h")

file(READ "${VERSION_HEADER}" VERSION_CONTENTS)

string(REGEX MATCH "#define[ \t]+LOGX_MAJOR_VERSION[ \t]+([0-9]+)" _ "${VERSION_CONTENTS}")
set(LOGX_MAJOR_VERSION "${CMAKE_MATCH_1}")

string(REGEX MATCH "#define[ \t]+LOGX_MINOR_VERSION[ \t]+([0-9]+)" _ "${VERSION_CONTENTS}")
set(LOGX_MINOR_VERSION "${CMAKE_MATCH_1}")

string(REGEX MATCH "#define[ \t]+LOGX_PATCH_VERSION[ \t]+([0-9]+)" _ "${VERSION_CONTENTS}")
set(LOGX_PATCH_VERSION "${CMAKE_MATCH_1}")

set(LOGX_VERSION
    "${LOGX_MAJOR_VERSION}.${LOGX_MINOR_VERSION}.${LOGX_PATCH_VERSION}"
)

project(LogX VERSION "${LOGX_VERSION}" LANGUAGES C)

# =====================================
# Global build settings
# =====================================

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(GNUInstallDirs)

# =====================================
# Dependencies
# =====================================
add_subdirectory(deps/cJSON)
add_subdirectory(deps/libyaml)
set(ZLIB_BUILD_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(deps/zlib)

# =====================================
# Public headers
# =====================================

file(GLOB LOGX_PUBLIC_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/logx/*.h")

# =====================================
# Subdirectories
# =====================================

add_subdirectory(logx)
add_subdirectory(tests)
add_subdirectory(examples)
add_subdirectory(benchmarks)

# =====================================
# CPack configuration
# =====================================

set(CPACK_PACKAGE_NAME "liblogx")
set(CPACK_PACKAGE_VENDOR "Kulasekaran")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "LogX - Production-grade Logger Library in C for Linux"
)

set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

set(CPACK_RESOURCE_FILE_LICENSE
    "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
)

# Debian
set(CPACK_GENERATOR "DEB")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER
    "Kulasekaran <kulasekaranslrk@gmail.com>"
)

set(CPACK_DEBIAN_PACKAGE_SECTION "libs")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

# Runtime dependencies
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "libyaml-0-2, libcjson1"
)

set(CPACK_PACKAGE_FILE_NAME
    "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}"
)

include(CPack)