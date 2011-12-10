# GNU makefile for exiftags

include ../../common.mk

# Tool flags:

# Must search parent directory for the source in 'others'.
ccflags		+= -I. -I..

# Library references

exiflib		= exiftags.a
exiflibdbg	= exiftagsdbg.a
exifcom		= exifcom,ff8
exiftags	= exiftags,ff8
exiftime	= exiftime,ff8

objs		= $(patsubst %.c, %.o, $(shell find . -name 'other' -prune -o -name '*.c' -print))
objsdbg		= $(objs:.o=.odf)

# Targets:

.PHONY:	normal debug apps all

$(exiflib):	$(objs)
	$(libfile) $@ $(objs)

$(exiflibdbg):	$(objsdbg)
	$(libfile) $@ $(objsdbg)

normal:	$(exiflib)
	@echo 'normal' built

debug:	$(exiflibdbg)
	@echo 'debug' built

$(exifcom):	other/exifcom.o other/getopt.o $(exiflib)
	$(link) -o $@ other/exifcom.o other/getopt.o $(exiflib)

$(exiftags):	other/exiftags.o other/getopt.o $(exiflib)
	$(link) -o $@ other/exiftags.o other/getopt.o $(exiflib)

$(exiftime):	other/exiftime.o other/getopt.o $(exiflib)
	$(link) -o $@ other/exiftime.o other/getopt.o $(exiflib)

apps:	$(exifcom) $(exiftags) $(exiftime)
	@echo 'apps' built

all:	normal debug apps
	@echo 'all' built

# Dependencies

-include $(objs:.o=.d)

