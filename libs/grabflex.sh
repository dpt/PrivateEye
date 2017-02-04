#!/bin/bash -e
#
# Fetch the source to the flex memory manager from the riscosopen.org site and
# compile it up with GCCSDK.
#
# by dpt

if [ -d flex ]; then
	echo "flex is downloaded."
	exit 0
fi

mkdir -p flex
cd flex

# Fetch
echo "Downloading flex."

export CVSROOT=:pserver:anonymous@riscosopen.org:/home/rool/cvsroot

# Would prefer to have: "for file in [flex, opts, swiextra]: download"
cvs -z9 co -p castle/RiscOS/Sources/Toolbox/Libs/flexlib/h/flex     > flex.h
cvs -z9 co -p castle/RiscOS/Sources/Toolbox/Libs/flexlib/h/opts     > opts.h
cvs -z9 co -p castle/RiscOS/Sources/Toolbox/Libs/flexlib/h/swiextra > swiextra.h
cvs -z9 co -p castle/RiscOS/Sources/Toolbox/Libs/flexlib/c/flex     > flex.c

# Build
echo "Building flex."

cat > GNUmakefile <<EOF
# GNU makefile for flex

include ../../common.mk

lib			= libflex.a
libdbg	= libflexdbg.a

objs		= flex.o
objsdbg	= \$(objs:.o=.odf)

.PHONY:	normal debug all

\$(lib):	\$(objs)
	\$(libfile) \$@ \$(objs)

\$(libdbg):	\$(objsdbg)
	\$(libfile) \$@ \$(objsdbg)

normal: \$(lib)
	@echo 'normal' built

debug:	\$(libdbg)
	@echo 'debug' built

all:	normal debug
	@echo 'all' built

-include \$(objs:.o=.d)
EOF

make all

