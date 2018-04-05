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
// Optimized memset, memcpy, and memmove routines, for 68K and PowerPC
// MLA - 3/20/95

#include "LG.h"

// some handy 68000 assembly defines
#define blo	bcs		// branch low (unsigned <)
#define bhs	bcc		// branch high or same (unsigned >=)

#if (defined(powerc) || defined(__powerc))	
// PowerPC optimized versions of standard memory ops

// memset specifically optimized for PPC
void *LG_memset(void *dest, int val, unsigned int count)
 {
 	uchar 	*dst=(uchar *) dest;
 	uchar		vl=val;
 	
 	if (count>=16)
 	 {
 	 	unsigned int 		tempcount;
 	 	double 					*dst_doub,double_stack,doub_vl;
 	 	unsigned short	short_val;
 	 								
 	 	// first try to get to a 4 byte boundary
 	 	if ((int) dst & 3) 
 	 	 {
 	 	 	tempcount = ((int) dst & 3);
 			count -= tempcount;
 			while (tempcount--) *(dst++) = vl;
 	 	 }
 	 	
 	 	dst_doub = (double *) dst;
 	 	tempcount = count>>3;
 	 	count -= tempcount<<3;
 	 	
 	 	// get a 64 bit version of val in doub_vl
 	 	short_val = val | val<<8;
 	 	val = (int) short_val |  ((int) short_val)<<16;
 	 	* (int *) (&double_stack) = val;
 	 	* ((int *) (&double_stack)+1) = val;
 	 	doub_vl = double_stack;
 	 	 	 	
 	 	while (tempcount--) *(dst_doub++) = doub_vl;
 	 	
 	 	dst = (uchar *) dst_doub;
 	 }
 	
 	while (count--) *(dst++) = vl;
 	
 	return(dest);
 }

void *LG_memcpy(void *dest, const void *source, unsigned int count)
 {
 	uchar 	*dst=(uchar *) dest;
 	uchar 	*src=(uchar *) source;

 	if (count>=16)
 	 {
 	 	unsigned int 		tempcount;
 	 	double 					*dst_doub,*src_doub;
 	 								
 	 	// first try to get to a 4 byte boundary
 	 	if ((int) dst & 3) 
 	 	 {
 	 	 	tempcount = ((int) dst & 3);
 			count -= tempcount;
 			while (tempcount--) *(dst++) = *(src++);
 	 	 }

 	 	dst_doub = (double *) dst;
 	 	src_doub = (double *) src;
 	 	tempcount = count>>3;
 	 	count -= tempcount<<3;
	
 	 	while (tempcount--) *(dst_doub++) = *(src_doub++);

 	 	dst = (uchar *) dst_doub;
 	 	src = (uchar *) src_doub;
   }
  
 	while (count--) *(dst++) = *(src++);

 	return(dest); 
 }
 
// checks to make sure it does overlapping memory correctly 
void *LG_memmove(void *dest, const void *source, unsigned int count)
 {
 	uchar 	*dst=(uchar *) dest;
 	uchar 	*src=(uchar *) source;

	// if src is past dest, copy from start
	if ((unsigned int) src + count < (unsigned int) dst)
	 {
	 	if (count>=16)
	 	 {
	 	 	unsigned int 		tempcount;
	 	 	double 					*dst_doub,*src_doub;
	 	 								
	 	 	// first try to get to a 4 byte boundary
	 	 	if ((int) dst & 3) 
	 	 	 {
	 	 	 	tempcount = ((int) dst & 3);
	 			count -= tempcount;
	 			while (tempcount--) *(dst++) = *(src++);
	 	 	 }
	
	 	 	dst_doub = (double *) dst;
	 	 	src_doub = (double *) src;
	 	 	tempcount = count>>3;
	 	 	count -= tempcount<<3;

	 	 	while (tempcount--) *(dst_doub++) = *(src_doub++);
	
	 	 	dst = (uchar *) dst_doub;
	 	 	src = (uchar *) src_doub;
	   }
	  
	 	while (count--) *(dst++) = *(src++);
	 }
	else	// else copy from end backwards
	 {
	 	src += count;
	 	dst += count;
	 	
	 	if (count>=16)
	 	 {
	 	 	unsigned int 		tempcount;
	 	 	double 					*dst_doub,*src_doub;
	 	 								
	 	 	// first try to get to a 4 byte boundary
	 	 	if ((int) dst & 3) 
	 	 	 {
	 	 	 	tempcount = ((int) dst & 3);
	 			count -= tempcount;
	 			while (tempcount--) *(--dst) = *(--src);
	 	 	 }
	
	 	 	dst_doub = (double *) dst;
	 	 	src_doub = (double *) src;
	 	 	tempcount = count>>3;
	 	 	count -= tempcount<<3;
		
	 	 	while (tempcount--) *(--dst_doub) = *(--src_doub);
	
	 	 	dst = (uchar *) dst_doub;
	 	 	src = (uchar *) src_doub;
	   }
	  
	 	while (count--) *(--dst) = *(--src);
	 }

 	return(dest); 
 }
 
 
#else

