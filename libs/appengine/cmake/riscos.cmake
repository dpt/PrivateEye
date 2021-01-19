# CMake toolchain file for RISC OS GCCSDK

set(crossbin $ENV{GCCSDK_INSTALL_CROSSBIN})
set(prefix arm-unknown-riscos-)
set(CMAKE_ASM_ASASM_COMPILER ${crossbin}/asasm)
set(CMAKE_CXX_COMPILER ${crossbin}/${prefix}g++)
set(CMAKE_C_COMPILER ${crossbin}/${prefix}gcc)

set(CMAKE_ASM_ASASM_FLAGS "" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS "-mlibscl -mhard-float -mpoke-function-name" CACHE STRING "" FORCE)

set(CMAKE_FIND_ROOT_PATH ${crossbin})

# Search for programs only in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# Search for libraries and headers only in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(TARGET_RISCOS 1)
