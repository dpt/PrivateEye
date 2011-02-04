; clz.c

        AREA    |C$$code|, CODE, READONLY

        EXPORT  clz

clz
        ; int clz(unsigned int x)
        ;
        CLZ     r0, r0
        MOV     pc, r14

        END
