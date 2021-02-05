#
# templheadr
#
# Copyright (c) Nettle developers 2000-2004
#
# vim: sw=4 ts=8 et

cmake_minimum_required(VERSION 3.18)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

project(templheadr VERSION 1.8.0 DESCRIPTION "Makes C headers from RISC OS window templates" LANGUAGES C)

add_executable(templheadr)

set_target_properties(templheadr PROPERTIES
    VERSION 1.8.0
    C_STANDARD 99
    OUTPUT_NAME templheadr)

target_sources(templheadr PRIVATE templheadr.c)

target_compile_options(templheadr PRIVATE -Wall -Wextra -pedantic)
