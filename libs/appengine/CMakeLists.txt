# CMakeLists.txt
#
# AppEngine
#
# Copyright (c) David Thomas, 2021
#
# vim: sw=4 ts=8 et

# TODO
#
# - Produce libraries with variant names for the options chosen (ARM
#   architecture, Fortify enabled, and so on).
# - Generate a version number for the library and place it in a header.
# - Perhaps drop the C target to C90.

cmake_minimum_required(VERSION 3.18)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

set(APPENGINE_REPO_ROOT $ENV{APPENGINE_ROOT})

list(APPEND CMAKE_MODULE_PATH ${APPENGINE_REPO_ROOT}/cmake/Modules)

include(FetchContent)


# ASM_ASASM is our CMake name for the GCCSDK objasm-format assembler 'asasm'.
project(AppEngine VERSION 0.6.8 DESCRIPTION "DPT's RISC OS application library" LANGUAGES C ASM_ASASM)

# The values set in the toolchain file aren't available until this point.
if(NOT DEFINED TARGET_RISCOS)
    message(FATAL_ERROR "AppEngine builds for RISC OS only")
endif()

if(TARGET_RISCOS)
    riscos_set_flags()
    # TODO: It's possible that those flags are already present so the appends above may be redundant. Apply it to targets instead?
endif()

