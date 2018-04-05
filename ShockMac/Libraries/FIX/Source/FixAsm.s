#
# Copyright (C) 2015-2018 Night Dive Studios, LLC.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#
#	File:		FixAsm.s
#
#	Contains:	PowerPC assembly implementations of Fixed math routines
#
#	Written by:	Ken Cobb. (fix_div_asm by Eric Traut)
#
# 	PPCAsm FixAsm.s -o FixAsm.s.o
#


	import gOVResult		; global variable from C program

	toc
		tc gOVResult[TC], gOVResult


	csect
	
#========================================================================
#	Multiplication routines
#========================================================================

;---------------------------------------
; fix fix_mul_asm(fix a, fix b)
;---------------------------------------
		EXPORT	.fix_mul_asm
	
	.fix_mul_asm:
		mulhw	r5,r3,r4				; int part into r5
		mullw	r3,r3,r4				; fract part into r3
		srwi	r3,r3,16				; shift fract portion to right
		rlwimi	r3,r5,16,0,15			; shift int part left 16 and place in
										; upper half of r3
		blr

;---------------------------------------
; fix24 fix24_mul_asm(fix24 a, fix24 b)
;---------------------------------------
	EXPORT	.fix24_mul_asm
	
	.fix24_mul_asm:
		mulhw	r5,r3,r4				; int part into r5
		mullw	r3,r3,r4				; fract part into r3
		srwi	r3,r3,8					; shift fract portion to right
		rlwimi	r3,r5,24,0,7			; shift int part left 24 and place in
										; upper 8 bits of r3
		blr

;---------------------------------------
; fix fix_mul_3_3_3_asm (fix a, fix b)
;---------------------------------------
	EXPORT	.fix_mul_3_3_3_asm
	
	.fix_mul_3_3_3_asm:
		mulhw	r5,r3,r4				; int part into r5
		mullw	r3,r3,r4				; fract part into r3
		srwi	r3,r3,29				; shift fract portion to right
		rlwimi	r3,r5,3,0,28			; shift int part left 3 and place in
										; upper 29 bits of r3
		blr

;---------------------------------------
; fix fix_mul_3_32_16_asm (fix a, fix b)
;---------------------------------------
	EXPORT	.fix_mul_3_32_16_asm
	
	.fix_mul_3_32_16_asm:
		mulhw	r5,r3,r4				; int part into r5
		mullw	r3,r3,r4				; fract part into r3
		srwi	r3,r3,13				; shift fract portion to right
		rlwimi	r3,r5,19,0,12			; shift int part left 19 and place in
										; upper 13 bits of r3
		blr

;---------------------------------------
; fix fix_mul_3_16_20_asm (fix a, fix b)
;---------------------------------------
	EXPORT	.fix_mul_3_16_20_asm
	
	.fix_mul_3_16_20_asm:
		mulhw	r5,r3,r4				; int part into r5
		srawi	r3,r5,1					; shift w/sign 1 to right
		blr

;---------------------------------------
; fix fix_mul_16_32_20_asm (fix a, fix b)
;---------------------------------------
	EXPORT	.fix_mul_16_32_20_asm
	
	.fix_mul_16_32_20_asm:
		mulhw	r5,r3,r4				; int part into r5
		mullw	r3,r3,r4				; fract part into r3
		srwi	r3,r3,4					; shift fract portion to right
		rlwimi	r3,r5,28,0,3			; shift int part left 28 and place in
										; upper 4 bits of r3
		blr

;---------------------------------------
; fix fast_fix_mul_int_asm (fix a, fix b);
;---------------------------------------
	EXPORT	.fast_fix_mul_int_asm
	
	.fast_fix_mul_int_asm:
		mulhw	r3,r3,r4				; return hi word of multiply
		blr

;---------------------------------------
; fix fix_mul_asm_safe(fix a, fix b)
;---------------------------------------
; checks for -1 return when it should really be 0 
		EXPORT	.fix_mul_asm_safe
	
	.fix_mul_asm_safe:
		mullw	r6,r3,r4				; fract part into r3
		mulhw	r5,r3,r4				; int part into r5
		cmpi	1,r6,-1					; check if r3 is -1
		srwi	r3,r6,16				; shift fract portion to right
		rlwimi	r3,r5,16,0,15			; shift int part left 16 and place in
										; upper half of r3
		cmpi	0,r3,-1					; is result -1?
		beq-	MaybeBadNum
NumOK:	
		blr

MaybeBadNum:
		beq		1,NumOK
		li		r3,0
		blr
		
#========================================================================
#	Division routine
#========================================================================

#
#	fix fix_div_asm(fix a, fix b);
#
#	On entry, this routine takes the following parameters:
#		r3 - 	operand a (fixed point, frac, or long format)
#		r4 -	operand b (fixed point, frac, or long format)
#	On exit:
#		r3 -	fixed point, frac, or long quotient
#
#	Within the routine, r0 and r5-r9 are used as scratch registers.
#	Condition register fields cr0 and cr1 are also used.
#
#	If b is zero, the function returns:
#		0x80000000 if a is negative
#		0x7FFFFFFF if a is zero or positive
#	If division overflows, the function returns:
#		0x80000000 if the result is negative
#		0x7FFFFFFF if the result is positive
#

		EXPORT	.fix_div_asm

