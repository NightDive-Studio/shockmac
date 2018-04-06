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
 * $Source: r:/prj/lib/src/2d/RCS/genw.c $
 * $Revision: 1.4 $
 * $Author: kevin $
 * $Date: 1994/08/16 12:50:15 $
 *
 * Routines to floor texture map a flat8 bitmap to a generic canvas.
 *
 * This file is part of the 2d library.
 *
 */

#include "stdio.h"
#include "tmapint.h"
#include "gente.h"
#include "grnull.h"
#include "grpix.h"
#include "poly.h"
#include "vtab.h"
#include "fl8tf.h"
#include "cnvdat.h"
#include "2dDiv.h"
#include "fl8tmapdv.h"

int gri_wall_umap_loop(grs_tmap_loop_info *tli);
int gri_wall_umap_loop_1D(grs_tmap_loop_info *tli);

// 68K stuff
#if !(defined(powerc) || defined(__powerc))	
asm int Handle_Wall_68K_Loop(fix u, fix v, fix du, fix dv, fix dy,
														 grs_tmap_loop_info *tli, uchar *start_pdest, uchar *t_bits, long gr_row);
asm int Handle_Wall_68K_Loop_1D(fix u, fix v, fix dv, fix dy,
														 	grs_tmap_loop_info *tli, uchar *start_pdest, uchar *t_bits, long gr_row);

// specific inner looops
asm void wall_opaque_log2(void);
asm void wall_opaque_clut_log2(void);
asm void wall_trans_log2(void);
asm void wall_trans_clut_log2(void);

// jump table for 68k inner loops
void (*wall_in_loop[8])(void) = {gr_not_imp, gr_not_imp, wall_opaque_log2, wall_trans_log2, 
																gr_not_imp,gr_not_imp, wall_opaque_clut_log2,wall_trans_clut_log2};
#endif

// globals used by 68K routines
uchar *w_clut_68K;
ulong	w_wlog_68K;
ulong	w_mask_68K;

