# GNU makefile for Fortify

include ../../common.mk

# Tools

ccflags		+= -I..

# Library references

lib		= libfortify.a
libdbg		= libfortifydbg.a

objs		= fortify.o
objsdbg		= $(objs:.o=.odf)

# Targets:

.PHONY:	normal debug all

$(lib):	$(objs)
	$(libfile) $@ $(objs)

$(libdbg):	$(objsdbg)
	$(libfile) $@ $(objsdbg)

normal:	$(lib)
	@echo 'normal' built

debug:	$(libdbg)
	@echo 'debug' built

all:	normal debug
	@echo 'all' built

# Dependencies

-include $(objs:.o=.d)

