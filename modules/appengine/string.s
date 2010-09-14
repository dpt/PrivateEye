; --------------------------------------------------------------------------
;    Name: String
; Purpose: Handling of strings
;  Author: © David Thomas, 1996-2009
; Version: $Id: string.s,v 1.4 2009-09-02 23:36:13 dpt Exp $
; --------------------------------------------------------------------------


; Note: This library uses ASCII 13 for output string terminators, but
;	accepts any control character as input terminator.


; Includes

	GET	Hdr.appengine

; Code

	AREA	|_string|, CODE, READONLY

	EXPORT	library_string

	EXPORT	string_read_memory_to_linefeed
	EXPORT	string_read_memory_to_control
	EXPORT	string_left_ellipsis
	EXPORT	string_right_ellipsis
	EXPORT	string_to_lower
	EXPORT	string_to_upper
	EXPORT	string_is_a_number
	EXPORT	string_strip_leading_spaces
	EXPORT	string_compare
	EXPORT	string_join
	EXPORT	string_leaf
	EXPORT	string_branch
	EXPORT	string_substitute_args
	EXPORT	string_length
	EXPORT	string_copy

library_string
	CMP	r0, #(library_string_table_end-library_string_table)/4
	ADDCC	pc, pc, r0, LSL #2
	MOV	pc, r14
library_string_table
	B	string_read_memory_to_linefeed
	B	string_read_memory_to_control
	B	string_left_ellipsis
	B	string_right_ellipsis
	B	string_to_lower
	B	string_to_upper
	B	string_is_a_number
	B	string_strip_leading_spaces
	B	string_compare
	B	string_join
	B	string_leaf
	B	string_branch
	B	string_substitute_args
	B	string_length
	B	string_copy
library_string_table_end


; Name:
;   string_read_memory_to_linefeed
; Purpose:
;   Reads a string from memory up to an LF, omitting any control-codes.
; Entry:
;   R1 -> input
; Exit:
;   R1 -> character after input's terminator
;   R2 -> output (up to 255 chars)
; Internally:
;   R3 = string length counter/index
;   R4 = character

string_read_memory_to_linefeed
	STMFD	r13!, {r3, r4, r14}
	LDR	r2, [r12]
	MOV	r3, #0
string_read_memory_to_linefeed_loop
	LDRB	r4, [r1], #1
	CMP	r4, #10				; if (char == LF) {
	BEQ	string_read_memory_to_linefeed_exit   ;	  exit }
	CMP	r4, #32				; if (char < 32) {
	BLT	string_read_memory_to_linefeed_loop   ;	  get next char }
	STRB	r4, [r2, r3]			; add char to string
	ADD	r3, r3, #1			; inc stringlen counter
	CMP	r3, #254			; if (stringlen <= 254) {
	BLE	string_read_memory_to_linefeed_loop   ;	  loop }
string_read_memory_to_linefeed_exit
	MOV	r4, #13			; terminate the string with CR {
	STRB	r4, [r2, r3]		;   }
	LDMFD	r13!, {r3, r4, pc}


; Name:
;   string_read_memory_to_control
; Purpose:
;   Reads a string from memory up to the first control-code found.
; Entry:
;   R1 -> input
; Exit:
;   R1 -> character after input's terminator
;   R2 -> output (up to 255 chars)
; Internally:
;   R3 = string length counter
;   R4 = character

string_read_memory_to_control
	STMFD	r13!, {r3, r4, r14}
	LDR	r2, [r12]
	MOV	r3, #0
string_read_memory_to_control_loop
	LDRB	r4, [r1], #1
	CMP	r4, #31			; char <= 31 {
	BLE	string_read_memory_to_control_exit ;   exit }
	STRB	r4, [r2, r3]		; add char to string
	ADD	r3, r3, #1		; inc stringlen counter
	CMP	r3, #254		; stringlen <= 254 {
	BLE	string_read_memory_to_control_loop ;   loop }
