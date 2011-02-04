; smull-fxp16.c

        AREA    |C$$code|, CODE, READONLY

        EXPORT  smull_fxp16

smull_fxp16
        ; int smull_fxp16(int x, int y)
        ;
        SMULL   r2, r3, r0, r1      ; R0 * R1 = R2,R3
        MOV     r0, r2, LSR #16
        ORR     r0, r0, r3, LSL #16
        MOV     pc, r14

        END
