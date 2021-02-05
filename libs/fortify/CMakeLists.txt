# CMakeLists.txt
#
# Fortify
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

# Referencing CMAKE_TOOLCHAIN_FILE avoids a warning on rebuilds.
if(NOT ${CMAKE_TOOLCHAIN_FILE} STREQUAL "")
    message(STATUS "Fortify: Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
endif()

project(Fortify VERSION 2.2.0 DESCRIPTION "Malloc debugger" LANGUAGES C)

# The values set in the toolchain file aren't available until this point.
if(TARGET_RISCOS)
    riscos_set_flags()
endif()

# Avoid a warning from CMake
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

add_library(Fortify)

set_target_properties(Fortify PROPERTIES
    VERSION 2.2.0
    DESCRIPTION "Malloc debugger"
    C_STANDARD 99
    ARCHIVE_OUTPUT_NAME fortify)

# Includes must be relative to above the source dir
# CHECK: This is just '..' and not relative to a var
target_include_directories(Fortify PUBLIC ..)

set(PUBLIC_HEADERS fortify.h ufortify.h)

# The public headers must be set as properties of the library, not as
# target_sources. The quoting is essential.
set_target_properties(Fortify PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

target_sources(Fortify PRIVATE fortify.c)

target_compile_options(Fortify PRIVATE -Wall -Wextra -pedantic)

target_compile_definitions(Fortify PUBLIC FORTIFY)


# Installation
#

# Set install location
# - We can't use set(...) with FORCE because that will override any passed-in
#   configuration values.
# - We can't use set(...) without FORCE because that won't override the default
#   value...
# - So, just check for the default value and override it if set.
if(${CMAKE_INSTALL_PREFIX} STREQUAL "/usr/local")
    set(CMAKE_INSTALL_PREFIX "install" CACHE PATH "CMake install prefix" FORCE)
endif()

# Install headers and library
install(TARGETS Fortify ARCHIVE)
