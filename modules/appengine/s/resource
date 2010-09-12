; --------------------------------------------------------------------------
;    Name: Resource
; Purpose: Application resource management
;  Author: © David Thomas, 1996-2009
; Version: $Id: resource.s,v 1.4 2009-09-02 23:36:13 dpt Exp $
; --------------------------------------------------------------------------


; 0.00 (30 Jan 1997)

; 0.01 (09 Feb 1997)
; - Fixed VS returns, R0 now points to error block.

; 0.02 (12 Jul 1997)
; - Factored VS returns out to resource_locate_error, saves one word.
; - Slight rearrangements to save another word.

; 0.03 (13 Sep 1997)
; - Caused aborts on non-UK systems, due to my eejit usage of R14 as a temp
;   register whilst calling a SWI.  Fixed.
; - Uses string_copy to deal with strings.


; Includes

	GET	AppEngine:SWI.Hdr.OS
	GET	Hdr.appengine

; Code

	AREA	|_resource|, CODE, READONLY

	IMPORT	string_copy

	EXPORT	library_resource

library_resource
	CMP	r0, #(library_resource_table_end-library_resource_table)/4
	ADDCC	pc, pc, r0, LSL #2
	MOV	pc, r14
library_resource_table
	B	resource_locate
library_resource_table_end


; Name:
;   resource_locate
; Purpose:
;   Determines the resource directory that should be used.
; Entry:
;   R1 -> application's directory specifier, e.g. '<App$Dir>'
; Exit:
;   R1 -> resource specifier for the current territory e.g.
;	  '<App$Dir>.Resources.Germany'.  Checks the path and if it doesn't
;	  exist defaults to '<App$Dir>.Resources.UK'.
; Internally:
;   R0 = character
;   R1 -> input specifier
;   R4 -> output
;   R6 -> start of output

resource_locate
	STMFD	r13!, {r0, r2-r6, r14}
	LDR	r2, [r12]
	MOV	r6, r2			; save this for the mo

        BL	string_copy		; R1 = input, R2 = output
	SUB     r2, r2, #1
	ADR	r1, resource_locate_res_string
        BL	string_copy
	SUB     r4, r2, #1		; R4 becomes stringptr

	MOV	r0, #240		; read the country number
	MOV	r1, #0
	MOV	r2, #255
	SWI	XOS_Byte
	BVS	resource_locate_error

	MOV	r3, r1
	MOV	r1, #&43
	MOV	r2, #2
	; R3 is the country no.  R4 points to the required position
	MOV	r5, #10			; force to 10 chars
	SWI	XOS_ServiceCall
	BVS	resource_locate_error
	MOV	r0, #13			; terminate
	STRB	r0, [r4, r5]

	MOV	r0, #17
	MOV	r1, r6
	MOV	r6, r4			; keep ptr
	SWI	XOS_File
	BVS	resource_locate_error
	CMP	r0, #0
	MOVEQ	r0, #'U'
	STREQB  r0, [r6], #1
	MOVEQ	r0, #'K'
	STREQB  r0, [r6], #1
	MOVEQ	r0, #13
	STREQB  r0, [r6], #1
	LDMFD	r13!, {r0, r2-r6, pc}
resource_locate_error
	ADD	r13, r13, #4
	LDMFD	r13!, {r2-r6, pc}


resource_locate_res_string
	DCB	".Resources.", 0
	ALIGN


	END
