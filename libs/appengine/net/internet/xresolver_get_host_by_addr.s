; ---------------------------------------------------------------------------
;    Name: Resolver2.s
; Purpose: Resolver library (using the ANT interface)
;  Author: David Thomas
; Version: $Id: xresolver_get_host_by_addr.s,v 1.2 2005-03-06 18:28:46 dpt Exp $
; ---------------------------------------------------------------------------

	AREA	|C$$code|, CODE, READONLY
|x$codeseg|

	EXPORT	xresolver_get_host_by_addr

xresolver_get_host_by_addr
	STMFD	r13!, {r4, r14}
	MOV	r4, r3	; hostent **host_ent
	MOV	r3, r2  ; int type
	MOV	r2, r1	; int length
	MOV	r1, r0	; const char *addr
	MOV	r0, #0
	SWI	&66001 ; XResolver_GetHost
	STRVC	r1, [r4]
	MOVVC	r0, #0
	LDMFD	r13!, {r4, pc}^

	END