string_read_memory_to_control_exit
	MOV	r4, #13			; terminate the string with CR {
	STRB	r4, [r2, r3]		;   }
	LDMFD	r13!, {r3, r4, pc}


; Name:
;   string_left_ellipsis
; Purpose:
;   Copies a string, truncating the left-hand side and prefixing '...' if
;   it's longer than the specified size.
; Entry:
;   R1 -> input
;   R2 = maximum length of string (maximum of 255, <3 is undefined)
; Exit:
;   R1 -> character after input's terminator
;   R2 preserved
;   R3 -> output (up to 255 chars)
; Internally:
;   R2 = number of characters that need copying
;   R4 = string length counter
;   R5 = character

string_left_ellipsis
	STMFD	r13!, {r2, r4, r5, r14}
	LDR	r3, [r12]
	MOV	r4, #0
string_left_ellipsis_findlength
	LDRB	r5, [r1, r4]
	CMP	r5, #31
	ADDGT	r4, r4, #1
	BGT	string_left_ellipsis_findlength
					; R4 now holds length of the $@R0
	CMP	r4, r2
	MOVLT	r2, r4			; if ( R4<R3 ) { R3=R4 }
	SUBGT	r2, r2, #3		; # of chars that need copying
	SUBGT	r4, r4, r2
	ADDGT	r1, r1, r4

	MOVGT	r5, #'.'		; prefix '...'
	STRGTB  r5, [r3], #1
	STRGTB  r5, [r3], #1
	STRGTB  r5, [r3], #1
string_left_ellipsis_copyloop
	LDRB	r5, [r1], #1
	STRB	r5, [r3], #1
	SUBS	r2, r2, #1
	BGT	string_left_ellipsis_copyloop
	MOV	r5, #13			; terminate the string with CR {
	STRB	r5, [r3], #1		;   }
	LDR	r3, [r12]		; yukky
	LDMFD	r13!, {r2, r4, r5, pc}


; Name:
;   string_right_ellipsis
; Purpose:
;   Copies a string, truncating the right-hand side and appending '...' if
;   it's longer than the specified size.
; Entry:
;   R1 -> input
;   R2 = maximum length of string (maximum of 255, <3 is undefined)
; Exit:
;   R1 -> character after input's terminator
;   R2 preserved
;   R3 -> output (up to 255 chars)
; Internally:
;   R2 = string length counter
;   R4 = character

string_right_ellipsis
	STMFD	r13!, {r2, r4, r14}
	LDR	r3, [r12]
	SUB	r2, r2, #3
string_right_ellipsis_loop
	LDRB	r4, [r1], #1
	CMP	r4, #13
	BEQ	string_right_ellipsis_escape
	STRB	r4, [r3], #1
	SUBS	r2, r2, #1
	BGT	string_right_ellipsis_loop
string_right_ellipsis_escape
	MOV	r4, #'.'
	STRB	r4, [r3], #1
	STRB	r4, [r3], #1
	STRB	r4, [r3], #1
	MOV	r4, #13			; terminate the string with CR {
	STRB	r4, [r3], #1		;   }
	LDR	r3, [r12]		; yukky
	LDMFD	r13!, {r2, r4, pc}


; Name:
;   string_to_lower
; Purpose:
;   Copies a string, converting it to lower case characters.
; Entry:
;   R1 -> input
; Exit:
;   R1 -> character after input's terminator
;   R2 -> output (up to 255 chars)
; Internally:
;   R3 = character
;   R4 = output string index

string_to_lower
	STMFD	r13!, {r3, r4, r14}
	LDR	r2, [r12]
	MOV	r4, #0
string_to_lower_loop
	LDRB	r3, [r1], #1
	CMP	r3, #'A'
	BLT	string_to_lower_no_change
	CMP	r3, #'Z'
	BGT	string_to_lower_no_change
	ORR	r3, r3, #2_00100000
