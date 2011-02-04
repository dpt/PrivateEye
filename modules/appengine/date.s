; --------------------------------------------------------------------------
;    Name: Date
; Purpose: Date operations
;  Author: © David Thomas, 1997-2009
; --------------------------------------------------------------------------

; 0.00 (30 Jan 1997)
; 0.01 (20 Jul 1997)
; - date_day_to_number and date_month_to_number rewritten totally so that
;   they will read the input string from any alignment and are completely
;   register-based rather than scanning a table.  They are a bit larger now.

; Includes

	GET	Hdr.appengine

; Code

	AREA	|_date|, CODE, READONLY

	EXPORT	library_date

library_date
	CMP	r0, #(library_date_table_end-library_date_table)/4
	ADDCC	pc, pc, r0, LSL #2
	MOV	pc, r14
library_date_table
	B	date_day_to_number
	B	date_month_to_number
	B	date_compare
library_date_table_end


; Name:
;   date_day_to_number
; Purpose:
;   Turns a day name string into the respective day number within the week.
; Entry:
;   R1 -> day name, only the first one or two characters are checked
; Exit:
;   R1 = day number within the week, starting at one for Sunday

	EXPORT	date_day_to_number

date_day_to_number
	STMFD	r13!, {r2, r14}
	LDRB	r2, [r1]
	ORR	r2, r2, #32		; force to lower
	LDRB	r14, [r1, #1]
	ORR	r14, r14, #32		; force to lower
	MOV	r1, #-1
	CMP	r2, #'m'
	MOVEQ	r1, #2
	CMP	r2, #'w'
	MOVEQ	r1, #4
	CMP	r2, #'f'
	MOVEQ	r1, #6
	CMP	r2, #'s'
	BNE	not_s
	CMP	r14, #'u'
	MOVEQ	r1, #1	; Su
	CMP	r14, #'a'
	MOVEQ	r1, #7	; Sa
not_s
	CMP	r2, #'t'
	BNE	not_t
	CMP	r14, #'u'
	MOVEQ	r1, #3	; Tu
	CMP	r14, #'h'
	MOVEQ	r1, #5	; Th
not_t
	LDMFD	r13!, {r2, pc}


; Name:
;   date_month_to_number
; Purpose:
;   Turns a month name string into the respective month number
; Entry:
;   R1 -> month name, only the first two or three characters are checked
; Exit:
;   R1 = month number, starting at one for January

	EXPORT	date_month_to_number

date_month_to_number
	STMFD	r13!, {r2-r4, r14}
	LDRB	r2, [r1]
	ORR	r2, r2, #32		; force to lower
	LDRB	r3, [r1, #1]
	ORR	r3, r3, #32		; force to lower
	LDRB	r4, [r1, #2]
	ORR	r4, r4, #32		; force to lower
	MOV	r1, #-1

	CMP	r2, #'d'		; d'fons
	MOVEQ	r1, #12
	CMP	r2, #'f'
	MOVEQ	r1, #2
	CMP	r2, #'o'
	MOVEQ	r1, #10
	CMP	r2, #'n'
	MOVEQ	r1, #11
	CMP	r2, #'s'
	MOVEQ	r1, #9

	CMP	r2, #'a'		; a'pu
	BNE	not_a
	CMP	r3, #'p'
	MOVEQ	r1, #4	; Ap
	CMP	r3, #'u'
	MOVEQ	r1, #8	; Au
not_a
	CMP	r2, #'m'		; mary
	BNE	not_ma
	CMP	r3, #'a'
	BNE	not_ma
	CMP	r4, #'r'
	MOVEQ	r1, #3	; Mar
	CMP	r4, #'y'
	MOVEQ	r1, #5	; May
not_ma
	CMP	r2, #'j'
	BNE	not_j
	CMP	r3, #'a'
	MOVEQ	r1, #1	; Jan
	CMP	r3, #'u'
	BNE	not_ju
	CMP	r4, #'l'
	MOVEQ	r1, #7	; Jul
	CMP	r4, #'n'
	MOVEQ	r1, #6	; Jun
not_j
not_ju
	LDMFD	r13!, {r2-r4, pc}

; Name:
;   date_compare
; Purpose:
;   Compares two UTC times (5-byte), they need not be word-aligned.
; Entry:
;   R1 -> UTC time A
;   R2 -> UTC time B
; Exit:
;   R1 = 1, if A > B (A is newer)
;	 0, if A = B (both are the same)
;	-1, if A < B (A is older)
; Internally:
;   R1 = UTC time A, major word
;   R2 = UTC time B, major word
;   R3 -> UTC time A
;   R4 -> UTC time B

date_compare
	STMFD	r13!, {r0, r2-r4, r14}
	MOV	r3, r1
	MOV	r4, r2

	ADD	r0, r3, #1		; time A
	BL	misc_load_word_from_unknown_alignment
	MOV	r2, r0

	ADD	r0, r4, #1		; time B
	BL	misc_load_word_from_unknown_alignment

	CMP	r2, r0
	LDREQB  r1, [r3]
	LDREQB  r2, [r4]
	CMPEQ	r1, r2
	MOVGT	r1, #1
	MOVEQ	r1, #0
	MOVLT	r1, #-1
	LDMFD	r13!, {r0, r2-r4, pc}


misc_load_word_from_unknown_alignment
	STMFD	r13!, {r1-r3, r14}
	BIC	r1, r0, #3
	LDMIA	r1, {r2, r3}
	AND	r1, r0, #3
	MOVS	r1, r1, LSL #3
	MOVNE	r2, r2, LSR r1
	RSBNE	r1, r1, #32
	ORRNE	r0, r2, r3, LSL r1	; R0 is word
	LDMFD	r13!, {r1-r3, pc}


	END
