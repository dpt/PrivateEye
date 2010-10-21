	AREA	|C$$code|, CODE, READONLY
|x$codeseg|

	EXPORT	xappengine_codec_op_base64_encode

xappengine_codec_op_base64_encode
	STMFD	r13!, {r2, r14}
	MOV	r2, r1			; length of input
	MOV	r1, r0			; input
	MOV	r0, #0			; 0
	SWI	&6D946  		; XAppEngine_CodecOp
	LDMFD	r13!, {r14}
	STRVC	r3, [r14]
	MOVVC	r0, #0
	LDMFD	r13!, {pc}

	END
