; ---------------------------------------------------------------------------
;    Name: Icon
; Purpose: Wimp icon operations
;  Author: © David Thomas, 1996-2009
; Version: $Id: icon.s,v 1.4 2009-09-02 23:36:12 dpt Exp $
; ---------------------------------------------------------------------------

; Includes

	GET	AppEngine:SWI.Hdr.OS
	GET	AppEngine:SWI.Hdr.Wimp

; Code

	AREA	|_icon|, CODE, READONLY

	IMPORT	string_compare
	IMPORT	string_copy
	IMPORT	string_left_ellipsis
	IMPORT	string_length

	EXPORT	library_icon

library_icon
	CMP	r0, #(library_icon_table_end-library_icon_table)/4
	ADDCC	pc, pc, r0, LSL #2
	MOV	pc, r14
library_icon_table
	; State
	MOV	pc, r14					; 0
	MOV	pc, r14					; 1
	; Text
	MOV	pc, r14					; 2
	B	icon_get_text				; 3
	B	icon_set_text				; 4
;	B	icon_set_text_with_left_ellipsis
;	B	icon_set_text_with_right_ellipsis
	; Numbers
;	B	icon_increment
;	B	icon_decrement
;	B	icon_get_int
;	B	icon_set_int
	; Validation
;	B	icon_get_validation_address
;	B	icon_get_validation
;	B	icon_set_validation
	; Whole-icon ops
;	B	icon_redraw
;	B	icon_get_size
;	B	icon_set_size
;	B	icon_get_selected
;	B	icon_set_selected
;	B	icon_get_shaded
;	B	icon_set_shaded
;	B	icon_get_deleted
;	B	icon_set_deleted
;	B	icon_get_radio_selection
;	B	icon_set_radio_selection
library_icon_table_end


icon_get_state
; r1/r2/r3=buffer/win/icon
	STMFD	r13!, {r0, r14}
	STMIA	r1, {r2, r3}
	SWI	XWimp_GetIconState
	LDMFD	r13!, {r0, pc}		; leave flags


icon_set_state
; r1/r2/r3/r4/r5=buffer/win/icon/eor/clear
	STMFD	r13!, {r0, r14}
	STMIA	r1, {r2-r5}
	SWI	XWimp_SetIconState
	LDMFD	r13!, {r0, pc}		; leave flags