int gri_wall_umap_loop(grs_tmap_loop_info *tli) {
   fix u,v,du,dv,dy,d;

	 // locals used to store copies of tli-> stuff, so its in registers on the PPC
	 int		k,y;
	 uchar	t_wlog;
	 ulong	t_mask;
	 long		*t_vtab;
	 uchar 	*t_bits;
	 uchar 	*p_dest;
	 fix		inv_dy;
	 uchar	temp_pix;
	 uchar *t_clut;
	 long		gr_row;
	 	 
#if InvDiv
   inv_dy = fix_div(fix_make(1,0),tli->w);
	 u=fix_mul_asm_safe(tli->left.u,inv_dy);
   du=fix_mul_asm_safe(tli->right.u,inv_dy)-u;
   v=fix_mul_asm_safe(tli->left.v,inv_dy); 
   dv=fix_mul_asm_safe(tli->right.v,inv_dy)-v;
#else
	 u=fix_div(tli->left.u,tli->w);
   du=fix_div(tli->right.u,tli->w)-u;
   v=fix_div(tli->left.v,tli->w); 
   dv=fix_div(tli->right.v,tli->w)-v;
#endif

   dy=tli->right.y-tli->left.y;

	 t_vtab = tli->vtab;
	 t_bits = tli->bm.bits;

	 w_clut_68K = t_clut = tli->clut;
	 w_mask_68K = t_mask = tli->mask;
	 w_wlog_68K = t_wlog = tli->bm.wlog;

	 gr_row = grd_bm.row;

// handle PowerPC loop
#if (defined(powerc) || defined(__powerc))	
   do {
      if ((d = fix_ceil(tli->right.y)-fix_ceil(tli->left.y)) > 0) {
 
         d =fix_ceil(tli->left.y)-tli->left.y;
 
#if InvDiv
         inv_dy = fix_div(fix_make(1,0),dy);
         du=fix_mul_asm_safe(du,inv_dy); 
         dv=fix_mul_asm_safe(dv,inv_dy); 
#else
         du=fix_div(du,dy); 
         dv=fix_div(dv,dy); 
#endif
         u+=fix_mul(du,d);
         v+=fix_mul(dv,d);

			 	 p_dest = grd_bm.bits + (gr_row*fix_cint(tli->left.y)) + tli->x; 
				 y = fix_cint(tli->right.y) - fix_cint(tli->left.y);
				 
         switch (tli->bm.hlog) {
         case GRL_OPAQUE:
            for (; y>0; y--) {
               k=t_vtab[fix_fint(v)]+fix_fint(u);
               *p_dest = t_bits[k];	// gr_fill_upixel(t_bits[k],t_x,y);
               p_dest += gr_row; u+=du; v+=dv;
            }
            break;
         case GRL_TRANS:
            for (; y>0; y--) {
               if (temp_pix = t_bits[t_vtab[fix_fint(v)]+fix_fint(u)]) 
                 *p_dest = temp_pix;			// gr_fill_upixel(t_bits[k],t_x,y);
               p_dest += gr_row; u+=du; v+=dv;
            }
            break;
         case GRL_OPAQUE|GRL_LOG2:
            for (; y>0; y--) {
               *p_dest = t_bits[((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask];	// gr_fill_upixel(t_bits[k],t_x,y);
               p_dest += gr_row; u+=du; v+=dv;
            }
            break;
         case GRL_TRANS|GRL_LOG2:
            for (; y>0; y--) {
               if (temp_pix = t_bits[((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask]) 
                 *p_dest = temp_pix;		// gr_fill_upixel(t_bits[k],t_x,y);
               p_dest += gr_row; u+=du; v+=dv;
            }
            break;
         case GRL_OPAQUE|GRL_CLUT:
            for (; y>0; y--) {
               *p_dest = t_clut[t_bits[t_vtab[fix_fint(v)]+fix_fint(u)]];	// gr_fill_upixel(t_clut[t_bits[k]],t_x,y);
               p_dest += gr_row; u+=du; v+=dv;
            }
            break;
         case GRL_TRANS|GRL_CLUT:
            for (; y>0; y--) {
               k=t_vtab[fix_fint(v)]+fix_fint(u);
               if (k=t_bits[k]) 
                 *p_dest = t_clut[k];	// gr_fill_upixel(t_clut[k],t_x,y);
               p_dest += gr_row; u+=du; v+=dv;
            }
            break;
         case GRL_OPAQUE|GRL_LOG2|GRL_CLUT:
            for (; y>0; y--) {
               *p_dest = t_clut[t_bits[((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask]];	// gr_fill_upixel(t_clut[t_bits[k]],t_x,y);
               p_dest += gr_row; u+=du; v+=dv;
            }
            break;
         case GRL_TRANS|GRL_LOG2|GRL_CLUT:
            for (; y>0; y--) {
               k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
               if (k=t_bits[k]) 
                 *p_dest = t_clut[k];	// gr_fill_upixel(t_clut[k],t_x,y);
               p_dest += gr_row; u+=du; v+=dv;
            }
            break;
         }
      } else if (d<0) return TRUE; /* punt this tmap */
      
			
      tli->w+=tli->dw;

 	// figure out new left u & v & i    
 			inv_dy = 0;
      k = tli->left.u+tli->left.du;
      y = tli->left.v+tli->left.dv;

#if InvDiv
      inv_dy = fix_div(fix_make(1,0),tli->w);
      u=fix_mul_asm_safe(k,inv_dy); 
      v=fix_mul_asm_safe(y,inv_dy); 
#else
      u=fix_div(k,tli->w); 
      v=fix_div(y,tli->w); 
#endif

			tli->left.u = k;
			tli->left.v = y;

 	// figure out new right u & v & i    
      k = tli->right.u+tli->right.du;
      y = tli->right.v+tli->right.dv;

#if InvDiv
      du=fix_mul_asm_safe(k,inv_dy)-u;
      dv=fix_mul_asm_safe(y,inv_dy)-v;
#else
      du=fix_div(k,tli->w)-u;
      dv=fix_div(y,tli->w)-v;
#endif

			tli->right.u = k;
			tli->right.v = y;

      tli->left.y+=tli->left.dy;
      tli->right.y+=tli->right.dy;
      dy=tli->right.y-tli->left.y;
      tli->x++;
         
   } while (--(tli->n) > 0);

	return FALSE;

// handle 68K loops
#else
	return(Handle_Wall_68K_Loop(u,v,du,dv,dy,tli,grd_bm.bits,t_bits,gr_row));
#endif
}

// Main 68K handler loop
#if !(defined(powerc) || defined(__powerc))	
asm int Handle_Wall_68K_Loop(fix u, fix v, fix du, fix dv, fix dy,
														 grs_tmap_loop_info *tli, uchar *start_pdest, uchar *t_bits, long gr_row)
 {
  movem.l	d0-d7/a0-a6,-(sp)

// load up vars
	move.l	64(sp),d4			// u
	move.l	68(sp),d5			// v
	move.l	72(sp),d6			// du
	move.l	76(sp),d7			// dv
	move.l	80(sp),d3			// dy
 	move.l	84(sp),a0			// *tli

@DoLoop: 	
//    if ((d = fix_ceil(tli->right.y)-fix_ceil(tli->left.y)) > 0) {
 	move.l	0x44(a0),d0
 	add.l		#0x0000FFFF,d0
 	clr.w		d0									// fix_ceil(tli->right.y)
 	move.l	0x20(a0),d1
 	add.l		#0x0000FFFF,d1
 	clr.w		d1									// fix_ceil(tli->left.y)
 	sub.l		d1,d0								// fix_ceil(tli->right.y)-fix_ceil(tli->left.y))
 	beq 		@Skip								// d==0, skip this map
 	bmi 		@Err								// d<0, punt

//   d =fix_ceil(tli->left.y)-tli->left.y;
//   du=fix_div(du,dy);
//   dv=fix_div(dv,dy);
//   u+=fix_mul(du,d);
//   v+=fix_mul(dv,d);
 
 	move.l	0x20(a0),d0				// tli->left.y
 	move.l	d0,d2
 	add.l		#0x0000FFFF,d2
 	clr.w		d2
 	sub.l		d0,d2							// d =fix_ceil(tli->left.y)-tli->left.y;
	
	fix_div_68k_d3(d6)			// inline function, returns result in d0
	move.l	d0,d6						//  du=fix_div(du,dy);
	
	fix_div_68k_d3(d7)			// inline function, returns result in d0
	move.l	d0,d7						//  dv=fix_div(dv,dy);
	
	move.l	d2,d0
	dc.l		0x4C060C01   		//  MULS.L    D6,D1:D0
	move.w	d1,d0
	swap		d0
	add.l		d0,d4 					// u+=fix_mul(du,d);
	    
	move.l	d2,d0
	dc.l		0x4C070C01   		//  MULS.L    D7,D1:D0
	move.w	d1,d0
	swap		d0
	add.l		d0,d5 					// v+=fix_mul(dv,d);

// t_yl = fix_cint(tli->left.y);
// t_yr = fix_cint(tli->right.y);
 	move.l	0x20(a0),d1
 	add.l		#0x0000FFFF,d1
 	clr.w		d1
 	swap		d1
	move.l	d1,d2	// save for later
	
 	move.l	0x44(a0),d0
 	add.l		#0x0000FFFF,d0
 	clr.w		d0
 	swap		d0
 	sub.l		d1,d0			// t_yr-t_yl

// p_dest = grd_bm.bits + (gr_row*t_yl) + t_x;
	dc.w		0x4C2F,0x2000,0x0060   //  MULU.L    $0060(A7),D2
	move.l	d2,a4
	add.l		4(a0),a4
	add.l		88(sp),a4
	
	// when we call the asm inner loop, a5 = t_bits, a4 = p_dest, a2 = gr_row
	// d0 = t_yr-t_yl, u,v,du,dv = d4-d7
	move.l	96(sp),a2
	move.l	92(sp),a5
	lea			wall_in_loop,a1
	moveq		#0,d1
	move.b	23(a0),d1
	dc.l		0x22711400      //  MOVEA.L   $00(A1,D1.W*4),A1
	jsr			(a1)
@Skip:

	move.l	0x18(a0),d2
  add.l		0x64(a0),d2			// tli->w+=tli->dw;
  move.l	d2,0x18(a0)
  
 	move.l	0x34(a0),d0			// tli->left.du
 	add.l	  0x24(a0),d0			// tli->left.u
 	move.l	d0,0x24(a0)
 	fix_div_68k_d2_d0(d4)		// inline function, returns result in d0
	move.l	d0,d4						// u=fix_div((tli->left.u+=tli->left.du),tli->w);
	
 	move.l	0x48(a0),d0
 	add.l		0x58(a0),d0
 	move.l	d0,0x48(a0)		// tli->right.u+=tli->right.du;
 	fix_div_68k_d2_d0(d6)		// inline function, returns result in d0
  sub.l		d4,d0
  move.l	d0,d6					//  du=fix_div(tli->right.u,tli->w)-u;
      
 	move.l	0x28(a0),d0
 	add.l		0x38(a0),d0	
 	move.l	d0,0x28(a0)		// tli->left.v+=tli->left.dv;
 	fix_div_68k_d2_d0(d5)		// inline function, returns result in d0
  move.l	d0,d5					// v=fix_div((tli->left.v+=tli->left.dv),tli->w);
     
 	move.l	0x4C(a0),d0
 	add.l		0x5C(a0),d0
 	move.l	d0,0x4C(a0)		// tli->right.v+=tli->right.dv;
 	fix_div_68k_d2_d0(d7)			// inline function, returns result in d0
 	sub.l		d5,d0
	move.l	d0,d7					//  dv=fix_div(tli->right.v,tli->w)-v;
	
 	move.l	0x30(a0),d0		// tli->left.y+=tli->left.dy;
 	add.l		d0,0x20(a0)

 	move.l	0x54(a0),d0
 	add.l		d0,0x44(a0)		// tli->right.y+=tli->right.dy;
     
 	move.l	0x44(a0),d3
 	sub.l		0x20(a0),d3		// dy=tli->right.y-tli->left.y;

	addq.l	#1,4(a0)			// tli->x++;
  		 
	subq.l	#1,(a0)
	bgt 		@DoLoop				//	} while (--(tli->n) > 0);

@Done:
  movem.l	(sp)+,d0-d7/a0-a6
	moveq		#FALSE,d0
  rts

@Err: 	
	moveq		#TRUE,d0
  movem.l	(sp)+,d0-d7/a0-a6
  rts
 }
 
   
 
// when we call the asm inner loop, a5 = t_bits, a4 = p_dest, a2 = gr_row
// d0 = t_yr-t_yl, u,v,du,dv = d4-d7
// handle inner loop for wall opaque (width log2) mode
asm void wall_opaque_log2(void)
 {
/*  for (y=t_yl; y<t_yr; y++) {
     int k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
     *p_dest = t_bits[k];	// gr_fill_upixel(t_bits[k],t_x,y);
     p_dest += gr_row; u+=du; v+=dv;*/
     
	move.l	w_mask_68K,d2
	move.l	w_wlog_68K,d3
	move.l	d7,a3
	move.l	d6,a1
	subq.w	#1,d0
	bmi.s		@Done

	moveq		#16,d6
@Loop:
	move.l	d5,d1
	asr.l		d6,d1 						// d1 = fix_fint(v)
	lsl.l		d3,d1							// d1 = (fix_fint(v)<<t_wlog
	move.l	d4,d7
	asr.l		d6,d7 						// d7 = fix_fint(u)
	add.l		d7,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))
	and.l		d2,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask
	move.b	(a5,d1.l),(a4)		// *p_dest = t_bits[k];
	add.l		a2,a4							// p_dest += gr_row;
	add.l		a1,d4							// u+=du
	add.l		a3,d5							// v+=dv
	dbra		d0,@Loop

@Done:		
	rts	   
 }

// handle inner loop for wall opaque clut (width log2) mode
asm void wall_opaque_clut_log2(void)
 {
/*  for (y=t_yl; y<t_yr; y++) {
     int k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
     *p_dest = t_clut[t_bits[k]];	// gr_fill_upixel(t_clut[t_bits[k]],t_x,y);
     p_dest += gr_row; u+=du; v+=dv;
  }*/

	move.l	w_clut_68K,a6     
	move.l	w_mask_68K,d2
	move.l	w_wlog_68K,d3
	move.l	d7,a3
	move.l	d6,a1
	subq.w	#1,d0
	bmi.s		@Done

	moveq		#16,d6
@Loop:
	move.l	d5,d1
	asr.l		d6,d1 						// d1 = fix_fint(v)
	lsl.l		d3,d1							// d1 = (fix_fint(v)<<t_wlog
	move.l	d4,d7
	asr.l		d6,d7 						// d7 = fix_fint(u)
	add.l		d7,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))
	and.l		d2,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask
	moveq		#0,d7
	move.b	(a5,d1.l),d7
	move.b	(a6,d7.w),(a4)		// *p_dest = t_bits[k];
	add.l		a2,a4							// p_dest += gr_row;
	add.l		a1,d4							// u+=du
	add.l		a3,d5							// v+=dv
	dbra		d0,@Loop

@Done:		
	rts	   
 }
 

// handle inner loop for wall transparent (width log2) mode
asm void wall_trans_log2(void)
 {
/*  for (y=t_yl; y<t_yr; y++) {
     int k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
     if (temp_pix = t_bits[k]) 
       *p_dest = temp_pix;		// gr_fill_upixel(t_bits[k],t_x,y);
     p_dest += gr_row; u+=du; v+=dv;*/
     
	move.l	w_mask_68K,d2
	move.l	w_wlog_68K,d3
	move.l	d7,a3
	move.l	d6,a1
	subq.w	#1,d0
	bmi.s		@Done

	moveq		#16,d6
@Loop:
	move.l	d5,d1
	asr.l		d6,d1 						// d1 = fix_fint(v)
	lsl.l		d3,d1							// d1 = (fix_fint(v)<<t_wlog
	move.l	d4,d7
	asr.l		d6,d7 						// d7 = fix_fint(u)
	add.l		d7,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))
	and.l		d2,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask
	move.b	(a5,d1.l),d7			// *p_dest = t_bits[k];
	beq.s		@skippix
	move.b	d7,(a4)
@skippix:
	add.l		a2,a4							// p_dest += gr_row;
	add.l		a1,d4							// u+=du
	add.l		a3,d5							// v+=dv
	dbra		d0,@Loop

@Done:		
	rts	   
 }
 
 
