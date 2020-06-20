# GNU makefile for libpng

HERE    	:= $(strip $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST)))))

include	$(APPENGINE_ROOT)/common.mk

# Include paths

ccflags		+= $(zlibinc) -I$(HERE)

# Library references

lib		= libpng.a
libdbg		= libpngdbg.a

objs		= png.o pngerror.o pngrio.o pngwio.o pngmem.o pngpread.o pngset.o pngget.o pngread.o pngrtran.o pngrutil.o pngtrans.o pngwrite.o pngwtran.o pngwutil.o
objsdbg		= $(objs:.o=.odf)

# Targets:

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
		-rm -f $(objs) $(objsdbg) $(lib) $(libdbg) $(deps)
		@echo Cleaned

# Dependencies

deps		= $(objs:.o=.d)
-include $(deps)
