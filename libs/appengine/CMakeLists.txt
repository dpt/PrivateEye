# CMakeLists.txt
#
# AppEngine
#
# Copyright (c) David Thomas, 2021
#
# vim: sw=4

# TODO
#
# - Produce libraries with variant names for the options chosen (ARM
#   architecture, Fortify enabled, and so on).
# - Generate a version number for the library and place it in a header.
# - Perhaps drop the C target to C90.
# - Depend on DPTLib, remove overlaps like the database code.

cmake_minimum_required(VERSION 3.18)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules/)

# ASM_ASASM is our CMake name for the GCCSDK objasm-format assembler 'asasm'.
project(AppEngine VERSION 0.6.8 DESCRIPTION "DPT's RISC OS application library" LANGUAGES C ASM_ASASM)

# The values set in the toolchain file aren't available until this point.

if(NOT DEFINED TARGET_RISCOS)
    message(FATAL_ERROR "AppEngine builds for RISC OS only")
endif()

if(TARGET_RISCOS AND NOT DEFINED ENV{GCCSDK_INSTALL_ENV})
    message(FATAL_ERROR "GCCSDK_INSTALL_ENV is not set")
endif()

if(TARGET_RISCOS)
    set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG" CACHE STRING "" FORCE) # overwrite default of '-O3 -NDEBUG'

    # I tried to get this to live in riscos.cmake but was defeated.
    set(CMAKE_EXECUTABLE_SUFFIX ,ff8)

    # For now, divide assembly targets into ancient or modern ARM (pivot=ARMv5).
    option(APPENGINE_ARMV5 "Utilise ARMv5" OFF)
    if(APPENGINE_ARMV5)
        string(APPEND CMAKE_C_FLAGS " -march=armv5 -mtune=xscale")
        string(APPEND CMAKE_ASM_ASASM_FLAGS "-cpu XScale -fpu FPA10")
    else()
        string(APPEND CMAKE_C_FLAGS " -march=armv2 -mtune=arm2")
        string(APPEND CMAKE_ASM_ASASM_FLAGS "-cpu ARM2 -fpu FPA10")
    endif()
endif()

# If this is enabled then the final binary will need to be linked against
# Fortify too.
option(USE_FORTIFY "Use Fortify" OFF)