// handle inner loop for wall transparent clut (width log2) mode
asm void wall_trans_clut_log2(void)
 {
/*  for (y=t_yl; y<t_yr; y++) {
       int k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
       if (k=t_bits[k]) 
         *p_dest = t_clut[k];	// gr_fill_upixel(t_clut[k],t_x,y);
       p_dest += gr_row; u+=du; v+=dv;
    }*/

	move.l	w_clut_68K,a6     
	move.l	w_mask_68K,d2
	move.l	w_wlog_68K,d3
	move.l	d7,a3
	move.l	d6,a1
	subq.w	#1,d0
	bmi.s		@Done

	moveq		#16,d6
@Loop:
	move.l	d5,d1
	asr.l		d6,d1 						// d1 = fix_fint(v)
	lsl.l		d3,d1							// d1 = (fix_fint(v)<<t_wlog
	move.l	d4,d7
	asr.l		d6,d7 						// d7 = fix_fint(u)
	add.l		d7,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))
	and.l		d2,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask
	moveq		#0,d7
	move.b	(a5,d1.l),d7
	beq.s		@skippix
	move.b	(a6,d7.w),(a4)		// *p_dest = t_bits[k];
@skippix:
	add.l		a2,a4							// p_dest += gr_row;
	add.l		a1,d4							// u+=du
	add.l		a3,d5							// v+=dv
	dbra		d0,@Loop

