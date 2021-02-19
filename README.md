PrivateEye
==========

PrivateEye is primarily an image viewer for RISC OS. It can load and display various file formats like Sprites, JPEGs, Drawfiles and ArtWorks. It can losslessly rotate JPEGs and display Exif metadata in a hierarchical tree display. It also contains a bitmap effects system for performing basic image adjustments.

In this Repository
------------------

* PrivateEye - image viewer
* TagCloud - demo app for tag cloud module
* AppEngine - a RISC OS desktop C library, somewhat like DeskLib, or the RISC OS Toolbox, etc.


Building PrivateEye
-------------------

Cross compile on Linux. This requires [GCCSDK](http://www.riscos.info/index.php/GCCSDK) and [CMake](https://cmake.org/) to be installed. (Previously it would build using Acorn C/C++ but that is no longer practical.)

Using CMake to generate a Makefile build:

``` bash
export APPENGINE_ROOT=<here>
cd <here>/apps/PrivateEye
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=${APPENGINE_ROOT}/cmake/riscos.cmake ../!PrivateEye
make install
```

Or ideally pass `-GNinja` to CMake and build using [Ninja](https://ninja-build.org/) for speeeed.

The build will automatically pull in and build in all the dependencies (including DPTLib which lives elsewhere on github, AppEngine which is local, libjpeg, libpng, etc.) If all's well a freshly-baked copy of !PrivateEye will be pooed out into the `build` directory.

Building TagCloud
-----------------

TagCloud is built in exactly the same way.