# Internal register usage
rNumH:		EQU		r5
rNumL:		EQU		r6
rDen:		EQU		r4
rTemp1:		EQU		r7
rTemp2:		EQU		r8
rTemp3:		EQU		r9

bNumNeg:	EQU		4+0
bDenNeg:	EQU		0
bQuotNeg:	EQU		4+3

.fix_div_asm:
	li		r0,0				# create a handy copy of zero
	cmpwi	0,rDen,0			# check for divide by zero
	srawi	rNumH,r3,16			# mask off high word of fixed number and sign-extend
	rlwinm	rNumL,r3,16,0,15	# mask off low word of fixed number
	beq		0,FixDivByZero		# branch if divide by zero

	# At this point we have a 64-bit number in r5:r6 and a 32-bit
	# number in r4.

FixDivDoIt:
	lwz		rTemp1,gOVResult[TC](RTOC)
	stw		r0,0(rTemp1)
	cmpwi	1,rNumH,0			# check sign of numerator
	crxor	bQuotNeg,bDenNeg,bNumNeg
								# calculate sign of result, put it into bit 4
	bf		bDenNeg,FixDivCheckNumer 	
								# check sign of denominator
	neg		rDen,rDen			# make denominator positive if it was negative

FixDivCheckNumer:
	bf		bNumNeg,FixDiv64Common	
								# continue if numerator is positive
	subfc	rNumL,rNumL,r0		# negate denominator, carrying as appropriate
	subfe	rNumH,rNumH,r0

FixDiv64Common:
	cmplw	0,rNumH,rDen		# check for overflow

	cntlzw	rTemp1,rDen			# shift numer and denom left
	xor		rNumH,rNumH,rNumL	#  until denom’s most sig. bit is one
	slw		rDen,rDen,rTemp1
	rlwnm	rNumH,rNumH,rTemp1,0,31
	slw		rNumL,rNumL,rTemp1
	xor		rNumH,rNumH,rNumL
	
	bge-	FixDivOverflow		# branch if overflow

	srwi	rTemp2,rDen,16		
	divwu	rTemp3,rNumH,rTemp2	# perform 32-bit by 16-bit division
	mullw	rTemp2,rTemp3,rTemp2
	subf	rNumH,rTemp2,rNumH	# calculate remainder
	slwi	rNumH,rNumH,16
	rlwimi	rNumH,rNumL,16,16,31
	slwi	rNumL,rNumL,16
	rlwinm	rTemp2,rDen,0,16,31
	mullw	rTemp2,rTemp3,rTemp2
	subfc	rNumH,rTemp2,rNumH
	subfe.	rTemp2,rTemp2,rTemp2
	add		rNumL,rNumL,rTemp3
	bge		FixDiv64ComputeLow
FixDiv64CorrectHigh:
	addc	rNumH,rNumH,rDen
	addze.	rTemp2,rTemp2
	subi	rNumL,rNumL,1
	blt		FixDiv64CorrectHigh

FixDiv64ComputeLow:
	srwi	rTemp2,rDen,16
	divwu	rTemp3,rNumH,rTemp2
	mullw	rTemp2,rTemp3,rTemp2
	subf	rNumH,rTemp2,rNumH
	slwi	rNumH,rNumH,16
	rlwimi	rNumH,rNumL,16,16,31
	slwi	rNumL,rNumL,16
	rlwinm	rTemp2,rDen,0,16,31
	mullw	rTemp2,rTemp3,rTemp2
	subfc	rNumH,rTemp2,rNumH
	subfe.	rTemp2,rTemp2,rTemp2
	add		rNumL,rNumL,rTemp3
	bge		FixDiv64Done
FixDiv64CorrectLow:
	addc	rNumH,rNumH,rDen
	addze.	rTemp2,rTemp2
	subi	rNumL,rNumL,1
	blt		FixDiv64CorrectLow

FixDiv64Done:
	addco.	r3,rNumL,r0
	bt		bQuotNeg,FixDiv64QuotientNeg	
							# see if we need to negate answer
	blt-	FixDivOverflow	# check for overflow case
	blr						# return to caller

FixDiv64QuotientNeg:
	neg.	r3,r3			# negate answer
	bgt-	FixDivOverflow	# check for overflow
	blr						# return to caller


FixDivOverflow:
	li		r0,1			# set global result code to 1
	lwz		rTemp1,gOVResult[TC](RTOC)
	stw		r0,0(rTemp1)
	lis		r3,-0x8000		# load max negative number
	addi	r3,r3,1			# which is 0x80000001
	btlr	bQuotNeg		# return if result is negative
	subi	r3,r3,2			# calc max positive number
	blr						# return to caller

