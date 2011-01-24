# Common GNU makefile for PrivateEye & co.

# Tools

prefix		= $(GCCSDK_INSTALL_CROSSBIN)/arm-unknown-riscos-

asm_		= /home/riscos/cross/arm-unknown-riscos/bin/asasm
cc_		= $(prefix)gcc
libfile_	= $(prefix)ar
link_		= $(prefix)gcc

# Tool flags:

asmflags	= -pedantic -target ARM6 -objasm -upper

ccflags		= -c -std=c99 -mlibscl $(throwback) $(cpu) $(warnings)
ccflags		+= -mpoke-function-name
# create foo.d with dependency information as well as compiling foo.c
ccflags		+= -MMD

libfileflags	= rc

linkflags	= $(throwback) -mlibscl

# C compiler options

cpu		= -mtune=xscale -march=armv3
throwback	= -mthrowback
warnings	= -Wall -Wundef -Wpointer-arith -Wuninitialized \
		  -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wunused \
		  -Wmissing-prototypes -Wmissing-declarations -Wnested-externs \
		  -Winline -Wno-unused -Wno-long-long -W -Wcast-qual -Wshadow

# Library references
# These are specified relative to the root dir where apps/libs live.

generalinc	= -I../../libs

appengine	= ../../libs/appengine/appengine.a
appenginedbg	= ../../libs/appengine/appenginedbg.a

exiftags	= ../../libs/exiftags/exiftags.a
exiftagsdbg	= ../../libs/exiftags/exiftagsdbg.a

flex		= ../../libs/flex/libflex.a
flexdbg		= ../../libs/flex/libflexdbg.a
flexinc		= -I../../libs/flex

fortify		= ../../libs/fortify/libfortify.a
fortifydbg	= ../../libs/fortify/libfortifydbg.a

libjpeg		= ../../libs/jpeg/libjpeg.a
libjpegdbg	= ../../libs/jpeg/libjpegdbg.a

libjpegtran	= ../../libs/jpeg/libtrans.a
libjpegtrandbg	= ../../libs/jpeg/libtransdbg.a

libpng		= ../../libs/png/libpng.a
libpngdbg	= ../../libs/png/libpngdbg.a

md5		= ../../libs/md5/libmd5.a
md5dbg		= ../../libs/md5/libmd5dbg.a

oslib		= -L$(GCCSDK_INSTALL_ENV)/lib -lOSLib32
oslibdbg	= $(oslib)
oslibinc	= -I$(GCCSDK_INSTALL_ENV)/include

zlib		= ../../libs/zlib/zlib.a
zlibdbg		= ../../libs/zlib/zlibdbg.a
zlibinc		= -I../../libs/zlib

# Combined tools and flags

asm		= $(asm_) $(asmflags)
cc		= $(cc_) $(ccflags)
every		= $(every_) $(everyflags)
libfile		= $(libfile_) $(libfileflags)
link		= $(link_) $(linkflags)

# Rule Patterns

.SUFFIXES:	.o .odf .om

.c.o:;		$(cc) -Os -DNDEBUG $< -o $@
.c.odf:;	$(cc) -g -DFORTIFY $< -o $@
.c.om:;		$(cc) -mmodule $< -o $@
.s.o:;		$(asm) $< -o $@
.s.odf:;	$(asm) $< -o $@
.s.om:;		$(asm) $< -o $@

# Targets:

# A problem is that this becomes the first encountered rule, causing the
# default rule to become make clean.
.PHONY: clean

clean:
	-find \( -name "*.o" -o -name "*.odf" -o -name "*.om" -o -name "*.a" -or -name "*,ff8" -or -name "*.d" \) -delete
	@echo Cleaned

