# CMakeLists.txt
#
# AppEngine
#
# Copyright (c) David Thomas, 2021
#

# TODO
#
# - Produce libraries with variant names for the options chosen (ARM
#   architecture, Fortify enabled, and so on).
# - Generate a version number for the library and place it in a header.
# - Perhaps drop the C target to C90.

if (NOT DEFINED ENV{GCCSDK_INSTALL_ENV})
    message(FATAL_ERROR "GCCSDK_INSTALL_ENV is not set")
endif()

cmake_minimum_required(VERSION 3.18)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules/)

set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG" CACHE STRING "" FORCE) # overwrite default of '-O3 -NDEBUG'

# For now, just divide it into ancient or modern ARM (pivot=ARMv5).
option(APPENGINE_ARMV5 "Utilise ARMv5" OFF)
if(APPENGINE_ARMV5)
    string(APPEND CMAKE_C_FLAGS " -march=armv5 -mtune=xscale" CACHE STRING "" FORCE)
else()
    string(APPEND CMAKE_C_FLAGS " -march=armv2 -mtune=arm2" CACHE STRING "" FORCE)
endif()

# ASM_ASASM is our CMake name for the GCCSDK objasm-format assembler 'asasm'.
project(AppEngine VERSION 0.6.8 DESCRIPTION "RISC OS application library" LANGUAGES C ASM_ASASM)

# If this is enabled then the final binary will need to be linked against
# Fortify too.
option(USE_FORTIFY "Use Fortify" OFF)

