# GNU makefile for libjpeg

HERE		:= $(strip $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST)))))

include $(APPENGINE_ROOT)/common.mk

# Include paths

# Must search parent directory for the source in 'others'.
ccflags		+= -I$(HERE) -I$(HERE)/.. $(generalinc) $(oslibinc)
ccflags		+= -DNO_GETENV

# Library references

libraries	= $(oslib) $(appengine)
librariesdbg	= $(oslib) $(appengine)

libjpeg		= libjpeg.a
libjpegdbg	= libjpegdbg.a
libtrans	= libtrans.a
libtransdbg	= libtransdbg.a
djpeg		= djpeg,ff8
cjpeg		= cjpeg,ff8
jpegtran	= jpegtran,ff8
rdjpgcom	= rdjpgcom,ff8
wrjpgcom	= wrjpgcom,ff8

sysdepmem	= jmemro.o
sysdepmemdbg	= $(sysdepmem:.o=.odf)

# library object files common to compression and decompression
comobjects	= jaricom.o jcomapi.o jutils.o jerror.o jmemmgr.o $(sysdepmem)
comobjectsdbg	= $(comobjects:.o=.odf)

# compression library object files
clibobjects	= jcapimin.o jcapistd.o jcarith.o jctrans.o jcparam.o jdatadst.o jcinit.o jcmaster.o jcmarker.o jcmainct.o jcprepct.o jccoefct.o jccolor.o jcsample.o jchuff.o jcdctmgr.o jfdctfst.o jfdctflt.o jfdctint.o
clibobjectsdbg	= $(clibobjects:.o=.odf)

# decompression library object files
dlibobjects	= jdapimin.o jdapistd.o jdarith.o jdtrans.o jdatasrc.o jdmaster.o jdinput.o jdmarker.o jdhuff.o jdmainct.o jdcoefct.o jdpostct.o jddctmgr.o jidctfst.o jidctflt.o jidctint.o jdsample.o jdcolor.o jquant1.o jquant2.o jdmerge.o
dlibobjectsdbg	= $(dlibobjects:.o=.odf)

# These objectfiles are included in libjpeg.a
libobjects	= $(clibobjects) $(dlibobjects) $(comobjects)
libobjectsdbg	= $(libobjects:.o=.odf)

# object files for sample applications (excluding library files)
cobjects	= cjpeg.o rdppm.o rdgif.o rdtarga.o rdrle.o rdbmp.o rdswitch.o cdjpeg.o
cobjectsdbg	= $(cobjects:.o=.odf)
dobjects	= djpeg.o wrppm.o wrgif.o wrtarga.o wrrle.o wrbmp.o rdcolmap.o cdjpeg.o
dobjectsdbg	= $(dobjects:.o=.odf)
trobjects	= jpegtran.o rdswitch.o cdjpeg.o transupp.o
trobjectsdbg	= $(trobjects:.o=.odf)

objs 		= $(libobjects) $(cobjects) $(dobjects) $(trobjects) rdjpgcom.o wrjpgcom.o
objsdbg		= $(libobjects:.o=.odf)

# Targets:

.PHONY:	normal debug apps all clean

$(libjpeg):	$(libobjects)
		$(libfile) $@ $(libobjects)

$(libjpegdbg):	$(libobjectsdbg)
		$(libfile) $@ $(libobjectsdbg)

$(libtrans):	transupp.o
		$(libfile) $@ transupp.o

$(libtransdbg):	transupp.odf
		$(libfile) $@ transupp.odf

normal:		$(libjpeg) $(libtrans)
		@echo 'normal' built

debug:		$(libjpegdbg) $(libtransdbg)
		@echo 'debug' built

$(djpeg):	$(dobjects) $(libjpeg)
		$(link) -o $@ $(dobjects) $(libjpeg) $(libraries)

$(cjpeg):	$(cobjects) $(libjpeg)
		$(link) -o $@ $(cobjects) $(libjpeg) $(libraries)

$(jpegtran):	$(trobjects) $(libjpeg)
		$(link) -o $@ $(trobjects) $(libjpeg) $(libraries)

$(rdjpgcom):	rdjpgcom.o
		$(link) -o $@ rdjpgcom.o $(libraries)

$(wrjpgcom):	wrjpgcom.o
		$(link) -o $@ wrjpgcom.o $(libraries)

apps:		$(djpeg) $(cjpeg) $(jpegtran) $(rdjpgcom) $(wrjpgcom)
		@echo 'apps' built

all:		normal debug apps
		@echo 'all' built

clean:
		-rm -f $(objs) $(objsdbg) $(libjpeg) $(libjpegdbg) $(libtrans) $(libtransdbg) $(djpeg) $(cjpeg) $(jpegtran) $(rdjpgcom) $(wrjpgcom) $(deps)
		@echo Cleaned

# Dependencies

deps		= $(objs:.o=.d)
-include $(deps)