string_to_lower_no_change
	CMP	r3, #32
	STRGEB  r3, [r2, r4]
	ADDGE	r4, r4, #1
	BGE	string_to_lower_loop
	MOV	r3, #13
	STRB	r3, [r2, r4]
	LDMFD	r13!, {r3, r4, pc}


; Name:
;   string_to_upper
; Purpose:
;   Copies a string, converting it to upper case characters.
; Entry:
;   R1 -> input
; Exit:
;   R1 -> character after input's terminator
;   R2 -> output (up to 255 chars)
; Internally:
;   R3 = character

string_to_upper
	STMFD	r13!, {r3, r14}
	LDR	r2, [r12]
string_to_upper_loop
	LDRB	r3, [r1], #1
	CMP	r3, #'a'
	BLT	string_to_upper_no_change
	CMP	r3, #'z'
	BGT	string_to_upper_no_change
	BIC	r3, r3, #2_00100000
string_to_upper_no_change
	STRB	r3, [r2], #1
	CMP	r3, #13
	BNE	string_to_upper_loop
	LDR	r2, [r12] ; yukky
	LDMFD	r13!, {r3, pc}


; Name:
;   string_is_a_number
; Purpose:
;   Verfies whether the given string is a valid number or not.
; Entry:
;   R1 -> input
; Exit:
;   R1 -> character after input's terminator
;   R2 = TRUE/FALSE for valid/invalid respectively
; Internally:
;   R3 = character

string_is_a_number
	STMFD	r13!, {r3, r14}
string_is_a_number_loop
	LDRB	r3, [r1], #1
	CMP	r3, #32
	MOVLT	r2, #-1
	LDMLTFD r13!, {r3, pc}
	CMP	r3, #'0'
	MOVLT	r2, #0
	LDMLTFD r13!, {r3, pc}
	CMP	r3, #'9'
	MOVGT	r2, #0
	LDMGTFD r13!, {r3, pc}
	B	string_is_a_number_loop


; Name:
;   string_strip_leading_spaces
; Purpose:
;   Removes all leading space and tab characters from the given string.
; Entry:
;   R1 -> input
; Exit:
;   R1 -> character after input's terminator
;   R2 -> output (up to 255 chars)
; Internally:
;   R3 = character
;   R4 = output string index

string_strip_leading_spaces
	STMFD	r13!, {r3, r4, r14}
	LDR	r2, [r12]
	MOV	r4, #0
string_strip_leading_spaces_loop
	LDRB	r3, [r1], #1
	CMP	r3, #' '
	CMPNE	r3, #9
	BEQ	string_strip_leading_spaces_loop
string_strip_leading_spaces_loop2
	CMP	r3, #32
	STRGEB  r3, [r2, r4]
	ADDGE	r4, r4, #1
	LDRGEB  r3, [r1], #1
	BGE	string_strip_leading_spaces_loop2
	MOV	r3, #13
	STRB	r3, [r2, r4]
	LDMFD	r13!, {r3, r4, pc}


; Name:
;   string_compare
; Purpose:
;   Compares two strings and returns an integer representing the
;   relationship of the two strings (less than, equal, greater than).
; Entry:
;   R1 -> string a
;   R2 -> string b
; Exit:
;   R1 -> character after string a's terminator
;   R2 -> character after string b's terminator
;   R3 = -1 a is less than b
;	  0 a equals b
;	  1 a is greater than b
; Internally:
;   R3, R4 = temp characters

string_compare
	STMFD	r13!, {r4, r14}
string_compare_loop
	LDRB	r3, [r1], #1
	LDRB	r4, [r2], #1

	CMP	r3, #32			; if (a<32 or b<32) {
	CMPGE	r4, #32			;
	BLT	string_compare_end	;   compare_end }

	CMP	r3, r4
	BEQ	string_compare_loop

	MOVGT	r3, #-1			; less
	MOVLT	r3, #1			; greater
	LDMFD	r13!, {r4, pc}

