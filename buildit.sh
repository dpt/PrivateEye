#!/bin/bash
##
# Build PrivateEye and TagCloud, pulling in all resources.
#
# Run me with:
#    docker run -it --rm -v "$PWD:/work" -w /work riscosdotinfo/riscos-gccsdk-4.7:latest ./buildit.sh

set -e
set -o pipefail

# Building OSLib will be super slow.
BUILD_OSLIB=false
# Build type.
BUILD_TYPE=MinSizeRel

scriptdir="$(cd "$(dirname "$0")" && pwd -P)"

# Dependencies

wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | apt-key add -
cmake --version || { apt-get update && \
                     apt-get install -y software-properties-common && \
                     apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main' && \
                     apt-get install -y cmake ; }

ninja --version || { apt-get update && \
                     apt-get install -y ninja-build ; }

rsync --version || { apt-get update && \
                     apt-get install -y rsync ; }

rename --version || { apt-get update && \
                      apt-get install -y rename ; }

# StrongCopy
mkdir bin && \
  wget -O bin/strongcopy https://github.com/gerph/alexwaugh-strongcopy/releases/download/v1.05/strongcopy-ubuntu-1.05 && \
  chmod u+x bin/strongcopy

source /home/riscos/gccsdk-params

if [[ ! -d libs/flex ]] ; then
    cd libs
    ./grabflex.sh
    cd ..
fi

if $BUILD_OSLIB ; then
    if [[ ! -d libs/oslib || ! -d "$GCCSDK_INSTALL_ENV/include/oslib/" ]] ; then
        cd libs
        svn co 'svn://svn.code.sf.net/p/ro-oslib/code/trunk/!OSLib' oslib
        cd oslib
        make ELFOBJECTTYPE=HARDFPU install
        cd ..
    fi
else
    if [[ ! -d libs/oslib-bin || ! -d "$GCCSDK_INSTALL_ENV/include/oslib/" ]] ; then
        cd libs
        mkdir -p oslib-bin
        cd oslib-bin
        if [[ ! -f oslib.zip ]] ; then
            wget -O oslib.zip 'https://sourceforge.net/projects/ro-oslib/files/OSLib/7.00/OSLib-elf-scl-app-hardfloat-7.00.zip/download?use_mirror=master'
            unzip oslib.zip
        fi
        mkdir -p "$GCCSDK_INSTALL_ENV/include/oslib/"
        mkdir -p "$GCCSDK_INSTALL_ENV/lib/"
        rename 's!/h/(.*)$!/$1.h!' oslib/h/*
        rsync -av oslib/ "$GCCSDK_INSTALL_ENV/include/oslib/"
        cp libOSLib32.a "$GCCSDK_INSTALL_ENV/lib/"
        cd ../../
    fi
fi

export APPENGINE_ROOT=${scriptdir}

# PrivateEye
cd "${scriptdir}/apps/PrivateEye"
rm -rf build && mkdir -p build && cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_TOOLCHAIN_FILE="${APPENGINE_ROOT}/cmake/riscos.cmake" '../!PrivatEye' || bash -i
ninja install || bash -i

# TagCloud
cd "${scriptdir}/apps/TagCloud"
rm -rf build && mkdir -p build && cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_TOOLCHAIN_FILE="${APPENGINE_ROOT}/cmake/riscos.cmake" '../!TagCloud' || bash -i
ninja install || bash -i