@Done:		
	rts	   
 }
#endif


void gri_trans_wall_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_TRANS|GRL_LOG2;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_TRANS;
   }
   tli->loop_func=(void (*)()) gri_wall_umap_loop;
   tli->left_edge_func=(void (*)()) gri_uvwy_edge;
   tli->right_edge_func=(void (*)()) gri_uvwy_edge;
}

void gri_opaque_wall_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_OPAQUE|GRL_LOG2;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_OPAQUE;
   }
   tli->loop_func=(void (*)()) gri_wall_umap_loop;
   tli->left_edge_func=(void (*)()) gri_uvwy_edge;
   tli->right_edge_func=(void (*)()) gri_uvwy_edge;
}

void gri_trans_clut_wall_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_TRANS|GRL_LOG2|GRL_CLUT;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_TRANS|GRL_CLUT;
   }
   tli->loop_func=(void (*)()) gri_wall_umap_loop;
   tli->left_edge_func=(void (*)()) gri_uvwy_edge;
   tli->right_edge_func=(void (*)()) gri_uvwy_edge;
}

void gri_opaque_clut_wall_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_OPAQUE|GRL_LOG2|GRL_CLUT;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_OPAQUE|GRL_CLUT;
   }
   tli->loop_func=(void (*)()) gri_wall_umap_loop;
   tli->left_edge_func=(void (*)()) gri_uvwy_edge;
   tli->right_edge_func=(void (*)()) gri_uvwy_edge;
}

