; --------------------------------------------------------------------------
;    Name: Heap
; Purpose: Sliding heap manager
;  Author: © David Thomas, 1997-2009
; --------------------------------------------------------------------------


; 0.00 (12 Jul 1997)
; - Coded with attention to unsigned operators, to allow correct behaviour
;   above 2Gb.
; - Some StrongARM optimisations applied (memory and ALU ops interleaved).

; 0.01 (23 Jul 1997)
; - Create_heap was returning the hp in R0 rather than in R1 as it should
;   have.
; - Delete_heap wasn't preserving R0 and R2.  It also wasn't loading back its
;   registers from the stack in event of an invalid_heap.
; - Claim_block, release_block and resize_block weren't preserving R0.
; - Documented all routines.
; - Re-worded the error_invalid_heap error.
; - Guard word 'AEHp' becomes 'GCHp'.

; 0.02 (06 Aug 1997)
; - Fixed dynamic area problem - wasn't putting the da_base in R10 after it
;   had been created.

; 0.03 (01 Aug 1998)
; - 'GCHp' becomes 'AEHp'.

; 0.04 (24 Oct 1998)
; - Fixed problem with jump table CMP - wasn't dividing by four.

; 0.05 (29 Jun 1999)
; - Errors weren't handled correctly.  If a dynamic area could not be created
;   then VS would be signalled, but R0 preserved when it should have been
;   pointing to an error block.


; Includes

	GET	AppEngine:SWI.Hdr.OS
	GET	AppEngine:SWI.Hdr.Wimp
	GET	AppEngine:Hdr.Types
	GET	Hdr.appengine
	GET	Hdr.heap

; Code

	AREA	|_heap|, CODE, READONLY

	IMPORT	memory_copy
	IMPORT	memory_fill

	EXPORT	library_heap

library_heap
	CMP	r0, #(heap_end-heap_start)/4
	ADDCC	pc, pc, r0, LSL #2
	MOV	pc, r14
heap_start
	B	create_heap
	B	delete_heap
	B	claim_block
	B	release_block
	B	resize_block
heap_end


create_heap
;
; Purpose
;   Creates a new heap.
; Entry
;   R1 = pointer to address to create heap at (usually =HIMEM),
;	 or zero to create a dynamic area for the heap
;   R2 = pointer to a name for the dynamic area
; Exit
;   R1 = pointer to heap
;   All other registers preserved
;
	STMFD	r13!, {r0, r2-r8, r10, r14}
	CMP	r1, #0			; dynamic area?
	BEQ	create_heap_in_dynamic	; yes
	MOV	r10, r1			; keep hp in R10
create_heap_in_slot
	MOV	r0, #-1			; determine the slot size
	MOV	r1, #-1
	SWI	XWimp_SlotSize
	MOVVC	r6, r0			; keep oldslot for later
	ADDVC	r0, r0, #hp_basesize
	MOVVC	r7, r0			; keep oldslot+hp_basesize for check
	MOVVC	r1, #-1
	SWIVC	XWimp_SlotSize
	ADDVS	r13, r13, #4
	LDMVSFD	r13!, {r2-r8, r10, pc}
	CMP	r0, r7			; CMP newslot,oldslot+hp_basesize
	BCS	create_heap_in_slot_no_error
create_heap_in_slot_error
	MOV	r0, r6			; restore oldslot
	MOV	r1, #-1
	SWI	XWimp_SlotSize
	ADDVS	r13, r13, #4
	LDMVSFD	r13!, {r2-r8, r10, pc}
	MOV	r1, #0			; failure
	LDMFD	r13!, {r0, r2-r8, r10, pc}	; return
create_heap_in_slot_no_error
	; R0 = newslot, R4 = oldslot
	MOV	r1, #0			; = hp_dynamic
	SUB	r5, r0, r6		; hp_size = newslot-oldslot
	MOV	r6, r0			; = slotsize
	B	create_heap_fill_in_fields
create_heap_in_dynamic
	MOV	r8, r2			; name
	MOV	r0, #0			; create da
	MOV	r1, #-1
	MOV	r2, #hp_basesize
	MOV	r3, #-1
	MOV	r4, #128		; flags
	MOV	r5, #-1
	MOV	r6, #0
	MOV	r7, #0
	SWI	XOS_DynamicArea
	MOVVC	r10, r3			; set hp
	MOVVC	r0, #2			; read da
	SWIVC	XOS_DynamicArea
	ADDVS	r13, r13, #4
	LDMVSFD	r13!, {r2-r8, r10, pc}
	MOV	r5, r2			; hp_size
	MOV	r6, r2			; 'slot'
