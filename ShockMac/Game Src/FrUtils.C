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
 * FrUtils.c
 * 
 *  MLA - 4/14/95
 *
 *  Contiains Mac specific utils for the renderer (fast draw slot view, full screen, etc.)
 *
 */
 
 #include "FrUtils.h"
 #include "Shock.h"

// ------------------
//  GLOBALS
// ------------------ 
Handle			gDoubleSizeOffHdl = NULL;
grs_canvas		gDoubleSizeOffCanvas;

//---------------------------------------------------------------------
//  Allocate the intermediate offscreen buffer for low-res mode in Shock.
//---------------------------------------------------------------------
int AllocDoubleBuffer(int w, int h)
{
	Size		dummy;

	FreeDoubleBuffer();													// If one's there, free it first.
	
	if (h == 259)															// Major Hack!!!  In slot view, the double
		h++;																		// buffer needs to be 260 (even number).
	
	MaxMem(&dummy);													// Compact heap before big alloc.

	gDoubleSizeOffHdl = NewHandle(w * h);						// Allocate new buffer.
	if (gDoubleSizeOffHdl)													// If successful, make a canvas for it.
	{
		HLockHi(gDoubleSizeOffHdl);
		gr_init_canvas(&gDoubleSizeOffCanvas, (uchar *)*gDoubleSizeOffHdl, BMT_FLAT8, w, h);
		return 1;
	}
	else
		return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void FreeDoubleBuffer(void)
{
	if (gDoubleSizeOffHdl)											// If there's a buffer,
	{
		HUnlock(gDoubleSizeOffHdl);
		DisposeHandle(gDoubleSizeOffHdl);					// free it.
		gDoubleSizeOffHdl = NULL;
	}
}


 // hard coded to copy from 56,57 to 56+536,57+259 to the screen 
 #define kFastSlotWide 536
 #define kFastSlotHigh 259
 #define kFastSlotLeft 56
 #define kFastSlotTop 57
 
  #define kFastSlotWide_Half 268
 #define kFastSlotHigh_Half 129

 #define LoadStoreTwoDoub(a,b) doub1 = src[a]; doub2 = src[b]; dest[a] = doub1; dest[b] = doub2;	

// copy the slot view from offscreen to on
#if (defined(powerc) || defined(__powerc))	
void Fast_Slot_Copy(grs_bitmap *bm)
   {
   	double 	*src,*dest,doub1,doub2;
   	int		rows = kFastSlotHigh;
   	int		src_rowb,dest_rowb;
   	
   	src = (double *) bm->bits;
   	src_rowb = bm->row>>3;
   	
   	dest = (double *) (gScreenAddress + (kFastSlotTop*gScreenRowbytes) + kFastSlotLeft);
   	dest_rowb = gScreenRowbytes>>3;
   	
   	while (rows--)
   	  {
		dest[0] = src[0];
		LoadStoreTwoDoub(1,2);
		LoadStoreTwoDoub(3,4);
		LoadStoreTwoDoub(5,6);
		LoadStoreTwoDoub(7,8);
		LoadStoreTwoDoub(9,10);
		LoadStoreTwoDoub(11,12);
		LoadStoreTwoDoub(13,14);
		LoadStoreTwoDoub(15,16);
		LoadStoreTwoDoub(17,18);
		LoadStoreTwoDoub(19,20);
		LoadStoreTwoDoub(21,22);
		LoadStoreTwoDoub(23,24);
		LoadStoreTwoDoub(25,26);
		LoadStoreTwoDoub(27,28);
		LoadStoreTwoDoub(29,30);
		LoadStoreTwoDoub(31,32);
		LoadStoreTwoDoub(33,34);
		LoadStoreTwoDoub(35,36);
		LoadStoreTwoDoub(37,38);
		LoadStoreTwoDoub(39,40);
		LoadStoreTwoDoub(41,42);
		LoadStoreTwoDoub(43,44);
		LoadStoreTwoDoub(45,46);
		LoadStoreTwoDoub(47,48);
		LoadStoreTwoDoub(49,50);
		LoadStoreTwoDoub(51,52);
		LoadStoreTwoDoub(53,54);
		LoadStoreTwoDoub(55,56);
		LoadStoreTwoDoub(57,58);
		LoadStoreTwoDoub(59,60);
		LoadStoreTwoDoub(61,62);
		LoadStoreTwoDoub(63,64);
		LoadStoreTwoDoub(65,66);
			 
   	  	dest += dest_rowb;
   	  	src += src_rowb;
   	  }
  }
#else
asm void Fast_Slot_Copy(grs_bitmap *bm)
 {
 	move.l		4(sp),a0
 	movem.l		d3-d4,-(sp)
 	move.l		(a0),d4						// src
 	move.w		8(a0),d1					// width
 	sub.w		#kFastSlotWide,d1
 	move.w		#kFastSlotHigh-1,d2	// height
 	move.l		d4,a0
 	
 	move.l		gScreenAddress,a1		// dest
 	move.l		gScreenRowbytes,d3	// screen width
 	move.l		d3,d0
 	sub.w		#kFastSlotWide,d3
 	move.w		#kFastSlotTop,d4
 	mulu.w		d4,d0
 	add.l			d0,a1
 	add.w			#kFastSlotLeft,a1		// final dest
 
@Loop: 	
 	moveq		#1,d4
 	
@ILoop:
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+		// 10
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+		// 20
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+		// 30
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+		// 40
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+		// 50
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+		// 60
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+		// 67
 	dbra			d4,@ILoop
 	
 	add.w			d1,a0
 	add.w			d3,a1
 	dbra			d2,@Loop
 	
 	movem.l		(sp)+,d3-d4
 	rts
 }
#endif


 // copy the full screen view from offscreen to on
 // hard coded to copy from 0,0 to 640,480 to the screen
 #if (defined(powerc) || defined(__powerc))	
void Fast_FullScreen_Copy(grs_bitmap *bm)
   {
   	double 	*src,*dest,doub1,doub2;
   	int		rows = 480;
   	int		src_rowb,dest_rowb;

   	src = (double *) bm->bits;
   	src_rowb = bm->row>>3;
   	
   	dest = (double *) gScreenAddress;
   	dest_rowb = gScreenRowbytes>>3;
   	
   	while (rows--)
   	  {
		LoadStoreTwoDoub(79,78);
		LoadStoreTwoDoub(77,76);
		LoadStoreTwoDoub(75,74);
		LoadStoreTwoDoub(73,72);
		LoadStoreTwoDoub(71,70);
		LoadStoreTwoDoub(69,68);
		LoadStoreTwoDoub(67,66);
		LoadStoreTwoDoub(65,64);
		LoadStoreTwoDoub(63,62);
		LoadStoreTwoDoub(61,60);
		LoadStoreTwoDoub(59,58);
		LoadStoreTwoDoub(57,56);
		LoadStoreTwoDoub(55,54);
		LoadStoreTwoDoub(53,52);
		LoadStoreTwoDoub(51,50);
		LoadStoreTwoDoub(49,48);
		LoadStoreTwoDoub(47,46);
		LoadStoreTwoDoub(45,44);
		LoadStoreTwoDoub(43,42);
		LoadStoreTwoDoub(41,40);
		LoadStoreTwoDoub(39,38);
		LoadStoreTwoDoub(37,36);
		LoadStoreTwoDoub(35,34);
		LoadStoreTwoDoub(33,32);
		LoadStoreTwoDoub(31,30);
		LoadStoreTwoDoub(29,28);
		LoadStoreTwoDoub(27,26);
		LoadStoreTwoDoub(25,24);
		LoadStoreTwoDoub(23,22);
		LoadStoreTwoDoub(21,20);
		LoadStoreTwoDoub(19,18);
		LoadStoreTwoDoub(17,16);
		LoadStoreTwoDoub(15,14);
		LoadStoreTwoDoub(13,12);
		LoadStoreTwoDoub(11,10);
		LoadStoreTwoDoub(9,8);
		LoadStoreTwoDoub(7,6);
		LoadStoreTwoDoub(5,4);
		LoadStoreTwoDoub(3,2);
		LoadStoreTwoDoub(1,0);
			 
   	  	dest += dest_rowb;
   	  	src += src_rowb;
   	  }
   }
#else   
asm void Fast_FullScreen_Copy(grs_bitmap *bm)
 {
 	move.l		4(sp),a0
 	movem.l		d3-d4,-(sp)
 	move.l		(a0),d4						// src
 	move.w		8(a0),d1					// width
 	sub.w		#640,d1
 	move.w		#479,d2	// height
 	move.l		d4,a0
 	
 	move.l		gScreenAddress,a1		// dest
 	move.l		gScreenRowbytes,d3	// screen width
 	sub.w		#640,d3
 
@Loop: 	
 	moveq		#3,d4
 	
@ILoop:
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+		// 10
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+		// 20
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+		// 30
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+
 	move.l		(a0)+,(a1)+		// 40
 	dbra			d4,@ILoop
 	
 	add.w			d1,a0
 	add.w			d3,a1
 	dbra			d2,@Loop
 	
 	movem.l		(sp)+,d3-d4
 	rts
 }
#endif

//=================================================================
// Doubling routines
extern Boolean SkipLines;

// copy the slot view from offscreen to on, doubling it
#if (defined(powerc) || defined(__powerc))	
extern "C" 
 {
 	extern void BlitLargeAlign(uchar *draw_buffer, int dstRowBytes, void *dstPtr, long w, long h, long modulus);
 	extern void BlitLargeAlignSkip(uchar *draw_buffer, int dstRowBytes, void *dstPtr, long w, long h, long modulus);
 }
 
void Fast_Slot_Double(grs_bitmap *bm, long w, long h)
 {
 	if (!SkipLines)
 		BlitLargeAlign(bm->bits, gScreenRowbytes, gScreenAddress + (kFastSlotTop*gScreenRowbytes) + kFastSlotLeft,w,h,bm->row);
 	else
 		BlitLargeAlignSkip(bm->bits, gScreenRowbytes, gScreenAddress + (kFastSlotTop*gScreenRowbytes) + kFastSlotLeft,w,h,bm->row);
 }
 
void FastSlotDouble2Canvas(grs_bitmap *bm, grs_canvas *destCanvas, long w, long h)
{
	if (SkipLines)
	{
		gr_clear(0xFF);
		BlitLargeAlignSkip(bm->bits, destCanvas->bm.row, destCanvas->bm.bits, w, h+1, bm->row);
	}
	else
		BlitLargeAlign(bm->bits, destCanvas->bm.row, destCanvas->bm.bits, w, h+1, bm->row);
}

#else
asm void Fast_Slot_Double(grs_bitmap *bm, long w, long h)
 {
 	move.l		4(sp),a0
 	movem.l		d3-d7/a2,-(sp)
 	move.l		(a0),d4						// src
 	move.w		8(a0),d1					// width
 	sub.w		#kFastSlotWide_Half,d1
 	move.w		#kFastSlotHigh_Half-1,d2	// height
 	move.l		d4,a0
 	
 	move.l		gScreenAddress,a1		// dest
 	move.l		gScreenRowbytes,d3	// screen width
 	move.l		d3,a2
 	move.l		d3,d0
 	add.w			d3,d3
 	sub.w		#kFastSlotWide,d3
 	move.w		#kFastSlotTop,d4
 	mulu.w		d4,d0
 	add.l			d0,a1
 	add.w			#kFastSlotLeft,a1		// final dest
 	add.l			a1,a2						// odd line dest

 	tst.b			SkipLines
 	bne.s			@SkipLoop
 	
@Loop: 	
 	move.w		#(kFastSlotWide_Half/4)-1,d4			
 	
@ILoop:
 	move.l		(a0)+,d0			// get 4 pixels
	move.b		d0,d5
	ror.w		#8,d5
	move.b		d0,d5				// double pixel 0
	move.w		d0,d6
	ror.w		#8,d0
	move.b		d0,d6				// double pixel 1
	swap			d6
	move.w		d5,d6				// pixel 1 & 2 in d6
	
	swap			d0
	move.b		d0,d5
	ror.w		#8,d5
	move.b		d0,d5
	move.w		d0,d7
	ror.w		#8,d0
	move.b		d0,d7
	swap			d7
	move.w		d5,d7				// pixel 3 & 4 in d7

 	move.l		d7,(a1)+
 	move.l		d6,(a1)+
 	move.l		d7,(a2)+
 	move.l		d6,(a2)+
 	 	
 	dbra			d4,@ILoop
 	
 	add.w			d1,a0
 	add.w			d3,a1
 	add.w			d3,a2
 	dbra			d2,@Loop
	bra			@Done

@SkipLoop: 	
 	move.w		#(kFastSlotWide_Half/4)-1,d4			
 	
@SkipILoop:
 	move.l		(a0)+,d0			// get 4 pixels
	move.b		d0,d5
	ror.w		#8,d5
	move.b		d0,d5				// double pixel 0
	move.w		d0,d6
	ror.w		#8,d0
	move.b		d0,d6				// double pixel 1
	swap			d6
	move.w		d5,d6				// pixel 1 & 2 in d6
	
	swap			d0
	move.b		d0,d5
	ror.w		#8,d5
	move.b		d0,d5
	move.w		d0,d7
	ror.w		#8,d0
	move.b		d0,d7
	swap			d7
	move.w		d5,d7				// pixel 3 & 4 in d7

 	move.l		d7,(a1)+
 	move.l		d6,(a1)+
 	 	
 	dbra			d4,@SkipILoop
 	
 	add.w			d1,a0
 	add.w			d3,a1
 	dbra			d2,@SkipLoop

@Done: 	
 	movem.l		(sp)+,d3-d7/a2
 	rts
 }
#endif

// copy the full screen view from offscreen to on
// hard coded to copy from 0,0 to 640,480 to the screen
#if (defined(powerc) || defined(__powerc))	
void Fast_FullScreen_Double(grs_bitmap *bm, long w, long h)
 {
 	if (!SkipLines)
 		BlitLargeAlign(bm->bits, gScreenRowbytes, gScreenAddress,w,h,bm->row);
 	else
 		BlitLargeAlignSkip(bm->bits, gScreenRowbytes, gScreenAddress,w,h,bm->row);
 }

void FastFullscreenDouble2Canvas(grs_bitmap *bm, grs_canvas *destCanvas, long w, long h)
{
	if (SkipLines)
	{
		gr_clear(0xFF);
		BlitLargeAlignSkip(bm->bits, destCanvas->bm.row, destCanvas->bm.bits, w, h+1, bm->row);
	}
	else
		BlitLargeAlign(bm->bits, destCanvas->bm.row, destCanvas->bm.bits, w, h+1, bm->row);
}

#else
asm void Fast_FullScreen_Double(grs_bitmap *bm, long w, long h)
 {
 	move.l		4(sp),a0
 	movem.l		d3-d7/a2,-(sp)
 	move.l		(a0),d4						// src
 	move.w		8(a0),d1					// width
 	sub.w		#320,d1
 	move.w		#240-1,d2	// height
 	move.l		d4,a0
 	
 	move.l		gScreenAddress,a1		// dest
 	move.l		a1,a2
 	move.l		gScreenRowbytes,d3	// screen width
 	add.l			d3,a2
 	
 	add.w			d3,d3
 	sub.w		#640,d3
 	
 	tst.b			SkipLines
 	bne.s			@SkipLoop
 	
@Loop: 	
 	move.w		#(320/4)-1,d4			
 	
@ILoop:
 	move.l		(a0)+,d0			// get 4 pixels
	move.b		d0,d5
	ror.w		#8,d5
	move.b		d0,d5				// double pixel 0
	move.w		d0,d6
	ror.w		#8,d0
	move.b		d0,d6				// double pixel 1
	swap			d6
	move.w		d5,d6				// pixel 1 & 2 in d6
	
	swap			d0
	move.b		d0,d5
	ror.w		#8,d5
	move.b		d0,d5
	move.w		d0,d7
	ror.w		#8,d0
	move.b		d0,d7
	swap			d7
	move.w		d5,d7				// pixel 3 & 4 in d7

 	move.l		d7,(a1)+
 	move.l		d6,(a1)+
 	move.l		d7,(a2)+
 	move.l		d6,(a2)+
 	 	
 	dbra			d4,@ILoop
 	
 	add.w			d1,a0
 	add.w			d3,a1
 	add.w			d3,a2
 	dbra			d2,@Loop
 	bra			@Done
 
@SkipLoop: 	
 	move.w		#(320/4)-1,d4			
 	
@SkipILoop:
 	move.l		(a0)+,d0			// get 4 pixels
	move.b		d0,d5
	ror.w		#8,d5
	move.b		d0,d5				// double pixel 0
	move.w		d0,d6
	ror.w		#8,d0
	move.b		d0,d6				// double pixel 1
	swap			d6
	move.w		d5,d6				// pixel 1 & 2 in d6
	
	swap			d0
	move.b		d0,d5
	ror.w		#8,d5
	move.b		d0,d5
	move.w		d0,d7
	ror.w		#8,d0
	move.b		d0,d7
	swap			d7
	move.w		d5,d7				// pixel 3 & 4 in d7

 	move.l		d7,(a1)+
 	move.l		d6,(a1)+
 	 	
 	dbra			d4,@SkipILoop
 	
 	add.w			d1,a0
 	add.w			d3,a1
 	dbra			d2,@SkipLoop
 
@Done:
 	movem.l		(sp)+,d3-d7/a2
 	rts
 }
#endif