# Referencing CMAKE_TOOLCHAIN_FILE avoids a warning on rebuilds.
if(NOT ${CMAKE_TOOLCHAIN_FILE} STREQUAL "")
    message(STATUS "AppEngine: Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
endif()

set(LIBRARY appengine)

add_library(${LIBRARY})

set_target_properties(${LIBRARY} PROPERTIES
    VERSION 0.6.8
    DESCRIPTION "RISC OS application library"
    C_STANDARD 99
    PREFIX ""  # remove 'lib' prefix
    ARCHIVE_OUTPUT_NAME_DEBUG appengine-debug
    ARCHIVE_OUTPUT_NAME_RELEASE appengine
    ARCHIVE_OUTPUT_NAME_RELWITHDEBINFO appengine-relwithdebinfo
    ARCHIVE_OUTPUT_NAME_MINSIZEREL appengine-minsizerel)

target_include_directories(${LIBRARY} PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/app
    ${CMAKE_SOURCE_DIR}/base
    ${CMAKE_SOURCE_DIR}/databases
    ${CMAKE_SOURCE_DIR}/datastruct
    ${CMAKE_SOURCE_DIR}/dialogues
    ${CMAKE_SOURCE_DIR}/gadgets
    ${CMAKE_SOURCE_DIR}/geom
    ${CMAKE_SOURCE_DIR}/graphics
    ${CMAKE_SOURCE_DIR}/io
    ${CMAKE_SOURCE_DIR}/net
    ${CMAKE_SOURCE_DIR}/text
    ${CMAKE_SOURCE_DIR}/vdu
    ${CMAKE_SOURCE_DIR}/wimp)

# Look for includes in the 'libs' dir so that #include "fortify/fortify.h"
# works. Also look in the 'flex' and 'zlib' dirs directly since includers of
# those headers omit the parent directory.
target_include_directories(${LIBRARY} PRIVATE
    ${CMAKE_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/../flex
    ${CMAKE_SOURCE_DIR}/../zlib)

set(PUBLIC_HEADERS
    ${CMAKE_SOURCE_DIR}/app/choices.h
    ${CMAKE_SOURCE_DIR}/app/keymap.h
    ${CMAKE_SOURCE_DIR}/app/wire.h
    ${CMAKE_SOURCE_DIR}/base/appengine.h
    ${CMAKE_SOURCE_DIR}/base/bitwise.h
    ${CMAKE_SOURCE_DIR}/base/bsearch.h
    ${CMAKE_SOURCE_DIR}/base/bytesex.h
    ${CMAKE_SOURCE_DIR}/base/errors.h
    ${CMAKE_SOURCE_DIR}/base/fxp.h
    ${CMAKE_SOURCE_DIR}/base/heap.h
    ${CMAKE_SOURCE_DIR}/base/messages.h
    ${CMAKE_SOURCE_DIR}/base/numstr.h
    ${CMAKE_SOURCE_DIR}/base/os.h
    ${CMAKE_SOURCE_DIR}/base/oserror.h
    ${CMAKE_SOURCE_DIR}/base/pack.h
    ${CMAKE_SOURCE_DIR}/base/primes.h
    ${CMAKE_SOURCE_DIR}/base/strings.h
    ${CMAKE_SOURCE_DIR}/base/utils.h
    ${CMAKE_SOURCE_DIR}/databases/digest-db.h
    ${CMAKE_SOURCE_DIR}/databases/filename-db.h
    ${CMAKE_SOURCE_DIR}/databases/pickle-reader-hash.h
    ${CMAKE_SOURCE_DIR}/databases/pickle-writer-hash.h
    ${CMAKE_SOURCE_DIR}/databases/pickle.h
    ${CMAKE_SOURCE_DIR}/databases/tag-db.h
    ${CMAKE_SOURCE_DIR}/datastruct/array.h
    ${CMAKE_SOURCE_DIR}/datastruct/atom.h
    ${CMAKE_SOURCE_DIR}/datastruct/bitarr.h
    ${CMAKE_SOURCE_DIR}/datastruct/bitvec.h
    ${CMAKE_SOURCE_DIR}/datastruct/hash.h
    ${CMAKE_SOURCE_DIR}/datastruct/hlist.h
    ${CMAKE_SOURCE_DIR}/datastruct/list.h
    ${CMAKE_SOURCE_DIR}/datastruct/ntree.h
    ${CMAKE_SOURCE_DIR}/datastruct/vector.h
    ${CMAKE_SOURCE_DIR}/dialogues/dcs-quit.h
    ${CMAKE_SOURCE_DIR}/dialogues/info.h
    ${CMAKE_SOURCE_DIR}/dialogues/name.h
    ${CMAKE_SOURCE_DIR}/dialogues/prog-info.h
    ${CMAKE_SOURCE_DIR}/dialogues/save.h
    ${CMAKE_SOURCE_DIR}/dialogues/scale.h
    ${CMAKE_SOURCE_DIR}/gadgets/card.h
    ${CMAKE_SOURCE_DIR}/gadgets/filerwin.h
    ${CMAKE_SOURCE_DIR}/gadgets/hist.h
    ${CMAKE_SOURCE_DIR}/gadgets/iconbar.h
    ${CMAKE_SOURCE_DIR}/gadgets/imageobwin.h
    ${CMAKE_SOURCE_DIR}/gadgets/metadata.h
    ${CMAKE_SOURCE_DIR}/gadgets/scroll-list.h
    ${CMAKE_SOURCE_DIR}/gadgets/slider.h
    ${CMAKE_SOURCE_DIR}/gadgets/tag-cloud.h
    ${CMAKE_SOURCE_DIR}/gadgets/tonemap-gadget.h
    ${CMAKE_SOURCE_DIR}/gadgets/treeview.h
    ${CMAKE_SOURCE_DIR}/geom/box.h
    ${CMAKE_SOURCE_DIR}/geom/layout.h
    ${CMAKE_SOURCE_DIR}/geom/packer.h
    ${CMAKE_SOURCE_DIR}/geom/trfm.h
    ${CMAKE_SOURCE_DIR}/graphics/artworks.h
    ${CMAKE_SOURCE_DIR}/graphics/awrender.h
    ${CMAKE_SOURCE_DIR}/graphics/colour.h
    ${CMAKE_SOURCE_DIR}/graphics/drawable.h
    ${CMAKE_SOURCE_DIR}/graphics/image-observer.h
    ${CMAKE_SOURCE_DIR}/graphics/image.h
    ${CMAKE_SOURCE_DIR}/graphics/jpegtran.h
    ${CMAKE_SOURCE_DIR}/graphics/sprite-effects.h
    ${CMAKE_SOURCE_DIR}/graphics/thumbnail.h
    ${CMAKE_SOURCE_DIR}/graphics/tonemap.h
    ${CMAKE_SOURCE_DIR}/io/basic.h
    ${CMAKE_SOURCE_DIR}/io/filing.h
    ${CMAKE_SOURCE_DIR}/io/md5.h
    ${CMAKE_SOURCE_DIR}/io/stream-mem.h
    ${CMAKE_SOURCE_DIR}/io/stream-mtfcomp.h
    ${CMAKE_SOURCE_DIR}/io/stream-packbits.h
    ${CMAKE_SOURCE_DIR}/io/stream-stdio.h
    ${CMAKE_SOURCE_DIR}/io/stream.h
    ${CMAKE_SOURCE_DIR}/net/internet.h
    ${CMAKE_SOURCE_DIR}/text/txtfmt.h
    ${CMAKE_SOURCE_DIR}/types.h
    ${CMAKE_SOURCE_DIR}/vdu/font.h
    ${CMAKE_SOURCE_DIR}/vdu/screen.h
    ${CMAKE_SOURCE_DIR}/vdu/sprite.h
    ${CMAKE_SOURCE_DIR}/wimp/colourpick.h
    ${CMAKE_SOURCE_DIR}/wimp/dialogue.h
    ${CMAKE_SOURCE_DIR}/wimp/event.h
    ${CMAKE_SOURCE_DIR}/wimp/filer.h
    ${CMAKE_SOURCE_DIR}/wimp/help.h
    ${CMAKE_SOURCE_DIR}/wimp/icon.h
    ${CMAKE_SOURCE_DIR}/wimp/iconbar.h
    ${CMAKE_SOURCE_DIR}/wimp/menu.h
    ${CMAKE_SOURCE_DIR}/wimp/pointer.h
    ${CMAKE_SOURCE_DIR}/wimp/window.h)

set(PRIVATE_HEADERS
    ${CMAKE_SOURCE_DIR}/base/bitwise/util.h
    ${CMAKE_SOURCE_DIR}/base/bsearch/bsearch-impl.h
    ${CMAKE_SOURCE_DIR}/base/bytesex/util.h
    ${CMAKE_SOURCE_DIR}/datastruct/atom/impl.h
    ${CMAKE_SOURCE_DIR}/datastruct/bitvec/impl.h
    ${CMAKE_SOURCE_DIR}/datastruct/hash/impl.h
    ${CMAKE_SOURCE_DIR}/datastruct/hlist/impl.h
    ${CMAKE_SOURCE_DIR}/datastruct/ntree/impl.h
    ${CMAKE_SOURCE_DIR}/datastruct/vector/impl.h
    ${CMAKE_SOURCE_DIR}/gadgets/scroll-list/impl.h
    ${CMAKE_SOURCE_DIR}/gadgets/tag-cloud/iconnames.h
    ${CMAKE_SOURCE_DIR}/gadgets/tag-cloud/impl.h
    ${CMAKE_SOURCE_DIR}/gadgets/tag-cloud/menunames.h
    ${CMAKE_SOURCE_DIR}/geom/packer/impl.h
    ${CMAKE_SOURCE_DIR}/graphics/drawable/drawable-artworks.h
    ${CMAKE_SOURCE_DIR}/graphics/drawable/drawable-bitmap.h
    ${CMAKE_SOURCE_DIR}/graphics/drawable/drawable-drawfile.h
    ${CMAKE_SOURCE_DIR}/graphics/drawable/drawable-jpeg.h
    ${CMAKE_SOURCE_DIR}/graphics/drawable/drawable-vector.h
    ${CMAKE_SOURCE_DIR}/graphics/image/artworks.h
    ${CMAKE_SOURCE_DIR}/graphics/image/bitmap.h
    ${CMAKE_SOURCE_DIR}/graphics/image/drawfile.h
    ${CMAKE_SOURCE_DIR}/graphics/image/formats.h
    ${CMAKE_SOURCE_DIR}/graphics/image/generic.h
    ${CMAKE_SOURCE_DIR}/graphics/image/gif.h
    ${CMAKE_SOURCE_DIR}/graphics/image/jpeg.h
    ${CMAKE_SOURCE_DIR}/graphics/image/png.h
    ${CMAKE_SOURCE_DIR}/graphics/image/sprite.h
    ${CMAKE_SOURCE_DIR}/graphics/sprite-effects/convolve.h
    ${CMAKE_SOURCE_DIR}/text/lang/emit.h
    ${CMAKE_SOURCE_DIR}/text/lang/error.h
    ${CMAKE_SOURCE_DIR}/text/lang/eval.h
    ${CMAKE_SOURCE_DIR}/text/lang/lex.h
    ${CMAKE_SOURCE_DIR}/text/lang/parse.h
    ${CMAKE_SOURCE_DIR}/text/lang/sym.h
    ${CMAKE_SOURCE_DIR}/text/lang/tokens.h
    ${CMAKE_SOURCE_DIR}/text/lang/tree.h
    ${CMAKE_SOURCE_DIR}/wimp/event/event-message.h
    ${CMAKE_SOURCE_DIR}/wimp/event/event-wimp.h
    ${CMAKE_SOURCE_DIR}/wimp/window/open.h)

set(GENERIC_SOURCES
    ${CMAKE_SOURCE_DIR}/app/choices/choices.c
    ${CMAKE_SOURCE_DIR}/app/keymap/keymap.c
    ${CMAKE_SOURCE_DIR}/app/wire/wire.c
    ${CMAKE_SOURCE_DIR}/base/appengine/heap.c
    ${CMAKE_SOURCE_DIR}/base/appengine/resource.c
    ${CMAKE_SOURCE_DIR}/base/bitwise/ceillog2.c
    ${CMAKE_SOURCE_DIR}/base/bitwise/countbits.c
    ${CMAKE_SOURCE_DIR}/base/bitwise/ctz.c
    ${CMAKE_SOURCE_DIR}/base/bitwise/floorlog2.c
    ${CMAKE_SOURCE_DIR}/base/bitwise/msb.c
    ${CMAKE_SOURCE_DIR}/base/bitwise/power2gt.c
    ${CMAKE_SOURCE_DIR}/base/bitwise/power2le.c
    ${CMAKE_SOURCE_DIR}/base/bitwise/reversebits.c
    ${CMAKE_SOURCE_DIR}/base/bsearch/bsearch-int.c
    ${CMAKE_SOURCE_DIR}/base/bsearch/bsearch-short.c
    ${CMAKE_SOURCE_DIR}/base/bsearch/bsearch-uint.c
    ${CMAKE_SOURCE_DIR}/base/bsearch/bsearch-ushort.c
    ${CMAKE_SOURCE_DIR}/base/bytesex/rev-l-block.c
    ${CMAKE_SOURCE_DIR}/base/bytesex/rev-l-m.c
    ${CMAKE_SOURCE_DIR}/base/bytesex/rev-l.c
    ${CMAKE_SOURCE_DIR}/base/bytesex/rev-s-block.c
    ${CMAKE_SOURCE_DIR}/base/bytesex/rev-s-m.c
    ${CMAKE_SOURCE_DIR}/base/bytesex/rev-s-pair-m.c
    ${CMAKE_SOURCE_DIR}/base/bytesex/rev-s-pair.c
    ${CMAKE_SOURCE_DIR}/base/bytesex/rev-s.c
    ${CMAKE_SOURCE_DIR}/base/errors/errors.c
    ${CMAKE_SOURCE_DIR}/base/errors/fatal.c
    ${CMAKE_SOURCE_DIR}/base/heap/heap.c
    ${CMAKE_SOURCE_DIR}/base/messages/messages.c
    ${CMAKE_SOURCE_DIR}/base/numstr/comma-dbl.c
    ${CMAKE_SOURCE_DIR}/base/numstr/comma-num.c
    ${CMAKE_SOURCE_DIR}/base/os/beep.c
    ${CMAKE_SOURCE_DIR}/base/os/inkey.c
    ${CMAKE_SOURCE_DIR}/base/oserror/check.c
    ${CMAKE_SOURCE_DIR}/base/oserror/plot.c
    ${CMAKE_SOURCE_DIR}/base/oserror/report-block.c
    ${CMAKE_SOURCE_DIR}/base/oserror/report.c
    ${CMAKE_SOURCE_DIR}/base/pack/pack.c
    ${CMAKE_SOURCE_DIR}/base/pack/unpack.c
    ${CMAKE_SOURCE_DIR}/base/primes/primes.c
    ${CMAKE_SOURCE_DIR}/base/strings/str_branch.c
    ${CMAKE_SOURCE_DIR}/base/strings/str_cpy.c
    ${CMAKE_SOURCE_DIR}/base/strings/str_dup.c
    ${CMAKE_SOURCE_DIR}/base/strings/str_leaf.c
    ${CMAKE_SOURCE_DIR}/base/strings/str_len.c
    ${CMAKE_SOURCE_DIR}/base/strings/str_n_cpy.c
    ${CMAKE_SOURCE_DIR}/base/strings/str_n_dup.c
    ${CMAKE_SOURCE_DIR}/base/strings/str_num.c
    ${CMAKE_SOURCE_DIR}/base/strings/str_term.c
    ${CMAKE_SOURCE_DIR}/base/strings/strcasecmp.c
    ${CMAKE_SOURCE_DIR}/base/strings/strncasecmp.c
    ${CMAKE_SOURCE_DIR}/base/strings/strnncasecmp.c
    ${CMAKE_SOURCE_DIR}/base/utils/degs-to-rads.c
    ${CMAKE_SOURCE_DIR}/base/utils/gcd.c
    ${CMAKE_SOURCE_DIR}/databases/digest-db/digest-db.c
    ${CMAKE_SOURCE_DIR}/databases/filename-db/filename-db.c
    ${CMAKE_SOURCE_DIR}/databases/pickle/delete.c
    ${CMAKE_SOURCE_DIR}/databases/pickle/hash-reader.c
    ${CMAKE_SOURCE_DIR}/databases/pickle/hash-writer.c
    ${CMAKE_SOURCE_DIR}/databases/pickle/pickle.c
    ${CMAKE_SOURCE_DIR}/databases/pickle/unpickle.c
    ${CMAKE_SOURCE_DIR}/databases/tag-db/tag-db.c
    ${CMAKE_SOURCE_DIR}/datastruct/array/delelem.c
    ${CMAKE_SOURCE_DIR}/datastruct/array/delelems.c
    ${CMAKE_SOURCE_DIR}/datastruct/array/grow.c
    ${CMAKE_SOURCE_DIR}/datastruct/array/shrinkwrap.c
    ${CMAKE_SOURCE_DIR}/datastruct/array/squeeze.c
    ${CMAKE_SOURCE_DIR}/datastruct/array/stretch.c
    ${CMAKE_SOURCE_DIR}/datastruct/atom/create.c
    ${CMAKE_SOURCE_DIR}/datastruct/atom/delete-block.c
    ${CMAKE_SOURCE_DIR}/datastruct/atom/delete.c
    ${CMAKE_SOURCE_DIR}/datastruct/atom/destroy.c
    ${CMAKE_SOURCE_DIR}/datastruct/atom/for-block.c
    ${CMAKE_SOURCE_DIR}/datastruct/atom/get.c
    ${CMAKE_SOURCE_DIR}/datastruct/atom/new.c
    ${CMAKE_SOURCE_DIR}/datastruct/atom/set.c
    ${CMAKE_SOURCE_DIR}/datastruct/bitarr/count.c
    ${CMAKE_SOURCE_DIR}/datastruct/bitvec/and.c
    ${CMAKE_SOURCE_DIR}/datastruct/bitvec/clear-all.c
    ${CMAKE_SOURCE_DIR}/datastruct/bitvec/clear.c
    ${CMAKE_SOURCE_DIR}/datastruct/bitvec/count.c
    ${CMAKE_SOURCE_DIR}/datastruct/bitvec/create.c
    ${CMAKE_SOURCE_DIR}/datastruct/bitvec/destroy.c
    ${CMAKE_SOURCE_DIR}/datastruct/bitvec/ensure.c
    ${CMAKE_SOURCE_DIR}/datastruct/bitvec/eq.c
    ${CMAKE_SOURCE_DIR}/datastruct/bitvec/get.c
    ${CMAKE_SOURCE_DIR}/datastruct/bitvec/length.c
    ${CMAKE_SOURCE_DIR}/datastruct/bitvec/next.c
    ${CMAKE_SOURCE_DIR}/datastruct/bitvec/or.c
    ${CMAKE_SOURCE_DIR}/datastruct/bitvec/set-all.c
    ${CMAKE_SOURCE_DIR}/datastruct/bitvec/set.c
    ${CMAKE_SOURCE_DIR}/datastruct/bitvec/toggle.c
    ${CMAKE_SOURCE_DIR}/datastruct/hash/create.c
    ${CMAKE_SOURCE_DIR}/datastruct/hash/destroy.c
    ${CMAKE_SOURCE_DIR}/datastruct/hash/insert.c
    ${CMAKE_SOURCE_DIR}/datastruct/hash/lookup-node.c
    ${CMAKE_SOURCE_DIR}/datastruct/hash/lookup.c
    ${CMAKE_SOURCE_DIR}/datastruct/hash/remove.c
    ${CMAKE_SOURCE_DIR}/datastruct/hash/walk-cont.c
    ${CMAKE_SOURCE_DIR}/datastruct/hash/walk.c
    ${CMAKE_SOURCE_DIR}/datastruct/hlist/append.c
    ${CMAKE_SOURCE_DIR}/datastruct/hlist/copy.c
    ${CMAKE_SOURCE_DIR}/datastruct/hlist/free.c
    ${CMAKE_SOURCE_DIR}/datastruct/hlist/length.c
    ${CMAKE_SOURCE_DIR}/datastruct/hlist/list.c
    ${CMAKE_SOURCE_DIR}/datastruct/hlist/map.c
    ${CMAKE_SOURCE_DIR}/datastruct/hlist/pop.c
    ${CMAKE_SOURCE_DIR}/datastruct/hlist/push.c
    ${CMAKE_SOURCE_DIR}/datastruct/hlist/reverse.c
    ${CMAKE_SOURCE_DIR}/datastruct/hlist/to-array.c
    ${CMAKE_SOURCE_DIR}/datastruct/list/add-head.c
    ${CMAKE_SOURCE_DIR}/datastruct/list/find.c
    ${CMAKE_SOURCE_DIR}/datastruct/list/init.c
    ${CMAKE_SOURCE_DIR}/datastruct/list/remove.c
    ${CMAKE_SOURCE_DIR}/datastruct/list/walk.c
    ${CMAKE_SOURCE_DIR}/datastruct/ntree/copy.c
    ${CMAKE_SOURCE_DIR}/datastruct/ntree/delete.c
    ${CMAKE_SOURCE_DIR}/datastruct/ntree/depth.c
    ${CMAKE_SOURCE_DIR}/datastruct/ntree/first-child.c
    ${CMAKE_SOURCE_DIR}/datastruct/ntree/free.c
    ${CMAKE_SOURCE_DIR}/datastruct/ntree/get-data.c
    ${CMAKE_SOURCE_DIR}/datastruct/ntree/insert-after.c
    ${CMAKE_SOURCE_DIR}/datastruct/ntree/insert-before.c
    ${CMAKE_SOURCE_DIR}/datastruct/ntree/insert.c
    ${CMAKE_SOURCE_DIR}/datastruct/ntree/last-child.c
    ${CMAKE_SOURCE_DIR}/datastruct/ntree/max-height.c
    ${CMAKE_SOURCE_DIR}/datastruct/ntree/n-nodes.c
    ${CMAKE_SOURCE_DIR}/datastruct/ntree/new.c
    ${CMAKE_SOURCE_DIR}/datastruct/ntree/next-sibling.c
    ${CMAKE_SOURCE_DIR}/datastruct/ntree/nth-child.c
    ${CMAKE_SOURCE_DIR}/datastruct/ntree/parent.c
    ${CMAKE_SOURCE_DIR}/datastruct/ntree/prepend.c
    ${CMAKE_SOURCE_DIR}/datastruct/ntree/prev-sibling.c
    ${CMAKE_SOURCE_DIR}/datastruct/ntree/set-data.c
    ${CMAKE_SOURCE_DIR}/datastruct/ntree/unlink.c
    ${CMAKE_SOURCE_DIR}/datastruct/ntree/walk.c
    ${CMAKE_SOURCE_DIR}/datastruct/vector/clear.c
    ${CMAKE_SOURCE_DIR}/datastruct/vector/create.c
    ${CMAKE_SOURCE_DIR}/datastruct/vector/destroy.c
    ${CMAKE_SOURCE_DIR}/datastruct/vector/get.c
    ${CMAKE_SOURCE_DIR}/datastruct/vector/length.c
    ${CMAKE_SOURCE_DIR}/datastruct/vector/set-length.c
    ${CMAKE_SOURCE_DIR}/datastruct/vector/set-width.c
    ${CMAKE_SOURCE_DIR}/datastruct/vector/width.c
    ${CMAKE_SOURCE_DIR}/dialogues/dcs-quit/dcs-quit.c
    ${CMAKE_SOURCE_DIR}/dialogues/info/info.c
    ${CMAKE_SOURCE_DIR}/dialogues/name/name.c
    ${CMAKE_SOURCE_DIR}/dialogues/prog-info/prog-info.c
    ${CMAKE_SOURCE_DIR}/dialogues/save/save.c
    ${CMAKE_SOURCE_DIR}/dialogues/scale/scale.c
    ${CMAKE_SOURCE_DIR}/gadgets/card/card.c
    ${CMAKE_SOURCE_DIR}/gadgets/filerwin/filerwin.c
    ${CMAKE_SOURCE_DIR}/gadgets/hist/hist.c
    ${CMAKE_SOURCE_DIR}/gadgets/iconbar/iconbar.c
    ${CMAKE_SOURCE_DIR}/gadgets/imageobwin/imageobwin.c
    ${CMAKE_SOURCE_DIR}/gadgets/metadata/metadata.c
    ${CMAKE_SOURCE_DIR}/gadgets/scroll-list/add-row.c
    ${CMAKE_SOURCE_DIR}/gadgets/scroll-list/autoscroll.c
    ${CMAKE_SOURCE_DIR}/gadgets/scroll-list/common.c
    ${CMAKE_SOURCE_DIR}/gadgets/scroll-list/create.c
    ${CMAKE_SOURCE_DIR}/gadgets/scroll-list/delete-rows.c
    ${CMAKE_SOURCE_DIR}/gadgets/scroll-list/destroy.c
    ${CMAKE_SOURCE_DIR}/gadgets/scroll-list/events.c
    ${CMAKE_SOURCE_DIR}/gadgets/scroll-list/get-bbox.c
    ${CMAKE_SOURCE_DIR}/gadgets/scroll-list/get-win.c
    ${CMAKE_SOURCE_DIR}/gadgets/scroll-list/make-vis.c
    ${CMAKE_SOURCE_DIR}/gadgets/scroll-list/marker.c
    ${CMAKE_SOURCE_DIR}/gadgets/scroll-list/refresh-all.c
    ${CMAKE_SOURCE_DIR}/gadgets/scroll-list/refresh-row.c
    ${CMAKE_SOURCE_DIR}/gadgets/scroll-list/selection.c
    ${CMAKE_SOURCE_DIR}/gadgets/scroll-list/set-handlers.c
    ${CMAKE_SOURCE_DIR}/gadgets/scroll-list/set-row-height.c
    ${CMAKE_SOURCE_DIR}/gadgets/scroll-list/where-insert.c
    ${CMAKE_SOURCE_DIR}/gadgets/scroll-list/which.c
    ${CMAKE_SOURCE_DIR}/gadgets/slider/slider.c
    ${CMAKE_SOURCE_DIR}/gadgets/tag-cloud/create.c
    ${CMAKE_SOURCE_DIR}/gadgets/tag-cloud/destroy.c
    ${CMAKE_SOURCE_DIR}/gadgets/tag-cloud/display.c
    ${CMAKE_SOURCE_DIR}/gadgets/tag-cloud/events.c
    ${CMAKE_SOURCE_DIR}/gadgets/tag-cloud/get-win.c
    ${CMAKE_SOURCE_DIR}/gadgets/tag-cloud/highlight.c
    ${CMAKE_SOURCE_DIR}/gadgets/tag-cloud/hover.c
    ${CMAKE_SOURCE_DIR}/gadgets/tag-cloud/init.c
    ${CMAKE_SOURCE_DIR}/gadgets/tag-cloud/layout.c
    ${CMAKE_SOURCE_DIR}/gadgets/tag-cloud/open.c
    ${CMAKE_SOURCE_DIR}/gadgets/tag-cloud/redraw.c
    ${CMAKE_SOURCE_DIR}/gadgets/tag-cloud/scaling.c
    ${CMAKE_SOURCE_DIR}/gadgets/tag-cloud/set-config.c
    ${CMAKE_SOURCE_DIR}/gadgets/tag-cloud/set-handlers.c
    ${CMAKE_SOURCE_DIR}/gadgets/tag-cloud/set-tags.c
    ${CMAKE_SOURCE_DIR}/gadgets/tag-cloud/shade.c
    ${CMAKE_SOURCE_DIR}/gadgets/tag-cloud/sort.c
    ${CMAKE_SOURCE_DIR}/gadgets/tag-cloud/toolbar.c
    ${CMAKE_SOURCE_DIR}/gadgets/tonemap-gadget/tonemap-gadget.c
    ${CMAKE_SOURCE_DIR}/gadgets/treeview/treeview.c
    ${CMAKE_SOURCE_DIR}/geom/box/contains-box.c
    ${CMAKE_SOURCE_DIR}/geom/box/contains-point.c
    ${CMAKE_SOURCE_DIR}/geom/box/could-hold.c
    ${CMAKE_SOURCE_DIR}/geom/box/grow.c
    ${CMAKE_SOURCE_DIR}/geom/box/intersection.c
    ${CMAKE_SOURCE_DIR}/geom/box/intersects.c
    ${CMAKE_SOURCE_DIR}/geom/box/is-empty.c
    ${CMAKE_SOURCE_DIR}/geom/box/round.c
    ${CMAKE_SOURCE_DIR}/geom/box/round4.c
    ${CMAKE_SOURCE_DIR}/geom/box/set-origin.c
    ${CMAKE_SOURCE_DIR}/geom/box/union.c
    ${CMAKE_SOURCE_DIR}/geom/layout/layout.c
    ${CMAKE_SOURCE_DIR}/geom/packer/packer.c
    ${CMAKE_SOURCE_DIR}/geom/trfm/trfm.c
    ${CMAKE_SOURCE_DIR}/graphics/artworks/callback.c
    ${CMAKE_SOURCE_DIR}/graphics/colour/name.c
    ${CMAKE_SOURCE_DIR}/graphics/drawable/drawable-artworks.c
    ${CMAKE_SOURCE_DIR}/graphics/drawable/drawable-bitmap.c
    ${CMAKE_SOURCE_DIR}/graphics/drawable/drawable-drawfile.c
    ${CMAKE_SOURCE_DIR}/graphics/drawable/drawable-jpeg.c
    ${CMAKE_SOURCE_DIR}/graphics/drawable/drawable-vector.c
    ${CMAKE_SOURCE_DIR}/graphics/drawable/drawable.c
    ${CMAKE_SOURCE_DIR}/graphics/image/artworks.c
    ${CMAKE_SOURCE_DIR}/graphics/image/bitmap.c
    ${CMAKE_SOURCE_DIR}/graphics/image/drawfile.c
    ${CMAKE_SOURCE_DIR}/graphics/image/formats.c
    ${CMAKE_SOURCE_DIR}/graphics/image/generic.c
    ${CMAKE_SOURCE_DIR}/graphics/image/gif.c
    ${CMAKE_SOURCE_DIR}/graphics/image/image-observer.c
    ${CMAKE_SOURCE_DIR}/graphics/image/image.c
    ${CMAKE_SOURCE_DIR}/graphics/image/jpeg-meta.c
    ${CMAKE_SOURCE_DIR}/graphics/image/jpeg-utils.c
    ${CMAKE_SOURCE_DIR}/graphics/image/jpeg.c
    ${CMAKE_SOURCE_DIR}/graphics/image/png.c
    ${CMAKE_SOURCE_DIR}/graphics/image/recognise.c
    ${CMAKE_SOURCE_DIR}/graphics/image/sprite.c
    ${CMAKE_SOURCE_DIR}/graphics/jpegtran/jpegtran.c
    ${CMAKE_SOURCE_DIR}/graphics/sprite-effects/blender.c
    ${CMAKE_SOURCE_DIR}/graphics/sprite-effects/blur.c
    ${CMAKE_SOURCE_DIR}/graphics/sprite-effects/clear.c
    ${CMAKE_SOURCE_DIR}/graphics/sprite-effects/convolve.c
    ${CMAKE_SOURCE_DIR}/graphics/sprite-effects/equalise.c
    ${CMAKE_SOURCE_DIR}/graphics/sprite-effects/expand.c
    ${CMAKE_SOURCE_DIR}/graphics/sprite-effects/grey.c
    ${CMAKE_SOURCE_DIR}/graphics/sprite-effects/sharpen.c
    ${CMAKE_SOURCE_DIR}/graphics/sprite-effects/tonemap.c
    ${CMAKE_SOURCE_DIR}/graphics/thumbnail/thumbnail.c
    ${CMAKE_SOURCE_DIR}/graphics/tonemap/tonemap.c
    ${CMAKE_SOURCE_DIR}/io/basic/file.c
    ${CMAKE_SOURCE_DIR}/io/filing/buffer.c
    ${CMAKE_SOURCE_DIR}/io/filing/filetype.c
    ${CMAKE_SOURCE_DIR}/io/filing/scan.c
    ${CMAKE_SOURCE_DIR}/io/md5/md5.c
    ${CMAKE_SOURCE_DIR}/io/stream/stream-mem.c
    ${CMAKE_SOURCE_DIR}/io/stream/stream-mtfcomp.c
    ${CMAKE_SOURCE_DIR}/io/stream/stream-packbitscomp.c
    ${CMAKE_SOURCE_DIR}/io/stream/stream-packbitsdecomp.c
    ${CMAKE_SOURCE_DIR}/io/stream/stream-stdio.c
    ${CMAKE_SOURCE_DIR}/io/stream/stream.c
    ${CMAKE_SOURCE_DIR}/net/internet/mime_fromfiletype.c
    ${CMAKE_SOURCE_DIR}/net/internet/mime_tofiletype.c
    ${CMAKE_SOURCE_DIR}/net/internet/uri_decode.c
    ${CMAKE_SOURCE_DIR}/text/lang/emit.c
    ${CMAKE_SOURCE_DIR}/text/lang/error.c
    ${CMAKE_SOURCE_DIR}/text/lang/eval.c
    ${CMAKE_SOURCE_DIR}/text/lang/lex.c
    ${CMAKE_SOURCE_DIR}/text/lang/main.c
    ${CMAKE_SOURCE_DIR}/text/lang/parse.c
    ${CMAKE_SOURCE_DIR}/text/lang/sym.c
    ${CMAKE_SOURCE_DIR}/text/txtfmt/txtfmt.c
    ${CMAKE_SOURCE_DIR}/vdu/font/match.c
    ${CMAKE_SOURCE_DIR}/vdu/screen/factors.c
    ${CMAKE_SOURCE_DIR}/vdu/screen/mode-vars.c
    ${CMAKE_SOURCE_DIR}/vdu/screen/pixel-size.c
    ${CMAKE_SOURCE_DIR}/vdu/screen/read-mode-vars.c
    ${CMAKE_SOURCE_DIR}/vdu/screen/screen_clip.c
    ${CMAKE_SOURCE_DIR}/vdu/sprite/colours.c
    ${CMAKE_SOURCE_DIR}/vdu/sprite/data.c
    ${CMAKE_SOURCE_DIR}/vdu/sprite/has-alpha.c
    ${CMAKE_SOURCE_DIR}/vdu/sprite/hist.c
    ${CMAKE_SOURCE_DIR}/vdu/sprite/info.c
    ${CMAKE_SOURCE_DIR}/vdu/sprite/mask-data.c
    ${CMAKE_SOURCE_DIR}/vdu/sprite/mask-pixel.c
    ${CMAKE_SOURCE_DIR}/vdu/sprite/mkgrey.c
    ${CMAKE_SOURCE_DIR}/vdu/sprite/mode.c
    ${CMAKE_SOURCE_DIR}/vdu/sprite/palettes.c
    ${CMAKE_SOURCE_DIR}/vdu/sprite/remap-luma.c
    ${CMAKE_SOURCE_DIR}/vdu/sprite/remap.c
    ${CMAKE_SOURCE_DIR}/vdu/sprite/select.c
    ${CMAKE_SOURCE_DIR}/vdu/sprite/size.c
    ${CMAKE_SOURCE_DIR}/wimp/colourpick/colourpick.c
    ${CMAKE_SOURCE_DIR}/wimp/dialogue/dialogue.c
    ${CMAKE_SOURCE_DIR}/wimp/event/event-message-group.c
    ${CMAKE_SOURCE_DIR}/wimp/event/event-message.c
    ${CMAKE_SOURCE_DIR}/wimp/event/event-wimp-group.c
    ${CMAKE_SOURCE_DIR}/wimp/event/event-wimp.c
    ${CMAKE_SOURCE_DIR}/wimp/event/event.c
    ${CMAKE_SOURCE_DIR}/wimp/filer/open-dir.c
    ${CMAKE_SOURCE_DIR}/wimp/help/help.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/button-type.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/drag-object.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/drag.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/filetype-name.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/find.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/get-bbox.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/get-double.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/get-flags.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/get-int.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/get-name.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/get-screen-bbox.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/get-text.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/group-set-flags.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/move.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/printf.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/range-set-flags.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/set-double.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/set-flags.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/set-int.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/set-radio.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/set-selected.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/set-text.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/set-validation.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/size.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/sprite-name.c
    ${CMAKE_SOURCE_DIR}/wimp/icon/validation-printf.c
    ${CMAKE_SOURCE_DIR}/wimp/iconbar/create-device-icon.c
    ${CMAKE_SOURCE_DIR}/wimp/iconbar/create-icon.c
    ${CMAKE_SOURCE_DIR}/wimp/menu/count.c
    ${CMAKE_SOURCE_DIR}/wimp/menu/create-from-desc.c
    ${CMAKE_SOURCE_DIR}/wimp/menu/create.c
    ${CMAKE_SOURCE_DIR}/wimp/menu/destroy.c
    ${CMAKE_SOURCE_DIR}/wimp/menu/open.c
    ${CMAKE_SOURCE_DIR}/wimp/menu/popup.c
    ${CMAKE_SOURCE_DIR}/wimp/menu/set-flags.c
    ${CMAKE_SOURCE_DIR}/wimp/menu/set-icon-flags.c
    ${CMAKE_SOURCE_DIR}/wimp/menu/set-submenu.c
    ${CMAKE_SOURCE_DIR}/wimp/menu/shade-all.c
    ${CMAKE_SOURCE_DIR}/wimp/menu/tick-exclusive.c
    ${CMAKE_SOURCE_DIR}/wimp/pointer/set-shape.c
    ${CMAKE_SOURCE_DIR}/wimp/window/capture.c
    ${CMAKE_SOURCE_DIR}/wimp/window/clone.c
    ${CMAKE_SOURCE_DIR}/wimp/window/create-from-mem.c
    ${CMAKE_SOURCE_DIR}/wimp/window/create.c
    ${CMAKE_SOURCE_DIR}/wimp/window/find-template-size.c
    ${CMAKE_SOURCE_DIR}/wimp/window/get-defn.c
    ${CMAKE_SOURCE_DIR}/wimp/window/get-title.c
    ${CMAKE_SOURCE_DIR}/wimp/window/load-sprites.c
    ${CMAKE_SOURCE_DIR}/wimp/window/load-template.c
    ${CMAKE_SOURCE_DIR}/wimp/window/open-as-menu-here.c
    ${CMAKE_SOURCE_DIR}/wimp/window/open-as-menu.c
    ${CMAKE_SOURCE_DIR}/wimp/window/open-at.c
    ${CMAKE_SOURCE_DIR}/wimp/window/open-here.c
    ${CMAKE_SOURCE_DIR}/wimp/window/read-furn-dims.c
    ${CMAKE_SOURCE_DIR}/wimp/window/read-max-vis-area.c
    ${CMAKE_SOURCE_DIR}/wimp/window/redraw.c
    ${CMAKE_SOURCE_DIR}/wimp/window/set-extent2.c
    ${CMAKE_SOURCE_DIR}/wimp/window/set-submenu-extent.c
    ${CMAKE_SOURCE_DIR}/wimp/window/set-title.c)

set(ARM_SOURCES
    ${CMAKE_SOURCE_DIR}/base/appengine/xappengine_codec_op_base64_decode.s
    ${CMAKE_SOURCE_DIR}/base/appengine/xappengine_codec_op_base64_encode.s
    ${CMAKE_SOURCE_DIR}/base/appengine/xappengine_heap_claim.s
    ${CMAKE_SOURCE_DIR}/base/appengine/xappengine_heap_create_dynamic.s
    ${CMAKE_SOURCE_DIR}/base/appengine/xappengine_heap_delete.s
    ${CMAKE_SOURCE_DIR}/base/appengine/xappengine_heap_release.s
    ${CMAKE_SOURCE_DIR}/base/appengine/xappengine_heap_resize.s
    ${CMAKE_SOURCE_DIR}/base/appengine/xappengine_resource_op_locate.s
    ${CMAKE_SOURCE_DIR}/base/heap/xosheap_resize_r3.s
    ${CMAKE_SOURCE_DIR}/graphics/awrender/awrender.s
    ${CMAKE_SOURCE_DIR}/net/internet/xresolver_get_host.s
    ${CMAKE_SOURCE_DIR}/net/internet/xresolver_get_host_by_addr.s
    ${CMAKE_SOURCE_DIR}/net/internet/xresolver_get_host_by_name.s)

# ARM sources that depend on the target architecture
#
# SMULL and UMULL need ARMv3M and above
# CLZ needs ARMv5 and above
set(ARM_BEFORE_V3M_SOURCES
    ${CMAKE_SOURCE_DIR}/base/fxp/smull-fxp16.c
    ${CMAKE_SOURCE_DIR}/base/fxp/umull-fxp16.c)
set(ARM_AFTER_V3M_SOURCES
    ${CMAKE_SOURCE_DIR}/base/fxp/target/arm-v3m/smull-fxp16.s
    ${CMAKE_SOURCE_DIR}/base/fxp/target/arm-v3m/umull-fxp16.s)
set(ARM_BEFORE_V5_SOURCES
    ${CMAKE_SOURCE_DIR}/base/bitwise/clz.c)
set(ARM_AFTER_V5_SOURCES
    ${CMAKE_SOURCE_DIR}/base/bitwise/target/arm-v5/clz.s)

set(ALL_SOURCES ${PUBLIC_HEADERS} ${GENERIC_SOURCES} ${ARM_SOURCES})
if(APPENGINE_ARMV5)
    list(APPEND ALL_SOURCES ${ARM_AFTER_V3M_SOURCES} ${ARM_AFTER_V5_SOURCES})
else()
    list(APPEND ALL_SOURCES ${ARM_BEFORE_V3M_SOURCES} ${ARM_BEFORE_V5_SOURCES})
endif()

target_sources(${LIBRARY} PRIVATE ${ALL_SOURCES})

# I'm using a generator expression here to stop asasm from receiving the C
# compiler's set of options.
target_compile_options(${LIBRARY}
    PRIVATE $<$<NOT:$<COMPILE_LANGUAGE:ASM_ASASM>>:-Wall -Wextra -pedantic>)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(${LIBRARY} PRIVATE APPENGINE_DEBUG)
endif()

# Library dependencies
#

# OSLib
target_include_directories(${LIBRARY} PRIVATE $ENV{GCCSDK_INSTALL_ENV}/include)
target_link_libraries(${LIBRARY} $ENV{GCCSDK_INSTALL_ENV}/lib/libOSLib32.a)

if(USE_FORTIFY)
    target_compile_definitions(${LIBRARY} PRIVATE FORTIFY)
    target_link_libraries(${LIBRARY} PUBLIC Fortify)
endif()



# Self-test executable
#

set(TEST aetest)

set(TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/app/wire/test/test.c
    ${CMAKE_SOURCE_DIR}/base/pack/test/test.c
    ${CMAKE_SOURCE_DIR}/databases/pickle/test/test.c
    ${CMAKE_SOURCE_DIR}/databases/tag-db/test/test.c
    ${CMAKE_SOURCE_DIR}/datastruct/array/test/test.c
    ${CMAKE_SOURCE_DIR}/datastruct/atom/test/test.c
    ${CMAKE_SOURCE_DIR}/datastruct/bitarr/test/test.c
    ${CMAKE_SOURCE_DIR}/datastruct/bitvec/test/test.c
    ${CMAKE_SOURCE_DIR}/datastruct/hash/test/test.c
    ${CMAKE_SOURCE_DIR}/datastruct/list/test/test.c
    ${CMAKE_SOURCE_DIR}/datastruct/ntree/test/test.c
    ${CMAKE_SOURCE_DIR}/datastruct/vector/test/test.c
    ${CMAKE_SOURCE_DIR}/geom/layout/test/test.c
    ${CMAKE_SOURCE_DIR}/geom/packer/test/test.c
    ${CMAKE_SOURCE_DIR}/io/filing/test/test.c
    ${CMAKE_SOURCE_DIR}/io/stream/test/test.c
    ${CMAKE_SOURCE_DIR}/test/main/main.c
    ${CMAKE_SOURCE_DIR}/test/txtscr.h
    ${CMAKE_SOURCE_DIR}/test/txtscr/txtscr.c
    ${CMAKE_SOURCE_DIR}/text/lang/test/test.c
    ${CMAKE_SOURCE_DIR}/text/txtfmt/test/test.c
    ${CMAKE_SOURCE_DIR}/vdu/font/test/test.c)

# Avoid a warning from CMake
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

add_executable(${TEST}
    ${TEST_SOURCES})

set_target_properties(${TEST} PROPERTIES
    DESCRIPTION "AppEngine test program"
    C_STANDARD 99
    OUTPUT_NAME_DEBUG AETestD,ff8
    OUTPUT_NAME_RELEASE AETest,ff8
    OUTPUT_NAME_RELWITHDEBINFO AETestR,ff8
    OUTPUT_NAME_MINSIZEREL AETestM,ff8)

target_include_directories(${TEST} PRIVATE
    ${CMAKE_SOURCE_DIR}/..)

# AETest dependencies
#

# OSLib
target_include_directories(${TEST} PRIVATE $ENV{GCCSDK_INSTALL_ENV}/include)

if(USE_FORTIFY)
    target_compile_definitions(${TEST} PRIVATE FORTIFY)
    target_link_libraries(${TEST} PUBLIC Fortify)
endif()

# Of course, AETest depends on AppEngine.
target_link_libraries(${TEST} ${LIBRARY})



# Note: This affects everything built by this file.
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR} CACHE PATH "CMake install prefix" FORCE)

# Install library & AETest into root of build directory.
install(TARGETS ${LIBRARY} ${TEST}
    CONFIGURATIONS Debug Release RelWithDebInfo MinSizeRel
    ARCHIVE DESTINATION .
    RUNTIME DESTINATION .)
