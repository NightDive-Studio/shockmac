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
/*
 * $Source: n:/project/lib/src/2d/RCS/fl8clear.c $
 * $Revision: 1.3 $
 * $Author: kaboom $
 * $Date: 1993/10/19 09:50:18 $
 * 
 * Routines for clearing a flat 8 canvas.
 *
 * This file is part of the 2d library.
 *
 * $Log: fl8clear.c $
 * Revision 1.3  1993/10/19  09:50:18  kaboom
 * Replaced #include "grd.h" with new headers split from grd.h.
 * 
 * Revision 1.2  1993/10/08  01:15:08  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.1  1993/02/16  14:14:00  kaboom
 * Initial revision
 */

#include <string.h>
#include "cnvdat.h"
#include "flat8.h"
#include "lg.h"

/* clear a flat8 canvas. */
#if (defined(powerc) || defined(__powerc))	
void flat8_clear (long color)
{
   uchar 	*p;
   int	 	h;
	 int		w;
	 int	  row;
   ushort	short_val;
	 double	double_stack,doub_vl;
	 uint 	firstbytes,middoubles,lastbytes,fb,md,lb;
	 uchar 	*dst;
   double *dst_doub;
   uint		temp;
	 
	 color &= 0x00ff;
   p = grd_bm.bits;
   h = grd_bm.h;
	 w = grd_bm.w;
	 row = grd_bm.row;
	 if (w>=16)	// only do doubles if at least two of them (16 bytes)
	  {
	 	 	// get a 64 bit version of color in doub_vl
	 	 	short_val = (uchar) color | color<<8;
	 	 	color = (int) short_val |  ((int) short_val)<<16;
	 	 	* (int *) (&double_stack) = color;
	 	 	* ((int *) (&double_stack)+1) = color;
	 	 	doub_vl = double_stack;

			lastbytes = w;
		 	if (firstbytes = (int) p & 3) // check for boundary problems
			 	lastbytes -= firstbytes;

		 	middoubles = lastbytes>>3;
		 	lastbytes -= middoubles<<3;
	  }
	 else
	 	{lastbytes = w; middoubles = 0;}
	 
	 fb = firstbytes,md = middoubles,lb = lastbytes;
   while (h--)
   {
// MLA - inlined this code
//		LG_memset (p, color, w);
     {
		  firstbytes = fb,middoubles = md,lastbytes = lb;
		 	dst = p;
		 	
		 	if (middoubles)
		 	 {
		 	 	// first get to a 4 byte boundary
		 		while (firstbytes--) *(dst++) = color;
		 	 	dst_doub = (double *) dst;
		 	 	
		 	 	// now do doubles		 	 	 	 	
		 	 	while (middoubles--) *(dst_doub++) = doub_vl;	
		 	 	dst = (uchar *) dst_doub;
		 	 }
		 	
		 	// do remaining bytes
		 	while (lastbytes--) *(dst++) = color;
     }
     
    p += row;
   }
}
#else
// 68k version
asm void flat8_clear (long color)
{
	move.l	4(a7),d0		// get color
	movem.l	d3-d6,-(sp)
	move.b	d0,d1
	ror.w		#8,d0
	move.b	d1,d0
	move.w	d0,d1
	swap		d0
	move.w	d1,d0				// get long of color in d0
	
 	move.l	grd_canvas,a0
 	move.l	(a0),a1			// dest
 	move.w	8(a0),d1		// width
 	move.w	10(a0),d2		// height
 	move.w	12(a0),d3		// rowbytes
 	
 	sub.w		d1,d3				// rowbytes - width
 	subq.w	#1,d2				// for dbra

	move.l	a1,d6
	andi.w	#1,d6
	beq.s		@aligned
	subq.w	#1,d1
@aligned:
	lea			@CCopyLoop+320,a0
	move.l	d1,d4
	andi.l	#0x0003,d1			// catch any remaining bytes (less than 4) in d5
	move.l	d1,d5
	asr.w		#1,d4
	andi.w	#0xfffE,d4			// clear low bit (4 bytes per move, 2 bytes per move instruction)
	sub.w		d4,a0						// address to jump to
 			
@CLoop: 	
	tst.w		d6
	beq.s		@alignOK
	move.b	d0,(a1)+
@alignOK:
	jmp			(a0)

@CCopyLoop:
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+		
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+			// 80

	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+		
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+	
	move.l	d0,(a1)+			// 160
 	
	bra.s		@CInLoop	
@CLLoop:
	move.b	d0,(a1)+
@CInLoop:
	dbra		d1,@CLLoop
	
	move.w	d5,d1				// get width extra back
 	add.w		d3,a1				// + rowbytes
 	dbra		d2,@CLoop
 	 	
 	movem.l	(sp)+,d3-d6
 	rts
}
#endif

