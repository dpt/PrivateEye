; --------------------------------------------------------------------------
;    Name: Socket
; Purpose: Internet socket operations
;  Author: © David Thomas, 1996-2009
; Version: $Id: socket.s,v 1.4 2009-09-02 23:36:13 dpt Exp $
; --------------------------------------------------------------------------

; Includes

	GET	Hdr.appengine

; Code

	AREA	|_socket|, CODE, READONLY

	EXPORT	library_socket

library_socket
	CMP	r0, #(library_socket_table_end-library_socket_table)/4
	ADDCC	pc, pc, r0, LSL #2
	MOV	pc, r14
library_socket_table
	B	socket_fd_zero
	B	socket_fd_copy
	B	socket_fd_set
	B	socket_fd_clr
	B	socket_fd_is_set
library_socket_table_end


; Name:
;   socket_fd_set
; Purpose:
;   Sets a bit within a socket file descriptor, indicating that the socket
;   is active.
; Entry:
;   R1 = socket number (0-255)
;   R2 -> file descriptor (32 bytes, 256 bits)
; Exit:
;   R1, R2 preserved
; Internally:
;   R3 = index/temporary
;   R14 = 1

socket_fd_set
	CMP	r1, #255
	MOVHI	pc, r14
	;
	STMFD	r13!, {r1-r3, r14}
	AND	r3, r1, #2_11100000     ; R3 = word offset in bits
	BIC	r1, r1, r3              ; R1 = bit offset within that word
	LDR	r3, [r2, r3, LSR #3]!
	MOV	r14, #1
	ORR	r3, r3, r14, LSL r1
	STR	r3, [r2]
	LDMFD	r13!, {r1-r3, pc}


; Name:
;   socket_fd_clr
; Purpose:
;   Clears a bit within a socket file descriptor, indicating that the socket
;   is inactive.
; Entry:
;   R1 = socket number (0-255)
;   R2 -> file descriptor (32 bytes, 256 bits)
; Exit:
;   R1, R2 preserved
; Internally:
;   R3 = index/temporary
;   R14 = 1

socket_fd_clr
	CMP	r1, #255
	MOVHI	pc, r14
	;
	STMFD	r13!, {r1-r3, r14}
	AND	r3, r1, #2_11100000     ; R3 = word offset in bits
	BIC	r1, r1, r3              ; R1 = bit offset within that word
	LDR	r3, [r2, r3, LSR #3]!
	MOV	r14, #1
	BIC	r3, r3, r14, LSL r1
	STR	r3, [r2]
	LDMFD	r13!, {r1-r3, pc}


; Name:
;   socket_fd_is_set
; Purpose:
;   Returns TRUE or FALSE, depending on whether a particular socket is
;   marked as active in the file descriptor.
; Entry:
;   R1 = socket number (0-255)
;   R2 -> file descriptor (32 bytes, 256 bits)
; Exit:
;   R1 = TRUE or FALSE depending on whether the socket is marked as active
;	 or not.
;   R2 preserved
; Internally:
;   R14 = index/temporary

socket_fd_is_set
	CMP	r1, #255
	MOVHI	pc, r14
	;
	STMFD	r13!, {r14}
	AND	r14, r1, #2_11100000    ; R3 = word offset in bits
	BIC	r1, r1, r14           	; R1 = bit offset within that word
	LDR	r14, [r2, r14, LSR #3]
	MOV	r14, r14, LSR r1
	TST	r14, #1
	MOVNE	r1, #-1
	MOVEQ	r1, #0
	LDMFD	r13!, {pc}


; Name:
;   socket_fd_zero
; Purpose:
;   Completely clears a socket file descriptor block.
; Entry:
;   R1 -> file descriptor (32 bytes, 256 bits)
; Exit:
;   R1 preserved
; Internally:
;   R2 to R9 = zero

socket_fd_zero
	STMFD	r13!, {r2-r9}
	MOV	r2, #0
	MOV	r3, #0
	MOV	r4, #0
	MOV	r5, #0
	MOV	r6, #0
	MOV	r7, #0
	MOV	r8, #0
	MOV	r9, #0
	STMIA	r1, {r2-r9}
	LDMFD	r13!, {r2-r9}
	;
	MOV	pc, r14


; Name:
;   socket_fd_copy
; Purpose:
;   Copies one file descriptor to another.
; Entry:
;   R1 -> destination file descriptor (32 bytes, 256 bits)
;   R2 -> source file descriptor
; Exit:
;   R1, R2 preserved
; Internally:
;   R3 to R10 = temporary registers for block transfer

socket_fd_copy
	STMFD	r13!, {r3-r10}
	LDMIA	r2, {r3-r10}
	STMIA	r1, {r3-r10}
	LDMFD	r13!, {r3-r10}
	;
	MOV	pc, r14


	END
