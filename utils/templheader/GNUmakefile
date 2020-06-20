# GNU makefile for templheadr

# Rule Patterns

.c.o:;		gcc -c -Wall -Wundef -Wpointer-arith -Wuninitialized -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wunused -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Winline -Wno-unused -Wno-long-long -W -Wcast-qual -Wshadow $< -o $@

# Targets:

.PHONY:	normal all

templheadr:	templheadr.o
	gcc -o $@ $<

normal:	templheadr
	@echo 'normal' built

all:	normal
	@echo 'all' built

clean:
	-rm -f templheadr.o templheadr
