; --------------------------------------------------------------------------
;    Name: Memory
; Purpose: Memory operations
;  Author: © David Thomas, 1995-2009
; --------------------------------------------------------------------------

; Includes

;	GET	AppEngine:Hdr.SWI.OS
	GET	Hdr.appengine

; Code

	AREA	|_memory|, CODE, READONLY

	EXPORT	library_memory

	EXPORT	memory_copy
	EXPORT	memory_fill

library_memory
	CMP	r0, #(library_memory_table_end-library_memory_table)/4
	ADDCC	pc, pc, r0, LSL #2
	MOV	pc, r14
library_memory_table
	B	memory_copy
	B	memory_fill
library_memory_table_end

; Name:
;   memory_copy
;
; Purpose:
;   Fast block copier.  Can cope with overlap.
;
; Entry:
;   R1 = pointer to input (byte-aligned)
;   R2 = pointer to output (byte-aligned)
;   R3 = number of bytes to copy
;
; Exit:
;   All registers preserved
;
; Bug:
;   There's a problem with backwards, unaligned copy in that it reads a word
;   too many on some byte transfers.  For example, copying from the start of
;   screen memory will cause an address exception.
;

memory_copy
	STMFD	r13!, {r0-r12, r14}
	CMP	r3, #0				; order!
	CMPNE	r2, r1
	LDMEQFD	r13!, {r0-r12, pc}		; no movement / no bytes
	BCC	forwards
	ADD	r0, r1, r3
	CMP	r2, r0
	BCC	backwards
forwards
	TST	r2, #3
	BEQ	forwards_align_dest_done
	LDRB	r0, [r1], #1
	STRB	r0, [r2], #1
	SUBS	r3, r3, #1
	LDMEQFD r13!, {r0-r12, pc}
	B	forwards
forwards_align_dest_done			; destination now aligned
	TST	r1, #3				; is source aligned?
	BNE	forwards_unaligned		; [no]
;	SWI	OS_WriteS
;	DCB	4,30,"forwards, aligned",0
;	ALIGN
forwards_thirty_two
	CMP	r3, #32
	BCC	forwards_sixteen
	LDMIA	r1!, {r0, r4-r10}
	STMIA	r2!, {r0, r4-r10}
	SUBS	r3, r3, #32
	LDMEQFD r13!, {r0-r12, pc}
	B	forwards_thirty_two
forwards_sixteen
	CMP	r3, #16
	BCC	forwards_eight
	LDMIA	r1!, {r0, r4-r6}
	STMIA	r2!, {r0, r4-r6}
	SUBS	r3, r3, #16
	LDMEQFD r13!, {r0-r12, pc}
forwards_eight
	CMP	r3, #8
	BCC	forwards_four
	LDMIA	r1!, {r0, r4}
	STMIA	r2!, {r0, r4}
	SUBS	r3, r3, #8
	LDMEQFD r13!, {r0-r12, pc}
forwards_four
	CMP	r3, #4
	BCC	forwards_bytes
	LDR	r0, [r1], #4
	STR	r0, [r2], #4
	SUBS	r3, r3, #4
	LDMEQFD r13!, {r0-r12, pc}
forwards_bytes
	CMP	r3, #0
	LDMEQFD r13!, {r0-r12, pc}
	LDR	r0, [r1]
forwards_bytes_loop
	STRB	r0, [r2], #1
	MOV	r0, r0, LSR #8
	SUBS	r3, r3, #1
	BNE	forwards_bytes_loop
	LDMFD	r13!, {r0-r12, pc}

forwards_unaligned
;	SWI	OS_WriteS
;	DCB	4,30,"forwards, unaligned",0
;	ALIGN
	AND	r12, r1, #3			; offset
	SUB	r1, r1, r12			; align src pointer
	MOV	r12, r12, LSL #3		; make it a byte-shift
	RSB	r14, r12, #32			; and get the inverse too
	LDR	r0, [r1], #4
	MOV	r0, r0, LSR r12			; keep ahead by one word
forwards_unaligned_thirty_two
	CMP	r3, #32
	BCC	forwards_unaligned_sixteen
	LDMIA	r1!, {r4-r11}
	ORR	r0, r0, r4, LSL r14
	MOV	r4, r4, LSR r12
	ORR	r4, r4, r5, LSL r14
	MOV	r5, r5, LSR r12
	ORR	r5, r5, r6, LSL r14
	MOV	r6, r6, LSR r12
	ORR	r6, r6, r7, LSL r14
	MOV	r7, r7, LSR r12
	ORR	r7, r7, r8, LSL r14
	MOV	r8, r8, LSR r12
	ORR	r8, r8, r9, LSL r14
	MOV	r9, r9, LSR r12
	ORR	r9, r9, r10, LSL r14
	MOV	r10, r10, LSR r12
	ORR	r10, r10, r11, LSL r14
	STMIA	r2!, {r0, r4-r10}
	SUBS	r3, r3, #32
	LDMEQFD r13!, {r0-r12, pc}
	MOV	r0, r11, LSR r12		; ahead
	B	forwards_unaligned_thirty_two
