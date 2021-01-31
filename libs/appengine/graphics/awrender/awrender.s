; ---------------------------------------------------------------------------
;     Name: awrender.s
;  Purpose: Low level ArtWorks rendering
;   Author: Tony Houghton
; ---------------------------------------------------------------------------

X_Bit                           EQU     &20000

XAWRender_FileInitAddress       EQU     &46080 + X_Bit
XAWRender_RenderAddress         EQU     &46081 + X_Bit
XAWRender_DocBounds             EQU     &46082 + X_Bit

V_flag  EQU 1:SHL:28

;       IMPORT  |__rt_stkovf_split_big|
        EXPORT  awrender_file_init
        EXPORT  awrender_render
        EXPORT  awrender_doc_bounds

; Arbitrary stack size needed
StackSize       EQU     1024
; This code doesn't actually extend/check the stack size, but APCS defines
; that there should be plenty, and it seems to cope with every file tried

        AREA    |AWRender$$Data|, DATA, NOINIT

callback_handler
        DCD     0
user_handle
        DCD     0
apcs_regs
        DCD     0, 0
callback_regs
        DCD     0, 0, 0, 0

callback_regs_offset    EQU     16      ; callback_regs - callback_handler
apcs_regs_offset        EQU     8       ; apcs_regs - callback_handler

; Start of code
        AREA    |AWRender$$Code|, CODE, READONLY

awrender_file_init
; a1 => document in resizable block
; a2 =  callback handler
; a3 =  document size
; a4 =  user handle
        PUSH    {v1-v6, lr}
        LDR     v1, = callback_handler
        STMIA   v1, {a2, a4, sl, fp}
        MOV     v1, a1
        SWI     XAWRender_FileInitAddress
        POPVS   {v1-v6, pc}
        MOV     v1, r0
        MOV     r12, r1
        MOV     r0, v1
        ADR     r1, callback_veneer
        MOV     lr, pc
        MOV     pc, v1
        MOVVC   a1, #0
        LDR     v1, = apcs_regs
        LDMIA   v1, {sl, fp}
        POP     {v1-v6, pc}

awrender_render
; a1 => document in resizable block
; a2 => info block
; a3 => transform matrix
; a4 => vdu block
; (sn = offset n from bottom of stack)
; s0 => resizable block
; s4 =  callback handler
; s8 =  wysiwyg
; s12 = output destination
; s16 = user handle
        MOV     ip, sp
        PUSH    {v1-v6, lr}
        LDR     v1, = callback_handler
        LDR     v2, [ip, #4]
        LDR     v3, [ip, #16]
        STMIA   v1, {v2, v3, sl, fp}
        MOV     v1, a1
        MOV     v2, a2
        SWI     XAWRender_RenderAddress
        POPVS   {v1-v6, pc}
        MOV     lr, ip
        MOV     v6, r0
        MOV     r12, r1
        MOV     r0, v1
        MOV     r1, v2
        LDMIA   lr, {r4-r7}
        ADR     r5, callback_veneer
        MOV     lr, pc
        MOV     pc, v6
        MOVVC   a1, #0
        LDR     v1, = apcs_regs
        LDMIA   v1, {sl, fp}
        POP     {v1-v6, pc}

        ALIGN

; On entry:
; r11 = reason code:
; 1 = colour, return with CC to use unaltered.
; 0 = memory:
;       R0 = new size for resizable block or -1 to read.
;       Oops, can't remember whether other input regs are relevant, I
;       set up all necessary data in awrender_handle.
;       Return values in R0-R3 corresponding to awrender_callback_regs or
;       set V and R0 points to error.
;       Also seems necessary to preserve all other regs up to r12.

callback_veneer
        TEQ     r11,#1                  ; Simply colour?
        BNE     not_colour
        CMN     r11,#0                  ; Clear C
        MOV     pc,lr
not_colour
        CMP     r11,#0
        MOVNE   pc, lr
        PUSH    {v1, sl, fp, ip, lr}
        LDR     v1, = callback_handler
        LDMIA   v1, {a2, a3, sl, fp}    ; Get APCS regs
        MOV     a4, a2
        ADD     a2, v1, #callback_regs_offset
        MOV     lr, pc
        MOV     pc, a4
        TEQ     a1, #0          ; Test whether there was an error
        ; Preserve any changes that might have been made to APCS sl or fp
        STR     sl, [v1, #apcs_regs_offset]
        STR     fp, [v1, #apcs_regs_offset + 4]
        ADD     v1, v1, #callback_regs_offset
        LDMEQIA v1, {r0-r3}
        ; New 32-bit return
        BNE     callback_error
        CMP     r0,#0   ; Clear V
        POP     {v1, sl, fp, ip, pc}
callback_error
        ; Set V
        CMP     r0,#&80000000
        CMNVC   r0,#&80000000
        POP     {v1, sl, fp, ip, pc}
        ; Original 26-bit return
        ;POP    {v1, sl, fp, ip, lr}
        ;BICEQS pc, lr, #V_flag ; Reflect above error status in V flag
        ;ORRNES pc, lr, #V_flag

awrender_doc_bounds
        PUSH    {a2,v1-v6,lr}
        SWI     XAWRender_DocBounds
        MOVVC   a1,#0
        LDR     a2,[sp],#4
        STMVCIA a2,{r2-r5}
        POP     {v1-v6,pc}

        END