string_compare_end
	; At here, one or the other is < 32.

	CMP	r3, #32			; are both < 32 ?
	CMPLT	r4, #32
	MOVLT	r3, #0			; equal
	LDMLTFD r13!, {r4, pc}

	CMP	r3, #32
	MOVGE	r3, #-1
	MOVLT	r3, #1
	LDMFD	r13!, {r4, pc}


; Name:
;   string_join
; Purpose:
;   Joins two strings together.  (Not much use with BASIC).
; Entry:
;   R1 -> string a
;   R2 -> string b
; Exit:
;   R1 -> character after string a's terminator
;   R2 -> character after string b's terminator
;   R3 -> new string
; Internally:
;   R3, R4 = temp characters

string_join
	STMFD	r13!, {r4, r5, r14}
	LDR	r3, [r12]
	MOV	r4, #0
string_join_loop_a
	LDRB	r5, [r1], #1
	CMP	r5, #32
	STRGEB  r5, [r3, r4]
	ADDGE	r4, r4, #1
	BGE	string_join_loop_a
string_join_loop_b
	LDRB	r5, [r2], #1
	CMP	r5, #32
	STRGEB  r5, [r3, r4]
	ADDGE	r4, r4, #1
	BGE	string_join_loop_b
	MOV	r5, #13
	STRB	r5, [r3, r4]
	LDMFD	r13!, {r4, r5, pc}


; Name:
;   string_leaf
; Purpose:
;   Returns the what comes after the specified separator character, or the
;   whole string if the separator doesn't exist.
; Entry:
;   R1 -> path
;   R2 = separator character
; Exit:
;   R1 -> character after the path's terminator
;   R2 preserved
;   R3 -> leaf
; Internally:
;   R4 = string index
;   R5 = temp character
;   R6 = R1 minus one (for checking if out of string)

string_leaf
	STMFD	r13!, {r4, r5, r6, r14}
	SUB	r6, r1, #1

string_leaf_scan_to_end
	LDRB	r5, [r1], #1
	CMP	r5, #32
	BGE	string_leaf_scan_to_end

	SUB	r1, r1, #2		; point to last char

string_leaf_scan_back_to_separator
	; if the separator is not actually in the string, skip and copy
	; the whole lot.
	CMP	r1, r6
	ADDEQ	r1, r1, #1
	BEQ	string_leaf_skip_and_copy_whole
	LDRB	r5, [r1], #-1
	CMP	r5, r2
	BNE	string_leaf_scan_back_to_separator
	ADDEQ	r1, r1, #2		; point to char after separator

string_leaf_skip_and_copy_whole
	LDR	r3, [r12]
	MOV	r4, #0
string_leaf_copy_to_end
	LDRB	r5, [r1], #1
	CMP	r5, #32
	STRGEB  r5, [r3, r4]
	ADDGE	r4, r4, #1
	BGE	string_leaf_copy_to_end

	MOV	r5, #13
	STRB	r5, [r3, r4]

	LDMFD	r13!, {r4, r5, r6, pc}


; Name:
;   string_branch
; Purpose:
;   Returns the branchname of a path.
; Entry:
;   R1 -> path
;   R2 = separator character
; Exit:
;   R1 -> character after string a's terminator
;   R2 preserved
;   R3 -> branch
; Internally:
;   R4 = pointer to last separator character, or 0 if not encountered
;   R14 = temp character

string_branch
	STMFD	r13!, {r4, r14}

	MOV	r4, #0		; no separator character found
	LDR	r3, [r12]
string_branch_copy
	LDRB	r14, [r1], #1
	TEQ	r14, r2
	MOVEQ	r4, r3
	CMP	r14, #32
	STRGEB	r14, [r3], #1
	BGE	string_branch_copy

	MOV	r14, #13
	TEQ	r4, #0
	STRNEB	r14, [r4]	; terminate at last separator, if present
	STREQB	r14, [r3]

	LDR	r3, [r12]	; yukky

	LDMFD	r13!, {r4, pc}


