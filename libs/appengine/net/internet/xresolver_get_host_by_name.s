; ---------------------------------------------------------------------------
;    Name: Resolver2.s
; Purpose: Resolver library (using the ANT interface)
;  Author: David Thomas
; ---------------------------------------------------------------------------

	AREA	|C$$code|, CODE, READONLY
|x$codeseg|

	EXPORT	xresolver_get_host_by_name

xresolver_get_host_by_name
	PUSH	{r2, r14}
	MOV	r2, r1	; hostent **host_ent
	MOV	r1, r0	; const char *addr
	MOV	r0, #0
	SWI	&66000 ; XResolver_GetHostByName
	STRVC	r1, [r2]
	MOVVC	r0, #0
	POP	{r2, pc}

	END
