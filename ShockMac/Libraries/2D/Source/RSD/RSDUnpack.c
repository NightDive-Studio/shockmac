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
// Rsd unpacking into a bitmap where row=width.
//
// 68K and PowerPC versions
//

#include "grs.h"
#include "rsdunpck.h"

// some handy 68000 assembly defines
#define blo	bcs		// branch low (unsigned <)
#define bhs	bcc		// branch high or same (unsigned >=)


#if defined(powerc) || defined(__powerc)
//----------------------------------------------------------------------------
// PowerPC version
#define kMinLongLoop 4			// minimum # of bytes to need before using long store loop

uchar *gr_rsd8_unpack(uchar *src, uchar *dest)
 {
 	uchar		code,val;
 	short		count,count2;
 	ushort	longcode;
 	ulong		longval, *longdest, *longsrc;
 
 	do
 	 {
 		code = *(src++);
 		if (!code) // run of bytes
 		 {	
 		 	count = *(src++); // get count
 		 	val = *(src++); 	// get val
 		 	
 		 	if (count>=kMinLongLoop)		// if at least kMinLongLoop bytes, do long word stuff
 		 	 {
 		 	 	longval = val + (((ulong) val)<<8);
 		 	 	longval += longval<<16;
 		 	 	count2 = count>>2;
 		 	 	count &= 3;
 		 	 	longdest = (ulong *) dest;
 		 	 	
 		 	 	while (count2--)
 		 	 	 	*(longdest++) = longval;
 		 	 	dest = (uchar *) longdest;	
 		 	 }
 		 	
 		 	// do rest of bytes
 		 	while (count--)
 		 		*(dest++) = val;
 		 }
 		else if (code<0x80) // dump (copy) bytes
 		 {
 		 	count = code;
 		 	if (code>=kMinLongLoop)		// if at least kMinLongLoop bytes, do long word stuff
 		 	 {
 		 	 	count2 = count>>2;
 		 	 	count &= 3;
 		 	 	longdest = (ulong *) dest;
 		 	 	longsrc = (ulong *) src;
 		 	 	
 		 	 	while (count2--)
 		 	 	 	*(longdest++) = *(longsrc++);
 		 	 	dest = (uchar *) longdest;	
 		 	 	src = (uchar *) longsrc;	
 		 	 }
 		 	
 		 	// do rest of bytes
 		 	while (count--)
 		 		*(dest++) = *(src++);
 		 }
 		else if (code>0x80) // skip (zero) bytes)
 		 {
 		 	count = code & 0x007f;	// clear high byte
 		 	val = longval = 0L;
 		 	
 		 	if (count>=kMinLongLoop)		// if at least kMinLongLoop bytes, do long word stuff
 		 	 {
 		 	 	count2 = count>>2;
 		 	 	count &= 3;
 		 	 	longdest = (ulong *) dest;
 		 	 	
 		 	 	while (count2--)
 		 	 	 	*(longdest++) = longval;
 		 	 	dest = (uchar *) longdest;	
 		 	 }
 		 	
 		 	// do rest of bytes
 		 	while (count--)
 		 		*(dest++) = val;
 		 }
 		else	// long opcode 
 		 {
 		 	longcode = * (ushort *) src; 		 	
 		 	src += 2L;
 		 	
 		 	if (!longcode) break;		// done?
			else if (longcode<0x8000)	// skip (zero)
			 {
	 		 	count = longcode;
	 		 	val = longval = 0L;
	 		 	
	 		 	if (count>=kMinLongLoop)		// if at least kMinLongLoop bytes, do long word stuff
	 		 	 {
	 		 	 	count2 = count>>2;
	 		 	 	count &= 3;
	 		 	 	longdest = (ulong *) dest;
	 		 	 	
	 		 	 	while (count2--)
	 		 	 	 	*(longdest++) = longval;
	 		 	 	dest = (uchar *) longdest;	
	 		 	 }
	 		 	
	 		 	// do rest of bytes
	 		 	while (count--)
	 		 		*(dest++) = val;
			 }
			else if (longcode<0xC000)	// dump (copy)
			 {
			 	count = longcode & 0x7fff;	// clear high bit
			 	
	 		 	if (count>=kMinLongLoop)		// if at least kMinLongLoop bytes, do long word stuff
	 		 	 {
	 		 	 	count2 = count>>2;
	 		 	 	count &= 3;
	 		 	 	longdest = (ulong *) dest;
	 		 	 	longsrc = (ulong *) src;
	 		 	 	
	 		 	 	while (count2--)
	 		 	 	 	*(longdest++) = *(longsrc++);
	 		 	 	dest = (uchar *) longdest;	
	 		 	 	src = (uchar *) longsrc;	
	 		 	 }
	 		 	
	 		 	// do rest of bytes
	 		 	while (count--)
	 		 		*(dest++) = *(src++);
			 }
			else	// run of bytes
			 {
	 		 	count = longcode & 0x3fff;
	 		 	val = *(src++); 	// get val
	 		 	
	 		 	if (count>=kMinLongLoop)		// if at least kMinLongLoop bytes, do long word stuff
	 		 	 {
	 		 	 	longval = val + (((ulong) val)<<8);
	 		 	 	longval += longval<<16;
	 		 	 	count2 = count>>2;
	 		 	 	count &= 3;
	 		 	 	longdest = (ulong *) dest;
	 		 	 	
	 		 	 	while (count2--)
	 		 	 	 	*(longdest++) = longval;
	 		 	 	dest = (uchar *) longdest;	
	 		 	 }
	 		 	
	 		 	// do rest of bytes
	 		 	while (count--)
	 		 		*(dest++) = val;
			 } 		 	
 		 }
 	 } 
 	while (true);
 	
 	return(dest);
 }
 
