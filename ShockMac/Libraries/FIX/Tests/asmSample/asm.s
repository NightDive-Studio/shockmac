cr1:	equ	1

linkageArea:		set 24	; constant comes from the PowerPC Runtime Architecture Document
CalleesParams:		set	32	; always leave space for GPR's 3-10
CalleesLocalVars:	set 0	; ClickHandler doesn't have any
numGPRs:			set 0	; num volitile GPR's (GPR's 13-31) used by ClickHandler
numFPRs:			set 0	; num volitile FPR's (FPR's 14-31) used by ClickHandler

spaceToSave:	set linkageArea + CalleesParams + CalleesLocalVars + 4*numGPRs + 8*numFPRs  


	; declare the C function DisplayAlert as external
	import .DisplayAlert
	
	import gHelloString		; global variable from C program
	import gGoodbyeString	; global variable from C program

	toc
		tc gHelloString[TC], gHelloString
		tc gGoodbyeString[TC], gGoodbyeString
	
	; Need to export both the function descriptor and an entry point for
	; ClickHandler.  The function entry point doesn't have to be a csect
	; name - it can be a regular label.  Since PPCC puts a '.' in front of
	; function names, the entry point label's name must begin with a '.'
	
	export ClickHandler[DS]
	export .ClickHandler[PR]

	; create a TOC entry for the the function descriptor for function.  This is 

	toc
		tc ClickHandler[TC], ClickHandler[DS]

	; The function descriptor contains definitions used for runtime linkage
	; of the function.  The address of the function entry point must be in 
	; the first longword.  The address of the start of the TOC for the  
	; module that contains the function (use the predefined PPCAsm
	; global tc0) must be in the second longword.  The third longword 
	; contains the environment pointer, which is not used by C and so is set
	; to 0.
	
	csect	ClickHandler[DS]
		dc.l	.ClickHandler[PR]
		dc.l	TOC[tc0]
		dc.l	0
		
	; This is the actual code of the ClickHandler function 

	csect	.ClickHandler[PR]

; PROLOGUE - called routine's responsibilities
		mflr	r0					; Get link register
		stw		r0, 0x0008(SP)		; Store the link resgister on the stack
		stwu	SP, -0x0038(SP)		; skip over the stack space where the caller
									; might have saved stuff
		
; FUNCTION BODY
		extsh	r3,r3
		li      r9,0
		cmpwi	r3,1
		bne		else_if
		lwz		r4,gHelloString[TC](RTOC)
		lwz     r9,0x0000(r4)
		b		end_of_if
else_if:
		cmpwi	cr1,r3,2
		bne		cr1,end_of_if
		lwz		r5,gGoodbyeString[TC](RTOC)
		lwz     r9,0x0000(r5)
end_of_if:
	
		ori     r3,r9,0x0000
		
		; Now call DisplayAlert.  The parameter is in register 3
		; No need to save any registers since we don't use them after this call.

		bl		.DisplayAlert
		nop						; this may be fixed up by the linker

; EPILOGUE - return sequence		
		lwz		r0,0x8+spaceToSave(SP)	; Get the saved link register
		addic	SP,SP,spaceToSave		; Reset the stack pointer
		mtlr	r0						; Reset the link register
		blr								; return via the link register
;		lwz       r0,0x0000(r0)
	