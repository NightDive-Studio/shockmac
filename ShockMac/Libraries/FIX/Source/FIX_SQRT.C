/*

Copyright (C) 2015-2018 Night Dive Studios, LLC.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
*/
//=================================================================
//
//		System Shock - �1994-1995 Looking Glass Technologies, Inc.
//
//		FIX_SQRT.c	-	Square root routine for fixed-point numbers.  Adapted from 80386 asm.
//
//=================================================================


//--------------------
//  Includes
//--------------------
#include "fix.h"

//--------------------
//  Table of square root guesses
//--------------------
ubyte  pGuessTable[256] =
{
	1,
	1, 1,
	2, 2, 2, 2,
	3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
};
	
//--------------------
//  Includes
//--------------------
#if defined(powerc) || defined(__powerc)
long long_sqrt(long num);
#else
asm long long_sqrt(long num);
#endif

//-----------------------------------------------------------------
//  Calculate the square root of a fixed-point number.
//-----------------------------------------------------------------
fix fix_sqrt(fix num)
{
	fix	res = long_sqrt(num);
	
	// Make the number a fix and return it
	return (res << 8);
}

// PowerPC  versions of quad_sqrt & long_sqrt
#if defined(powerc) || defined(__powerc)
//-----------------------------------------------------------------
//  Calculate the square root of a wide (64-bit) number.
//-----------------------------------------------------------------
long quad_sqrt(long hi, long lo)
{
//	uchar	testb, trans;
//	uchar	shift;
//	long		divisor, temp, savediv, rem;

	// Parameter checking
	
	if (hi == 0)										// If there is no high word
	{
		if (lo > 0)										// If lo word is positive, just call long_sqrt()
			return(long_sqrt(lo));
		if (lo == 0)									// If lo word is zero, return 0.
			return(0);
	}
	if (hi < 0)											// If a negative number, return 0.
		return(0);
	
	// If 'hi' is non-zero, call FixMath's WideSquareRoot.
	
	wide	a;
	a.hi = hi;
	a.lo = lo;
	return(WideSquareRoot(&a));

/*  We gave it the ol' college try, but WideSquareRoot is faster in this case.

	// Find the highest byte that is non-zero (look only in hiword, since we're assured at this point
	// that it is non-zero), look up a good first guess for that byte, and shift it the appropriate amount.
	
	testb = (uchar)(hi >> 24);
	if (testb != 0)
	{
		trans = pGuessTable[testb];
		shift = 12 + 16;
		goto q_found_byte;
	}
	testb = (uchar)(hi >> 16);
	if (testb != 0)
	{
		trans = pGuessTable[testb];
		shift = 8 + 16;
		goto q_found_byte;
	}
	testb = (uchar)(hi >> 8);
	if (testb != 0)
	{
		trans = pGuessTable[testb];
		shift = 4 + 16;
		goto q_found_byte;
	}
	testb = (uchar)hi;
	if (testb != 0)
	{
		trans = pGuessTable[testb];
		shift = 16;
	}
	
	// We now have the good initial guess.  Shift it the appropriate amount.
	
q_found_byte:
	divisor = trans;
	divisor = divisor << shift;
	
	// Experience has shown that we almost always go through the loop
	// just about three times.  To avoid compares and jumps, we iterate
	// three times without even thinking about it, and then start checking
	// to see if our answer is correct.

	for (int i = 0; i < 2; i++)
	{
		temp = AsmWideDivide(hi, lo, divisor);
		divisor += temp;
		divisor = divisor >> 1;
	}
	
	// Starting with the third iteration, we now actually check for a match.
	
	while (true)
	{
		AWide	orig, test;
		
		temp = AsmWideDivide(hi, lo, divisor);
		if (temp == divisor)
			break;
		
		AsmWideMultiply(temp, divisor, &test);
		orig.hi = hi;
		orig.lo = lo;
		AsmWideSub(&orig, &test);
		rem = orig.lo;
		
		savediv = divisor;
		divisor += temp;
		divisor = divisor >> 1;
		if (temp == divisor)
		{
			if (rem != 0)
				divisor++;
			break;
		}
		else if (savediv == divisor)
		{
			if (rem != 0)
				divisor++;
			break;
		}
	}
	return (divisor);  */
}

