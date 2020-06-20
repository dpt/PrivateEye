# GNU makefile for Fortify

HERE		:= $(strip $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST)))))

include $(APPENGINE_ROOT)/common.mk

# Include paths

ccflags		+= -I$(HERE)/..

# Library references

lib		= libfortify.a
libdbg		= libfortifydbg.a

objs		= fortify.o
objsdbg		= $(objs:.o=.odf)

# Targets

.PHONY:	normal debug all clean

$(lib):		$(objs)
		$(libfile) $@ $(objs)

$(libdbg):	$(objsdbg)
		$(libfile) $@ $(objsdbg)

normal:		$(lib)
		@echo 'normal' built

debug:		$(libdbg)
		@echo 'debug' built

all:		normal debug
		@echo 'all' built

clean:
		-rm -f $(objs) $(objsdbg) $(lib) $(libdbg)
		@echo Cleaned

# Dependencies

-include $(objs:.o=.d)
