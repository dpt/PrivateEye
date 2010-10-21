; ---------------------------------------------------------------------------
;    Name: xresolver_get_host.s
; Purpose: Resolver library (using the ANT interface)
;  Author: David Thomas
; Version: $Id: xresolver_get_host.s,v 1.2 2005-03-06 18:28:45 dpt Exp $
; ---------------------------------------------------------------------------

	AREA	|C$$code|, CODE, READONLY
|x$codeseg|

	EXPORT	xresolver_get_host

xresolver_get_host
	STMFD	r13!, {r2, r14}
	MOV	r2, r1	; hostent **host_ent
	SWI	&66001 ; XResolver_GetHost
	STRVC	r1, [r2]
	MOVVC	r0, #0
	LDMFD	r13!, {r2, pc}

	END
