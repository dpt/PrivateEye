; --------------------------------------------------------------------------
;    Name: URL
; Purpose: URL utilities
;  Author: © David Thomas, 1997-2009
; Version: $Id: url.s,v 1.4 2009-09-02 23:36:13 dpt Exp $
; --------------------------------------------------------------------------

; Includes

	GET	AppEngine:SWI.Hdr.OS
	GET	AppEngine:Hdr.Types
	GET	Hdr.appengine

; Code

	AREA	|_url|, CODE, READONLY

	EXPORT	library_url

library_url
	CMP	r0, #(library_url_table_end-library_url_table)/4
	ADDCC	pc, pc, r0, LSL #2
	MOV	pc, r14
library_url_table
	B	url_split
	;B	 url_decode ; decodes the '%hh' stuff
library_url_table_end


; Name:
;   url_split
; Purpose:
;   Takes a URL and separates it into its constituent parts.  Strings are
;   returned CR-terminated.
; Entry:
;   R1 -> URL
; Exit:
;   R1 -> URL's terminator
;   R2 -> method string
;   R3 -> hostname
;   R4 = port number (defaults to 80)
;   R5 -> path (defaults to '/')
; Internally:
;   R1 -> next input character
;   R6 = character
;   R7 = 13 (for terminating with CR)
;   R8 -> next output character (in private workspace)

url_split

	STMFD	r13!, {r6, r7, r8, r14}

	LDR	r8, [r12]

	MOV	r7, #13

	MOV	r2, r8
url_split_method_loop
	LDRB	r6, [r1], #1
	CMP	r6, #':'
	STRNEB  r6, [r8], #1
	BNE	url_split_method_loop
	STRB	r7, [r8], #1

url_split_verify_correctness_and_that
	CMP	r6, #':'
	LDREQB  r6, [r1], #1
	CMPEQ	r6, #'/'
	LDREQB  r6, [r1], #1
	CMPEQ	r6, #'/'
	BNE	url_split_exit_with_error

	MOV	r3, r8
url_split_hostname_loop
	LDRB	r6, [r1], #1
	CMP	r6, #' '
	BLT	url_split_hostname_exit
	CMP	r6, #':'
	CMPNE	r6, #'/'
	STRNEB  r6, [r8], #1
	BNE	url_split_hostname_loop
url_split_hostname_exit
	STRB	r7, [r8], #1

	CMP	r6, #':'
	BNE	url_no_port

	STMFD	r13!, {r0, r2}
	MOV	r0, #10
	SWI	XOS_ReadUnsigned
	MOV	r4, r2
	LDMFD	r13!, {r0, r2}
	LDRB	r6, [r1], #1
	BVC	url_split_path

url_no_port
	MOV	r4, #80

url_split_path
	MOV	r5, r8
	CMP	r6, #' '
	BGT	url_split_path_loop
	MOV	r6, #'/'
	STRB	r6, [r8], #1
	STRB	r7, [r8], #1
	LDMFD	r13!, {r6, r7, r8, pc}
url_split_path_loop
	; r6 contains the last char
	CMP	r6, #' '
	STRGTB  r6, [r8], #1
	LDRGTB  r6, [r1], #1
	BGT	url_split_path_loop
	STRB	r7, [r8], #1
	LDMFD	r13!, {r6, r7, r8, pc}

url_split_exit_with_error
	ADR	r0, error_bad_url
	LDMFD	r13!, {r6, r7, r8, r14}
	;TEQ	pc, pc
	;ORRNES	pc, r14, #V	; 26-bit return
	MSR	CPSR_f, #V
	MOV	pc, r14

error_bad_url
	DCD	error_base + error_invalid_url
	DCB	"Bad URL",0
	ALIGN


	END
