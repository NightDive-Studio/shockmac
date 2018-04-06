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
 * $Source: n:/project/lib/src/2d/RCS/fl8fl8.c $
 * $Revision: 1.5 $
 * $Author: kaboom $
 * $Date: 1993/10/19 09:50:21 $
 * 
 * Routines for drawing flat 8 bitmaps into a flat 8 canvas.
 *
 * This file is part of the 2d library.
 *
 * $Log: fl8fl8.c $
 * Revision 1.5  1993/10/19  09:50:21  kaboom
 * Replaced #include <grd.h" with new headers split from grd.h.
 * 
 * Revision 1.4  1993/10/08  01:15:13  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.3  1993/07/12  23:30:56  kaboom
 * Inline memmove() uses movs.
 * 
 * Revision 1.2  1993/03/29  18:22:11  kaboom
 * Changed to inline version of memmove.
 * 
 * Revision 1.1  1993/02/16  14:14:16  kaboom
 * Initial revision
 * 
 ********************************************************************
 * Log from old flat8.c:
 * Revision 1.7  1992/12/14  18:08:40  kaboom
 * Added handing for transparency in the flat8_flat8_ubitmap()
 * routine.
 */

#include <string.h>
#include "bitmap.h"
#include "cnvdat.h"
#include "flat8.h"
#include "lg.h"

#if (defined(powerc) || defined(__powerc))	
void flat8_flat8_ubitmap (grs_bitmap *bm, short x, short y)
{
	uchar 	*m_src;
	uchar 	*m_dst;
	int 		w = bm->w;
	int 		h = bm->h;
	int 		i;
	int			brow,grow;
	
	
	brow = bm->row;
	grow = grd_bm.row;
	
	m_src = bm->bits;
	m_dst = grd_bm.bits + grow*y + x;
	
	if (bm->flags & BMF_TRANS)
		while (h--) {
		 for (i=0; i<w; i++)
		    if (m_src[i]!=0) m_dst[i]=m_src[i];
		 m_src += brow;
		 m_dst += grow;
		}
	else
		while (h--) {
		 LG_memmove (m_dst, m_src, w);
		 
		 m_src += brow;
		 m_dst += grow;
		}
}

#else
// 68k version
asm void flat8_flat8_ubitmap (grs_bitmap *bm, short x, short y)
 {
 	move.l	4(sp),a0				// get bm
 	move.w	8(sp),d1				// get x
 	move.w	10(sp),d2				// get y
 	
 	movem.l	d3-d7/a2,-(sp)
 	
 	move.l	(a0),a1					// src
 	move.w	12(a0),d3				// src rowbytes
 	move.w	8(a0),d5				// width
 	move.w	10(a0),d6				// height
 	
 	move.l	a0,d7
 	move.l	grd_canvas,a0
 	move.l	(a0),a2					// dest
 	move.w	12(a0),d4				// dest rowbytes
 	
 	add.w		d1,a2						// dest + x
 	mulu.w	d4,d2
 	add.l		d2,a2						// dest + x + grow*y
 	
 	move.l	d7,a0
 	move.w	6(a0),d0				// get flags
 	andi.w	#BMF_TRANS,d0		// trans?
 	bne 		@Transparent

@Solid:
	sub.w		d5,d3
	sub.w		d5,d4						// rowbytes - width
	subq.w	#1,d6						// for dbra

	// make sure address is aligned
	move.l	a2,d1
 	andi.w	#1,d1
 	beq.s		@aligned
 	subq.w	#1,d5
@aligned:
	lea			@SCopyLoop+320,a0
	move.l	d5,d7
	andi.l	#0x0003,d5			// catch any remaining bytes (less than 4) in d5
	move.l	d5,d0
	asr.w		#1,d7
	andi.w	#0xfffE,d7			// clear low bit (4 bytes per move, 2 bytes per move instruction)
	sub.w		d7,a0						// address to jump to
	
@SLoop:
	tst.w		d1
	beq.s		@alignOK
	move.b	(a1)+,(a2)+			// move a byte to align it
@alignOK:
	jmp			(a0)

@SCopyLoop:
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+		
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+		
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+		
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+		// 40

	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+		
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+		
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+		
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+		// 80

	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+		
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+		
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+		
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+		// 120
	
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+		
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+		
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+		
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+
	move.l	(a1)+,(a2)+		// 160

	bra.s		@SInLoop	
@SLLoop:
	move.b	(a1)+,(a2)+
@SInLoop:
	dbra		d5,@SLLoop
	
	add.w		d3,a1				// add rowbytes
	add.w		d4,a2
	move.w	d0,d5				// get left over
	dbra		d6,@SLoop

 	movem.l	(sp)+,d3-d7/a2
 	rts

@Transparent:
	sub.w		d5,d3
	sub.w		d5,d4						// rowbytes - width
	subq.w	#1,d6						// for dbra
	subq.w	#1,d5
	move.w	d5,d7

@TLoop:
	move.b	(a1)+,d0
	beq.s		@skip
@TCopy:
	move.b	d0,(a2)+
	dbra		d5,@TLoop
	bra.s		@out
		
@TSkipLoop:
	move.b	(a1)+,d0
	bne.s		@TCopy
	
@skip:
	addq.w	#1,a2
	dbra		d5,@TSkipLoop

@out:
	add.w		d3,a1				// add rowbytes
	add.w		d4,a2
	move.w	d7,d5				// get count again
	dbra		d6,@TLoop
 	
 	movem.l	(sp)+,d3-d7/a2
 	rts
 }
#endif
