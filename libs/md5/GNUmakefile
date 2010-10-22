# GNU makefile for md5

include ../../common.mk

# Tools

ccflags		+= -I.

# Library references

lib		= libmd5.a
libdbg		= libmd5dbg.a
md5		= md5,ff8

cobjs		= $(shell find -name 'other' -prune -o -name '*.c' -print)
objs		= $(cobjs:.c=.o)
objsdbg		= $(objs:.o=.odf)

# Targets:

.PHONY:	normal debug apps all

$(lib):	$(objs)
	$(libfile) $@ $(objs)

$(libdbg):	$(objsdbg)
	$(libfile) $@ $(objsdbg)

normal:	$(lib)
	@echo 'normal' built

debug:	$(libdbg)
	@echo 'debug' built

$(md5):	other/main.o $(lib)
	$(link) -o $@ other/main.o $(lib)

apps:	$(md5)
	@echo 'apps' built

all:	normal debug apps
	@echo 'all' built

# Dependencies

-include $(objs:.o=.d)

