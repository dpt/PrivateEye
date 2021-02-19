#!/bin/bash -e
#
# Fetch the source to the flex memory manager from the riscosopen.org site and
# compile it up with GCCSDK.
#
# Copyright (c) David Thomas, 2021
#

if [ -d flex ]; then
	echo "flex is downloaded."
	exit 0
fi

mkdir -p flex
cd flex

# Fetch
echo "Downloading flex."

git clone https://gitlab.riscosopen.org/RiscOS/Sources/Toolbox/ToolboxLib.git
mv ./ToolboxLib/flexlib/h/flex flex.h
mv ./ToolboxLib/flexlib/h/opts opts.h
mv ./ToolboxLib/flexlib/h/swiextra swiextra.h
mv ./ToolboxLib/flexlib/c/flex flex.c
rm -rf ./ToolboxLib

# Write out a CMakeLists.txt

cat > CMakeLists.txt <<EOF
# CMakeLists.txt
#
# flex
#
# Copyright (c) David Thomas, 2021
#
# vim: sw=4 ts=8 et
#

cmake_minimum_required(VERSION 3.18)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

project(flex DESCRIPTION "Sliding heap allocator" LANGUAGES C)

# The values set in the toolchain file aren't available until this point.

if(NOT DEFINED TARGET_RISCOS)
    message(FATAL_ERROR "flex builds for RISC OS only")
endif()

if(TARGET_RISCOS)
    riscos_set_flags()
endif()

# Referencing CMAKE_TOOLCHAIN_FILE avoids a warning on rebuilds.
if(NOT \${CMAKE_TOOLCHAIN_FILE} STREQUAL "")
    message(STATUS "flex: Using toolchain file: \${CMAKE_TOOLCHAIN_FILE}")
endif()

add_library(flex)

set_target_properties(flex PROPERTIES
    DESCRIPTION "Sliding heap allocator"
    C_STANDARD 90)

target_sources(flex PRIVATE flex.h opts.h swiextra.h flex.c)

target_compile_options(flex PRIVATE -Wall -Wextra -pedantic -Wno-unused-but-set-variable -Wno-sign-compare -Wno-unknown-pragmas -Wno-format)
EOF

cd -

