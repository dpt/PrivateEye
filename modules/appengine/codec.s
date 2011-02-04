; --------------------------------------------------------------------------
;    Name: Codec
; Purpose: Encoding and decoding into/from various formats
;  Author: © David Thomas, 1996-2009
; --------------------------------------------------------------------------

; 0.00 (30 Jan 1997)
; 0.01 (10 Feb 1997)
; - Codec_base64_encode was writing out the wrong number of =‘s after the
;   output string.  This caused the decoder to mess up.

; Includes

	GET	Hdr.appengine

; Code

	AREA	|_codec|, CODE, READONLY

	EXPORT	library_codec

library_codec
	CMP	r0, #(library_codec_table_end-library_codec_table)/4
	ADDCC	pc, pc, r0, LSL #2
	MOV	pc, r14
library_codec_table
	B	codec_base64_encode
	B	codec_base64_decode
library_codec_table_end


; Name:
;   codec_base64_encode
; Purpose:
;   Encodes data into a base64 string.
; Entry:
;   R0 = 0
;   R1 = input
;   R2 = length of input
; Exit:
;   R3 -> encoded string
; Internally:
;   R0 = temp character
;   R2 = end of input
;   R3 -> output
;   R4 -> base64 encoding table
;   R5 = three input bytes
;   R6 = output shift
;   R7 = 63, for ANDing with input bytes

codec_base64_encode
	STMFD	r13!, {r0-r2, r4-r7, r14}
	ADD	r2, r1, r2	; end of input
	LDR	r3, [r12]
	ADR	r4, codec_base64_encode_table
	MOV	r7, #&3F

	; Read three bytes, in big-endian style.
codec_base64_encode_loop
	LDRB	r5, [r1, #2]
	LDRB	r0, [r1, #1]
	ORR	r5, r5, r0, LSL #8
	LDRB	r0, [r1], #3
	ORR	r5, r5, r0, LSL #16

	; Write out four base64 characters.
	MOV	r6, #24
codec_base64_encode_might_be_more
	SUB	r6, r6, #6
	CMP	r6, #0
	BLT	codec_base64_encode_carry_on
	AND	r0, r7, r5, LSR r6
	LDRB	r0, [r4, r0]
	STRB	r0, [r3], #1
	B	codec_base64_encode_might_be_more

codec_base64_encode_carry_on
	CMP	r1, r2
	BLT	codec_base64_encode_loop

	; Now, skip back and insert the equals into place.
	SUB	r6, r1, r2
	SUB	r3, r3, r6
	MOV	r0, #'='
	CMP	r6, #1
	STRGEB  r0, [r3], #1
	CMP	r6, #2
	STREQB  r0, [r3], #1

	; Terminate the string.
	MOV	r0, #13
	STRB	r0, [r3], #1

	LDR	r3, [r12]
	LDMFD	r13!, {r0-r2, r4-r7, pc}

codec_base64_encode_table
	DCB	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	DCB	"abcdefghijklmnopqrstuvwxyz"
	DCB	"0123456789+/"


; Name:
;   codec_base64_decode
; Purpose:
;   Decodes base64 encoded data into a string.
; Entry:
;   R0 = 1
;   R1 = base64 encoded data
;   R2 = length of encoded data
; Exit:
;   R3 -> decoded string
; Internally:
;   R0 = number of characters read indicator
;   R2 = word-aligned end of input
;   R3 -> output
;   R4 = 'joined' bytes
;   R5 = temp
;   R6 -> base64 decoding table
;   R7 = 255, for byte extraction

codec_base64_decode
	STMFD	r13!, {r0-r2, r4-r7, r14}
	LDR	r3, [r12]

	ADD	r2, r2, #3
	BIC	r2, r2, #3
	ADD	r2, r2, r1

	ADR	r6, codec_base64_decode_table-32
	MOV	r7, #255
codec_base64_decode_loop
	MOV	r0, #0

	LDRB	r5, [r1]
	LDRB	r5, [r6, r5]
	MOV	r4, r5, LSL #18

	LDRB	r5, [r1, #1]
	LDRB	r5, [r6, r5]
	CMP	r5, #255
	ORRNE	r4, r4, r5, LSL #12
	ORRNE	r0, r0, #2_001
	BEQ	codec_base64_decode_skippity_hop

	LDRB	r5, [r1, #2]
	LDRB	r5, [r6, r5]
	CMP	r5, #255
	ORRNE	r4, r4, r5, LSL #6
	ORRNE	r0, r0, #2_010
	BEQ	codec_base64_decode_skippity_hop

	LDRB	r5, [r1, #3]
	LDRB	r5, [r6, r5]
	CMP	r5, #255
	ORRNE	r4, r4, r5
	ORRNE	r0, r0, #2_100

codec_base64_decode_skippity_hop
	AND	r5, r7, r4, LSR #16
	TST	r0, #2_001
	STRNEB  r5, [r3], #1

	ANDNE	r5, r7, r4, LSR #8
	TSTNE	r0, #2_010
	STRNEB  r5, [r3], #1

	ANDNE	r5, r7, r4
	TSTNE	r0, #2_100
	STRNEB  r5, [r3], #1

	ADD	r1, r1, #4
	CMP	r1, r2
	BLT	codec_base64_decode_loop

codec_base64_decode_exit
	MOV	r5, #13
	STRB	r5, [r3], #1

	LDR	r3, [r12]
	LDMFD	r13!, {r0-r2, r4-r7, pc}

codec_base64_decode_table
	DCB	255,255,255,255,255,255,255,255
	DCB	255,255,255, 62,255,255,255, 63
	DCB	 52, 53, 54, 55, 56, 57, 58, 59
	DCB	 60, 61,255,255,255,255,255,255
	DCB	255,  0,  1,  2,  3,  4,  5,  6
	DCB	  7,  8,  9, 10, 11, 12, 13, 14
	DCB	 15, 16, 17, 18, 19, 20, 21, 22
	DCB	 23, 24, 25,255,255,255,255,255
	DCB	255, 26, 27, 28, 29, 30, 31, 32
	DCB	 33, 34, 35, 36, 37, 38, 39, 40
	DCB	 41, 42, 43, 44, 45, 46, 47, 48
	DCB	 49, 50, 51,255,255,255,255,255

	END
