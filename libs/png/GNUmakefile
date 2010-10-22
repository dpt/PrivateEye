# GNU makefile for libpng

include ../../common.mk

# Tools

ccflags		+= $(zlibinc) -I.

# Library references

lib		= libpng.a
libdbg		= libpngdbg.a

objs		= png.o pngerror.o pngrio.o pngwio.o pngmem.o pngpread.o pngset.o pngget.o pngread.o pngrtran.o pngrutil.o pngtrans.o pngwrite.o pngwtran.o pngwutil.o
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