forwards_unaligned_sixteen
	CMP	r3, #16
	BCC	forwards_unaligned_eight
	LDMIA	r1!, {r4-r7}
	ORR	r0, r0, r4, LSL r14
	MOV	r4, r4, LSR r12
	ORR	r4, r4, r5, LSL r14
	MOV	r5, r5, LSR r12
	ORR	r5, r5, r6, LSL r14
	MOV	r6, r6, LSR r12
	ORR	r6, r6, r7, LSL r14
	STMIA	r2!, {r0, r4-r6}
	SUBS	r3, r3, #16
	LDMEQFD r13!, {r0-r12, pc}
	MOV	r0, r7, LSR r12			; ahead
forwards_unaligned_eight
	CMP	r3, #8
	BCC	forwards_unaligned_four
	LDMIA	r1!, {r4, r5}
	ORR	r0, r0, r4, LSL r14
	MOV	r4, r4, LSR r12
	ORR	r4, r4, r5, LSL r14
	STMIA	r2!, {r0, r4}
	SUBS	r3, r3, #8
	LDMEQFD r13!, {r0-r12, pc}
	MOV	r0, r5, LSR r12			; ahead
forwards_unaligned_four
	CMP	r3, #4
	BCC	forwards_unaligned_bytes
	LDR	r4, [r1], #4
	ORR	r0, r0, r4, LSL r14
	STR	r0, [r2], #4
	SUBS	r3, r3, #4
	LDMEQFD r13!, {r0-r12, pc}
	MOV	r0, r4, LSR r12			; ahead
forwards_unaligned_bytes
	CMP	r3, #0
	LDMEQFD r13!, {r0-r12, pc}
	LDR	r4, [r1]
	ORR	r0, r0, r4, LSL r14
forwards_unaligned_bytes_loop
	STRB	r0, [r2], #1
	MOV	r0, r0, LSR #8
	SUBS	r3, r3, #1
	BNE	forwards_unaligned_bytes_loop
	LDMFD	r13!, {r0-r12, pc}

backwards
	ADD	r1, r1, r3
	ADD	r2, r2, r3
