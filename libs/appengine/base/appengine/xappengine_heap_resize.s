	AREA	|C$$code|, CODE, READONLY
|x$codeseg|

	EXPORT	xappengine_heap_resize

xappengine_heap_resize
	MOV	r12, r14
	STMFD	r13!, {r4}
	MOV	r4, r3 		; pointer to success flag
	MOV	r3, r2 		; bytes to be added or removed from the block
	MOV	r2, r1 		; pointer to anchor
	MOV	r1, r0 		; pointer to heap
	MOV	r0, #4 		; resize
	SWI	&6D949 		; XAppEngine_Heap
	BVS	xappengine_heap_resize_exit
	TEQ	r4, #0
	STRNE	r1, [r4] 	; success flag
	MOV	r0, #0
xappengine_heap_resize_exit
	LDMFD	r13!, {r4}
	MOVS	pc, r12

	END