icon_get_text_address
; in r1/r2/r3=buffer/win/icon
; out r1 -> icon's text buffer
	STMFD	r13!, {r0, r14}
	BL	icon_get_state
	LDR	r1, [r1, #28]
	LDMFD	r13!, {r0, pc}


icon_get_text

; Purpose:
;   Reads the specified icon's text.
; Entry:
;   R1 = window handle
;   R2 = icon handle
; Exit:
;   R1, R2 preserved
;   R3 -> icon text, up to 255 chars, CR terminated
; Internally:
;   R0 = temp/character

	STMFD	r13!, {r0-r2, r4, r14}

	LDR	r0, [r12]		; get address of temp space
	STMIA	r0, {r1, r2}
	MOV	r1, r0
	SWI	XWimp_GetIconState
	ADDVS	r13, r13, #4
	LDMVSFD r13!, {r1-r2, r4, pc}

	MOV	r3, r1			; output ptr
	MOV	r4, #0			; init index

	LDR	r0, [r1, #24]		; icon flags

	TST	r0, #&00000001		; if not text then
	BEQ	icon_get_text_exit	;   return with empty string }
	TST	r0, #&00000100		; if indirected then
	LDRNE	r1, [r1, #28]		;   point to indirected string,
	ADDEQ	r1, r1, #28		;   else, point to icondata string }

icon_get_text_loop
	LDRB	r0, [r1], #1
	CMP	r0, #32
	STRGEB  r0, [r3, r4]
	ADDGE	r4, r4, #1		; inc index
	BGE	icon_get_text_loop
icon_get_text_exit
	MOV	r0, #13			; terminate the string with CR
	STRB	r0, [r3, r4]

	LDMFD	r13!, {r0-r2, r4, pc}


icon_set_text

; Purpose:
;   Sets the specified icon's text.
; Entry:
;   R1 = window handle
;   R2 = icon handle
;   R3 -> icon text, up to 255 chars, control terminated.
; Exit:
;   R1, R2, R3 preserved
; Internally:
;   R0 = temp/character

	STMFD	r13!, {r0-r7, r14}

	LDR	r0, [r12]		; get address of temp space
	ADD	r0, r0, #256		; ... point past string buffer
	STMIA	r0, {r1, r2}		; !temp=win:temp!4=icon
	MOV	r1, r0			; fix the registers
	SWI	XWimp_GetIconState
	ADDVS	r13, r13, #4		; ignore stored R0
	LDMVSFD r13!, {r1-r7, pc}	; return with R0->error block

	ADD	r0, r1, #24
	LDMIA	r0, {r4-r7}		; R4 = flags
					; R5 -> buffer
					; R6 -> validation
					; R7 = buffer length

	TST	r4, #&00000001		; if not text and
	TSTNE	r4, #&00000100		;   not indirected then
	LDMEQFD r13!, {r0-r7, pc}	;   return with no action

	CMP	r7, #1			; if <=1 bytes then
	LDMLEFD r13!, {r0-r7, pc}	;   return with no action

	STMFD	r13!, {r1}
	MOV	r1, r3
	BL	string_length		; R1 'corrupted', R2 = length
	LDMFD	r13!, {r1}
	; preserve r2 (strlength) from now on

	STMFD	r13!, {r1}
	CMP	r2, r7
	MOVGE	r1, r3			; -> source string
	SUBGE	r2, r7, #1		; = max length
	BLGE	string_left_ellipsis	; R1 'corrupted', R3 -> output
	LDMFD	r13!, {r1}

	STMFD	r13!, {r1-r7}
	MOV	r1, r3
	MOV	r2, r5
	BL	string_compare		; R3 is 0 if identical
	CMP	r3, #0
	LDMFD	r13!, {r1-r7}
	LDMEQFD r13!, {r0-r7, pc}	;   return with no action

	STMFD	r13!, {r1, r2}
	MOV	r1, r3
	MOV	r2, r5
	BL	string_copy
	LDMFD	r13!, {r1, r2}

	STMFD	r13!, {r1, r2}
	LDR	r0, [r1]
	ADD	r5, r1, #8
	; Scan the validation string for an Rx command & adjust size
	TST	r4, #&00000004
	LDMIA	r5, {r1-r4}
	BLNE	icon_adjust_bbox
	SWI	XWimp_ForceRedraw
	LDMFD	r13!, {r1, r2}

	LDMIA	r1, {r3, r4}
	ADD	r1, r1, #40
	SWI	XWimp_GetCaretPosition
	MOV	r6, r1
	LDMIA	r6!, {r0, r1}
	CMP	r0, r3
	CMPEQ	r1, r4
	MOVEQ	r5, r2
	LDMEQIA	r6, {r2-r4}
	SWIEQ	XWimp_SetCaretPosition

	LDMFD	r13!, {r0-r7, pc}

icon_adjust_bbox
	; R6 points to validation string
	STMFD	r13!, {r0, r14}
icon_adjust_bbox_find_r
	LDRB	r0, [r6], #1
	CMP	r0, #32
	MOVLT	r0, #'0'
	BLT	icon_adjust_bbox_found_r
	ORR	r0, r0, #32		; make lower-case
	CMP	r0, #'r'
	LDREQB	r0, [r6], #1		; read the border type no.
	BEQ	icon_adjust_bbox_found_r	; skip fwd if we've found 'r'
icon_adjust_bbox_find_next
	LDRB	r0, [r6], #1
	CMP	r0, #32
	MOVLT	r0, #'0'
	BLT	icon_adjust_bbox_found_r
	CMP	r0, #';'
	BNE	icon_adjust_bbox_find_next
	B	icon_adjust_bbox_find_r

icon_adjust_bbox_found_r
	MOV	r5, #1			; default
	CMP	r0, #'1'
	CMPNE	r0, #'2'
	CMPNE	r0, #'5'
	MOVEQ	r5, #4
	CMP	r0, #'3'
	CMPNE	r0, #'4'
	MOVEQ	r5, #8
	CMP	r0, #'6'
	MOVEQ	r5, #12
	CMP	r0, #'7'
	MOVEQ	r5, #9

	ADD	r1, r1, r5
	ADD	r2, r2, r5
	SUB	r3, r3, r5
	SUB	r4, r4, r5

	TST	r5, #1			; is it a 'thin' border? (bit 1 set)
	LDMEQFD	r13!, {r0, pc}		;   no, return

	STMFD	r13!, {r1}
	ADR	r0, icon_adjust_bbox_vdu_in
	ADR	r1, icon_adjust_bbox_vdu_out
	SWI	XOS_ReadVduVariables
	LDMFD	r13!, {r1}

	MOV	r0, #1
	LDR	r6, icon_adjust_bbox_vdu_out
	MOV	r6, r0, LSL r6
	SUB	r6, r6, #1
	LDR	r7, icon_adjust_bbox_vdu_out+4
	MOV	r7, r0, LSL r7
	SUB	r7, r7, #1

	ADD	r1, r1, r6		; round up/down the bbox
	ADD	r2, r2, r7
	BIC	r1, r1, r6
	BIC	r2, r2, r7
	BIC	r3, r3, r6
	BIC	r4, r4, r7

	LDMFD	r13!, {r0, pc}

icon_adjust_bbox_vdu_in
	DCD	4, 5, -1
icon_adjust_bbox_vdu_out
	DCD	0, 0


;	STMFD	r13!, {r0-r7}
;	MOV	r0, r2
;	ADR	r1, buff
;	MOV	r2, #12
;	SWI	OS_ConvertCardinal4
;	SWI	OS_WriteI+4
;	SWI	OS_WriteI+30
;	SWI	OS_Write0
;	SWI	OS_NewLine
;	MOV	r0, r7
;	ADR	r1, buff
;	MOV	r2, #12
;	SWI	OS_ConvertCardinal4
;	SWI	OS_Write0
;	LDMFD	r13!, {r0-r7}

;buff
;	DCB	"           ",0
;	ALIGN


	END
