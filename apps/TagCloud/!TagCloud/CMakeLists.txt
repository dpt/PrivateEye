# CMakeLists.txt
#
# TagCloud
#
# Copyright (c) David Thomas, 2021
#
# vim: sw=4 ts=8 et

# TODO
#
# - Installation

cmake_minimum_required(VERSION 3.18)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

set(APPENGINE_REPO_ROOT $ENV{APPENGINE_ROOT})

list(APPEND CMAKE_MODULE_PATH ${APPENGINE_REPO_ROOT}/cmake/Modules)

include(FetchContent)


project(TagCloud VERSION 0.0.1 DESCRIPTION "Image viewer" LANGUAGES C ASM_ASASM)

# If a host tools build was requested, then build them alone
set(TEMPLHEADR_DIR ${APPENGINE_REPO_ROOT}/utils/templheader)
if(HOST_TOOLS_ONLY)
    message(STATUS "${PROJECT_NAME}: Building host tools only")
    add_subdirectory(${TEMPLHEADR_DIR} templheader-bin)
    return()
endif()

# The values set in the toolchain file aren't available until this point.
if(NOT DEFINED TARGET_RISCOS)
    message(FATAL_ERROR "${PROJECT_NAME} builds for RISC OS only")
endif()

if(TARGET_RISCOS)
    riscos_set_flags()
endif()

# Referencing CMAKE_TOOLCHAIN_FILE avoids a warning on rebuilds.
if(NOT ${CMAKE_TOOLCHAIN_FILE} STREQUAL "")
    message(STATUS "${PROJECT_NAME}: Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
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

# Build TagCloud itself
message(STATUS "${PROJECT_NAME}: Building targets only")

# Libs... (set this before imports)
#
set(LIBS_DIR ${CMAKE_SOURCE_DIR}/../../../libs)

# Import DPTLib
FetchContent_Declare(DPTLib
    GIT_REPOSITORY $ENV{HOME}/SyncProjects/github/DPTLib
    GIT_TAG HEAD)
# Expanding out FetchContent_MakeAvailable(DPTLib) here so I can use EXCLUDE_FROM_ALL
FetchContent_GetProperties(DPTLib)
if(NOT DPTLib_POPULATED)
    FetchContent_Populate(DPTLib)
    add_subdirectory(${dptlib_SOURCE_DIR} ${dptlib_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# Import AppEngine
set(APPENGINE_DIR ${LIBS_DIR}/appengine)
set(APPENGINE_INCLUDE ${APPENGINE_DIR}/../)
add_subdirectory(${APPENGINE_DIR} appengine-build EXCLUDE_FROM_ALL)

# Import flex
set(FLEX_DIR ${LIBS_DIR}/flex)
set(FLEX_INCLUDE ${FLEX_DIR})
add_subdirectory(${FLEX_DIR} flex-build EXCLUDE_FROM_ALL)


# Avoid a warning from CMake
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

add_executable(TagCloud)

target_include_directories(TagCloud
    PRIVATE
    ${APPENGINE_INCLUDE}
    ${DPTLIB_INCLUDE}
    ${FLEX_INCLUDE}
    ${CMAKE_SOURCE_DIR})

target_link_libraries(TagCloud PRIVATE AppEngine DPTLib flex)

set_target_properties(TagCloud PROPERTIES
    DESCRIPTION "Tag Cloud demo"
    C_STANDARD 99
    RUNTIME_OUTPUT_NAME !RunImage)

set(ALL_HEADERS
    app.h
    globals.h
    iconbar.h
    menunames.h
    makecloud.h)

set(ALL_SOURCES
    globals.c
    iconbar.c
    makecloud.c
    tags.c)

target_sources(TagCloud PRIVATE ${ALL_HEADERS} ${ALL_SOURCES})


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
add_custom_target(TagCloudIconNames ALL DEPENDS ${ICONNAMES_H})

add_dependencies(TagCloud TagCloudIconNames)

target_include_directories(TagCloud PRIVATE ${CMAKE_CURRENT_BINARY_DIR})


# Build options
#

# Ensure that compilers and assemblers don't receive each other's options.
target_compile_options(TagCloud
    PRIVATE
    $<$<NOT:$<COMPILE_LANGUAGE:ASM_ASASM>>:-Wall -Wextra -pedantic>
    $<$<COMPILE_LANGUAGE:ASM_ASASM>:-Pedantic>)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(TagCloud PRIVATE PRIVATEEYE_DEBUG)
endif()

# Library dependencies
#

if(TARGET_RISCOS)
    # OSLib (& anything else in GCCSDK)
    target_include_directories(TagCloud PRIVATE $ENV{GCCSDK_INSTALL_ENV}/include)
    target_link_libraries(TagCloud PRIVATE $ENV{GCCSDK_INSTALL_ENV}/lib/libOSLib32.a)
endif()

option(USE_FORTIFY "Use Fortify" OFF)
if(USE_FORTIFY)
    target_link_libraries(TagCloud PRIVATE Fortify)
endif()



# Installation
#

# (force this for now)
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/!TagCloud" CACHE PATH "CMake install prefix" FORCE)

install(TARGETS TagCloud RUNTIME
    DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES !Boot,feb !Help !Run,feb !Sprites,ff9 AppEngine,ffa
    DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY Resources
    DESTINATION ${CMAKE_INSTALL_PREFIX})
