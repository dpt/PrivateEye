# CMakeLists.txt
#
# PrivateEye
#
# Copyright (c) David Thomas, 2021-2022
#
# vim: sw=4 ts=8 et

cmake_minimum_required(VERSION 3.18)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

set(APPENGINE_REPO_ROOT $ENV{APPENGINE_ROOT})

list(APPEND CMAKE_MODULE_PATH ${APPENGINE_REPO_ROOT}/cmake/Modules)

include(FetchContent)


project(PrivateEye VERSION 3.1.0 DESCRIPTION "Image viewer" LANGUAGES C ASM ASM_ASASM)

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


find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)


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

# Build PrivateEye itself
message(STATUS "${PROJECT_NAME}: Building targets only")

# Libs... (set this before imports)
#
set(LIBS_DIR ${APPENGINE_REPO_ROOT}/libs)

# Override libpng's configuration
set(DFA_XTRA ${CMAKE_CURRENT_SOURCE_DIR}/pngusr-privateeye.dfa CACHE FILEPATH "File containing extra configuration settings" FORCE)

# Import DPTLib
FetchContent_Declare(DPTLib
    GIT_REPOSITORY https://github.com/dpt/DPTLib
    GIT_TAG a30b2320ab4ed32b401fb03622c015a27fe3cd56)
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

# Import libjpeg
set(JPEG_DIR ${LIBS_DIR}/jpeg)
set(JPEG_INCLUDE ${JPEG_DIR})
add_subdirectory(${JPEG_DIR} jpeg-build EXCLUDE_FROM_ALL)

# Import exiftags
set(EXIFTAGS_DIR ${LIBS_DIR}/exiftags)
set(EXIFTAGS_INCLUDE ${EXIFTAGS_DIR})
add_subdirectory(${EXIFTAGS_DIR} exiftags-build EXCLUDE_FROM_ALL)

# Import flex
set(FLEX_DIR ${LIBS_DIR}/flex)
set(FLEX_INCLUDE ${FLEX_DIR})
add_subdirectory(${FLEX_DIR} flex-build EXCLUDE_FROM_ALL)

# Import MD5
set(MD5_DIR ${LIBS_DIR}/md5)
set(MD5_INCLUDE ${MD5_DIR})
add_subdirectory(${MD5_DIR} md5-build EXCLUDE_FROM_ALL)


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
    ${CMAKE_SOURCE_DIR})

target_link_libraries(PrivateEye PRIVATE AppEngine DPTLib exiftags flex jpeg trans MD5)

set_target_properties(PrivateEye PROPERTIES
    DESCRIPTION "Image viewer"
    C_STANDARD 99
    RUNTIME_OUTPUT_NAME !RunImage)

set(ALL_HEADERS
    actions.h
    canvas.h
    choices-colour.h
    choicesdat.h
    clipboard.h
    dataxfer.h
    display.h
    ffg.h
    globals.h
    iconbar.h
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
    eye.c
    ffg.c
    globals.c
    iconbar.c
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

target_sources(PrivateEye PRIVATE ${ALL_HEADERS} ${ALL_SOURCES})


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
    target_compile_definitions(PrivateEye PRIVATE FORTIFY)
    target_link_libraries(PrivateEye PRIVATE Fortify)
endif()


# Generate StrongHelp format manual
#

set(HELP_SRC_DIR ${CMAKE_SOURCE_DIR}/../Help)
set(HELP_DST Help,3d6)

set(ALL_HELP_SOURCES
    ${HELP_SRC_DIR}/!Root
    ${HELP_SRC_DIR}/Choices
    ${HELP_SRC_DIR}/ChoicesSpr,fca
    ${HELP_SRC_DIR}/Controls
    ${HELP_SRC_DIR}/Copyright
    ${HELP_SRC_DIR}/Effects
    ${HELP_SRC_DIR}/EffectsSpr,fca
    ${HELP_SRC_DIR}/Features
    ${HELP_SRC_DIR}/Histogram
    ${HELP_SRC_DIR}/History
    ${HELP_SRC_DIR}/ImageInfo
    ${HELP_SRC_DIR}/Keymap
    ${HELP_SRC_DIR}/Metadata
    ${HELP_SRC_DIR}/Rotate
    ${HELP_SRC_DIR}/Sprites,fca
    ${HELP_SRC_DIR}/Supported
    ${HELP_SRC_DIR}/Viewers)

find_program(STRONGCOPY
    strongcopy
    PATHS ${APPENGINE_REPO_ROOT}/bin
    NO_DEFAULT_PATH
    REQUIRED)
add_custom_command(OUTPUT ${HELP_DST}
    COMMAND ${STRONGCOPY} -v -o ${HELP_DST} ${HELP_SRC_DIR}
    DEPENDS ${ALL_HELP_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_custom_target(Manual ALL DEPENDS ${HELP_DST})


# Installation
#

# (force this for now)
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/install" CACHE PATH "CMake install prefix" FORCE)
set(INSTALL_APPDIR ${CMAKE_INSTALL_PREFIX}/Apps/Graphics/!PrivatEye)

install(TARGETS PrivateEye RUNTIME
    DESTINATION ${INSTALL_APPDIR})
install(FILES !Boot,feb !Help !Run,feb !Sprites,ff9 !Sprites11,ff9 AppEngine,ffa
    DESTINATION ${INSTALL_APPDIR})
install(DIRECTORY Resources
    DESTINATION ${INSTALL_APPDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${HELP_DST}
    DESTINATION ${INSTALL_APPDIR}/Resources)
install(DIRECTORY ../Package/RiscPkg
    DESTINATION ${CMAKE_INSTALL_PREFIX})
