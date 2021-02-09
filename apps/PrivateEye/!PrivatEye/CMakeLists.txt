# CMakeLists.txt
#
# PrivateEye
#
# Copyright (c) David Thomas, 2021
#
# vim: sw=4 ts=8 et

# TODO
#
# - Have seen doubling of linker args
# - Have seen doubling of compiler/assembler args
# - Installation

cmake_minimum_required(VERSION 3.18)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

set(APPENGINE_REPO_ROOT $ENV{APPENGINE_ROOT})

list(APPEND CMAKE_MODULE_PATH ${APPENGINE_REPO_ROOT}/cmake/Modules)

include(FetchContent)


project(PrivateEye VERSION 3.1.0 DESCRIPTION "Image viewer" LANGUAGES C ASM_ASASM)

# If a host tools build was requested, then build them alone
set(TEMPLHEADR_DIR ${APPENGINE_REPO_ROOT}/utils/templheader)
if(HOST_TOOLS_ONLY)
    message(STATUS "PrivateEye: Building host tools only")
    add_subdirectory(${TEMPLHEADR_DIR} templheader-bin)
    return()
endif()

# The values set in the toolchain file aren't available until this point.
if(NOT DEFINED TARGET_RISCOS)
    message(FATAL_ERROR "PrivateEye builds for RISC OS only")
endif()

if(TARGET_RISCOS)
    riscos_set_flags()
endif()