#else // !(defined(powerc) || defined(__powerc))
//----------------------------------------------------------------------------
// 68K version
asm uchar *gr_rsd8_unpack(uchar* src, uchar *dst)
 {
 	move.l	4(A7),a0		// get src
 	move.l	8(A7),a1		// get dest
 	
@unpackloop:
 	moveq		#0,d0			
	move.b	(a0)+,d0		// get first code
	beq.s		@run				// zero = run of bytes
	cmp.b		#0x80,d0
	beq.s		@long_op		// 0x80 = long opcode
 	bhi.s		@skip				// >0x80 = skip(zero) bytes
										
@dump:								// <0x80 = dump (copy) bytes
	cmp.w		#16,d0
	blo.s		@DumpLast

	move.w	d0,d2
	andi.w	#0x000f,d2	// get remaining bytes in d2
	lsr.w		#4,d0				// get # of 16 byte sets in d0
	subq.w	#1,d0				// -1 for dbra
	
@dumplp:
	move.l	(a0)+,(a1)+
	move.l	(a0)+,(a1)+
	move.l	(a0)+,(a1)+	
	move.l	(a0)+,(a1)+	// copy 16 pixels
	dbra		d0,@dumplp
	
	move.w	d2,d0				// get remaining # of bytes
	subq.w	#1,d0				// -1 for dbra
	bmi.s		@DumpEnd
	
@DumpLoop:						// copy whatever bytes are left
	move.b	(a0)+,(a1)+
@DumpLast:
	dbra		d0,@DumpLoop
@DumpEnd:
	bra.s		@unpackloop

@run:									// store out a run of the same byte
	move.b	(a0)+,d0		// get count
@run2:
	move.b	(a0)+,d1		// get value

	cmp.w		#16,d0
	blo.s		@RunLast		// if less than 16 bytes, just set normally
	
	move.b	d1,d2
	asl.w		#8,d1
	move.b	d2,d1
	move.w	d1,d2
	swap		d1
	move.w	d2,d1				// get 32 bits of value in d1
	
@prerunlp:
	move.w	d0,d2
	andi.w	#0x000f,d2	// get remaining bytes in d2
	lsr.w		#4,d0				// get # of 16 byte sets in d0
	subq.w	#1,d0				// -1 for dbra
	
@runlp:
	move.l	d1,(a1)+
	move.l	d1,(a1)+
	move.l	d1,(a1)+
	move.l	d1,(a1)+		// set 16 pixels
	dbra		d0,@runlp

	move.w	d2,d0				// get remaining # of bytes
	subq.w	#1,d0				// -1 for dbra
	bmi.s		@RunEnd			// any remaining pixels?
	
@RunLoop:							// set whatever bytes are left
	move.b	d1,(a1)+
@RunLast:
	dbra		d0,@RunLoop
@RunEnd:
	bra.s		@unpackloop
	
@skip:								// set sequence to zeros (set zero value and jump into run loop above
	moveq		#0,d1
	andi.w	#0x007f,d0	// clear high bit
	
	cmp.w		#16,d0
	blo.s		@RunLast		// if less than 16 bytes, just set normally
	bra.s		@prerunlp		// otherwise go to main loop

// code to handle long opcodes
@long_op:
	move.w	(a0)+,d0		// get long opcode
	beq.s		@rsd_done
	
	cmp.w		#0x8000,d0
	bhs.s		@long_run_or_dump

// long skip
	moveq		#0,d1				// set zero value
	cmp.w		#16,d0
	blo.s		@RunLast		// if less than 16 bytes, just set normally
	bra.s		@prerunlp		// otherwise go to main loop

@long_run_or_dump:
	cmp.w		#0xC000,d0
	bhs.s		@long_run
	
// long dump
	andi.w	#0x7fff,d0		// clear high bit
	bra 		@dump

@long_run:	
	andi.w	#0x3fff,d0		// clear high bits
	bra.s		@run2

@rsd_done:
	move.l	a1,a0					// return final destination pointer
 	rts
 }

#endif
