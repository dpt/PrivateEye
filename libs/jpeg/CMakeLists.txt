# CMakeLists.txt
#
# libjpeg
#
# Copyright (c) David Thomas, 2021
#
# vim: sw=4 ts=8 et

cmake_minimum_required(VERSION 3.18)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

project(jpeg VERSION 8 DESCRIPTION "IJG libjpeg" LANGUAGES C)

# The values set in the toolchain file aren't available until this point.

if(TARGET_RISCOS)
    riscos_set_flags()
endif()

# Referencing CMAKE_TOOLCHAIN_FILE avoids a warning on rebuilds.
if(NOT ${CMAKE_TOOLCHAIN_FILE} STREQUAL "")
    message(STATUS "libjpeg: Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
endif()

if(TARGET_RISCOS)
    set(SYSDEPMEM jmemro.c)
else()
    message(FATAL_ERROR "Incomplete CMakeLists.txt")
endif()

set(COMMON_SOURCES
    jaricom.c
    jcomapi.c
    jutils.c
    jerror.c
    jmemmgr.c
    ${SYSDEPMEM})

set(CLIB_SOURCES
    jcapimin.c
    jcapistd.c
    jcarith.c
    jctrans.c
    jcparam.c
    jdatadst.c
    jcinit.c
    jcmaster.c
    jcmarker.c
    jcmainct.c
    jcprepct.c
    jccoefct.c
    jccolor.c
    jcsample.c
    jchuff.c
    jcdctmgr.c
    jfdctfst.c
    jfdctflt.c
    jfdctint.c)

set(DLIB_SOURCES
    jdapimin.c
    jdapistd.c
    jdarith.c
    jdtrans.c
    jdatasrc.c
    jdmaster.c
    jdinput.c
    jdmarker.c
    jdhuff.c
    jdmainct.c
    jdcoefct.c
    jdpostct.c
    jddctmgr.c
    jidctfst.c
    jidctflt.c
    jidctint.c
    jdsample.c
    jdcolor.c
    jquant1.c
    jquant2.c
    jdmerge.c)

set(LIBSOURCES
    ${COMMON_SOURCES}
    ${CLIB_SOURCES}
    ${DLIB_SOURCES})

set(TRAN_SOURCES
    transupp.c)

set(CJPEG_SOURCES
    cjpeg.c
    rdppm.c
    rdgif.c
    rdtarga.c
    rdrle.c
    rdbmp.c
    rdswitch.c
    cdjpeg.c)

set(DJPEG_SOURCES
    djpeg.c
    wrppm.c
    wrgif.c
    wrtarga.c
    wrrle.c
    wrbmp.c
    rdcolmap.c
    cdjpeg.c)

set(JPEGTRAN_SOURCES
    jpegtran.c
    rdswitch.c
    cdjpeg.c
    ${TRAN_SOURCES})

set(RDJPGCOM_SOURCES
    rdjpgcom.c)

set(WRJPGCOM_SOURCES
    wrjpgcom.c)

add_library(jpeg)
set_target_properties(jpeg PROPERTIES
    VERSION 8.0.0
    C_STANDARD 90)

if(TARGET_RISCOS)
    target_include_directories(jpeg PRIVATE $ENV{APPENGINE_ROOT}/libs) # unpleasant - to get at appengine's headers for jmemro.c
endif()
target_sources(jpeg PRIVATE ${LIBSOURCES})
target_compile_options(jpeg PRIVATE -Wall -Wextra -pedantic)
target_compile_definitions(jpeg PRIVATE NO_GETENV) # for jmemmgr

add_library(trans)
set_target_properties(trans PROPERTIES
    VERSION 8.0.0
    C_STANDARD 90)
target_sources(trans PRIVATE ${TRAN_SOURCES})
target_compile_options(trans PRIVATE -Wall -Wextra -pedantic)

# Library dependencies
#

if(TARGET_RISCOS)
    # OSLib (& anything else in GCCSDK)
    target_include_directories(jpeg PUBLIC $ENV{GCCSDK_INSTALL_ENV}/include)
    target_link_libraries(jpeg PUBLIC $ENV{GCCSDK_INSTALL_ENV}/lib/libOSLib32.a)
endif()

# TODO: Build executables

