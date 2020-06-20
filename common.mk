# Common GNU makefile for AppEngine-based applications
#

APPENGINE_ROOT	?= $(strip $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST)))))

# Toolchain
#
prefix		= $(GCCSDK_INSTALL_CROSSBIN)/arm-unknown-riscos-

# Toolchain tools
#
asm_		= $(GCCSDK_INSTALL_CROSSBIN)/asasm
cc_		= $(prefix)gcc
libfile_	= $(prefix)ar
link_		= $(prefix)gcc

# Our tools
#
templheadr_     = $(APPENGINE_ROOT)/utils/templheader/templheadr

# Tool flags
#
asmflags	= -pedantic -cpu=ARM6 -Uppercase
cstd		= -std=c99
scl		= -mlibscl -mhard-float
ccflags		= -c $(cstd) $(scl) $(throwback) $(cpu) $(warnings)
ccflags		+= -mpoke-function-name
# create foo.d with dependency information as well as compiling foo.c
ccflags		+= -MMD
libfileflags	= rc
linkflags	= $(scl) $(throwback)
templheadrflags	=

# C compiler options
#
cpu		= -march=armv3 -mtune=xscale
throwback	= -mthrowback
warnings	= -Wall -Wundef -Wpointer-arith -Wuninitialized \
		  -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wunused \
		  -Wmissing-prototypes -Wmissing-declarations -Wnested-externs \
		  -Winline -Wno-unused -Wno-long-long -W -Wcast-qual -Wshadow \
		  -pedantic

# Toolchain library references
#
oslib		= -L$(GCCSDK_INSTALL_ENV)/lib -lOSLib32
oslibdbg	= $(oslib)
oslibinc	= -I$(GCCSDK_INSTALL_ENV)/include

# Our library references
#
generalinc	= -I$(APPENGINE_ROOT)/libs

appengine	= $(APPENGINE_ROOT)/libs/appengine/appengine.a
appenginedbg	= $(APPENGINE_ROOT)/libs/appengine/appenginedbg.a

exiftags	= $(APPENGINE_ROOT)/libs/exiftags/exiftags.a
exiftagsdbg	= $(APPENGINE_ROOT)/libs/exiftags/exiftagsdbg.a

flex		= $(APPENGINE_ROOT)/libs/flex/libflex.a
flexdbg		= $(APPENGINE_ROOT)/libs/flex/libflexdbg.a
flexinc		= -I$(APPENGINE_ROOT)/libs/flex

fortify		= $(APPENGINE_ROOT)/libs/fortify/libfortify.a
fortifydbg	= $(APPENGINE_ROOT)/libs/fortify/libfortifydbg.a

libjpeg		= $(APPENGINE_ROOT)/libs/jpeg/libjpeg.a
libjpegdbg	= $(APPENGINE_ROOT)/libs/jpeg/libjpegdbg.a

libjpegtran	= $(APPENGINE_ROOT)/libs/jpeg/libtrans.a
libjpegtrandbg	= $(APPENGINE_ROOT)/libs/jpeg/libtransdbg.a

libpng		= $(APPENGINE_ROOT)/libs/png/libpng.a
libpngdbg	= $(APPENGINE_ROOT)/libs/png/libpngdbg.a

md5		= $(APPENGINE_ROOT)/libs/md5/libmd5.a
md5dbg		= $(APPENGINE_ROOT)/libs/md5/libmd5dbg.a

zlib		= $(APPENGINE_ROOT)/libs/zlib/zlib.a
zlibdbg		= $(APPENGINE_ROOT)/libs/zlib/zlibdbg.a
zlibinc		= -I$(APPENGINE_ROOT)/libs/zlib

# Combined tools and flags
#
asm		= $(asm_) $(asmflags)
cc		= $(cc_) $(ccflags)
every		= $(every_) $(everyflags)
libfile		= $(libfile_) $(libfileflags)
link		= $(link_) $(linkflags)
templheadr      = $(templheadr_) $(templheadrflags)

# Rule Patterns
#
.SUFFIXES:	.o .odf .om

.c.o:;		$(cc) -O2 -DNDEBUG $< -o $@
.c.odf:;	$(cc) -g -DFORTIFY $< -o $@
.c.om:;		$(cc) -mmodule $< -o $@
.s.o:;		$(asm) $< -o $@
.s.odf:;	$(asm) $< -o $@
.s.om:;		$(asm) $< -o $@