# Referencing CMAKE_TOOLCHAIN_FILE avoids a warning on rebuilds.
if(NOT ${CMAKE_TOOLCHAIN_FILE} STREQUAL "")
    message(STATUS "AppEngine: Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
endif()

# Import DPTLib
FetchContent_Declare(DPTLib
    GIT_REPOSITORY $ENV{HOME}/SyncProjects/github/DPTLib
    GIT_TAG HEAD)
FetchContent_MakeAvailable(DPTLib)

# Import Fortify
# If this is enabled then the final binary will need to be linked against
# Fortify too.
option(USE_FORTIFY "Use Fortify" OFF)
if(USE_FORTIFY)
    add_subdirectory(${APPENGINE_REPO_ROOT}/libs/fortify fortify-build)
endif()

add_library(AppEngine)

target_link_libraries(AppEngine PUBLIC DPTLib)

set_target_properties(AppEngine PROPERTIES
    VERSION 0.6.8
    DESCRIPTION "DPT's RISC OS application C library"
    C_STANDARD 99
    PREFIX ""  # remove 'lib' prefix
    ARCHIVE_OUTPUT_NAME_DEBUG AppEngine-debug
    ARCHIVE_OUTPUT_NAME_RELEASE AppEngine
    ARCHIVE_OUTPUT_NAME_RELWITHDEBINFO AppEngine-relwithdebinfo
    ARCHIVE_OUTPUT_NAME_MINSIZEREL AppEngine-minsizerel)

# Look for includes in the 'libs' dir so that #include "fortify/fortify.h"
# works. Also look in the 'flex' and 'zlib' dirs directly since includers of
# those headers omit the parent directory.
target_include_directories(AppEngine
    PUBLIC
    .. # AppEngine headers are found relative to our parent dir
    PRIVATE
    ../flex
    ../zlib)

set(PUBLIC_HEADERS
    app/choices.h
    app/keymap.h
    app/wire.h
    base/appengine.h
    base/bitwise.h
    base/bsearch.h
    base/bytesex.h
    base/errors.h
    base/fxp.h
    base/heap.h
    base/messages.h
    base/numstr.h
    base/os.h
    base/oserror.h
    base/pack.h
    base/primes.h
    base/strings.h
    base/utils.h
    datastruct/array.h
    dialogues/dcs-quit.h
    dialogues/info.h
    dialogues/name.h
    dialogues/prog-info.h
    dialogues/save.h
    dialogues/scale.h
    gadgets/card.h
    gadgets/effects.h
    gadgets/filerwin.h
    gadgets/hist.h
    gadgets/iconbar.h
    gadgets/imageobwin.h
    gadgets/metadata.h
    gadgets/scroll-list.h
    gadgets/slider.h
    gadgets/tag-cloud.h
    gadgets/tonemap-gadget.h
    gadgets/treeview.h
    geom/trfm.h
    graphics/artworks.h
    graphics/awrender.h
    graphics/colour.h
    graphics/drawable.h
    graphics/image-observer.h
    graphics/image.h
    graphics/jpegtran.h
    graphics/sprite-effects.h
    graphics/thumbnail.h
    graphics/tonemap.h
    io/basic.h
    io/filing.h
    io/md5.h
    net/internet.h
    text/txtfmt.h
    types.h
    vdu/font.h
    vdu/screen.h
    vdu/sprite.h
    wimp/colourpick.h
    wimp/dialogue.h
    wimp/event.h
    wimp/filer.h
    wimp/help.h
    wimp/icon.h
    wimp/iconbar.h
    wimp/menu.h
    wimp/pointer.h
    wimp/window.h)

set(PRIVATE_HEADERS
    base/bitwise/util.h
    base/bsearch/bsearch-impl.h
    base/bytesex/util.h
    datastruct/atom/impl.h
    datastruct/bitvec/impl.h
    datastruct/hash/impl.h
    datastruct/hlist/impl.h
    datastruct/ntree/impl.h
    datastruct/vector/impl.h
    gadgets/scroll-list/impl.h
    gadgets/tag-cloud/iconnames.h
    gadgets/tag-cloud/impl.h
    gadgets/tag-cloud/menunames.h
    graphics/drawable/drawable-artworks.h
    graphics/drawable/drawable-bitmap.h
    graphics/drawable/drawable-drawfile.h
    graphics/drawable/drawable-jpeg.h
    graphics/drawable/drawable-vector.h
    graphics/image/artworks.h
    graphics/image/bitmap.h
    graphics/image/drawfile.h
    graphics/image/formats.h
    graphics/image/generic.h
    graphics/image/gif.h
    graphics/image/jpeg.h
    graphics/image/png.h
    graphics/image/sprite.h
    graphics/sprite-effects/convolve.h
    wimp/event/event-message.h
    wimp/event/event-wimp.h
    wimp/window/open.h)

set(GENERIC_SOURCES
    app/choices/choices.c
    app/keymap/keymap.c
    app/wire/wire.c
    base/appengine/heap.c
    base/appengine/resource.c
    base/bitwise/ceillog2.c
    base/bitwise/countbits.c
    base/bitwise/ctz.c
    base/bitwise/floorlog2.c
    base/bitwise/msb.c
    base/bitwise/power2gt.c
    base/bitwise/power2le.c
    base/bitwise/reversebits.c
    base/bsearch/bsearch-int.c
    base/bsearch/bsearch-short.c
    base/bsearch/bsearch-uint.c
    base/bsearch/bsearch-ushort.c
    base/bytesex/rev-l-block.c
    base/bytesex/rev-l-m.c
    base/bytesex/rev-l.c
    base/bytesex/rev-s-block.c
    base/bytesex/rev-s-m.c
    base/bytesex/rev-s-pair-m.c
    base/bytesex/rev-s-pair.c
    base/bytesex/rev-s.c
    base/errors/errors.c
    base/errors/fatal.c
    base/heap/heap.c
    base/messages/messages.c
    base/numstr/comma-dbl.c
    base/numstr/comma-num.c
    base/os/beep.c
    base/os/inkey.c
    base/oserror/check.c
    base/oserror/plot.c
    base/oserror/report-block.c
    base/oserror/report.c
    base/pack/pack.c
    base/pack/unpack.c
    base/primes/primes.c
    base/strings/str_branch.c
    base/strings/str_cpy.c
    base/strings/str_dup.c
    base/strings/str_leaf.c
    base/strings/str_len.c
    base/strings/str_n_cpy.c
    base/strings/str_n_dup.c
    base/strings/str_num.c
    base/strings/str_term.c
    base/strings/strcasecmp.c
    base/strings/strncasecmp.c
    base/strings/strnncasecmp.c
    base/utils/degs-to-rads.c
    base/utils/gcd.c
    datastruct/array/delelem.c
    datastruct/array/delelems.c
    datastruct/array/grow.c
    datastruct/array/shrinkwrap.c
    datastruct/array/squeeze.c
    datastruct/array/stretch.c
    dialogues/dcs-quit/dcs-quit.c
    dialogues/info/info.c
    dialogues/name/name.c
    dialogues/prog-info/prog-info.c
    dialogues/save/save.c
    dialogues/scale/scale.c
    gadgets/card/card.c
    gadgets/effects/effects.c
    gadgets/filerwin/filerwin.c
    gadgets/hist/hist.c
    gadgets/iconbar/iconbar.c
    gadgets/imageobwin/imageobwin.c
    gadgets/metadata/metadata.c
    gadgets/scroll-list/add-row.c
    gadgets/scroll-list/autoscroll.c
    gadgets/scroll-list/common.c
    gadgets/scroll-list/create.c
    gadgets/scroll-list/delete-rows.c
    gadgets/scroll-list/destroy.c
    gadgets/scroll-list/events.c
    gadgets/scroll-list/get-bbox.c
    gadgets/scroll-list/get-win.c
    gadgets/scroll-list/make-vis.c
    gadgets/scroll-list/marker.c
    gadgets/scroll-list/refresh-all.c
    gadgets/scroll-list/refresh-row.c
    gadgets/scroll-list/selection.c
    gadgets/scroll-list/set-handlers.c
    gadgets/scroll-list/set-row-height.c
    gadgets/scroll-list/where-insert.c
    gadgets/scroll-list/which.c
    gadgets/slider/slider.c
    gadgets/tag-cloud/create.c
    gadgets/tag-cloud/destroy.c
    gadgets/tag-cloud/display.c
    gadgets/tag-cloud/events.c
    gadgets/tag-cloud/get-win.c
    gadgets/tag-cloud/highlight.c
    gadgets/tag-cloud/hover.c
    gadgets/tag-cloud/init.c
    gadgets/tag-cloud/layout.c
    gadgets/tag-cloud/open.c
    gadgets/tag-cloud/redraw.c
    gadgets/tag-cloud/scaling.c
    gadgets/tag-cloud/set-config.c
    gadgets/tag-cloud/set-handlers.c
    gadgets/tag-cloud/set-tags.c
    gadgets/tag-cloud/shade.c
    gadgets/tag-cloud/sort.c
    gadgets/tag-cloud/toolbar.c
    gadgets/tonemap-gadget/tonemap-gadget.c
    gadgets/treeview/treeview.c
    geom/trfm/trfm.c
    graphics/artworks/callback.c
    graphics/colour/name.c
    graphics/drawable/drawable-artworks.c
    graphics/drawable/drawable-bitmap.c
    graphics/drawable/drawable-drawfile.c
    graphics/drawable/drawable-jpeg.c
    graphics/drawable/drawable-vector.c
    graphics/drawable/drawable.c
    graphics/image/artworks.c
    graphics/image/bitmap.c
    graphics/image/drawfile.c
    graphics/image/formats.c
    graphics/image/generic.c
    graphics/image/gif.c
    graphics/image/image-observer.c
    graphics/image/image.c
    graphics/image/jpeg-meta.c
    graphics/image/jpeg-utils.c
    graphics/image/jpeg.c
    graphics/image/png.c
    graphics/image/recognise.c
    graphics/image/sprite.c
    graphics/jpegtran/jpegtran.c
    graphics/sprite-effects/blender.c
    graphics/sprite-effects/blur.c
    graphics/sprite-effects/clear.c
    graphics/sprite-effects/convolve.c
    graphics/sprite-effects/equalise.c
    graphics/sprite-effects/expand.c
    graphics/sprite-effects/grey.c
    graphics/sprite-effects/sharpen.c
    graphics/sprite-effects/tonemap.c
    graphics/thumbnail/thumbnail.c
    graphics/tonemap/tonemap.c
    io/basic/file.c
    io/filing/buffer.c
    io/filing/filetype.c
    io/filing/scan.c
    io/md5/md5.c
    net/internet/mime_fromfiletype.c
    net/internet/mime_tofiletype.c
    net/internet/uri_decode.c
    text/txtfmt/txtfmt.c
    vdu/font/match.c
    vdu/screen/factors.c
    vdu/screen/mode-vars.c
    vdu/screen/pixel-size.c
    vdu/screen/read-mode-vars.c
    vdu/screen/screen_clip.c
    vdu/sprite/colours.c
    vdu/sprite/data.c
    vdu/sprite/has-alpha.c
    vdu/sprite/hist.c
    vdu/sprite/info.c
    vdu/sprite/mask-data.c
    vdu/sprite/mask-pixel.c
    vdu/sprite/mkgrey.c
    vdu/sprite/mode.c
    vdu/sprite/palettes.c
    vdu/sprite/remap-luma.c
    vdu/sprite/remap.c
    vdu/sprite/select.c
    vdu/sprite/size.c
    wimp/colourpick/colourpick.c
    wimp/dialogue/dialogue.c
    wimp/event/event-message-group.c
    wimp/event/event-message.c
    wimp/event/event-wimp-group.c
    wimp/event/event-wimp.c
    wimp/event/event.c
    wimp/filer/open-dir.c
    wimp/help/help.c
    wimp/icon/button-type.c
    wimp/icon/drag-object.c
    wimp/icon/drag.c
    wimp/icon/filetype-name.c
    wimp/icon/find.c
    wimp/icon/get-bbox.c
    wimp/icon/get-double.c
    wimp/icon/get-flags.c
    wimp/icon/get-int.c
    wimp/icon/get-name.c
    wimp/icon/get-screen-bbox.c
    wimp/icon/get-text.c
    wimp/icon/group-set-flags.c
    wimp/icon/move.c
    wimp/icon/printf.c
    wimp/icon/range-set-flags.c
    wimp/icon/set-double.c
    wimp/icon/set-flags.c
    wimp/icon/set-int.c
    wimp/icon/set-radio.c
    wimp/icon/set-selected.c
    wimp/icon/set-text.c
    wimp/icon/set-validation.c
    wimp/icon/size.c
    wimp/icon/sprite-name.c
    wimp/icon/validation-printf.c
    wimp/iconbar/create-device-icon.c
    wimp/iconbar/create-icon.c
    wimp/menu/count.c
    wimp/menu/create-from-desc.c
    wimp/menu/create.c
    wimp/menu/destroy.c
    wimp/menu/open.c
    wimp/menu/popup.c
    wimp/menu/set-flags.c
    wimp/menu/set-icon-flags.c
    wimp/menu/set-submenu.c
    wimp/menu/shade-all.c
    wimp/menu/tick-exclusive.c
    wimp/pointer/set-shape.c
    wimp/window/capture.c
    wimp/window/clone.c
    wimp/window/create-from-mem.c
    wimp/window/create.c
    wimp/window/find-template-size.c
    wimp/window/get-defn.c
    wimp/window/get-title.c
    wimp/window/load-sprites.c
    wimp/window/load-template.c
    wimp/window/open-as-menu-here.c
    wimp/window/open-as-menu.c
    wimp/window/open-at.c
    wimp/window/open-here.c
    wimp/window/read-furn-dims.c
    wimp/window/read-max-vis-area.c
    wimp/window/redraw.c
    wimp/window/set-extent2.c
    wimp/window/set-submenu-extent.c
    wimp/window/set-title.c)

set(ARM_SOURCES
    base/appengine/xappengine_codec_op_base64_decode.s
    base/appengine/xappengine_codec_op_base64_encode.s
    base/appengine/xappengine_heap_claim.s
    base/appengine/xappengine_heap_create_dynamic.s
    base/appengine/xappengine_heap_delete.s
    base/appengine/xappengine_heap_release.s
    base/appengine/xappengine_heap_resize.s
    base/appengine/xappengine_resource_op_locate.s
    base/heap/xosheap_resize_r3.s
    graphics/awrender/awrender.s
    net/internet/xresolver_get_host.s
    net/internet/xresolver_get_host_by_addr.s
    net/internet/xresolver_get_host_by_name.s)

# ARM sources that depend on the target architecture
#
# SMULL and UMULL need ARMv3M and above
# CLZ needs ARMv5 and above
set(ARM_BEFORE_V3M_SOURCES
    base/fxp/smull-fxp16.c
    base/fxp/umull-fxp16.c)
set(ARM_AFTER_V3M_SOURCES
    base/fxp/target/arm-v3m/smull-fxp16.s
    base/fxp/target/arm-v3m/umull-fxp16.s)
set(ARM_BEFORE_V5_SOURCES
    base/bitwise/clz.c)
set(ARM_AFTER_V5_SOURCES
    base/bitwise/target/arm-v5/clz.s)

set(ALL_SOURCES ${PUBLIC_HEADERS} ${GENERIC_SOURCES} ${ARM_SOURCES})
if(ARMV5)
    list(APPEND ALL_SOURCES ${ARM_AFTER_V3M_SOURCES} ${ARM_AFTER_V5_SOURCES})
else()
    list(APPEND ALL_SOURCES ${ARM_BEFORE_V3M_SOURCES} ${ARM_BEFORE_V5_SOURCES})
endif()

target_sources(AppEngine PRIVATE ${ALL_SOURCES})

# Ensure that compilers and assemblers don't receive each other's options.
target_compile_options(AppEngine
    PRIVATE
    $<$<NOT:$<COMPILE_LANGUAGE:ASM_ASASM>>:-Wall -Wextra -pedantic>
    $<$<COMPILE_LANGUAGE:ASM_ASASM>:-Pedantic>)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(AppEngine PRIVATE APPENGINE_DEBUG)
endif()

# Library dependencies
#

if(TARGET_RISCOS)
    # OSLib (& anything else in GCCSDK)
    target_include_directories(AppEngine PUBLIC $ENV{GCCSDK_INSTALL_ENV}/include)
    target_link_libraries(AppEngine PUBLIC $ENV{GCCSDK_INSTALL_ENV}/lib/libOSLib32.a)
endif()

if(USE_FORTIFY)
    target_link_libraries(AppEngine PUBLIC Fortify)
endif()


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
# - We have to use EXCLUDE_FROM_ALL to stop the PUBLIC_HEADERs getting
#   installed by install(TARGETS ...).
install(TARGETS AppEngine ARCHIVE
    PUBLIC_HEADER EXCLUDE_FROM_ALL)
# - We can't use install(TARGETS ... PUBLIC_HEADER) to install headers, as
#   expected, since that flattens the include directory's hierarchy. Use
#   install(DIRECTORY ...) instead.
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include
    DESTINATION ${CMAKE_INSTALL_PREFIX}
    FILES_MATCHING PATTERN "*.h")


# Self-test executable
#

option(BUILD_TESTS "Build test program" OFF)

if(BUILD_TESTS)
    set(TEST_SOURCES
        app/wire/test/test.c
        base/pack/test/test.c
        datastruct/array/test/test.c
        io/filing/test/test.c
        test/main/main.c
        text/txtfmt/test/test.c
        vdu/font/test/test.c)

    # Avoid a warning from CMake
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

    add_executable(AppEngineTest ${TEST_SOURCES})

    set_target_properties(AppEngineTest PROPERTIES
        DESCRIPTION "AppEngine test program"
        C_STANDARD 99
        OUTPUT_NAME_DEBUG AppEngineTest-debug
        OUTPUT_NAME_RELEASE AppEngineTest
        OUTPUT_NAME_RELWITHDEBINFO AppEngineTest-relwithdebinfo
        OUTPUT_NAME_MINSIZEREL AppEngineTest-minsizerel)


    # AETest dependencies
    # Of course, AETest depends on AppEngine.
    target_link_libraries(AppEngineTest AppEngine)

    install(TARGETS AppEngineTest RUNTIME
        DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()
