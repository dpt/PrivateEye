# CMakeLists.txt
#
# MD5
#
# Copyright (c) David Thomas, 2021
#
# vim: sw=4 ts=8 et
#

cmake_minimum_required(VERSION 3.18)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH $ENV{APPENGINE_ROOT}/cmake/Modules)

project(MD5 VERSION 0.0.0 DESCRIPTION "MD5" LANGUAGES C)

# The values set in the toolchain file aren't available until this point.
if(TARGET_RISCOS)
    riscos_set_flags()
endif()

# Referencing CMAKE_TOOLCHAIN_FILE avoids a warning on rebuilds.
if(NOT ${CMAKE_TOOLCHAIN_FILE} STREQUAL "")
    message(STATUS "MD5: Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
endif()

# Avoid a warning from CMake
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

add_library(MD5)

set_target_properties(MD5 PROPERTIES
    VERSION 0.0.0
    DESCRIPTION "MD5"
    C_STANDARD 90
    ARCHIVE_OUTPUT_NAME md5)

set(PUBLIC_HEADERS md5.h)

# The public headers must be set as properties of the library, not as
# target_sources. The quoting is essential.
set_target_properties(MD5 PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

target_sources(MD5 PRIVATE md5.h md5.c)

target_compile_options(MD5 PRIVATE -Wall -Wextra -pedantic -fno-strict-aliasing)
