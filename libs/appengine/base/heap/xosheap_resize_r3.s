
;	extern xosheap_resize_r3(char *heap,
;			         int   required_change,
;				 int  *actual_change);

	AREA	|C$$code|, CODE, READONLY
|x$codeseg|

	EXPORT  xosheap_resize_r3

xosheap_resize_r3
	MOV	r12, r14
	MOV	r3, r1		; size increase, or -ve for decrease
	MOV	r1, r0		; pointer to heap
	MOV	r0, #5		; reason: increase size of heap
	SWI	&2001D		; XOS_Heap
	STR	r3, [r2]	; save actual change in size, even if V set
	MOVVC	r0, #0
	MOV	pc, r12

	END
