	AREA	|C$$code|, CODE, READONLY
|x$codeseg|

	EXPORT	xappengine_resource_op_locate

xappengine_resource_op_locate
	STMFD	r13!, {r1, r2, r14}
	MOV	r2, r1
	MOV	r1, r0
	MOV	r0, #0
	SWI	&6D940 		; XAppEngine_ResourceOp
	STRVC	r1, [r2]
	MOVVC	r0, #0
	LDMFD	r13!, {r1, r2, pc}^

	END
