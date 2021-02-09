# CMakeLists.txt
#
# zlib
#
# Copyright (c) David Thomas, 2021
#
# vim: sw=4 ts=8 et

cmake_minimum_required(VERSION 3.18)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

project(zlib VERSION 1.2.25 DESCRIPTION "IJG zlib" LANGUAGES C)

# The values set in the toolchain file aren't available until this point.

if(TARGET_RISCOS)
    riscos_set_flags()
endif()

# Referencing CMAKE_TOOLCHAIN_FILE avoids a warning on rebuilds.
if(NOT ${CMAKE_TOOLCHAIN_FILE} STREQUAL "")
    message(STATUS "zlib: Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
endif()

set(HEADERS
    deflate.h
    inffast.h
    inffixed.h
    inflate.h
    inftrees.h
    trees.h
    zconf.h
    zlib.h
    zutil.h)

set(SOURCES
    adler32.c
    compress.c
    crc32.c
    gzio.c
    uncompr.c
    deflate.c
    trees.c
    zutil.c
    inflate.c
    infback.c
    inftrees.c
    inffast.c)

add_library(zlib)
set_target_properties(zlib PROPERTIES
    VERSION 1.2.25
    C_STANDARD 90)

target_sources(zlib PRIVATE ${HEADERS} ${SOURCES})
target_compile_options(zlib PRIVATE -Wall -Wextra -pedantic) # -Wno-unused-parameter)
