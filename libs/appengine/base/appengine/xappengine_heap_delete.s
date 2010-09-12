	AREA	|C$$code|, CODE, READONLY
|x$codeseg|

	EXPORT	xappengine_heap_delete

xappengine_heap_delete
	MOV	r12, r14
	MOV	r1, r0 		; pointer to heap
	MOV	r0, #1 		; delete
	SWI	&6D949 		; XAppEngine_Heap
	MOVVC	r0, #0
	MOVS	pc, r12

	END
	
