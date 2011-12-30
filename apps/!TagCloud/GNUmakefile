# GNU makefile for TagCloud

include ../../common.mk

ccflags		+= $(generalinc) $(flexinc) $(oslibinc) $(zlibinc)
# Nobble TCP/IP-dependent stuff as we've no headers for it at the moment.
ccflags		+= -DAPPENGINE_NO_TCPIP

# Library references

libraries	= $(oslib) $(fortify) $(flex) $(appengine)
librariesdbg	= $(oslibdbg) $(fortifydbg) $(flexdbg) $(appenginedbg)

app		= !RunImage,ff8
appdbg		= !RunImageDF,ff8

objs		= $(patsubst %.c, %.o, $(shell find . -name '*.c' -print))
objsdbg		= $(objs:.o=.odf)

# Targets:

.PHONY:	normal module debug all

$(app):	$(objs) $(fortify) $(flex) $(appengine)
	$(link) -o $@ $(objs) $(libraries)

$(appdbg):	$(objsdbg) $(fortifydbg) $(flexdbg) $(appenginedbg)
	$(link) -o $@ $(objsdbg) $(librariesdbg)

normal:	$(app)
	@echo 'normal' built

debug:	$(appdbg)
	@echo 'debug' built

all:	normal debug
	@echo 'all' built

# Dependencies

-include $(objs:.o=.d)

