	AREA	|C$$code|, CODE, READONLY
|x$codeseg|

	EXPORT	xappengine_heap_release

xappengine_heap_release
	MOV	r12, r14
	MOV	r3, r1		; **ae_anchor
	LDR	r2, [r3]	; pointer to anchor
	MOV	r1, r0		; pointer to heap
	MOV	r0, #3		; release
	SWI	&6D949		; XAppEngine_Heap
	STRVC	r1, [r3]	; zero (pointer to anchor)
	MOVVC	r0, #0
	MOVS	pc, r12

	END