create_heap_fill_in_fields
	; Needs: R1 = DA handle  R5 = hp_size  R6 = slot  R10 = hp
	LDR	r0, guardword
	MOV	r2, #0
	MOV	r3, #hp_baseanchors
	MOV	r4, #hp_basesize
	SUB	r5, r5, r4
	STMIA	r10, {r0-r6}
	ADD	r1, r10, #hp_headersize
	MOV	r2, #hp_baseanchors*8
	MOV	r3, #0
	BL	memory_fill
	MOV	r1, r10			; success
	LDMFD	r13!, {r0, r2-r8, r10, pc}


delete_heap
;
; Purpose
;   Deletes a heap.
; Entry
;   R1 = pointer to heap
; Exit
;   R1 = 0 (i.e. heap to return)
;   All other registers preserved
;
	STMFD	r13!, {r0, r2, r10, r14}
	LDR	r0, guardword		; }
	MOV	r10, r1			; keep hp in R10
	LDR	r14, [r10]		; } check the heap
	CMP	r0, r14			; }
	LDMNEFD	r13!, {r0, r2, r10, r14}	; }
	BNE	invalid_heap		; }
	LDR	r1, [r10, #hp_dynamic]	; using a dynamic area?
	CMP	r1, #0
	BNE	delete_heap_in_dynamic	; yes
delete_heap_in_slot
	LDR	r0, [r10, #hp_slotsize]	; IF FNslotsize(-hp%!16-hp%!20)
	LDR	r14, [r10, #hp_size]	; i.e. remove all heap memory
	SUB	r0, r0, r14
	LDR	r14, [r10, #hp_free]
	SUB	r0, r0, r14
	MOV	r1, #-1
	SWI	XWimp_SlotSize
	B	delete_heap_exit
delete_heap_in_dynamic
	; R1 holds dynamic area handle
	MOV	r0, #1			; remove dynamic area
	SWI	XOS_DynamicArea
	MOVVC	r1, #0			; hp = 0
delete_heap_exit
	LDMVCFD	r13!, {r0, r2, r10, pc}
	ADD	r13, r13, #4		; error
	LDMFD	r13!, {r2, r10, pc}


claim_block
;
; Purpose
;   Claims a block of memory from a heap.
; Entry
;   R1 = pointer to heap
;   R2 = size of block required
; Exit
;   R1 = pointer to anchor
;   All other registers preserved
;
	STMFD	r13!, {r0, r2-r4, r10, r14}
	LDR	r0, guardword		; }
	MOV	r10, r1			; keep hp in R10
	LDR	r14, [r10]		; } check the heap
	CMP	r0, r14			; }
	LDMNEFD	r13!, {r0, r2-r4, r10, r14}	; }
	BNE	invalid_heap		; }
	CMP	r2, #0			; check for 0 byte allocation
	MOVEQ	r1, #0
	LDMEQFD	r13!, {r0, r2-r4, r10, pc}
	; get an anchor
	BL	newanchor
	ADDVS	r13, r13, #4
	LDMVSFD	r13!, {r2-r4, r10, pc}
	CMP	r0, #0			; newanchor failed?
	MOVEQ	r1, #0			; yes {
	LDMEQFD	r13!, {r0, r2-r4, r10, pc}	;  exit with R1=0 }
	MOV	r1, r0			; keep anchor
	; allocate some space
	ADD	r0, r2, #3		; round size up ((size+3) AND NOT 3)
	BIC	r0, r0, #3
	MOV	r4, r0			; keep rounded-up size
	BL	allocate		; R10 = hp, R0 = size
	ADDVS	r13, r13, #4
	LDMVSFD	r13!, {r2-r4, r10, pc}
	CMP	r0, #0			; allocate failed?
	MOVEQ	r0, r1			; yes, R0 = anchor, R10 = hp
	BLEQ	delanchor		; yes, delete the now-unused anchor
	MOVEQ	r1, #0			; ...
	LDMEQFD	r13!, {r0, r2-r4, r10, pc}	; yes, exit with R1=0
	; update the anchor
	LDR	r14, [r10, #hp_size]
	ADD	r3, r10, r14
	STMIA	r1, {r3, r4}		; !anchor=hp+hp!16; anchor!4=size
	ADD	r14, r14, r4
	STR	r14, [r10, #hp_size]	; hp%!16+=size%
	LDR	r14, [r10, #hp_free]
	SUB	r14, r14, r4
	STR	r14, [r10, #hp_free]	; hp%!20-=size%
	LDMFD	r13!, {r0, r2-r4, r10, pc}


release_block
;
; Purpose
;   Releases a block of memory from a heap.
; Entry
;   R1 = pointer to heap
;   R2 = pointer to anchor
; Exit
;   R1 = 0 (i.e. anchor to return)
;   All other registers preserved
;
	STMFD	r13!, {r0, r2-r6, r10, r14}
	LDR	r0, guardword		; }
	MOV	r10, r1			; keep hp in R10
	LDR	r14, [r10]		; } check the heap
	CMP	r0, r14			; }
	LDMNEFD	r13!, {r0, r2-r6, r10, r14}	; }
	BNE	invalid_heap		; }
	; If there's anything above the block, then shift it down.
	MOV	r4, r2			; keep anchor
	LDMIA	r4, {r5-r6}		; R5=address, R6=size
	ADD	r1, r5, r6
	MOV	r2, r5
	LDR	r3, [r10, #hp_size]
	SUB	r3, r3, r1
	ADD	r3, r3, r10
	BL	memory_copy
	; Adjust the heap size
	LDR	r14, [r10, #hp_size]	; hp%!16-=size%
	SUB	r14, r14, r6
	STR	r14, [r10, #hp_size]
	LDR	r14, [r10, #hp_free]	; hp%!20+=size%
	ADD	r14, r14, r6
	STR	r14, [r10, #hp_free]
	; Go through the anchors and fiddle 'em down.
	MOV	r0, r1			; from above
	RSB	r1, r6, #0		; -size
	BL	adjustanchors
	MOV	r0, r4			; anchor
	BL	delanchor		; R0 becomes zeroed
	BL	reduce
	MOV	r1, #0			; zero the return anchor
	LDMFD	r13!, {r0, r2-r6, r10, pc}


resize_block
;
; Purpose
;   Resizes a block of memory in a heap.
; Entry
;   R1 = pointer to heap
;   R2 = pointer to anchor
;   R3 = number of bytes to be added or removed from the end of the specified
;	 block (a signed integer)
; Exit
;   R1 = -1 for success, 0 for failure
;   All other registers preserved
; Notes
;   +ve change, move the rest of the heap up by the difference
;   -ve change, move the rest of the heap down over the part of the area to
;		be discarded.
;
	STMFD	r13!, {r0, r2-r6, r10, r14}
	LDR	r0, guardword		; }
	MOV	r10, r1			; keep hp in R10
	LDR	r14, [r10]		; } check the heap
	CMP	r0, r14			; }
	LDMNEFD	r13!, {r0, r2-r6, r10, r14}	; }
	BNE	invalid_heap		; }
	; Align the change size
	ADD	r3, r3, #3
	BIC	r3, r3, #3
	CMP	r3, #0
	BEQ	resize_block_exit_success ; resize by 0 bytes is ok
	MOV	r4, r2			; keep anchor
	LDMIA	r4, {r5-r6}		; R5=address, R6=size
	ADD	r14, r6, r3
	CMP	r14, #0			; would the block become too small?
	MOVLE	r1, #0			; yes { failure }
	LDMLEFD	r13!, {r0, r2-r6, r10, pc}
	CMP	r3, #0
	BLE	resize_block_skip_allocate
	; allocate more memory
	MOV	r0, r3
	BL	allocate
	ADDVS	r13, r13, #4
	LDMVSFD	r13!, {r2-r6, r10, pc}
	CMP	r0, #0
	MOVEQ	r1, #0			; failure
	LDMEQFD	r13!, {r0, r2-r6, r10, pc}
