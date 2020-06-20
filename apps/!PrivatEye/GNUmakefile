# GNU makefile for PrivateEye
#

HERE		:= $(strip $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST)))))

include $(APPENGINE_ROOT)/common.mk

# Features
#

# Enable features
ccflags		+= -DEYE_TAGS -DEYE_THUMBVIEW -DEYE_ZONES -DEYE_META -DEYE_CANVAS

# Disable AppEngine's TCP/IP-dependent stuff as we've no headers for it at the moment.
ccflags		+= -DAPPENGINE_NO_TCPIP

# Include paths
#
ccflags		+= $(generalinc) $(flexinc) $(oslibinc) $(zlibinc)

# Library references
#
libraries	= $(oslib) $(fortify) $(flex) $(appengine) $(exiftags) $(libjpeg) $(libjpegtran) $(libpng) $(md5) $(zlib)
librariesdbg	= $(oslibdbg) $(fortifydbg) $(flexdbg) $(appenginedbg) $(exiftagsdbg) $(libjpegdbg) $(libjpegtrandbg) $(libpngdbg) $(md5dbg) $(zlibdbg)

# App
#
app		= !RunImage,ff8
appdbg		= !RunImageDF,ff8

objs		= $(patsubst %.c, %.o, $(shell find $(HERE) -name '*.c' -print))
objsdbg		= $(objs:.o=.odf)

# Targets
#
.PHONY:		normal module debug all clean

$(app):		$(objs) $(fortify) $(flex) $(appengine) $(exiftags) $(libjpeg) $(libjpegtran) $(libpng) $(md5) $(zlib)
		$(link) -o $@ $(objs) $(libraries)

$(appdbg):	$(objsdbg) $(fortifydbg) $(flexdbg) $(appenginedbg) $(exiftagsdbg) $(libjpegdbg) $(libjpegtrandbg) $(libpngdbg) $(md5dbg) $(zlibdbg)
		$(link) -o $@ $(objsdbg) $(librariesdbg)

normal:		$(app)
		@echo 'normal' built

debug:		$(appdbg)
		@echo 'debug' built

all:		normal debug
		@echo 'all' built

iconnames.h: 	Resources/UK/Templates,fec
		$(templheadr) $< $@

$(objs):	iconnames.h GNUmakefile

clean:
		-rm -f $(objs) $(objsdbg) $(app) $(appdbg) iconnames.h $(deps)
		@echo Cleaned

# Dependencies

deps		= $(objs:.o=.d)
-include $(deps)
