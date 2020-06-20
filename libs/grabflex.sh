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

git clone https://gitlab.riscosopen.org/RiscOS/Sources/Toolbox/ToolboxLib.git
mv ./ToolboxLib/flexlib/h/flex flex.h
mv ./ToolboxLib/flexlib/h/opts opts.h
mv ./ToolboxLib/flexlib/h/swiextra swiextra.h
mv ./ToolboxLib/flexlib/c/flex flex.c
rm -rf ./ToolboxLib

# Write out a Makefile

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

cd -

