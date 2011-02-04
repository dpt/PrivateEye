
;	extern xosheap_resize_r3(char *heap,
;			         int   required_change,
;				 int  *actual_change);

	AREA	|C$$code|, CODE, READONLY
|x$codeseg|

	EXPORT  xosheap_resize_r3

xosheap_resize_r3
	MOV	r12, r14
	MOV	r3, r1
	MOV	r1, r0
	MOV	r0, #5
	SWI	&2001D 		; XOS_Heap
	STR	r3, [r2] 	; even if it failed
	MOVVC	r0, #0
	MOVS	pc, r12

	END