extern "C"
{
extern int HandleWallLoop1D_PPC(grs_tmap_loop_info *tli,
																fix u, fix v, fix dv, fix dy,
																uchar *t_clut, long *t_vtab, uchar *o_bits,
																long gr_row, ulong t_mask, ulong t_wlog);
}
/*
int HandleWallLoop1D_C(grs_tmap_loop_info *tli,
																fix u, fix v, fix dv, fix dy,
																uchar *t_clut, long *t_vtab, uchar *o_bits,
																long gr_row, ulong t_mask, ulong t_wlog)
 {
 	register int		k,y;
 	register fix		inv_dy;
 	register uchar  *grd_bits,*p_dest,*t_bits;
 	register fix		ry,ly;
 	
 	ry = tli->right.y;
 	ly = tli->left.y;
 	
 	grd_bits = grd_bm.bits + tli->x; 
 	tli->x += tli->n;
   do {
      if ((k = fix_ceil(ry)-fix_ceil(ly)) > 0) {
 
         k =fix_ceil(ly)-ly;
        	
         dv=fix_div(dv,dy); 
         v+=fix_mul(dv,k);

			 	 p_dest = grd_bits + (gr_row*fix_cint(ly)); 
				 y = fix_cint(ry) - fix_cint(ly);
				 t_bits = o_bits + fix_fint(u);
         for (; y>0; y--) 
          {
           k=((fix_fint(v)<<t_wlog))&t_mask;
           *p_dest = t_clut[t_bits[k]];	// gr_fill_upixel(t_clut[t_bits[k]],t_x,y);
           v+=dv; p_dest += gr_row; 
          }
      } else if (k<0) return TRUE; // punt this tmap 
			
      tli->w+=tli->dw;

 	// figure out new left u & v   
      k = tli->left.u+tli->left.du;
      y = tli->left.v+tli->left.dv;

      inv_dy = fix_div(fix_make(1,0),tli->w);
      u=fix_mul_asm_safe(k,inv_dy); 
      v=fix_mul_asm_safe(y,inv_dy); 

			tli->left.u = k;
			tli->left.v = y;

 	// figure out new right u & v   
      tli->right.u += tli->right.du;
      y = tli->right.v+tli->right.dv;
      dv=fix_mul_asm_safe(y,inv_dy)-v;
			tli->right.v = y;

      ly+=tli->left.dy;
      ry+=tli->right.dy;
      dy=ry-ly;
      grd_bits++;
   } while (--(tli->n) > 0);

 	tli->right.y = ry;
 	tli->left.y = ly;

  return false;
 }
*/

