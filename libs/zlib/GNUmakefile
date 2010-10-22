# GNU makefile for zlib

include ../../common.mk

# Tools

ccflags		+= -I.

# Library references

lib		= zlib.a
libdbg		= zlibdbg.a

objs		= adler32.o compress.o crc32.o deflate.o infback.o inffast.o inflate.o inftrees.o trees.o uncompr.o zutil.o
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