FixDivByZero:
	li		r0,2			# set global result code to 2
	lwz		rTemp1,gOVResult[TC](RTOC)
	stw		r0,0(rTemp1)
	cmpwi	0,r3,0			# is r3 negative?
	lis		r3,-0x8000		# load max negative number
	addi	r3,r3,1			# which is 0x80000001
	bge		PosNum			# branch if positive
	blr
PosNum:
	subi	r3,r3,2			# calc max positive number
	blr						# return to caller



# -----------------------------------------
# fix fix24_div_asm(fix a, fix b, fix c)
# -----------------------------------------
	EXPORT	.fix24_div_asm

.fix24_div_asm:
	li		r0,0				# create a handy copy of zero
	cmpwi	0,rDen,0			# check for divide by zero
	srawi	rNumH,r3,24			# mask off high word of fixed number and sign-extend
	rlwinm	rNumL,r3,8,0,23		# mask off low word of fixed number
	beq		0,FixDivByZero		# branch if divide by zero
	
	b		FixDivDoIt			# otherwise, do the divide


#========================================================================
#	Combination routines
#========================================================================

# -----------------------------------------
# fix fix_mul_div_asm(fix a, fix b, fix c)
#
#	Computes (a * b) / c, preserving the 
#	48-bit intermediate result.
# -----------------------------------------
	EXPORT	.fix_mul_div_asm
	
	.fix_mul_div_asm:
		cmpwi	0,r5,0				# check for divide by zero
		beq		0,FixDivByZero		# branch if divide by zero
		
		mr		r0,r5				# save denominator
		mulhw	r5,r3,r4			# int part into r5
		mullw	r6,r3,r4			# fract part into r6
		mr		r4,r0				# put denominator into r4
		li		r0,0				# create a handy copy of zero
		b		FixDivDoIt
		


#========================================================================
#	Wide math functions
#========================================================================

#	wide *AsmWideAdd(wide *target, wide *source);
#	
#	On entry, this routine takes the following parameters:
#		r3 - 	pointer to target
#		r4 -	pointer to source
#	On exit:
#		r3 -	pointer to target
#
#	Within the routine, r6-r9 are used as scratch registers.
#

	EXPORT .AsmWideAdd		# export the code symbol

# Internal register usage
rAH:		EQU		r6
rAL:		EQU		r7
rBH:		EQU		r8
rBL:		EQU		r9

.AsmWideAdd:
	lwz		rAL,4(r3)		# load low word of operand A
	lwz		rBL,4(r4)		# load low word of operand B
	lwz		rAH,0(r3)		# load high word of operand A
	addc	rAL,rAL,rBL		# add low words together
	lwz		rBH,0(r4)		# load high word of operand B
	stw		rAL,4(r3)		# store low word of result
	adde	rAH,rAH,rBH		# add high words with carry
	stw		rAH,0(r3)		# store high word of result
	blr						# return to caller
	

	
#	wide *AsmWideSub(wide *A, wide *B);
#	
#	On entry, this routine takes the following parameters:
#		r3 - 	pointer to A
#		r4 -	pointer to B
#	On exit:
#		r3 -	pointer to A (= A - B)
#
#	Within the routine, r6-r9 are used as scratch registers.
#

	EXPORT .AsmWideSub		# export the code symbol

.AsmWideSub:
	lwz		rAL,4(r3)		# load low word of operand A
	lwz		rBL,4(r4)		# load low word of operand B
	lwz		rAH,0(r3)		# load high word of operand A
	subfc	rAL,rBL,rAL		# lowA = lowA - lowB
	lwz		rBH,0(r4)		# load high word of operand B
	stw		rAL,4(r3)		# store low word of result
	subfe	rAH,rBH,rAH		# hiA = hiA - hiB (w/ carry)
	stw		rAH,0(r3)		# store high word of result
	blr						# return to caller


#	wide *AsmWideMultiply(long multiplicand, long multiplier, wide *target);
#	
#	On entry, this routine takes the following parameters:
#		r3 - 	operand A
#		r4 -	operand B
#		r5 -	pointer to result location
#	On exit:
#		r3 -	pointer to result
#
#	Within the routine, r6-r9 are used as scratch registers.
#

	EXPORT .AsmWideMultiply	# export the code symbol

.AsmWideMultiply:
	mullw	r6,r3,r4		# multiply low word
	mulhw	r7,r3,r4		# multiply high word (signed)
	stw		r6,4(r5)		# store low word of result
	mr		r3,r5			# move return parameter into place
	stw		r7,0(r5)		# store low word of result
	blr						# return to caller


# -----------------------------------------
# long AsmWideDivide(long hi, long lo, long divisor)
#
#	Divides a wide number (hi:lo) by the divisor. 
#	Returns long result in r3.
# -----------------------------------------
	EXPORT	.AsmWideDivide
	
	.AsmWideDivide:
		li		r0,0				# create a handy copy of zero
		cmpwi	0,r5,0				# check for divide by zero
		beq		0,FixDivByZero		# branch if divisor is zero
		
		mr		r7,r5				# save divisor for now
		mr		r5,r3				# put hi into r5
		mr		r6,r4				# put lo into r6
		mr		r4,r7				# put divisor into r4
		b		FixDivDoIt