// ==================================================================
// 1D versions 
int gri_wall_umap_loop_1D(grs_tmap_loop_info *tli) {
   fix u,v,dv,dy,d;

	 // locals used to store copies of tli-> stuff, so its in registers on the PPC
	 int		k,y;
	 uchar	t_wlog;
	 ulong	t_mask;
	 long		*t_vtab;
	 uchar 	*t_bits,*o_bits;
	 uchar 	*p_dest;
	 fix		inv_dy;
	 uchar	temp_pix;
	 uchar *t_clut;
	 long		gr_row;
	 
#if InvDiv
   inv_dy = fix_div(fix_make(1,0),tli->w);
	 u=fix_mul_asm_safe(tli->left.u,inv_dy);
   v=fix_mul_asm_safe(tli->left.v,inv_dy); 
   dv=fix_mul_asm_safe(tli->right.v,inv_dy)-v;
#else
	 u=fix_div(tli->left.u,tli->w);
   v=fix_div(tli->left.v,tli->w); 
   dv=fix_div(tli->right.v,tli->w)-v;
#endif

   dy=tli->right.y-tli->left.y;

	 t_vtab = tli->vtab;
	 o_bits = tli->bm.bits;

	 w_clut_68K = t_clut = tli->clut;
	 w_mask_68K = t_mask = tli->mask;
	 w_wlog_68K = t_wlog = tli->bm.wlog;

	 gr_row = grd_bm.row;

// handle PowerPC loop
#if (defined(powerc) || defined(__powerc))	
	return HandleWallLoop1D_PPC(tli, u, v, dv, dy, t_clut, t_vtab, o_bits, gr_row, t_mask, t_wlog);
// handle 68K loops
#else
	return(Handle_Wall_68K_Loop_1D(u,v,dv,dy,tli,grd_bm.bits,o_bits,gr_row)); 
#endif
}

