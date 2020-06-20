# GNU makefile for exiftags

HERE    	:= $(strip $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST)))))

include $(APPENGINE_ROOT)/common.mk

# Tool flags

# Must search parent directory for the source in 'others'.
ccflags		+= -I$(HERE) -I$(HERE)/..

# Library references

exiflib		= exiftags.a
exiflibdbg	= exiftagsdbg.a
exifcom		= exifcom,ff8
exiftags	= exiftags,ff8
exiftime	= exiftime,ff8

objs		= $(patsubst %.c, %.o, $(shell find $(HERE) -name 'other' -prune -o -name '*.c' -print))
objsdbg		= $(objs:.o=.odf)

# Targets

.PHONY:	normal debug apps all clean

$(exiflib):	$(objs)
		$(libfile) $@ $(objs)

$(exiflibdbg):	$(objsdbg)
		$(libfile) $@ $(objsdbg)

normal:		$(exiflib)
		@echo 'normal' built

debug:		$(exiflibdbg)
		@echo 'debug' built

$(exifcom):	other/exifcom.o other/getopt.o $(exiflib)
		$(link) -o $@ other/exifcom.o other/getopt.o $(exiflib)

$(exiftags):	other/exiftags.o other/getopt.o $(exiflib)
		$(link) -o $@ other/exiftags.o other/getopt.o $(exiflib)

$(exiftime):	other/exiftime.o other/getopt.o $(exiflib)
		$(link) -o $@ other/exiftime.o other/getopt.o $(exiflib)

apps:		$(exifcom) $(exiftags) $(exiftime)
		@echo 'apps' built

all:		normal debug apps
		@echo 'all' built

clean:
		-rm -rf $(objs) $(objsdbg) $(exiflib) $(exiflibdbg) $(exifcom) $(exiftags) $(exiftime) $(deps)
		@echo Cleaned

# Dependencies

deps		= $(objs:.o=.d)
-include $(deps)
