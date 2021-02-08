# CMakeLists.txt
#
# exiftags
#
# Copyright (c) David Thomas, 2021
#
# vim: sw=4 ts=8 et

cmake_minimum_required(VERSION 3.18)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

#list(APPEND CMAKE_MODULE_PATH ${APPENGINE_REPO_ROOT}/cmake/Modules)

project(exiftags VERSION 0.0.0 DESCRIPTION "exiftags" LANGUAGES C)

if(TARGET_RISCOS)
    riscos_set_flags()
endif()

# Referencing CMAKE_TOOLCHAIN_FILE avoids a warning on rebuilds.
if(NOT ${CMAKE_TOOLCHAIN_FILE} STREQUAL "")
    message(STATUS "exiftags: Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
endif()

add_library(exiftags)

set_target_properties(exiftags PROPERTIES
    VERSION 0.0.0
    DESCRIPTION "exiftags"
    C_STANDARD 99
    PREFIX ""  # remove 'lib' prefix
    )

set(SOURCES
    asahi.c
    canon.c
    casio.c
    exif.c
    exifgps.c
    exifutil.c
    fuji.c
    jpeg.c
    leica.c
    makers.c
    minolta.c
    nikon.c
    olympus.c
    panasonic.c
    sanyo.c
    sigma.c
    tagdefs.c
    timevary.c)
set(HEADERS
    exif.h
    exifint.h
    jpeg.h
    makers.h
    timevary.h)

set(ALL_SOURCES ${HEADERS} ${SOURCES})

target_compile_options(exiftags PRIVATE -Wall -Wextra -pedantic)

target_sources(exiftags PRIVATE ${ALL_SOURCES})

