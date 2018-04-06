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
//                                                     
// $Source: r:/prj/lib/src/2d/RCS/fl8dbl.asm $
// $Revision: 1.3 $
// $Author: kevin $
// $Date: 1994/09/08 00:00:18 $
//
// Bitmap doubling primitives.
//

#include "grs.h"
#include "cnvdat.h"
#include "flat8.h"
#include "blndat.h"

#if !(defined(powerc) || defined(__powerc))
asm void Handle_Smooth_H_Asm(int	endh, int endv, long srcAdd, long dstAdd,
															uchar *src, uchar *dst, uchar *local_grd_half_blend);
asm void Handle_Smooth_HV_Asm(int tempH, int tempW, int temp,
															uchar	*shvd_read_row1, uchar *shvd_write, 
															uchar *shvd_read_row2, uchar *shvd_read_blend,
															uchar *dstPtr);
#endif

// ------------------------------------------------------------------------
// PowerPC routines
// ------------------------------------------------------------------------
// ========================================================================
void flat8_flat8_h_double_ubitmap(grs_bitmap *bm)
 {
DebugStr("\pcall mark"); 	
/* 	int		h,v,endh,endv;
 	uchar *src=bm->bits, *dst=grd_bm.bits;
 	long	srcAdd,dstAdd;
 	uchar	temp;

 	srcAdd = bm->row-bm->w;
 	dstAdd = grd_bm.row - (bm->w<<1);
 	endh = bm->w;
 	endv = bm->h;
 	
 	for (v=0; v<endv; v++)
 	 {
 	 	for (h=0; h<endh; h++)
 	 	 {
 	 	 	temp = *(src++);
 	 	 	*(dst++) = temp;
 	 	 	*(dst++) = temp;
 	 	 }
 	 	 
 	 	src+=srcAdd;
 	 	dst+=dstAdd; 
 	 }*/
 }


// ========================================================================
void flat8_flat8_smooth_h_double_ubitmap(grs_bitmap *srcb, grs_bitmap *dstb)
 {
 	int			h,v,endh,endv;
 	uchar	 *src=srcb->bits, *dst=dstb->bits;
 	long		srcAdd,dstAdd;
	ushort	curpix,tempshort;
	uchar 	*local_grd_half_blend;
	
	local_grd_half_blend = grd_half_blend;
 	if (!local_grd_half_blend) return;
 	
 	srcAdd = (srcb->row-srcb->w)-1;
 	dstAdd = dstb->row - (srcb->w<<1);
 	endh = srcb->w-1;
 	endv = srcb->h;
 	
#if defined(powerc) || defined(__powerc)
 	for (v=0; v<endv; v++)
 	 {
 	 	curpix = * (short *) src;
 	 	src+=2;
 	 	for (h=0; h<endh; h++)
 	 	 {
 	 	 	tempshort = curpix & 0xff00;
 	 	 	tempshort |= local_grd_half_blend[curpix];
 	 	 	* (ushort *) dst = tempshort;
 	 	 	dst += 2;
 	 	 	curpix = (curpix<<8) | *(src++);
 	 	 }
 	 	
 	 	// double last pixel
 	 	curpix>>=8;
 	 	*(dst++) = curpix;
 	 	*(dst++) = curpix;
		
 	 	src+=srcAdd;
 	 	dst+=dstAdd; 
 	 }
#else
	Handle_Smooth_H_Asm(endh,endv,srcAdd,dstAdd,src,dst,local_grd_half_blend);
#endif
 }

#if !(defined(powerc) || defined(__powerc))
asm void Handle_Smooth_H_Asm(int	endh, int endv, long srcAdd, long dstAdd,
															uchar *src, uchar *dst, uchar *local_grd_half_blend)
 {
 	movem.l	d2-d7/a2-a4,-(sp)
 	
 	movem.l	40(sp),d2-d5/a0-a2	// load up parms
 	
 	subq.w	#1,d2
 	beq.s		@Done
 	subq.w	#1,d3		// for dbra
 	beq.s		@Done
	move.w	d2,d7
 	moveq		#0,d0
 	
@OLoop:
	move.w	(a0)+,d0							// curpix = * (short *) src;  src+=2;
	move.w	d7,d2		// restore h
@ILoop: 	
	move.w	d0,d1
	andi.w	#0xff00,d1		// curpix & 0xff00
	move.b	(a2,d0.l),d1	// |= local_grd_half_blend[curpix];
	move.w	d1,(a1)+
	lsl.w		#8,d0
	move.b	(a0)+,d0
	dbra		d2,@ILoop		// h loop

	lsr.w		#8,d0
	move.b	d0,(a1)+
	move.b	d0,(a1)+
	
	add.l		d4,a0				// src+=srcAdd
	add.l		d5,a1				// dst+=dstAdd

 	dbra		d3,@OLoop		// v loop
 	
@Done:
 	movem.l	(sp)+,d2-d7/a2-a4
 	rts
 }
