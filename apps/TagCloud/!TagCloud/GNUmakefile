# GNU makefile for TagCloud
#

HERE		:= $(strip $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST)))))

include $(APPENGINE_ROOT)/common.mk

# Disable AppEngine's TCP/IP-dependent stuff as we've no headers for it at the moment.
ccflags		+= -DAPPENGINE_NO_TCPIP

# Include paths
#
ccflags		+= $(generalinc) $(flexinc) $(oslibinc) $(zlibinc)

# Library references
#
libraries	= $(oslib) $(fortify) $(flex) $(appengine)
librariesdbg	= $(oslibdbg) $(fortifydbg) $(flexdbg) $(appenginedbg)

# App
#
app		= !RunImage,ff8
appdbg		= !RunImageDF,ff8

objs		= $(patsubst %.c, %.o, $(shell find $(HERE) -name '*.c' -print))
objsdbg		= $(objs:.o=.odf)

# Targets
#
.PHONY:		normal module debug all clean

$(app):		$(objs) $(fortify) $(flex) $(appengine)
		$(link) -o $@ $(objs) $(libraries)

$(appdbg):	$(objsdbg) $(fortifydbg) $(flexdbg) $(appenginedbg)
		$(link) -o $@ $(objsdbg) $(librariesdbg)

normal:		$(app)
		@echo 'normal' built

debug:		$(appdbg)
		@echo 'debug' built

all:		normal debug
		@echo 'all' built

$(objs):	GNUmakefile

clean:
		-rm -f $(objs) $(objsdbg) $(app) $(appdbg) $(deps)
		@echo Cleaned

# Dependencies

deps		= $(objs:.o=.d)
-include $(deps)