//-----------------------------------------------------------------
//  Calculate the square root of a long number.
//-----------------------------------------------------------------
long long_sqrt(long num)
{
	fix		savediv;
	fix		temp;
	uchar	testb, trans;
	uchar	shift;
	fix		divisor;
	short		i;
	
	if (num == 0)								// A bit of error checking.
		return (0);
	if (num < 0)
		return (0);
		
	// Find the highest byte that is non-zero, look up a good first guess for
	// that byte, and shift it the appropriate amount.
	
	testb = (uchar)(num >> 24);
	if (testb != 0)
	{
		trans = pGuessTable[testb];
		shift = 12;
		goto found_byte;
	}
	testb = (uchar)(num >> 16);
	if (testb != 0)
	{
		trans = pGuessTable[testb];
		shift = 8;
		goto found_byte;
	}
	testb = (uchar)(num >> 8);
	if (testb != 0)
	{
		trans = pGuessTable[testb];
		shift = 4;
		goto found_byte;
	}
	testb = (uchar)num;
	if (testb != 0)
	{
		trans = pGuessTable[testb];
		shift = 0;
	}

	// We now have the good initial guess.  Shift it the appropriate amount.
	
found_byte:
	divisor = trans;
	divisor = divisor << shift;
	
	// Experience has shown that we almost always go through the loop
	// just about three times.  To avoid compares and jumps, we iterate
	// three times without even thinking about it, and then start checking
	// to see if our answer is correct.

	for (i = 0; i < 2; i++)
	{
		temp = num / divisor;
		divisor += temp;
		divisor = divisor >> 1;
	}
	
	// Starting with the third iteration, we now actually check for a match.
	
	while (true)
	{
		temp = num / divisor;
		if (temp == divisor)
			break;
		
		savediv = divisor;
		divisor += temp;
		divisor = divisor >> 1;
		if (temp == divisor)
		{
			if (num % savediv != 0)
				divisor++;
			break;
		}
		else if (savediv == divisor)
		{
			if (num % savediv != 0)
				divisor++;
			break;
		}
	}
	return (divisor);
}

#else
// 68k versions of quad_sqrt & long_sqrt
asm long quad_sqrt(long hi, long lo)
 {
 	move.l	4(sp),d1				// check hi
 	bne.s		must_use_quad
	
	move.l	8(sp),-(sp)
	jsr		long_sqrt
	addq.w	#4,sp
	rts
 
must_use_quad:
 	move.l		8(sp),d0			// low
 	movem.l		d3-d6,-(sp)		// save regs
 	move.l		d0,d5				// save lo
 	move.l		d1,d6				// save high

//;;;;;;;;;;;;;;;;;;;;;;;;;;;

 	lea			pGuessTable,a0		// get table address

// we find the highest byte of the argument that is non-zero,
// look up a good first guess for that byte,
// and shift it the appropriate amount (cl)
q_test_A:
	swap			d1			// ror	edx,16

	move.w		d1,d3
	andi.w		#0xff00,d3		// or	dh, dh		       // check high 8 bits of eax
	bmi.s			q_return_zero	// js	return_zero	       // arg is negative!
	beq.s			q_test_B			// jz	q_test_B
	ror.w		#8,d3
	move.b		d3,d1				// mov	al, dh		       // prepare for xlat
	move.w		#16+12,d2		// mov	cl, 16+12
	bra.s			q_found_bits		// jmp	q_found_bits

q_test_B:
	tst.b			d1						// or	dl, dl		       // check next 8 bits of eax
	beq.s			q_test_C			// jz	q_test_C
											// mov	al, dl		       // prepare for xlat
	move.w		#16+8,d2			// mov	cl, 16+8
	bra.s			q_found_bits		// jmp	q_found_bits

q_test_C:
	swap			d1						// ror	edx,16

	move.w		d1,d3
	andi.w		#0xff00,d3		// or	dh, dh		       // check next 8 bits of eax
	beq.s			q_test_D			// jz	q_test_D
	ror.w		#8,d3
	move.b		d3,d1				// mov	al, ah		       // prepare for xlat
	move.w		#16+4,d2			// mov	cl, 16+4
	bra.s			q_found_bits		// jmp	q_found_bits

q_test_D:			       	       // deal with low 8 bits of eax
	moveq		#16,d2				// mov	cl, 16

// now we generate the good initial guess

q_found_bits:
	moveq		#0,d3
	andi.w		#0x00ff,d1		// clear high byte
	move.b		(a0,d1.w),d3		//	xlat
											// movzx	ebx, al
	asl.l			d2, d3				// shl	ebx, cl		       // bx now has the first guess

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


// experience has shown that we almost always go through the loop
// just about three times.  to avoid compares and jumps, we iterate
// three times without even thinking about it, and then start checking
// if our answer is correct.

q_newton_loop:
	move.l	d5, d0
	move.l	d6,d1
	dc.l		0x4C430401      //  divu.l	d3,d1:d0
	add.l		d0,d3
	roxr.l	#1,d3

	move.l	d5, d0
	move.l	d6,d1
	dc.l		0x4C430401      //  divu.l	d3,d1:d0
	add.l		d0,d3
	roxr.l	#1,d3
	
// starting with the third iteration, we start actually checking
q_newton_loop2:
	move.l	d5, d0
	move.l	d6,d1
	dc.l		0x4C430401      //  divu.l	d3,d1:d0
	cmp.l		d0,d3					// cmp	eax, ebx
	beq.s		q_bx_is_correct		// je	q_bx_is_correct	       // if bx = ax, we win
	move.l	d3,d2					// mov	ecx, ebx		       // save old bx for comparing
	add.l		d0,d3					// add	ebx, eax
	roxr.l	#1,d3					// rcr	ebx, 1		       // bx <- (bx+ax)/2
	cmp.l		d0,d3					// cmp	eax, ebx		       // compare old ax with average
	beq.s		q_bx_is_close		// je	q_bx_is_close
	cmp.l		d2,d3					// cmp	ecx, ebx		       // compare old bx with average
	beq.s		q_bx_is_close		// je	q_bx_is_close
	bra.s		q_newton_loop2		// jmp	q_newton_loop2
	
// now we must find which is closer to x, bx^2 or (bx+1)^2
// x = bx*(bx+1)+r = bx^2 + bx + r
//   which is closer to bx^2 than to bx^2 + 2bx + 1
//   only if r = 0!  luckily we have r in dx from the divide
q_bx_is_close:
	tst.l		d1							// or	edx,edx
	beq.s		q_bx_is_correct		// jz	q_bx_is_correct
	addq.l		#1,d3					// inc	ebx

q_bx_is_correct:
	move.l		d3,d0				// mov	eax, ebx
 	movem.l		(sp)+,d3-d6		// restore regs
	rts									// ret

q_return_zero:
	moveq		#0,d0				// sub	eax, eax	       // return 0
 	movem.l		(sp)+,d3-d6		// restore regs
	rts									// ret
 }