; Name:
;   string_substitute_args
; Purpose:
;   Substitutes a set of strings into a template string.
; Entry:
;   R1 -> template string, containing '%0', '%1', etc. (up to %4)
;   R3 -> string to substitute for '%0' or = 0 for no string
;   (etc.)
; Exit:
;   R1 -> character after template string's terminator
;   R2 -> substituted string
; Internally:
;   R0 = temp character

string_substitute_args
	STMFD	r13!, {r0, r7, r14}
	LDR	r2, [r12]
string_substitute_args_main_loop
	LDRB	r0, [r1], #1
	CMP	r0, #32
	BLT	string_substitute_args_exit
	CMP	r0, #'%'
	STRNEB  r0, [r2], #1
	BNE	string_substitute_args_main_loop

	; Found a '%'
	LDRB	r0, [r1], #1
	CMP	r0, #32
	BLT	string_substitute_args_exit
	BL	string_substitute_args_insert

	B	string_substitute_args_main_loop

string_substitute_args_exit
	MOV	r0, #0
	STRB	r0, [r2], #1
	LDR	r2, [r12]
	LDMFD	r13!, {r0, r7, pc}

string_substitute_args_insert
	MOV	r7, #1

	CMP	r0, #'0'
	MOVEQ	r7, r3
	CMP	r0, #'1'
	MOVEQ	r7, r4
	CMP	r0, #'2'
	MOVEQ	r7, r5
	CMP	r0, #'3'
	MOVEQ	r7, r6

	CMP	r7, #0		; null pointers generate nothing
	MOVEQ   pc, r14

	CMP	r7, #1		; anything else generates '%<char>'.
	MOVEQ	r7, #'%'
	STREQB  r7, [r2], #1
	STREQB  r0, [r2], #1
	MOVEQ   pc, r14

string_substitute_args_insert_loop
	LDRB	r0, [r7], #1
	CMP	r0, #32
	MOVLT   pc, r14
	STRB	r0, [r2], #1
	B	string_substitute_args_insert_loop


; Name:
;   string_length
; Purpose:
;   Returns the length of a string.
; Entry:
;   R1 -> string
; Exit:
;   R1 -> character after string's terminator
;   R2 = length
; Internally:
;   R2 = temp character
;   R3 = counter

string_length
	STMFD	r13!, {r3-r5, r14}
	MOV	r3, #0
	MOV	r4, #255
string_length_loop
	LDR	r5, [r1], #4

	AND	r2, r4, r5
	CMP	r2, #32
	ADDGE	r3, r3, #1

	ANDGE	r2, r4, r5, LSR #8
	CMPGE	r2, #32
	ADDGE	r3, r3, #1

	ANDGE	r2, r4, r5, LSR #16
	CMPGE	r2, #32
	ADDGE	r3, r3, #1

	ANDGE	r2, r4, r5, LSR #24
	CMPGE	r2, #32
	ADDGE	r3, r3, #1

	BGE	string_length_loop

string_length_exit
	MOV	r2, r3
	LDMFD	r13!, {r3-r5, pc}


; Name:
;   string_copy
; Purpose:
;   Copies a string.
; Entry:
;   R1 -> input string
;   R2 -> output
; Exit:
;   R1 -> character after input string's terminator
;   R2 -> character after output string's terminator
; Internally:
;   R3 = temp character

string_copy
	STMFD	r13!, {r3, r14}
string_copy_loop
	LDRB	r3, [r1], #1
	CMP	r3, #32
	STRGEB	r3, [r2], #1
	BGE	string_copy_loop
	MOV	r3, #13
	STRB	r3, [r2], #1
	LDMFD	r13!, {r3, pc}


	END