# Referencing CMAKE_TOOLCHAIN_FILE avoids a warning on rebuilds.
if(NOT ${CMAKE_TOOLCHAIN_FILE} STREQUAL "")
    message(STATUS "PrivateEye: Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
endif()

# Build the native 'host' tools (...) in this dir
set(HOSTTOOLSDIR "${CMAKE_BINARY_DIR}/host")

# Create a directory in which to build the host tools
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${HOSTTOOLSDIR}")

# Generate the host tools build
#
# We use -DCMAKE_C_COMPILER to force the use of the system compiler, otherwise
# the child build may inherit a cross-compiler via the CC environment variable.
# This is bound to break on Windows.
execute_process(COMMAND ${CMAKE_COMMAND}
    -G "${CMAKE_GENERATOR}"
    -DCMAKE_BUILD_TYPE='Release'
    -DCMAKE_C_COMPILER='cc'
    -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
    -DHOST_TOOLS_ONLY=1
    ${CMAKE_SOURCE_DIR}
    WORKING_DIRECTORY "${HOSTTOOLSDIR}")

# Build the host tools (by recursing)
execute_process(COMMAND ${CMAKE_COMMAND} --build . --target templheadr WORKING_DIRECTORY "${HOSTTOOLSDIR}")

# Build PrivateEye itself
message(STATUS "PrivateEye: Building targets only")

# Libs... (set this before imports)
#
set(LIBS_DIR ${CMAKE_SOURCE_DIR}/../../../libs)

# Import DPTLib
FetchContent_Declare(DPTLib
    GIT_REPOSITORY $ENV{HOME}/SyncProjects/github/DPTLib
    GIT_TAG HEAD)

FetchContent_MakeAvailable(DPTLib)

# Import AppEngine
set(APPENGINE_DIR ${LIBS_DIR}/appengine)
set(APPENGINE_INCLUDE ${APPENGINE_DIR}/../)
add_subdirectory(${APPENGINE_DIR} appengine-build)

# Import libjpeg
set(JPEG_DIR ${LIBS_DIR}/jpeg)
set(JPEG_INCLUDE ${JPEG_DIR})
add_subdirectory(${JPEG_DIR} jpeg-build)

# Import exiftags
set(EXIFTAGS_DIR ${LIBS_DIR}/exiftags)
set(EXIFTAGS_INCLUDE ${EXIFTAGS_DIR})
add_subdirectory(${EXIFTAGS_DIR} exiftags-build)

# Import flex
set(FLEX_DIR ${LIBS_DIR}/flex)
set(FLEX_INCLUDE ${FLEX_DIR})
add_subdirectory(${FLEX_DIR} flex-build)

# Import MD5
set(MD5_DIR ${LIBS_DIR}/md5)
set(MD5_INCLUDE ${MD5_DIR})
add_subdirectory(${MD5_DIR} md5-build)

# Import libpng
set(PNG_DIR ${LIBS_DIR}/png)
set(PNG_INCLUDE ${PNG_DIR})
add_subdirectory(${PNG_DIR} png-build)

# Import zlib
set(ZLIB_DIR ${LIBS_DIR}/zlib)
set(ZLIB_INCLUDE ${ZLIB_DIR})
add_subdirectory(${ZLIB_DIR} zlib-build)


# Avoid a warning from CMake
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

add_executable(PrivateEye)

target_include_directories(PrivateEye
    PRIVATE
    ${APPENGINE_INCLUDE}
    ${DPTLIB_INCLUDE}
    ${EXIFTAGS_INCLUDE}
    ${FLEX_INCLUDE}
    ${JPEG_INCLUDE}
    ${MD5_INCLUDE}
    ${PNG_INCLUDE}
    ${ZLIB_INCLUDE}
    ${CMAKE_SOURCE_DIR})

target_link_libraries(PrivateEye
    PRIVATE
    ${EXIFTAGS_LIB}
    ${FLEX_LIB}
    ${JPEGTRAN_LIB}
    ${JPEG_LIB}
    ${MD5_LIB}
    ${PNG_LIB}
    ${ZLIB_LIB}
    AppEngine
    DPTLib)

set_target_properties(PrivateEye PROPERTIES
    VERSION 3.1.0
    DESCRIPTION "Image viewer"
    C_STANDARD 99
    OUTPUT_NAME_DEBUG PrivateEye-debug
    OUTPUT_NAME_RELEASE PrivateEye
    OUTPUT_NAME_RELWITHDEBINFO PrivateEye-relwithdebinfo
    OUTPUT_NAME_MINSIZEREL PrivateEye-minsizerel)

set(ALL_HEADERS
    actions.h
    canvas.h
    choices-colour.h
    choicesdat.h
    clipboard.h
    dataxfer.h
    display.h
    effects.h
    ffg.h
    globals.h
    iconbar.h
    iconnames2.h
    imagecache.h
    info.h
    keymap.h
    menunames.h
    privateeye.h
    quit.h
    rotate.h
    save.h
    scale.h
    tags-common.h
    tags-search.h
    tags.h
    thumbview.h
    to-spr.h
    viewer.h
    zones.h)

set(ALL_SOURCES
    actions.c
    canvas.c
    choices-colour.c
    choicesdat.c
    clipboard.c
    dataxfer.c
    display.c
    effects.c
    eye.c
    ffg.c
    globals.c
    iconbar.c
    imagecache.c
    info.c
    keymap.c
    quit.c
    rotate.c
    save.c
    scale.c
    tags-common.c
    tags-search.c
    tags.c
    thumbview.c
    to-spr.c
    viewer.c
    zones.c)

target_sources(PrivateEye PRIVATE ${ALL_SOURCES})


# Generate iconnames.h
#

set(TEMPLATES ${CMAKE_SOURCE_DIR}/Resources/UK/Templates,fec)
set(ICONNAMES_H ${CMAKE_CURRENT_BINARY_DIR}/iconnames.h)

find_program(TEMPLHEADR
             templheadr
             PATHS ${HOSTTOOLSDIR}/templheader-bin
             PATH_SUFFIXES Debug Release RelWithDebInfo MinSizeRel
             NO_DEFAULT_PATH
             REQUIRED)
add_custom_command(OUTPUT ${ICONNAMES_H}
                   COMMAND ${TEMPLHEADR} ${TEMPLATES} ${ICONNAMES_H}
                   MAIN_DEPENDENCY ${TEMPLATES})
add_custom_target(PrivateEyeIconNames ALL DEPENDS ${ICONNAMES_H})

add_dependencies(PrivateEye PrivateEyeIconNames)

target_include_directories(PrivateEye PRIVATE ${CMAKE_CURRENT_BINARY_DIR})


# Build options
#

# Ensure that compilers and assemblers don't receive each other's options.
target_compile_options(PrivateEye
    PRIVATE
    $<$<NOT:$<COMPILE_LANGUAGE:ASM_ASASM>>:-Wall -Wextra -pedantic>
    $<$<COMPILE_LANGUAGE:ASM_ASASM>:-Pedantic>)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(PrivateEye PRIVATE PRIVATEEYE_DEBUG)
endif()

# Feature options
#

option(FEATURE_CANVAS "Enable canvas feature" OFF)
option(FEATURE_METADATA "Enable metadata feature" ON)
option(FEATURE_TAGGING "Enable tagging feature" OFF)
option(FEATURE_THUMBVIEW "Enable thumbview feature" OFF)
option(FEATURE_ZONES "Enable zones feature" OFF)

if(FEATURE_CANVAS)
    target_compile_definitions(PrivateEye PRIVATE EYE_CANVAS)
endif()
if(FEATURE_METADATA)
    target_compile_definitions(PrivateEye PRIVATE EYE_META)
endif()
if(FEATURE_TAGGING)
    target_compile_definitions(PrivateEye PRIVATE EYE_TAGS)
endif()
if(FEATURE_THUMBVIEW)
    target_compile_definitions(PrivateEye PRIVATE EYE_THUMBVIEW)
endif()
if(FEATURE_ZONES)
    target_compile_definitions(PrivateEye PRIVATE EYE_ZONES)
endif()

# Library dependencies
#

if(TARGET_RISCOS)
    # OSLib (& anything else in GCCSDK)
    target_include_directories(PrivateEye PRIVATE $ENV{GCCSDK_INSTALL_ENV}/include)
    target_link_libraries(PrivateEye PRIVATE $ENV{GCCSDK_INSTALL_ENV}/lib/libOSLib32.a)
endif()

option(USE_FORTIFY "Use Fortify" OFF)
if(USE_FORTIFY)
    target_link_libraries(PrivateEye PRIVATE Fortify)
endif()

target_link_libraries(PrivateEye PRIVATE
    jpeg trans exiftags flex MD5 png zlib)



# Installation
#

## Set install location
## - We can't use set(...) with FORCE because that will override any passed-in
##   configuration values.
## - We can't use set(...) without FORCE because that won't override the default
##   value...
## - So, just check for the default value and override it if set.
#if(${CMAKE_INSTALL_PREFIX} STREQUAL "/usr/local")
#    set(CMAKE_INSTALL_PREFIX "install" CACHE PATH "CMake install prefix" FORCE)
#endif()
#
## Install headers and library
## - We have to use EXCLUDE_FROM_ALL to stop the PUBLIC_HEADERs getting
##   installed by install(TARGETS ...).
#install(TARGETS PrivateEye ARCHIVE
#    PUBLIC_HEADER EXCLUDE_FROM_ALL)
## - We can't use install(TARGETS ... PUBLIC_HEADER) to install headers, as
##   expected, since that flattens the include directory's hierarchy. Use
##   install(DIRECTORY ...) instead.
#install(DIRECTORY ${CMAKE_SOURCE_DIR}/include
#    DESTINATION ${CMAKE_INSTALL_PREFIX}
#    FILES_MATCHING PATTERN "*.h")