asm long long_sqrt(long num)
 {
 	move.l		4(sp),d0				// get num
 	movem.l		d3-d5,-(sp)		// save regs
 	move.l		d0,d5				// save num
 	
 	lea			pGuessTable,a0		// get table address
	move.l		d0,d1				// mov	edx, eax
	swap			d1						// ror	edx, 16		       // get high 16 bits of arg in dx

// we find the highest byte of the argument that is non-zero,
// look up a good first guess for that byte,
// and shift it the appropriate amount (cl)
test_A:
	move.w		d1,d3
	andi.w		#0xff00,d3		// or	dh, dh		       // check high 8 bits of eax
	bmi.s			return_zero		// js	return_zero	       // arg is negative!
	beq.s			test_B				// jz	test_B
	ror.w		#8,d3
	move.b		d3,d1				// mov	al, dh		       // prepare for xlat
	moveq		#12,d2				// mov	cl, 12
	bra.s			found_bits			// jmp	found_bits

test_B:
	tst.b			d1						// or	dl, dl		       // check next 8 bits of eax
	beq.s			test_C				// jz	test_C
											// mov	al, dl		       // prepare for xlat
	moveq		#8,d2				// mov	cl, 8
	bra.s			found_bits			// jmp	found_bits

test_C:
	move.w		d0,d3
	andi.w		#0xff00,d3		// or	ah, ah		       // check next 8 bits of eax
	beq.s			test_D				// jz	test_D
	move.b		d0,d1				// mov	al, ah		       // prepare for xlat
	moveq		#4,d2				// mov	cl, 4
	bra.s			found_bits			// jmp	found_bits

test_D:			       	       // deal with low 8 bits of eax
	tst.b			d0						// or	al, al		       // avoid divide by 0
	beq.s			return_zero		// jz	return_zero
	moveq		#0,d2				// sub	cl, cl

// now we generate the good initial guess

found_bits:
	andi.l			#0x00ff,d1		// clear high bytes
	move.b		(a0,d1.w),d1		// xlat
											// movzx	ebx, al
	asl.l			d2,d1				// shl	ebx, cl		       // bx now has the first guess

// experience has shown that we almost always go through the loop
// just about three times.  to avoid compares and jumps, we iterate
// three times without even thinking about it, and then start checking
// if our answer is correct.

newton_loop:
	move.l		d5,d0
	divu.w		d1,d0
	add.w			d0,d1
	roxr.w		#1,d1

	move.l		d5,d0
	divu.w		d1,d0
	add.w			d0,d1
	roxr.w		#1,d1

// starting with the third iteration, we start actually checking

newton_loop2:
	move.l		d5,d0
	divu.w		d1,d0
	cmp.w		d0,d1				// cmp	ax, bx
	beq.s			bx_is_correct	// je	bx_is_correct	       // if bx = ax, we win
	move.w		d1,d2				// mov	cx, bx		       // save old bx for comparing
	add.w			d0,d1				// add	bx, ax
	roxr.w		#1,d1				// rcr	bx, 1		       // bx <- (bx+ax)/2
	cmp.w		d0,d1				// cmp	ax, bx		       // compare old ax with average
	beq.s			bx_is_close		// je	bx_is_close
	cmp.w		d2,d1				// cmp	cx, bx		       // compare old bx with average
	beq.s			bx_is_close		// je	bx_is_close
	bra.s			newton_loop2
	
// now we must find which is closer to x, bx^2 or (bx+1)^2
// x = bx*(bx+1)+r = bx^2 + bx + r
//   which is closer to bx^2 than to bx^2 + 2bx + 1
//   only if r = 0!  luckily we have r in dx from the divide
bx_is_close:
	swap			d0
	tst.w			d0
	beq.s			bx_is_correct
	addq.l			#1,d1

bx_is_correct:
	move.l		d1,d0		// mov	eax, ebx
 	movem.l		(sp)+,d3-d5		// restore regs
	rts							// ret

return_zero:
	moveq		#0,d0		// sub	eax, eax	       // return 0
 	movem.l		(sp)+,d3-d5		// restore regs
	rts							// ret
 }
#endif