// Main 68K handler loop
#if !(defined(powerc) || defined(__powerc))	
asm int Handle_Wall_68K_Loop_1D(fix u, fix v, fix dv, fix dy,
														 		grs_tmap_loop_info *tli, uchar *start_pdest, uchar *t_bits, long gr_row)
 {
  movem.l	d0-d7/a0-a6,-(sp)

// load up vars
	move.l	64(sp),d4			// u
	move.l	68(sp),d5			// v
//	move.l	72(sp),d6			// du
	move.l	72(sp),d7			// dv
	move.l	76(sp),d3			// dy
 	move.l	80(sp),a0			// *tli

@DoLoop: 	
//    if ((d = fix_ceil(tli->right.y)-fix_ceil(tli->left.y)) > 0) {
 	move.l	0x44(a0),d0
 	add.l		#0x0000FFFF,d0
 	clr.w		d0									// fix_ceil(tli->right.y)
 	move.l	0x20(a0),d1
 	add.l		#0x0000FFFF,d1
 	clr.w		d1									// fix_ceil(tli->left.y)
 	sub.l		d1,d0								// fix_ceil(tli->right.y)-fix_ceil(tli->left.y))
 	beq 		@Skip								// d==0, skip this map
 	bmi 		@Err								// d<0, punt

//   d =fix_ceil(tli->left.y)-tli->left.y;
//   du=fix_div(du,dy);
//   dv=fix_div(dv,dy);
//   u+=fix_mul(du,d);
//   v+=fix_mul(dv,d);
 
 	move.l	0x20(a0),d0				// tli->left.y
 	move.l	d0,d2
 	add.l		#0x0000FFFF,d2
 	clr.w		d2
 	sub.l		d0,d2							// d =fix_ceil(tli->left.y)-tli->left.y;
		
	fix_div_68k_d3(d7)			// inline function, returns result in d0
	move.l	d0,d7						//  dv=fix_div(dv,dy);
		    
	move.l	d2,d0
	dc.l		0x4C070C01   		//  MULS.L    D7,D1:D0
	move.w	d1,d0
	swap		d0
	add.l		d0,d5 					// v+=fix_mul(dv,d);

// t_yl = fix_cint(tli->left.y);
// t_yr = fix_cint(tli->right.y);
 	move.l	0x20(a0),d1
 	add.l		#0x0000FFFF,d1
 	clr.w		d1
 	swap		d1
	move.l	d1,d2	// save for later
	
 	move.l	0x44(a0),d0
 	add.l		#0x0000FFFF,d0
 	clr.w		d0
 	swap		d0
 	sub.l		d1,d0			// t_yr-t_yl

// p_dest = grd_bm.bits + (gr_row*t_yl) + t_x;
	dc.w		0x4C2F,0x2000,0x005C   //  MULU.L    $005C(A7),D2
	move.l	d2,a4
	add.l		4(a0),a4
	add.l		84(sp),a4
	
	// when we call the asm inner loop, a5 = t_bits, a4 = p_dest, a2 = gr_row
	// d0 = t_yr-t_yl, u,v,du,dv = d4-d7
	move.l	92(sp),a2
	move.l	88(sp),a5
	move.l	d4,d6
	swap		d6
	add.w		d6,a5		// and in u to t_bits
	
// --------------------------------------------------------------------
/*  for (y=t_yl; y<t_yr; y++) {
     int k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
     *p_dest = t_clut[t_bits[k]];	// gr_fill_upixel(t_clut[t_bits[k]],t_x,y);
     p_dest += gr_row; u+=du; v+=dv;
  }*/

	move.l	w_clut_68K,a6     
	move.l	w_mask_68K,d2
	move.l	w_wlog_68K,d3
	move.l	d7,a3
	subq.w	#1,d0
	bmi.s		@LL_Done

	moveq		#16,d6
@LL_Loop:
	move.l	d5,d1
	asr.l		d6,d1 						// d1 = fix_fint(v)
	lsl.l		d3,d1							// d1 = (fix_fint(v)<<t_wlog
	and.l		d2,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask
	moveq		#0,d7
	move.b	(a5,d1.l),d7
	move.b	(a6,d7.w),(a4)		// *p_dest = t_bits[k];
	add.l		a2,a4							// p_dest += gr_row;
	add.l		a3,d5							// v+=dv
	dbra		d0,@LL_Loop

@LL_Done:		
// --------------------------------------------------------------------

@Skip:
	move.l	0x18(a0),d2
  add.l		0x64(a0),d2			// tli->w+=tli->dw;
  move.l	d2,0x18(a0)

  // calc inverse of tli->w
  move.l	#0x010000,d0
  fix_div_68k_d2_d0(d4)
  move.l	d0,d2						// d2 = inverse tli->w
  
 	move.l	0x34(a0),d0			// tli->left.du
 	add.l	  0x24(a0),d0			// tli->left.u
 	move.l	d0,0x24(a0)
 	fix_mul_d2_d0(d4)				// u=fix_div((tli->left.u+=tli->left.du),tli->w);
	
 	move.l	0x48(a0),d0
 	add.l		0x58(a0),d0
 	move.l	d0,0x48(a0)		// tli->right.u+=tli->right.du;
      
 	move.l	0x28(a0),d0
 	add.l		0x38(a0),d0	
 	move.l	d0,0x28(a0)		// tli->left.v+=tli->left.dv;
 	fix_mul_d2_d0(d5)			// v=fix_div((tli->left.v+=tli->left.dv),tli->w);
     
 	move.l	0x4C(a0),d0
 	add.l		0x5C(a0),d0
 	move.l	d0,0x4C(a0)		// tli->right.v+=tli->right.dv;
 	fix_mul_d2_d0(d7)			
	sub.l		d5,d7					//  dv=fix_div(tli->right.v,tli->w)-v;
	
 	move.l	0x30(a0),d0		// tli->left.y+=tli->left.dy;
 	add.l		d0,0x20(a0)

 	move.l	0x54(a0),d0
 	add.l		d0,0x44(a0)		// tli->right.y+=tli->right.dy;
     
 	move.l	0x44(a0),d3
 	sub.l		0x20(a0),d3		// dy=tli->right.y-tli->left.y;

	addq.l	#1,4(a0)			// tli->x++;
  		 
	subq.l	#1,(a0)
	bgt 		@DoLoop				//	} while (--(tli->n) > 0);

@Done:
  movem.l	(sp)+,d0-d7/a0-a6
	moveq		#FALSE,d0
  rts

@Err: 	
	moveq		#TRUE,d0
  movem.l	(sp)+,d0-d7/a0-a6
  rts
 }
#endif


void gri_opaque_clut_wall1d_umap_init(grs_tmap_loop_info *tli)
 {
// MLA - Wall1d is always log2
/*   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {*/
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_OPAQUE|GRL_LOG2|GRL_CLUT;
/*   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_OPAQUE|GRL_CLUT;
   }*/
   tli->loop_func=(void (*)()) gri_wall_umap_loop_1D;
   tli->left_edge_func=(void (*)()) gri_uvwy_edge;
   tli->right_edge_func=(void (*)()) gri_uvwy_edge;
 }
 

