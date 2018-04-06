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
// $Source: r:/prj/lib/src/2d/RCS/vtab.c $
// $Revision: 1.1 $
// $Author: kevin $
// $Date: 1994/07/28 01:23:36 $
//
// Procedure to create temporary vtab.
//
// This file is part of the 2d library.
//

#include "grs.h"
#include "vtab.h"
#include "buffer.h"


// build a table of line starts for the bitmap parameter
// PowerPC version
#if (defined(powerc) || defined(__powerc))	
long *gr_make_vtab (grs_bitmap *bm)
 {
 	void 	*mem;
 	long	*dest;
 	long	i,add,row;
 	long	maxh;
 	
 	mem = gr_alloc_temp(bm->h<<2);
 	row = bm->row;
	add = 0L;
	maxh = bm->h;
	dest = (long *) mem;
	
	for (i=0; i<maxh; i++)
	 {
	 	*(dest++) = add;
	 	add += row;
	 }

 	return((long *) mem);
 }
 
 
// 68K version
#else

long *xgr_make_vtab (grs_bitmap *bm);
long *xgr_make_vtab (grs_bitmap *bm)
 {
 	void 	*mem;
 	long	*dest;
 	long	i,add,row;
 	long	maxh;
 	
 	mem = gr_alloc_temp(bm->h<<2);
 	row = bm->row;
	add = 0L;
	maxh = bm->h;
	dest = (long *) mem;
	
	for (i=0; i<maxh; i++)
	 {
	 	*(dest++) = add;
	 	add += row;
	 }

 	return((long *) mem);
 }

asm long *gr_make_vtab (grs_bitmap *bm)
 {
 	movem.l	d3/a2,-(sp)
 	
 	move.l	12(sp),a2				// a2 = *bm
 	move.w	10(a2),d0				// d0 = bm->h
 	ext.l		d0
 	lsl.l		#2,d0
 	move.l	d0,-(sp)
 	jsr			gr_alloc_temp		// returns ptr in a0
 	addq.w	#4,sp
 	
 	moveq		#0,d1
	moveq		#0,d2
 	moveq		#0,d3
 	move.w	0x0c(a2),d1			// get row
 	move.l	a0,a1						// a1 = dest, a0 = mem
 	move.w	10(a2),d3				// d3 = bm->h
 	subq.w	#1,d3
 		
@loop:
	move.l	d2,(a1)+
	add.l		d1,d2
	dbra		d3,@loop
	
 	movem.l	(sp)+,d3/a2
 	rts
 }
#endif
