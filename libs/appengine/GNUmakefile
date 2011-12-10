# GNU makefile for appengine

include ../../common.mk

# Tools

ccflags		+= $(generalinc) $(flexinc) $(oslibinc) $(zlibinc)
# Nobble TCP/IP-dependent stuff as we've no headers for it at the moment.
ccflags		+= -DAPPENGINE_NO_TCPIP

# Library references

libraries_test	= $(oslib) $(fortifydbg)

lib		= appengine.a
libdbg		= appenginedbg.a
libmod		= appendingmod.a
testapp		= aetest,ff8

# Read all objects from the filesys then swap '.c' or '.s' for '.o'.
# This isn't going to skip the duplicate / target source files (asm).
#obj	= $(patsubst %.s, %.o, $(patsubst %.c, %.o, $(filter %.c %.s, $(wildcard ./*/*))))
#obj	= $(patsubst %.c, %.o, $(filter %.c, $(wildcard ./*/*/*)))
# is this getting /all/ source files or just those three levels deep?

# target dirs need to be chosen based on cpu
cobjs		= $(shell find . -name 'test' -prune -o -name 'target' -prune -o -name '*.c' -print)
aobjs		= $(shell find . -name 'test' -prune -o -name 'target' -prune -o -name '*.s' -print)
objs		= $(cobjs:.c=.o) $(aobjs:.s=.o)
objsdbg		= $(objs:.o=.odf)
objsmod		= $(objs:.o=.om)
objstst		= $(patsubst %.c, %.odf, $(shell find . -wholename '*/test/*.c' -print))

# Targets:

.PHONY:	normal debug module all

$(lib):	$(objs)
	$(libfile) $@ $(objs)

$(libdbg):	$(objsdbg)
	$(libfile) $@ $(objsdbg)

$(libmod):	$(objsmod)
	$(libfile) $@ $(objsmod)

normal:	$(lib)
	@echo 'normal' built

debug:	$(libdbg)
	@echo 'debug' built

module:	$(libmod)
	@echo 'module' built

$(testapp):	$(objstst) $(libdbg)
	$(link) -o $@ $(objstst) $(libdbg) $(libraries_test)

test:	$(testapp)
	@echo 'test' built

all:	normal debug module
	@echo 'all' built

# Dependencies

-include $(objs:.o=.d)

