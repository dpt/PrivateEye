# CMakeLists.txt
#
# libpng
#
# Copyright (c) David Thomas, 2021
#
# vim: sw=4 ts=8 et

cmake_minimum_required(VERSION 3.18)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

project(png VERSION 1.2.25 DESCRIPTION "IJG libpng" LANGUAGES C)

# The values set in the toolchain file aren't available until this point.

if(TARGET_RISCOS)
    riscos_set_flags()
endif()

# Referencing CMAKE_TOOLCHAIN_FILE avoids a warning on rebuilds.
if(NOT ${CMAKE_TOOLCHAIN_FILE} STREQUAL "")
    message(STATUS "libpng: Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
endif()

set(HEADERS
    png.h
    pngconf.h)

set(SOURCES
    png.c
    pngerror.c
    pngget.c
    pngmem.c
    pngpread.c
    pngread.c
    pngrio.c
    pngrtran.c
    pngrutil.c
    pngset.c
    pngtest.c
    pngtrans.c
    pngvcrd.c
    pngwio.c
    pngwrite.c
    pngwtran.c
    pngwutil.c)

add_library(png)
set_target_properties(png PROPERTIES
    VERSION 1.2.25
    C_STANDARD 90)

if(TARGET_RISCOS)
    target_include_directories(png PRIVATE $ENV{APPENGINE_ROOT}/libs/zlib) # for zlib
endif()
target_sources(png PRIVATE ${HEADERS} ${SOURCES})
target_compile_options(png PRIVATE -Wall -Wextra -pedantic -Wno-unused-parameter)
