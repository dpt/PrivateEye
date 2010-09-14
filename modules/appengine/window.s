; --------------------------------------------------------------------------
;    Name: Window
; Purpose: Wimp window operations
;  Author: © David Thomas, 1996-2009
; Version: $Id: window.s,v 1.5 2009-09-08 23:05:37 dpt Exp $
; --------------------------------------------------------------------------


; 0.00 30 Jan 1997

; 0.01 09 Feb 1997
; - Fixed VS returns, R0 now points to error block.

; 0.02 14 Jul 1998

; 0.03 24 Jul 1998
; - Removed unused routines.


; Includes

	GET	AppEngine:SWI.Hdr.Wimp
	GET	Hdr.appengine

; Code

	AREA	|_window|, CODE, READONLY

	EXPORT  library_window

library_window
	CMP	r0, #(library_window_table_end-library_window_table)/4
	ADDLO	pc, pc, r0, LSL #2
	MOV	pc, r14
library_window_table
	MOV	pc, r14
	MOV	pc, r14
	MOV	pc, r14
	MOV	pc, r14
	MOV	pc, r14
	MOV	pc, r14
	B	win_get_title
	B	win_set_title
	B	export_drag_entry_points
library_window_table_end


; Purpose:
;   Returns the specified window's title string.
; Entry:
;   R1 = window handle
; Exit:
;   R1 preserved
;   R2 -> window title, up to 255 chars, CR terminated.
; Internally:
;   R3 = string index
; Notes;
;   - It doesn't matter whether the title bar is indirected or not.
;   - If it's not a text title bar, then the string will be empty.

win_get_title
	STMFD	r13!, {r0, r1, r3, r14}
	MOV	r2, r1
	LDR	r1, [r12]
	STR	r2, [r1]		; window handle
	ORR	r1, r1, #1		; header only
	SWI	XWimp_GetWindowInfo
	ADDVS	r13, r13, #4
	LDMVSFD r13!, {r1, r3, pc}
	BIC	r1, r1, #1

	MOV	r2, r1			; output ptr
	MOV	r3, #0			; init index

	LDR	r0, [r1, #60]		; title flags
	TST	r0, #1<<0		; if (not text) {
	BEQ	win_get_title_exit	;   return with empty string }
	TST	r0, #1<<8		; if (indirected) {
	LDRNE	r1, [r1, #76]		;   point to indirected string,
	ADDEQ	r1, r1, #76		;   else, point to icondata string }

win_get_title_loop
	LDRB	r0, [r1], #1
	CMP	r0, #32
	STRGEB  r0, [r2, r3]
	ADDGE	r3, r3, #1		; inc stringlen counter
	BGE	win_get_title_loop
win_get_title_exit
	MOV	r0, #13			; terminate the string with CR {
	STRB	r0, [r2, r3]		;   }
	LDMFD	r13!, {r0, r1, r3, pc}


; Purpose:
;   Sets the specified window's title string.
; Entry:
;   R1 = window handle
;   R2 -> new window title, up to 255 chars, control-terminated.
; Exit:
;   R1, R2 preserved
; Internally:
;   R3 = string index
; Notes;
;   - No error will be reported if it's not a text title bar.

win_set_title
	STMFD	r13!, {r0-r3, r14}
	MOV	r3, r1
	LDR	r1, [r12]
	STR	r3, [r1]		; window handle
	ORR	r1, r1, #1		; header only
	SWI	XWimp_GetWindowInfo
	ADDVS	r13, r13, #4
	LDMVSFD r13!, {r1-r3, pc}
	BIC	r1, r1, #1
	LDR	r0, [r1, #60]		; title flags
	TST	r0, #1<<0		; if (not text) {
	LDMEQFD r13!, {r0-r3, pc}	;   return (with no error) }
	TST	r0, #1<<8		; if (indirected) {
	LDRNE	r3, [r1, #76]		;   point to indirected string,
	ADDEQ	r3, r1, #76		;   else, point to icondata string }
win_set_title_loop
	LDRB	r0, [r2], #1
	STRB	r0, [r3], #1
	CMP	r0, #32
	BGE	win_set_title_loop
	; Refresh the title bar
	SWI	XWimp_GetWindowOutline
	BVS	win_set_title_error_return
	MOV	r0, #-1
	ADD	r5, r1, #4
	LDMIA	r5, {r1-r4}
	SUB	r2, r4, #44
	SWI	XWimp_ForceRedraw
	LDMVCFD r13!, {r0-r3, pc}

win_set_title_error_return
	ADD	r13, r13, #4		; VS
	LDMFD	r13!, {r1-r3, pc}


export_drag_entry_points
	ADR	r0, draw
	ADR	r1, undraw
	ADR	r2, redraw
	MOV	pc, r14

draw
undraw
	MOV	pc, r14

redraw
	STMFD	r13!, {r14}
	LDR	r14, [r12]		; increment pollword
	ADD	r14, r14, #1
	STR	r14, [r12]
	LDMFD	r13!, {pc}

	END