# Referencing CMAKE_TOOLCHAIN_FILE avoids a warning on rebuilds.
if(NOT ${CMAKE_TOOLCHAIN_FILE} STREQUAL "")
    message(STATUS "AppEngine: Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
endif()

set(LIBRARY ${PROJECT_NAME})

add_library(${LIBRARY})

set_target_properties(${LIBRARY} PROPERTIES
    VERSION 0.6.8
    DESCRIPTION "DPT's RISC OS application C library"
    C_STANDARD 99
    PREFIX ""  # remove 'lib' prefix
    ARCHIVE_OUTPUT_NAME_DEBUG ${LIBRARY}-debug
    ARCHIVE_OUTPUT_NAME_RELEASE ${LIBRARY}
    ARCHIVE_OUTPUT_NAME_RELWITHDEBINFO ${LIBRARY}-relwithdebinfo
    ARCHIVE_OUTPUT_NAME_MINSIZEREL ${LIBRARY}-minsizerel)

# Look for includes in the 'libs' dir so that #include "fortify/fortify.h"
# works. Also look in the 'flex' and 'zlib' dirs directly since includers of
# those headers omit the parent directory.
target_include_directories(${LIBRARY}
    PUBLIC
    ${CMAKE_SOURCE_DIR}
    PRIVATE
    ..
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
    databases/digest-db.h
    databases/filename-db.h
    databases/pickle-reader-hash.h
    databases/pickle-writer-hash.h
    databases/pickle.h
    databases/tag-db.h
    datastruct/array.h
    datastruct/atom.h
    datastruct/bitarr.h
    datastruct/bitvec.h
    datastruct/hash.h
    datastruct/hlist.h
    datastruct/list.h
    datastruct/ntree.h
    datastruct/vector.h
    dialogues/dcs-quit.h
    dialogues/info.h
    dialogues/name.h
    dialogues/prog-info.h
    dialogues/save.h
    dialogues/scale.h
    gadgets/card.h
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
    geom/box.h
    geom/layout.h
    geom/packer.h
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
    io/stream-mem.h
    io/stream-mtfcomp.h
    io/stream-packbits.h
    io/stream-stdio.h
    io/stream.h
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
    geom/packer/impl.h
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
    text/lang/emit.h
    text/lang/error.h
    text/lang/eval.h
    text/lang/lex.h
    text/lang/parse.h
    text/lang/sym.h
    text/lang/tokens.h
    text/lang/tree.h
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
    databases/digest-db/digest-db.c
    databases/filename-db/filename-db.c
    databases/pickle/delete.c
    databases/pickle/hash-reader.c
    databases/pickle/hash-writer.c
    databases/pickle/pickle.c
    databases/pickle/unpickle.c
    databases/tag-db/tag-db.c
    datastruct/array/delelem.c
    datastruct/array/delelems.c
    datastruct/array/grow.c
    datastruct/array/shrinkwrap.c
    datastruct/array/squeeze.c
    datastruct/array/stretch.c
    datastruct/atom/create.c
    datastruct/atom/delete-block.c
    datastruct/atom/delete.c
    datastruct/atom/destroy.c
    datastruct/atom/for-block.c
    datastruct/atom/get.c
    datastruct/atom/new.c
    datastruct/atom/set.c
    datastruct/bitarr/count.c
    datastruct/bitvec/and.c
    datastruct/bitvec/clear-all.c
    datastruct/bitvec/clear.c
    datastruct/bitvec/count.c
    datastruct/bitvec/create.c
    datastruct/bitvec/destroy.c
    datastruct/bitvec/ensure.c
    datastruct/bitvec/eq.c
    datastruct/bitvec/get.c
    datastruct/bitvec/length.c
    datastruct/bitvec/next.c
    datastruct/bitvec/or.c
    datastruct/bitvec/set-all.c
    datastruct/bitvec/set.c
    datastruct/bitvec/toggle.c
    datastruct/hash/create.c
    datastruct/hash/destroy.c
    datastruct/hash/insert.c
    datastruct/hash/lookup-node.c
    datastruct/hash/lookup.c
    datastruct/hash/remove.c
    datastruct/hash/walk-cont.c
    datastruct/hash/walk.c
    datastruct/hlist/append.c
    datastruct/hlist/copy.c
    datastruct/hlist/free.c
    datastruct/hlist/length.c
    datastruct/hlist/list.c
    datastruct/hlist/map.c
    datastruct/hlist/pop.c
    datastruct/hlist/push.c
    datastruct/hlist/reverse.c
    datastruct/hlist/to-array.c
    datastruct/list/add-head.c
    datastruct/list/find.c
    datastruct/list/init.c
    datastruct/list/remove.c
    datastruct/list/walk.c
    datastruct/ntree/copy.c
    datastruct/ntree/delete.c
    datastruct/ntree/depth.c
    datastruct/ntree/first-child.c
    datastruct/ntree/free.c
    datastruct/ntree/get-data.c
    datastruct/ntree/insert-after.c
    datastruct/ntree/insert-before.c
    datastruct/ntree/insert.c
    datastruct/ntree/last-child.c
    datastruct/ntree/max-height.c
    datastruct/ntree/n-nodes.c
    datastruct/ntree/new.c
    datastruct/ntree/next-sibling.c
    datastruct/ntree/nth-child.c
    datastruct/ntree/parent.c
    datastruct/ntree/prepend.c
    datastruct/ntree/prev-sibling.c
    datastruct/ntree/set-data.c
    datastruct/ntree/unlink.c
    datastruct/ntree/walk.c
    datastruct/vector/clear.c
    datastruct/vector/create.c
    datastruct/vector/destroy.c
    datastruct/vector/get.c
    datastruct/vector/length.c
    datastruct/vector/set-length.c
    datastruct/vector/set-width.c
    datastruct/vector/width.c
    dialogues/dcs-quit/dcs-quit.c
    dialogues/info/info.c
    dialogues/name/name.c
    dialogues/prog-info/prog-info.c
    dialogues/save/save.c
    dialogues/scale/scale.c
    gadgets/card/card.c
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
    geom/box/contains-box.c
    geom/box/contains-point.c
    geom/box/could-hold.c
    geom/box/grow.c
    geom/box/intersection.c
    geom/box/intersects.c
    geom/box/is-empty.c
    geom/box/round.c
    geom/box/round4.c
    geom/box/set-origin.c
    geom/box/union.c
    geom/layout/layout.c
    geom/packer/packer.c
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
    io/stream/stream-mem.c
    io/stream/stream-mtfcomp.c
    io/stream/stream-packbitscomp.c
    io/stream/stream-packbitsdecomp.c
    io/stream/stream-stdio.c
    io/stream/stream.c
    net/internet/mime_fromfiletype.c
    net/internet/mime_tofiletype.c
    net/internet/uri_decode.c
    text/lang/emit.c
    text/lang/error.c
    text/lang/eval.c
    text/lang/lex.c
    text/lang/main.c
    text/lang/parse.c
    text/lang/sym.c
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
if(APPENGINE_ARMV5)
    list(APPEND ALL_SOURCES ${ARM_AFTER_V3M_SOURCES} ${ARM_AFTER_V5_SOURCES})
else()
    list(APPEND ALL_SOURCES ${ARM_BEFORE_V3M_SOURCES} ${ARM_BEFORE_V5_SOURCES})
endif()

target_sources(${LIBRARY} PRIVATE ${ALL_SOURCES})

# Ensure that compilers and assemblers don't receive each other's options.
target_compile_options(${LIBRARY}
    PRIVATE
    $<$<NOT:$<COMPILE_LANGUAGE:ASM_ASASM>>:-Wall -Wextra -pedantic>
    $<$<COMPILE_LANGUAGE:ASM_ASASM>:-Pedantic>)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(${LIBRARY} PRIVATE $<UPPER_CASE:${LIBRARY}>_DEBUG)
endif()

# Library dependencies
#

if(TARGET_RISCOS)
    # OSLib (& anything else in GCCSDK)
    target_include_directories(${LIBRARY} PUBLIC $ENV{GCCSDK_INSTALL_ENV}/include)
    target_link_libraries(${LIBRARY} PUBLIC $ENV{GCCSDK_INSTALL_ENV}/lib/libOSLib32.a)
endif()

if(USE_FORTIFY)
    target_compile_definitions(${LIBRARY} PRIVATE FORTIFY)
    target_link_libraries(${LIBRARY} PUBLIC Fortify)
endif()


# Self-test executable
#

set(TEST ${PROJECT_NAME}Test)

set(TEST_SOURCES
    app/wire/test/test.c
    base/pack/test/test.c
    databases/pickle/test/test.c
    databases/tag-db/test/test.c
    datastruct/array/test/test.c
    datastruct/atom/test/test.c
    datastruct/bitarr/test/test.c
    datastruct/bitvec/test/test.c
    datastruct/hash/test/test.c
    datastruct/list/test/test.c
    datastruct/ntree/test/test.c
    datastruct/vector/test/test.c
    geom/layout/test/test.c
    geom/packer/test/test.c
    io/filing/test/test.c
    io/stream/test/test.c
    test/main/main.c
    test/txtscr.h
    test/txtscr/txtscr.c
    text/lang/test/test.c
    text/txtfmt/test/test.c
    vdu/font/test/test.c)

# Avoid a warning from CMake
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

add_executable(${TEST} ${TEST_SOURCES})

set_target_properties(${TEST} PROPERTIES
    DESCRIPTION "AppEngine test program"
    C_STANDARD 99
    OUTPUT_NAME_DEBUG ${TEST}-debug
    OUTPUT_NAME_RELEASE ${TEST}
    OUTPUT_NAME_RELWITHDEBINFO ${TEST}-relwithdebinfo
    OUTPUT_NAME_MINSIZEREL ${TEST}-minsizerel)

target_include_directories(${TEST} PRIVATE ..)

# AETest dependencies
# Of course, AETest depends on AppEngine.
target_link_libraries(${TEST} ${LIBRARY})


# Installation
#

# Note: This affects everything built by this file.
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR} CACHE PATH "CMake install prefix" FORCE)

# Install library & test program into root of build directory.
install(TARGETS ${LIBRARY} ${TEST}
    CONFIGURATIONS Debug Release RelWithDebInfo MinSizeRel
    ARCHIVE DESTINATION .
    RUNTIME DESTINATION .)