// 68K optimized versions of standard memory ops
asm void *LG_memset(void *dest, int val, unsigned int count)
 {
 	move.l	4(a7),a0		// get dest
 	move.l	8(a7),d0		// get val
 	move.l	12(a7),d1		// get count
 	 	
 	cmp.l		#8,d1
 	blt			@InLoop
 	
 	// first get dest address aligned 	
 	move.l	a0,d2
 	btst		#0,d2
 	beq.s		@aligned
 	move.b	d0,(a0)+
 	subq.l	#1,d1

@aligned:
	move.b	d0,d2
	asl.w		#8,d0
	move.b	d2,d0
	move.w	d0,d2
	swap		d0
	move.w	d2,d0		// get long of val in d0

@OutLoop:
	cmp.l		#128,d1
	blt.s		@less128
 	sub.l		#128,d1
	jmp			@JumpIn	// do 32 longs
	
@less128:
 	move.l	d1,d2		// get count
 	asr.l		#2,d2		// div 4
	andi.l	#3,d1		// only 0-3 bytes left
	 	
 	lea			@JumpIn,a1
 	add.w		d2,d2
 	neg.w		d2
 	jmp			64(a1,d2.w)
 
@JumpIn: 	
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+	// 10 	

 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+	// 20 	

 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+
 	move.l	d0,(a0)+	// 30 	

 	move.l	d0,(a0)+
 	move.l	d0,(a0)+	// 32
 	
 	cmp.l		#8,d1
 	bge.s		@OutLoop
 	bra.s		@InLoop
 	 	
@Less8:								// do last bytes
	move.b	d0,(a0)+
@InLoop:
	dbra		d1,@Less8

@Done:
	move.l	4(a7),a0		// return address
	rts 	
 }

asm void *LG_memcpy(void *dest, const void *source, unsigned int count)
 {
 	move.l	4(a7),a0		// get dest
 	move.l	8(a7),a1		// get src
 	move.l	12(a7),d1		// get count
 	 	
 	move.l	a2,-(sp)
 	 	
 	cmp.l		#8,d1
 	blt			@InLoop
 	
 	// first get dest address aligned 	
 	move.l	a0,d2
 	btst		#0,d2
 	beq.s		@aligned
 	move.b	(a1)+,(a0)+
 	subq.l	#1,d1

@aligned:
@OutLoop:
	cmp.l		#128,d1
	blt.s		@less128
 	sub.l		#128,d1
	jmp			@JumpIn	// do 32 longs
	
@less128:
 	move.l	d1,d2		// get count
 	asr.l		#2,d2		// div 4
	andi.l	#3,d1		// only 0-3 bytes left
	 	
 	lea			@JumpIn,a2
 	add.w		d2,d2
 	neg.w		d2
 	jmp			64(a2,d2.w)
 
@JumpIn: 	
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+	// 10 	

 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+	// 20 	

 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+	// 30 	

 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+	// 32 	
 	
 	cmp.l		#8,d1
 	bge.s		@OutLoop
 	bra.s		@InLoop
 	 	
@Less8:								// do last bytes
	move.b	(a1)+,(a0)+
@InLoop:
	dbra		d1,@Less8

@Done:
	move.l	(sp)+,a2
	move.l	4(a7),a0		// return address
	rts 	
 }
 
asm void *LG_memmove(void *dest, const void *source, unsigned int count)
 {
 	move.l	4(a7),a0		// get dest
 	move.l	8(a7),a1		// get src
 	move.l	12(a7),d1		// get count
 	 	
 	move.l	a2,-(sp)
 	 	 	 	
 	move.l	a1,a2
 	add.l		d1,a2
 	cmp.l		a2,a0				// if dest < src+count, use reverse
 	blo 		@UseReverse 	 	 	
 	 	
 	cmp.l		#8,d1
 	blt			@InLoop
 	
 	// first get dest address aligned 	
 	move.l	a0,d2
 	btst		#0,d2
 	beq.s		@aligned
 	move.b	(a1)+,(a0)+
 	subq.l	#1,d1

@aligned:
@OutLoop:
	cmp.l		#128,d1
	blt.s		@less128
 	sub.l		#128,d1
	jmp			@JumpIn	// do 32 longs
	
@less128:
 	move.l	d1,d2		// get count
 	asr.l		#2,d2		// div 4
	andi.l	#3,d1		// only 0-3 bytes left
	 	
 	lea			@JumpIn,a2
 	add.w		d2,d2
 	neg.w		d2
 	jmp			64(a2,d2.w)
 
@JumpIn: 	
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+	// 10 	

 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+	// 20 	

 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+	// 30 	

 	move.l	(a1)+,(a0)+
 	move.l	(a1)+,(a0)+	// 32 	
 	
 	cmp.l		#8,d1
 	bge.s		@OutLoop
 	bra.s		@InLoop
 	 	
@Less8:								// do last bytes
	move.b	(a1)+,(a0)+
@InLoop:
	dbra		d1,@Less8
	bra 		@Done
	
@UseReverse:
	add.l		d1,a0		// start at the end of the pointers
	add.l		d1,a1
	
 	cmp.l		#8,d1
 	blt			@RInLoop
 	
 	// first get dest address aligned 	
 	move.l	a0,d2
 	btst		#0,d2
 	beq.s		@Raligned
 	move.b	-(a1),-(a0)
 	subq.l	#1,d1

@Raligned:
@ROutLoop:
	cmp.l		#128,d1
	blt.s		@Rless128
 	sub.l		#128,d1
	jmp			@RJumpIn	// do 32 longs
	
@Rless128:
 	move.l	d1,d2		// get count
 	asr.l		#2,d2		// div 4
	andi.l	#3,d1		// only 0-3 bytes left
	 	
 	lea			@RJumpIn,a2
 	add.w		d2,d2
 	neg.w		d2
 	jmp			64(a2,d2.w)
 
@RJumpIn: 	
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)	// 10 	

 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)	// 20 	

 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)	// 30 	

 	move.l	-(a1),-(a0)
 	move.l	-(a1),-(a0)	// 32 	
 	
 	cmp.l		#8,d1
 	bge.s		@ROutLoop
 	bra.s		@RInLoop
 	 	
@RLess8:								// do last bytes
	move.b	-(a1),-(a0)
@RInLoop:
	dbra		d1,@RLess8

@Done:
	move.l	(sp)+,a2
	move.l	4(a7),a0		// return address
	rts 	
 }
 
#endif 
