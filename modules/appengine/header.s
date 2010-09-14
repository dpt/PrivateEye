; ---------------------------------------------------------------------------
;    Name: AppEngine module header
; Purpose: Header for the AppEngine library system's shared module
;  Author: © David Thomas, 1996-2009
; Version: $Id: header.s,v 1.7 2009-09-08 23:05:37 dpt Exp $
; ---------------------------------------------------------------------------

; Includes

	GET	AppEngine:SWI.Hdr.MessageTrans
	GET	AppEngine:SWI.Hdr.OS
	GET	AppEngine:Hdr.Types
	GET	Hdr.appengine

; Code

	AREA	|Module$$Header|, CODE, READONLY

	IMPORT	library_resource
	IMPORT	library_string
	IMPORT	library_window
	IMPORT	library_icon
	IMPORT	library_date
	IMPORT	library_socket
	IMPORT	library_codec
	IMPORT	library_url
	IMPORT	library_memory
	IMPORT	library_heap

	ENTRY

; --------------------------------------------------------------------------

	DCD	0			; start code
	DCD	module_initialise	; initialisation code
	DCD	module_finalise		; finalisation code
	DCD	0			; service call handler
	DCD	module_title		; title string
	DCD	module_help		; help string
	DCD	0			; help and command keyword table
	DCD	swi_base		; SWI chunk base number
	DCD	module_swi_handler	; SWI handler code
	DCD	module_swi_table	; SWI decoding table
	DCD	0			; SWI decoding code
	DCD	0			; MessageTrans file (3.5+)
        DCD	module_flags		; flags

module_flags
	DCD	1			; 32-bit compatible

; --------------------------------------------------------------------------

module_initialise
	STMFD	r13!, {r14}
	MOV	r0, #6
	MOV	r3, #sizeof_workspace
	SWI	XOS_Module
	STRVC	r2, [r12]		; place wksp address in private word
	LDMFD	r13!, {pc}

module_finalise
	STMFD	r13!, {r14}
	MOV	r0, #7
	LDR	r2, [r12]
	SWI	XOS_Module
	LDMFD	r13!, {pc}

; --------------------------------------------------------------------------

module_title
	DCB	"AppEngine", 0

module_help
	DCB	"AppEngine", 9, "0.16 (09 Sep 2009)"
	DCB     " © David Thomas, 1996-2009", 0
	ALIGN

; --------------------------------------------------------------------------

module_swi_table
	DCB	"AppEngine", 0

	; SWIs with the 'Op' suffix represent a suite of distinct but related
	; routines often operating on an external entity, whereas those
	; without the suffix represent a set of interacting routines.

	DCB	"ResourceOp", 0
	DCB	"StringOp", 0
	DCB	"WindowOp", 0
	DCB	"IconOp", 0
	DCB	"DateOp", 0
	DCB	"SocketOp", 0
	DCB	"CodecOp", 0
	DCB	"URLOp", 0
	DCB	"MemoryOp", 0
	DCB	"Heap", 0

	DCB	0
	ALIGN

module_swi_handler
	CMP	r11, #(swi_handler_table_end-swi_handler_table)/4
	ADDCC	pc, pc, r11, LSL #2
	B	swi_unknown
swi_handler_table
	B	library_resource	; &4D940 ;K
	B	library_string		; &4D941 ;K
	B	library_window		; &4D942 ;W
	B	library_icon		; &4D943 ;W
	B	library_date		; &4D944 ;A
	B	library_socket		; &4D945 ;S
	B	library_codec		; &4D946 ;S
	B	library_url		; &4D947 ;S
	B	library_memory		; &4D948 ;K
	B	library_heap		; &4D949 ;W
swi_handler_table_end

swi_unknown
	STMFD	r13!, {r14}
	ADR	r0, error_swi_unknown
	MOV	r1, #0				; Use global messages
	MOV	r2, #0				; Internal buffer
	ADR	r4, module_title		; Parameter 0
	SWI	XMessageTrans_ErrorLookup	; returns with V set
	LDMFD	r13!, {pc}
error_swi_unknown
	DCD	&1E6
	DCB	"BadSWI", 0
	ALIGN


	END
