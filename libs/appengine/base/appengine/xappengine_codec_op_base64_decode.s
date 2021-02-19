	AREA	|C$$code|, CODE, READONLY
|x$codeseg|

	EXPORT	xappengine_codec_op_base64_decode

xappengine_codec_op_base64_decode
	PUSH	{r2, r14}
	MOV	r2, r1			; length of input
	MOV	r1, r0			; input
	MOV	r0, #1			; 1
	SWI	&6D946  		; XAppEngine_CodecOp
	POP	{r14}
	STRVC	r3, [r14]
	MOVVC	r0, #0
	POP	{pc}

	END