backwards_loop
	TST	r2, #3
	BEQ	backwards_align_dest_done
	LDRB	r0, [r1, #-1]!
	STRB	r0, [r2, #-1]!
	SUBS	r3, r3, #1
	LDMEQFD r13!, {r0-r12, pc}
	B	backwards_loop
backwards_align_dest_done			; destination now aligned
	TST	r1, #3				; is source aligned?
	BNE	backwards_unaligned		; [no]
;	SWI	OS_WriteS
;	DCB	4,30,"backwards, aligned",0
;	ALIGN
backwards_thirty_two
	CMP	r3, #32
	BCC	backwards_sixteen
	LDMDB	r1!, {r0, r4-r10}
	STMDB	r2!, {r0, r4-r10}
	SUBS	r3, r3, #32
	LDMEQFD r13!, {r0-r12, pc}
	B	backwards_thirty_two
backwards_sixteen
	CMP	r3, #16
	BCC	backwards_eight
	LDMDB	r1!, {r0, r4-r6}
	STMDB	r2!, {r0, r4-r6}
	SUBS	r3, r3, #16
	LDMEQFD r13!, {r0-r12, pc}
backwards_eight
	CMP	r3, #8
	BCC	backwards_four
	LDMDB	r1!, {r0, r4}
	STMDB	r2!, {r0, r4}
	SUBS	r3, r3, #8
	LDMEQFD r13!, {r0-r12, pc}
backwards_four
	CMP	r3, #4
	BCC	backwards_bytes
	LDR	r0, [r1, #-4]!
	STR	r0, [r2, #-4]!
	SUBS	r3, r3, #4
	LDMEQFD r13!, {r0-r12, pc}
backwards_bytes
	CMP	r3, #0
	LDMEQFD r13!, {r0-r12, pc}
	LDR	r0, [r1, #-4]
backwards_bytes_loop
	STRB	r0, [r2, #-1]!
	MOV	r0, r0, LSR #8
	SUBS	r3, r3, #1
	BNE	backwards_bytes_loop
	LDMFD	r13!, {r0-r12, pc}

backwards_unaligned
;	SWI	OS_WriteS
;	DCB	4,30,"backwards, unaligned",0
;	ALIGN
	AND	r12, r1, #3			; offset
	SUB	r1, r1, r12			; align src pointer
	MOV	r12, r12, LSL #3		; make it a byte-shift
	RSB	r14, r12, #32			; and get the inverse too
backwards_unaligned_thirty_two
	CMP	r3, #32
	BCC	backwards_unaligned_sixteen
	ADD	r1, r1, #4
	LDMDB	r1!, {r0, r4-r11}
	MOV	r0, r0, LSR r12
	ORR	r0, r0, r4, LSL r14
	MOV	r4, r4, LSR r12
	ORR	r4, r4, r5, LSL r14
	MOV	r5, r5, LSR r12
	ORR	r5, r5, r6, LSL r14
	MOV	r6, r6, LSR r12
	ORR	r6, r6, r7, LSL r14
	MOV	r7, r7, LSR r12
	ORR	r7, r7, r8, LSL r14
	MOV	r8, r8, LSR r12
	ORR	r8, r8, r9, LSL r14
	MOV	r9, r9, LSR r12
	ORR	r9, r9, r10, LSL r14
	MOV	r10, r10, LSR r12
	ORR	r10, r10, r11, LSL r14
	STMDB	r2!, {r0, r4-r10}
	SUBS	r3, r3, #32
	LDMEQFD r13!, {r0-r12, pc}
	B	backwards_unaligned_thirty_two
backwards_unaligned_sixteen
	CMP	r3, #16
	BCC	backwards_unaligned_eight
	ADD	r1, r1, #4
	LDMDB	r1!, {r0, r4-r7}
	MOV	r0, r0, LSR r12
	ORR	r0, r0, r4, LSL r14
	MOV	r4, r4, LSR r12
	ORR	r4, r4, r5, LSL r14
	MOV	r5, r5, LSR r12
	ORR	r5, r5, r6, LSL r14
	MOV	r6, r6, LSR r12
	ORR	r6, r6, r7, LSL r14
	STMDB	r2!, {r0, r4-r6}
	SUBS	r3, r3, #16
	LDMEQFD r13!, {r0-r12, pc}
backwards_unaligned_eight
	CMP	r3, #8
	BCC	backwards_unaligned_four
	ADD	r1, r1, #4
	LDMDB	r1!, {r0, r4, r5}
	MOV	r0, r0, LSR r12
	ORR	r0, r0, r4, LSL r14
	MOV	r4, r4, LSR r12
	ORR	r4, r4, r5, LSL r14
	STMDB	r2!, {r0, r4}
	SUBS	r3, r3, #8
	LDMEQFD r13!, {r0-r12, pc}
backwards_unaligned_four
	CMP	r3, #4
	BCC	backwards_unaligned_bytes
	ADD	r1, r1, #4
	LDMDB	r1!, {r0, r4}
	MOV	r0, r0, LSR r12
	ORR	r0, r0, r4, LSL r14
	STR	r0, [r2, #-4]!
	SUBS	r3, r3, #4
	LDMEQFD r13!, {r0-r12, pc}
backwards_unaligned_bytes
	CMP	r3, #0
	LDMEQFD r13!, {r0-r12, pc}
	ADD	r1, r1, #4
	LDMDB	r1!, {r0, r4}
	MOV	r0, r0, LSR r12
	ORR	r0, r0, r4, LSL r14
backwards_unaligned_bytes_loop
	STRB	r0, [r2, #-1]!
	MOV	r0, r0, LSR #8
	SUBS	r3, r3, #1
	BNE	backwards_unaligned_bytes_loop
	LDMFD	r13!, {r0-r12, pc}


; Name:
;   memory_fill
;
; Purpose:
;   Fast memory filler.
;
; Entry:
;   R1 = pointer to area to fill (word aligned)
;   R2 = number of bytes to fill
;   R3 = value to fill area with (word)
;
; Exit:
;   R1 = pointer to byte after last filled byte
;   R2 = 0, if successfull
;   R3 preserved

memory_fill
	STMFD	r13!, {r0, r4-r12, r14}	; preserve all but those used
	MOV	r0, r3
	MOV	r4, r3
	MOV	r5, r3
	MOV	r6, r3
	MOV	r7, r3
	MOV	r8, r3
	MOV	r9, r3
	MOV	r10, r3
	MOV	r11, r3
	MOV	r12, r3
	MOV	r14, r3

fill_48
	CMP	r2, #48
	BCC	fill_32
fill_48_loop
	STMIA	r1!, {r0, r3-r12, r14}
	SUB	r2, r2, #48
	CMP	r2, #48
	BHS	fill_48_loop

fill_32
	CMP	r2, #32
	BCC	fill_16
	STMIA	r1!, {r0, r3-r9}
	SUB	r2, r2, #32

fill_16
	CMP	r2, #16
	BCC	fill_8
	STMIA	r1!, {r0, r3-r5}
	SUB	r2, r2, #16

fill_8
	CMP	r2, #8
	BCC	fill_4
	STMIA	r1!, {r0, r3}
	SUB	r2, r2, #8

fill_4
	CMP	r2, #4
	BCC	fill_2
	STR	r0, [r1], #4
	SUB	r2, r2, #4

fill_2
	CMP	r2, #2
	BCC	fill_1
	STRB	r0, [r1], #1
	MOV	r0, r0, LSR #8		; adjust so pattern comes out ok
	STRB	r0, [r1], #1
	SUB	r2, r2, #2

fill_1
	CMP	r2, #1
	BCC	fill_0
	MOV	r0, r0, LSR #8		; adjust so pattern comes out ok
	STRB	r0, [r1], #1
	SUB	r2, r2, #1

fill_0
	LDMFD	r13!, {r0, r4-r12, pc}

	END
