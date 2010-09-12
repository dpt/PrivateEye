	AREA	|C$$code|, CODE, READONLY
|x$codeseg|

	EXPORT	xappengine_heap_create_dynamic

xappengine_heap_create_dynamic
	MOV	r12, r14
	MOV	r3, r1 		; **ae_heap
	MOV	r2, r0 		; pointer to a name for the dynamic area
	MOV	r1, #0 		; create a dynamic area for the heap
	MOV	r0, #0 		; create
	SWI	&6D949 		; XAppEngine_Heap
	MOVVSS	pc, r12
	TEQ	r3, #0
	STRNE	r1, [r3]
	MOV	r0, #0
	MOVS	pc, r12

	END
