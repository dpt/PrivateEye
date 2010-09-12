	AREA	|C$$code|, CODE, READONLY
|x$codeseg|

	EXPORT	xappengine_heap_claim

xappengine_heap_claim
	MOV	r12, r14
	MOV	r3, r2 		; **ae_anchor
	MOV	r2, r1 		; pointer to heap
	MOV	r1, r0 		; size of block required (bytes)
	MOV	r0, #2 		; claim
	SWI	&6D949 		; XAppEngine_Heap
	MOVVSS	pc, r12
	TEQ	r3, #0
	STRNE	r1, [r3]
	MOV	r0, #0
	MOVS	pc, r12

	END
	
