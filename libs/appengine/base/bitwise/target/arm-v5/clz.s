; clz.c
; $Id: clz.s,v 1.1 2010-01-10 21:54:23 dpt Exp $

        AREA    |C$$code|, CODE, READONLY

        EXPORT  clz

clz
        ; int clz(unsigned int x)
        ;
        CLZ     r0, r0
        MOV     pc, r14

        END