resize_block_skip_allocate
	; memory_copy(addr+size,addr+size+change,hp+hp!16-addr-size)
	ADD	r1, r5, r6
	ADD	r2, r1, r3
	MOV	r0, r3			; keep
	LDR	r3, [r10, #hp_size]
	SUB	r3, r3, r1
	ADD	r3, r3, r10
	BL	memory_copy
	MOV	r3, r0
	; Adjust the heap size
	LDR	r14, [r10, #hp_size]	; hp!16+=size
	ADD	r14, r14, r3
	STR	r14, [r10, #hp_size]
	LDR	r14, [r10, #hp_free]	; hp!20-=size
	SUB	r14, r14, r3
	STR	r14, [r10, #hp_free]
	CMP	r3, #0
	BLLT	reduce
	LDR	r14, [r4, #4]		; anchor!4+=change
	ADD	r14, r14, r3
	STR	r14, [r4, #4]
	MOV	r0, r1			; R1 from addr+size above
	MOV	r1, r3
	BL	adjustanchors
resize_block_exit_success
	MOV	r1, #-1			; success
	LDMFD	r13!, {r0, r2-r6, r10, pc}


newanchor
;
; Purpose
;   Allocate a new anchor, extending the heap if required.
; Entry
;   R10 = pointer to heap
; Exit
;   R0 = pointer to anchor
;   All other registers preserved
;
	STMFD	r13!, {r1-r3, r14}
	LDR	r0, [r10, #hp_freeanchors]
	CMP	r0, #0
	BNE	newanchor_enough
newanchor_not_enough
	; Allocate enough free space for the anchors
	MOV	r0, #hp_baseanchors*8
	BL	allocate
	LDMVSFD	r13!, {r1-r3, pc}
	CMP	r0, #0
	LDMEQFD	r13!, {r1-r3, pc}	; failed, exit with R0=0
	; R1 'addr' = hp+hp_headersize+(hp!8+hp!12)*8
	LDR	r2, [r10, #hp_usedanchors]
	ADD	r1, r10, #hp_headersize	; positioned for SA-opt
	LDR	r14, [r10, #hp_freeanchors]
	ADD	r2, r2, r14
	ADD	r1, r1, r2, LSL #3	; hp+hp_headersize+(hp!8+hp!12)*8
	; R2 = addr+hp_baseanchors*8
	ADD	r2, r1, #hp_baseanchors*8
	; R3 = hp!16-addr+hp
	LDR	r3, [r10, #hp_size]
	SUB	r3, r3, r1
	ADD	r3, r3, r10
	BL	memory_copy
	; *** could shuffle around here to not need the duplicated loads
	LDR	r14, [r10, #hp_freeanchors] ; inc free anchors
	ADD	r14, r14, #hp_baseanchors
	STR	r14, [r10, #hp_freeanchors]
	LDR	r14, [r10, #hp_size]	; inc size of heap
	ADD	r14, r14, #hp_baseanchors*8
	STR	r14, [r10, #hp_size]
	LDR	r14, [r10, #hp_free]	; dec free memory
	SUB	r14, r14, #hp_baseanchors*8
	STR	r14, [r10, #hp_free]
	; memzero(addr,hp_baseanchors*8)
	; R1 = addr, from above
	MOV	r2, #hp_baseanchors*8
	MOV	r3, #0			; zero the area
	BL	memory_fill
	; adjustanchors(1,hp_baseanchors%*8)
	MOV	r0, #1
	MOV	r1, #hp_baseanchors*8	; from above
	BL	adjustanchors
newanchor_enough
	; Search for and allocate a free anchor
	ADD	r2, r10, #hp_headersize
newanchor_enough_loop
	LDR	r0, [r2], #8
	CMP	r0, #0
	BNE	newanchor_enough_loop
	SUB	r0, r2, #8		; correct for auto-inc
	LDR	r14, [r10, #hp_usedanchors]	; inc used anchors
	ADD	r14, r14, #1
	STR	r14, [r10, #hp_usedanchors]
	LDR	r14, [r10, #hp_freeanchors]	; dec free anchors
	SUB	r14, r14, #1
	STR	r14, [r10, #hp_freeanchors]
	LDMFD	r13!, {r1-r3, pc}

delanchor
;
; Purpose
;   Delete an anchor.
; Entry
;   R0 = pointer to anchor
;   R10 = pointer to heap
; Exit
;   R0 = 0 (i.e. anchor to return)
;   All other registers preserved
;
	STMFD	r13!, {r14}
	MOV	r14, #0
	STR	r14, [r0]
	STR	r14, [r0, #4]
	LDR	r14, [r10, #hp_usedanchors]	; dec used anchors
	SUB	r14, r14, #1
	STR	r14, [r10, #hp_usedanchors]
	LDR	r14, [r10, #hp_freeanchors]	; inc free anchors
	ADD	r14, r14, #1
	STR	r14, [r10, #hp_freeanchors]
	MOV	r0, #0
	LDMFD	r13!, {pc}

adjustanchors
;
; Purpose
;   Adjust the pointers of all anchors in a specific range.
; Entry
;   R0 = first address the change affects (use 1 for all)
;   R1 = change (bytes, signed integer)
;   R10 = pointer to heap
; Exit
;   All registers preserved
;
	STMFD	r13!, {r0, r2-r3, r14}
	LDR	r3, [r10, #hp_usedanchors]
	ADD	r2, r10, #hp_headersize	; positioned for SA-opt
	LDR	r14, [r10, #hp_freeanchors]
	ADD	r3, r3, r14
	ADD	r3, r2, r3, LSL #3	; hp+hp_headersize+(hp!8+hp!12)*8
adjustanchors_loop
	LDR	r14, [r2], #8
	CMP	r14, r0
	ADDCS	r14, r14, r1
	STRCS	r14, [r2, #-8]
	CMP	r2, r3
	BCC	adjustanchors_loop
	LDMFD	r13!, {r0, r2-r3, pc}

allocate
;
; Purpose
;   Add some more memory to our free space.
; Entry
;   R0 = min no of bytes to allocate
;   R10 = pointer to heap
; Exit
;   R0 = 0 for success, -1 for failure
;   All other registers preserved
;
	STMFD	r13!, {r14}
	LDR	r14, [r10, #hp_free]	; free space
	CMP	r14, r0			; enough free space already there?
	LDMCSFD	r13!, {pc}		; yes { exit }
	SUB	r0, r0, r14		; compensate the size requested
	BL	slotsize		; get some more memory
	LDMVSFD	r13!, {pc}
	CMP	r0, #0			; failed?
	LDRNE	r14, [r10, #hp_free]	; yes {
	ADDNE	r14, r14, r0		;  put back the free counter
	STRNE	r14, [r10, #hp_free]	;
	MOVNE	r0, #-1			;  and return with -1 }
	LDMFD	r13!, {pc}

reduce
;
; Purpose
;   Remove as much memory as possible from our free space.
; Entry
;   R10 = pointer to heap
; Exit
;   All registers preserved
;
	STMFD	r13!, {r0-r1, r14}
	; Use R1 rather than R14, as R14 will be corrupted by the branch.
	LDR	r1, [r10, #hp_free]
	RSB	r0, r1, #0		; = -hp!20
	BL	slotsize
	ADD	r1, r1, r0		; R1 -= slotsize(-hp!20)
	STR	r1, [r10, #hp_free]
	LDMFD	r13!, {r0-r1, pc}


slotsize
;
; Purpose
;   Change the size of the heap.
; Entry
;   R0 = change (bytes, signed integer)
;   R10 = pointer to heap
; Exit
;   R0 = actual change (bytes, signed integer)
;   All registers preserved
; Note
;   Wimp_SlotSize corrupts R4!
;
	STMFD	r13!, {r1-r5, r14}
	LDR	r14, [r10, #hp_dynamic]	; using a dynamic area?
	CMP	r14, #0
	BNE	slotsize_dynamic
slotsize_slot
	LDR	r5, [r10, #hp_slotsize]
	ADD	r0, r5, r0		; oldslot+change
	MOV	r3, r0			; keep oldslot+change
	MOV	r1, #-1
	SWI	XWimp_SlotSize
	LDMVSFD	r13!, {r1-r5, pc}
	CMP	r0, r3			; enough allocated?
	BCS	slotsize_slot_no_error	; yes
slotsize_slot_error
	MOV	r0, r5			; oldslot
	MOV	r1, #-1
	SWI	XWimp_SlotSize
	LDMVSFD	r13!, {r1-r5, pc}
	MOV	r0, #0			; failure
	LDMFD	r13!, {r1-r5, pc}
slotsize_slot_no_error
	STR	r0, [r10, #hp_slotsize]
	SUB	r0, r0, r5		; change=newslot-oldslot
	LDMFD	r13!, {r1-r5, pc}
slotsize_dynamic
	MOV	r3, r0			; keep change
	MOV	r1, r0			; change
	MOV	r0, r14			; handle
	SWI	XOS_ChangeDynamicArea
	LDMVSFD	r13!, {r1-r5, pc}
	CMP	r3, #0			; adjust the size change
	RSBLT	r0, r1, #0
	MOVGE	r0, r1
	; R0 = change
	LDR	r5, [r10, #hp_slotsize]
	ADD	r5, r5, r0
	STR	r5, [r10, #hp_slotsize]
	LDMFD	r13!, {r1-r5, pc}


guardword
	DCB	"AEHp"


invalid_heap
;
; Purpose
;   Return to caller with R0 pointing to an "invalid heap" error block.
;
	ADR	r0, invalid_heap_block
	;TEQ	pc, pc
	;ORRNES	pc, r14, #V	; 26-bit return
	MSR	CPSR_f, #V
	MOV	pc, r14

invalid_heap_block
	DCD	error_base + error_invalid_heap
	DCB	"Bad heap pointer given in R1", 0
	ALIGN

	END