#endif


// ========================================================================
// src = eax, dest = edx
void flat8_flat8_smooth_hv_double_ubitmap(grs_bitmap *src, grs_bitmap *dst)
 {
 	int	 		tempH, tempW, temp, savetemp;
 	uchar		*srcPtr,*dstPtr;
 	uchar		*shvd_read_row1, *shvd_write, *shvd_read_row2, *shvd_read_blend;
 	ushort	tempc;
 	
 	
 	dst->row <<= 1;
 	flat8_flat8_smooth_h_double_ubitmap(src,dst);
	
	dst->row = tempW = dst->row >> 1;
	dstPtr = dst->bits;
	
	tempH = src->h-1;
	temp =  src->w << 1;
	dstPtr += temp;
	temp = -temp;
	
	shvd_read_row1 = dstPtr;
	dstPtr += tempW;
	shvd_write = dstPtr - 1;
	dstPtr += tempW;
	shvd_read_row2 = dstPtr;
	shvd_read_blend = grd_half_blend;
	savetemp = temp;
	
#if defined(powerc) || defined(__powerc)
	do
	 {
		do
		 {
		 	tempc = shvd_read_row1[temp];
		 	tempc |= ((ushort) shvd_read_row2[temp]) << 8;
		 	temp++;
		 	
		 	shvd_write[temp] = shvd_read_blend[tempc];
		 }
		while (temp!=0);
		
		if (--tempH==0) break;
		
		shvd_read_row1 = dstPtr;
		dstPtr += tempW;
		shvd_write = dstPtr - 1;
		dstPtr += tempW;
		shvd_read_row2 = dstPtr;
		temp = savetemp;
	 }
	while (true);
	
	// do last row
	srcPtr = dstPtr + savetemp;
	dstPtr += tempW + savetemp;
	savetemp = -savetemp;
	
	for (;savetemp>0; savetemp--)
	  *(dstPtr++) = *(srcPtr++);
#else	  
	Handle_Smooth_HV_Asm(tempH,tempW,temp,
											 shvd_read_row1, shvd_write, 
											 shvd_read_row2, shvd_read_blend,
											 dstPtr);
#endif	
 }
 
#if !(defined(powerc) || defined(__powerc))
asm void Handle_Smooth_HV_Asm(int tempH, int tempW, int temp,
															uchar	*shvd_read_row1, uchar *shvd_write, 
															uchar *shvd_read_row2, uchar *shvd_read_blend,
															uchar *dstPtr)
 {
 	movem.l	d2-d7/a2-a4,-(sp)
 	movem.l	40(sp),d2-d4/a0-a4	// load up parms
 	moveq		#0,d0
 	move.l	d4,d7
 	
@ILoop: 	
 	move.b	(a2,d4.l),d0
 	lsl.w		#8,d0 
 	move.b	(a0,d4.l),d0
 	move.b	(a3,d0.l),1(a1,d4.l)
 	addq.l	#1,d4
 	bne.s		@ILoop

	subq.l	#1,d2
	beq.s		@Out

	move.l	a4,a0
	add.w		d3,a4
	move.l	a4,a1
	subq.l	#1,a1
	add.w		d3,a4
	move.l	a4,a2
	move.l	d7,d4
	bra.s		@ILoop
	
// do last row
@Out:
	move.l	a4,a0
	add.l		d7,a0		// srcPtr = dstPtr + savetemp;
	add.l		d3,a4
	add.l		d7,a4		// dstPtr += tempW + savetemp;
	
	neg.l		d7			// savetemp = -savetemp;
	beq.s		@Done
	
@LastLoop:
	move.b	(a0)+,(a4)+
	dbra		d7,@LastLoop

@Done:
 	movem.l	(sp)+,d2-d7/a2-a4
 	rts
 }
#endif
