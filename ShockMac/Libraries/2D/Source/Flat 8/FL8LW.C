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
 * $Source: r:/prj/lib/src/2d/RCS/fl8lw.c $
 * $Revision: 1.4 $
 * $Author: kevin $
 * $Date: 1994/08/16 12:50:16 $
 *
 * Routines to wall floor map a flat8 bitmap to a generic canvas.
 *
 * This file is part of the 2d library.
 *
 */

#include "tmapint.h"
#include "gente.h"
#include "grpix.h"
#include "poly.h"
#include "scrmac.h"
#include "vtab.h"
#include "fl8tf.h"
#include "cnvdat.h"
#include "2dDiv.h"
#include "fl8tmapdv.h"

int gri_lit_wall_umap_loop(grs_tmap_loop_info *tli);
int gri_lit_wall_umap_loop_1D(grs_tmap_loop_info *tli);

// 68K stuff
#if !(defined(powerc) || defined(__powerc))	
asm int Handle_Wall_Lit_68K_Loop(fix u, fix v, fix du, fix dv, fix dy,
														 		 grs_tmap_loop_info *tli, uchar *start_pdest, uchar *t_bits, long gr_row,
														 		 fix i, fix di);
asm int Handle_Wall_Lit_68K_Loop_1D(fix u, fix v, fix dv, fix dy,
														 		 		grs_tmap_loop_info *tli, uchar *start_pdest, uchar *t_bits, long gr_row,
														 		 		fix i, fix di);

// specific inner looops
asm void wall_lit_opaque_log2(void);
asm void wall_lit_trans_log2(void);
				
#endif

// globals used by 68K routines
ulong	w_l_wlog_68K;
ulong	w_l_mask_68K;
uchar	*w_l_ltab;

int gri_lit_wall_umap_loop(grs_tmap_loop_info *tli) {
   fix u,v,i,du,dv,di,dy,d;

	 // locals used to store copies of tli-> stuff, so its in registers on the PPC
	 int		k,y;
	 ulong	t_mask;
	 ulong	t_wlog;
	 uchar 	*t_bits;
	 uchar 	*p_dest;
	 long		gr_row;
   uchar *g_ltab;
	 fix		inv_dy;
	 long		*t_vtab;
	 
#if InvDiv
   inv_dy = fix_div(fix_make(1,0),tli->w);
	 u=fix_mul_asm_safe(tli->left.u,inv_dy); 
   du=fix_mul_asm_safe(tli->right.u,inv_dy)-u;
   v=fix_mul_asm_safe(tli->left.v,inv_dy); 
   dv=fix_mul_asm_safe(tli->right.v,inv_dy)-v;
   i=fix_mul_asm_safe(tli->left.i,inv_dy);
   di=fix_mul_asm_safe(tli->right.i,inv_dy)-i;
   if (di>=-256 && di<=256) i+=1024;
#else
	 u=fix_div(tli->left.u,tli->w); 
   du=fix_div(tli->right.u,tli->w)-u;
   v=fix_div(tli->left.v,tli->w); 
   dv=fix_div(tli->right.v,tli->w)-v;
   i=fix_div(tli->left.i,tli->w);
   di=fix_div(tli->right.i,tli->w)-i;
   if (di>=-256 && di<=256) i+=1024;
#endif

   dy=tli->right.y-tli->left.y;

	 w_l_mask_68K = t_mask = tli->mask;
	 w_l_wlog_68K = t_wlog = tli->bm.wlog;
   w_l_ltab = g_ltab = grd_screen->ltab;
	 t_vtab = tli->vtab;
	 t_bits = tli->bm.bits;
	 gr_row = grd_bm.row;

// handle PowerPC loop
#if (defined(powerc) || defined(__powerc))	
   do {      
      if ((d = fix_ceil(tli->right.y)-fix_ceil(tli->left.y)) > 0) {
      	 d =fix_ceil(tli->left.y)-tli->left.y;
         
#if InvDiv
         inv_dy = fix_div(fix_make(1,0)<<8,dy);
         di=fix_mul_asm_safe_light(di,inv_dy);
         inv_dy>>=8;
         du=fix_mul_asm_safe(du,inv_dy);
         dv=fix_mul_asm_safe(dv,inv_dy);
#else
         du=fix_div(du,dy);
         dv=fix_div(dv,dy);
         di=fix_div(di,dy);
#endif
         u+=fix_mul(du,d);
         v+=fix_mul(dv,d);
         i+=fix_mul(di,d);
			 	  
			   y = fix_cint(tli->right.y) - fix_cint(tli->left.y);
			 	 p_dest = grd_bm.bits + (gr_row*fix_cint(tli->left.y)) + tli->x;

         switch (tli->bm.hlog) {
         case GRL_OPAQUE:
            for (; y>0; y--) {
               k=t_vtab[fix_fint(v)]+fix_fint(u);
               *p_dest = g_ltab[t_bits[k]+fix_light(i)]; // gr_fill_upixel(g_ltab[t_bits[k]+fix_light(i)],t_x,y);
               p_dest += gr_row;	u+=du; v+=dv; i+=di;
            }
            break;
         case GRL_TRANS:
            for (; y>0; y--) {
               k=t_vtab[fix_fint(v)]+fix_fint(u);
               if (k=t_bits[k]) *p_dest = g_ltab[k+fix_light(i)];	// gr_fill_upixel(g_ltab[k+fix_light(i)],t_x,y);
               p_dest += gr_row;	u+=du; v+=dv; i+=di;
            }
            break;
         case GRL_OPAQUE|GRL_LOG2:
           for (; y>0; y--) {
               k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
               *p_dest = g_ltab[t_bits[k]+fix_light(i)];
               p_dest += gr_row;	u+=du; v+=dv; i+=di;
            }
            break;
         case GRL_TRANS|GRL_LOG2:
            for (; y>0; y--) {
               k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
               if (k=t_bits[k]) *p_dest = g_ltab[k+fix_light(i)];	// gr_fill_upixel(g_ltab[k+fix_light(i)],t_x,y);
               p_dest += gr_row;	u+=du; v+=dv; i+=di;
            }
            break;
         }
      } else if (d<0) return TRUE; /* punt this tmap */
      
      tli->w+=tli->dw;
 
 	// figure out new left u & v & i    
 			inv_dy = 0;
      k = tli->left.u+tli->left.du;
      y = tli->left.v+tli->left.dv;
      tli->left.i+=tli->left.di;

#if InvDiv
      inv_dy = fix_div(fix_make(1,0),tli->w);
      u=fix_mul_asm_safe(k,inv_dy); 
      v=fix_mul_asm_safe(y,inv_dy); 
      i=fix_mul_asm_safe(tli->left.i,inv_dy); 
	    if (di>=-256 && di<=256) i+=1024;
#else
      u=fix_div(k,tli->w); 
      v=fix_div(y,tli->w); 
      i=fix_div(tli->left.i,tli->w); 
	    if (di>=-256 && di<=256) i+=1024;
#endif

			tli->left.u = k;
			tli->left.v = y;

 	// figure out new right u & v & i    
      k = tli->right.u+tli->right.du;
      y = tli->right.v+tli->right.dv;
      tli->right.i+=tli->right.di;

#if InvDiv
      du=fix_mul_asm_safe(k,inv_dy)-u;
      dv=fix_mul_asm_safe(y,inv_dy)-v;
      di=fix_mul_asm_safe(tli->right.i,inv_dy)-i;
#else
      du=fix_div(k,tli->w)-u;
      dv=fix_div(y,tli->w)-v;
      di=fix_div(tli->right.i,tli->w)-i;
#endif
			tli->right.u = k;
			tli->right.v = y;
	      
      tli->left.y+=tli->left.dy;
      tli->right.y+=tli->right.dy;
      dy=tli->right.y-tli->left.y;
      tli->x++;
   } while (--(tli->n) > 0);
   
   return FALSE; /* tmap OK */

// handle 68K loops
#else
	 return(Handle_Wall_Lit_68K_Loop(u,v,du,dv,dy,tli,grd_bm.bits,t_bits,gr_row, i, di)); 
#endif
}

// Main 68K handler loop
#if !(defined(powerc) || defined(__powerc))	
asm int Handle_Wall_Lit_68K_Loop(fix u, fix v, fix du, fix dv, fix dy,
														 		 grs_tmap_loop_info *tli, uchar *start_pdest, uchar *t_bits, long gr_row,
														 		 fix i, fix di)
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
//   di=fix_div(di,dy);
//   u+=fix_mul(du,d);
//   v+=fix_mul(dv,d);
//   i+=fix_mul(di,d);
 
 	move.l	0x20(a0),d0				// tli->left.y
 	move.l	d0,d2
 	add.l		#0x0000FFFF,d2
 	clr.w		d2
 	sub.l		d0,d2							// d =fix_ceil(tli->left.y)-tli->left.y;
	
	fix_div_68k_d3(d6)			// inline function, returns result in d0
	move.l	d0,d6						//  du=fix_div(du,dy);
	
	fix_div_68k_d3(d7)			// inline function, returns result in d0
	move.l	d0,d7						//  dv=fix_div(dv,dy);
	
	move.l	104(sp),d0
	fix_div_68k_d3(d0)	// inline function, returns result in d0
	move.l	d0,104(sp)			//  di=fix_div(di,dy);	
	
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

	move.l	d7,a4						// save d7
	move.l	104(sp),d7
	move.l	d2,d0
	dc.l		0x4C070C01   		//  MULS.L    D7,D1:D0
	move.w	d1,d0
	swap		d0
	add.l		d0,100(sp)			// i+=fix_mul(di,d);
	move.l	a4,d7						// restore d7

// check for (di>=-256 && di<=256)
	move.l	104(sp),d0
 	cmpi.l   #0xFFFFFF00,d0
	BLT.S    @NoAdd
	CMPI.L   #0x00000100,d0
	BGT.S    @NoAdd      
	ADDI.L   #0x00000200,100(sp)
@NoAdd:

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

	// when we call the asm inner loop, a5 = t_bits, a4 = p_dest, d1 = gr_row
	// d0 = t_yr-t_yl, u,v,du,dv = d4-d7, a2 = i, a3 = di
	move.l	96(sp),d1
	move.l	92(sp),a5
	move.l	100(sp),a2
	move.l	104(sp),a3
	
	cmp.b		#(GRL_OPAQUE|GRL_LOG2),23(a0)		
	beq.s		@Opq
	jsr			wall_lit_trans_log2
	bra.s		@Skip
@Opq:
	jsr			wall_lit_opaque_log2
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
 	move.l	d0,0x48(a0)			// tli->right.u+=tli->right.du;
 	fix_div_68k_d2_d0(d6)		// inline function, returns result in d0
  sub.l		d4,d0
  move.l	d0,d6						//  du=fix_div(tli->right.u,tli->w)-u;
      
 	move.l	0x28(a0),d0
 	add.l		0x38(a0),d0	
 	move.l	d0,0x28(a0)			// tli->left.v+=tli->left.dv;
 	fix_div_68k_d2_d0(d5)		// inline function, returns result in d0
  move.l	d0,d5						// v=fix_div((tli->left.v+=tli->left.dv),tli->w);
     
 	move.l	0x4C(a0),d0
 	add.l		0x5C(a0),d0
 	move.l	d0,0x4C(a0)			// tli->right.v+=tli->right.dv;
 	fix_div_68k_d2_d0(d7)		// inline function, returns result in d0
 	sub.l		d5,d0
	move.l	d0,d7						//  dv=fix_div(tli->right.v,tli->w)-v;
	
 	move.l	0x2C(a0),d0
 	add.l		0x3C(a0),d0	
 	move.l	d0,0x2C(a0)			// tli->left.i+=tli->left.di;
 	fix_div_68k_d2_d0(d3)		// inline function, returns result in d0
  move.l	d0,100(sp)			// i=fix_div((tli->left.i+=tli->left.di),tli->w);
     
 	move.l	0x50(a0),d0
 	add.l		0x60(a0),d0
 	move.l	d0,0x50(a0)			// tli->right.i+=tli->right.di;
 	fix_div_68k_d2_d0(d3)		// inline function, returns result in d0
 	sub.l		100(sp),d0
	move.l	d0,104(sp)			//  di=fix_div(tli->right.i,tli->w)-i;

// check for (di>=-256 && di<=256)
 	cmpi.l   #0xFFFFFF00,d0
	BLT.S    @NoAdd2
	CMPI.L   #0x00000100,d0
	BGT.S    @NoAdd2     
	ADDI.L   #0x00000200,100(sp)
@NoAdd2:
	
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


// ========================================================================
// when we call the asm inner loops, a5 = t_bits, a4 = p_dest, 
// d0 = t_yr-t_yl,u,v,du,dv = d4-d7, a2 = i, a3 = di, d1 = gr_row
// can trash all other regs except the stack pointer (a7)

// handle inner loop for lit opaque (log2) wall mode
asm void wall_lit_opaque_log2(void)
 {
/*  for (y=t_yl; y<t_yr; y++) {
     int k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
     *p_dest = g_ltab[t_bits[k]+fix_light(i)]; // gr_fill_upixel(g_ltab[t_bits[k]+fix_light(i)],t_x,y);
     p_dest += gr_row;	u+=du; v+=dv; i+=di;
  }*/

	move.l	a0,-(sp)
	move.l	d1,a0
	move.l	w_l_mask_68K,d2
	move.l	w_l_wlog_68K,d3
	move.l	w_l_ltab,a6	
	move.l	d7,a1
	subq.w	#1,d0
	bmi.s		@Done

@Loop:
	move.l	d5,d1
	swap		d1
	ext.l		d1		 						// d1 = fix_fint(v)
	lsl.l		d3,d1							// d1 = (fix_fint(v)<<t_wlog
	move.l	d4,d7
	swap		d7
	ext.l		d7		 						// d7 = fix_fint(u)
	add.l		d7,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))
	and.l		d2,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask

	move.l	a2,d7
	lsr.l		#8,d7
//	swap		d7
//	lsl.w		#8,d7
	move.b	(a5,d1.l),d7
	move.b	(a6,d7.w),(a4)
	
	add.l		a0,a4							// p_dest += gr_row;
	add.l		d6,d4							// u+=du
	add.l		a1,d5							// v+=dv
	add.l		a3,a2							// i+=di
	dbra		d0,@Loop

@Done:
	move.l	(sp)+,a0
	rts	   
 }
 
// handle inner loop for lit transparent (log2) wall mode
asm void wall_lit_trans_log2(void)
 {
/*  for (y=t_yl; y<t_yr; y++) {
     int k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
     if (k=t_bits[k]) *p_dest = g_ltab[t_bits[k]+fix_light(i)];	// gr_fill_upixel(g_ltab[k+fix_light(i)],t_x,y);
     p_dest += gr_row;	u+=du; v+=dv; i+=di;
  }*/

	move.l	a0,-(sp)
	move.l	d1,a0
	move.l	w_l_mask_68K,d2
	move.l	w_l_wlog_68K,d3
	move.l	w_l_ltab,a6	
	move.l	d7,a1
	subq.w	#1,d0
	bmi.s		@Done

@Loop:
	move.l	d5,d1
	swap		d1
	ext.l		d1		 						// d1 = fix_fint(v)
	lsl.l		d3,d1							// d1 = (fix_fint(v)<<t_wlog
	move.l	d4,d7
	swap		d7
	ext.l		d7		 						// d7 = fix_fint(u)
	add.l		d7,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))
	and.l		d2,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask

	move.l	a2,d7
	lsr.l		#8,d7
//	swap		d7
//	lsl.w		#8,d7
	move.b	(a5,d1.l),d7
	tst.b		d7
	beq.s		@skippix
	move.b	(a6,d7.w),(a4)

@skippix:	
	add.l		a0,a4							// p_dest += gr_row;
	add.l		d6,d4							// u+=du
	add.l		a1,d5							// v+=dv
	add.l		a3,a2							// i+=di
	dbra		d0,@Loop

@Done:
	move.l	(sp)+,a0
	rts	   
 }
#endif


void gri_trans_lit_wall_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_TRANS|GRL_LOG2;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_TRANS;
   }
   tli->loop_func=(void (*)()) gri_lit_wall_umap_loop;
   tli->left_edge_func=(void (*)()) gri_uviwy_edge;
   tli->right_edge_func=(void (*)()) gri_uviwy_edge;
}

void gri_opaque_lit_wall_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_OPAQUE|GRL_LOG2;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_OPAQUE;
   }
   tli->loop_func=(void (*)()) gri_lit_wall_umap_loop;
   tli->left_edge_func=(void (*)()) gri_uviwy_edge;
   tli->right_edge_func=(void (*)()) gri_uviwy_edge;
}

extern "C"
{
extern int HandleWallLitLoop1D_PPC(grs_tmap_loop_info *tli,
																		fix u, fix v, fix i, fix dv, fix di, fix dy,
																		uchar *g_ltab, long *t_vtab, uchar *o_bits,
																		long gr_row, ulong t_mask, ulong t_wlog);
}
/*
int HandleWallLitLoop1D_C(grs_tmap_loop_info *tli,
													fix u, fix v, fix i, fix dv, fix di, fix dy,
													uchar *g_ltab, uchar *o_bits,
													long gr_row, ulong t_mask, ulong t_wlog)
 {
 	 fix 		d, inv_dy;
 	 register fix lefty, righty;
	 long	 	k,y;
	 uchar 	*t_bits;
	 uchar 	*p_dest;
 
   lefty = tli->left.y;
   righty = tli->right.y;
   do
   {   		     
      if ((d = fix_ceil(righty) - fix_ceil(lefty)) > 0)
      { 
      	 d =fix_ceil(lefty) - lefty;
         
         inv_dy = fix_div(fix_make(1,0)<<8,dy);    	
         dv=fix_mul_asm_safe(dv,inv_dy>>8);
         di=fix_mul_asm_safe_light(di,inv_dy);

         v+=fix_mul(dv,d);
         i+=fix_mul(di,d);
			 	 
			 	 if (di>=-256 && di<=256) i+=256;
			 	  
			   y = fix_cint(righty) - fix_cint(lefty);
			 	 p_dest = grd_bm.bits + (gr_row*fix_cint(lefty)) + tli->x;
				 t_bits = o_bits + fix_fint(u);
				 
				 // inner loop
         for (; y>0; y--) 
          {
           k=(fix_fint(v)<<t_wlog)&t_mask;
           *p_dest = g_ltab[t_bits[k]+fix_light(i)]; 
           p_dest += gr_row;	v+=dv; i+=di;
          }
          
      } else if (d<0) return TRUE; // punt this tmap 
      
      tli->w += tli->dw;
 
 	// figure out new left u & v & i    
      k = tli->left.u + tli->left.du;
      y = tli->left.v + tli->left.dv;
      tli->left.i += tli->left.di;

      inv_dy = fix_div(fix_make(1,0),tli->w);
      u=fix_mul_asm_safe(k,inv_dy); 
      v=fix_mul_asm_safe(y,inv_dy); 
      i=fix_mul_asm_safe(tli->left.i,inv_dy); 

			tli->left.u = k;
			tli->left.v = y;

 	// figure out new right u & v & i    
      k = tli->right.u + tli->right.du;
      y = tli->right.v + tli->right.dv;
      tli->right.i += tli->right.di;

      dv=fix_mul_asm_safe(y,inv_dy)-v;
      di=fix_mul_asm_safe(tli->right.i,inv_dy)-i;
			if (di>=-256 && di<=256) i+=1024;
		
			tli->right.u = k;
			tli->right.v = y;
	      
      lefty += tli->left.dy;
      righty += tli->right.dy;
      dy = righty - lefty;
      tli->x++;
   } while (--(tli->n) > 0);

	 tli->left.y = lefty;
	 tli->right.y = righty;
	 
   return FALSE; // tmap OK 
 }
*/

// ==================================================================================
// Wall_1D versions of routines
int gri_lit_wall_umap_loop_1D(grs_tmap_loop_info *tli) {
   fix u,v,i,dv,di,dy;

	 // locals used to store copies of tli-> stuff, so its in registers on the PPC
	 int		k,y;
	 ulong	t_mask;
	 ulong	t_wlog;
	 long		gr_row;
   uchar *g_ltab;
	 uchar 	*o_bits;
	 fix		inv_dy;
	 	 
#if InvDiv
   inv_dy = fix_div(fix_make(1,0),tli->w);
	 u=fix_mul_asm_safe(tli->left.u,inv_dy);    
   v=fix_mul_asm_safe(tli->left.v,inv_dy); 
   dv=fix_mul_asm_safe(tli->right.v,inv_dy)-v;
   i=fix_mul_asm_safe(tli->left.i,inv_dy);
   di=fix_mul_asm_safe(tli->right.i,inv_dy)-i;
	 if (di>=-256 && di<=256) i+=512;
#else
	 u=fix_div(tli->left.u,tli->w);    
   v=fix_div(tli->left.v,tli->w); 
   dv=fix_div(tli->right.v,tli->w)-v;
   i=fix_div(tli->left.i,tli->w);
   di=fix_div(tli->right.i,tli->w)-i;
	 if (di>=-256 && di<=256) i+=512;
#endif

   dy=tli->right.y-tli->left.y;

	 w_l_mask_68K = t_mask = tli->mask;
	 w_l_wlog_68K = t_wlog = tli->bm.wlog;
   w_l_ltab = g_ltab = grd_screen->ltab;
	 o_bits = tli->bm.bits;
	 gr_row = grd_bm.row;

// handle PowerPC loop
#if (defined(powerc) || defined(__powerc))	 
	return HandleWallLitLoop1D_PPC(tli, u, v, i, dv, di, dy, g_ltab, NULL, o_bits,
													 			 gr_row, t_mask, t_wlog);
// handle 68K loops
#else
	return(Handle_Wall_Lit_68K_Loop_1D(u,v,dv,dy,tli,grd_bm.bits,o_bits,gr_row, i, di));
#endif
}

// Main 68K handler loop
#if !(defined(powerc) || defined(__powerc))	
asm int Handle_Wall_Lit_68K_Loop_1D(fix u, fix v, fix dv, fix dy,
														 		 grs_tmap_loop_info *tli, uchar *start_pdest, uchar *t_bits, long gr_row,
														 		 fix i, fix di)
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
//   dv=fix_div(dv,dy);
//   di=fix_div(di,dy);
//   v+=fix_mul(dv,d);
//   i+=fix_mul(di,d);
 
 	move.l	0x20(a0),d0				// tli->left.y
 	move.l	d0,d2
 	add.l		#0x0000FFFF,d2
 	clr.w		d2
 	sub.l		d0,d2							// d =fix_ceil(tli->left.y)-tli->left.y;
		
	fix_div_68k_d3(d7)			// inline function, returns result in d0
	move.l	d0,d7						//  dv=fix_div(dv,dy);
	
	move.l	100(sp),d0
	fix_div_68k_d3(d0)			// inline function, returns result in d0
	move.l	d0,100(sp)			//  di=fix_div(di,dy);	
		    
	move.l	d2,d0
	dc.l		0x4C070C01   		//  MULS.L    D7,D1:D0
	move.w	d1,d0
	swap		d0
	add.l		d0,d5 					// v+=fix_mul(dv,d);

	move.l	d7,a4						// save d7
	move.l	100(sp),d7
	move.l	d2,d0
	dc.l		0x4C070C01   		//  MULS.L    D7,D1:D0
	move.w	d1,d0
	swap		d0
	add.l		d0,96(sp)			// i+=fix_mul(di,d);
	move.l	a4,d7						// restore d7

// check for (di>=-256 && di<=256)
	move.l	100(sp),d0
 	cmpi.l  #0xFFFFFF00,d0
	BLT.S   @NoAdd
	CMPI.L  #0x00000100,d0
	BGT.S   @NoAdd      
	ADDI.L  #0x00000200,96(sp)
@NoAdd:

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

	// when we call the asm inner loop, a5 = t_bits, a4 = p_dest, d1 = gr_row
	// d0 = t_yr-t_yl, u,v,du,dv = d4-d7, a2 = i, a3 = di
	move.l	92(sp),d1
	move.l	88(sp),a5
	move.l	d4,d6
	swap		d6
	add.w		d6,a5				 // add in u to t_bits
	
	move.l	96(sp),a2
	move.l	100(sp),a3
	
// ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
/*  for (y=t_yl; y<t_yr; y++) {
     int k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
     *p_dest = g_ltab[t_bits[k]+fix_light(i)]; // gr_fill_upixel(g_ltab[t_bits[k]+fix_light(i)],t_x,y);
     p_dest += gr_row;	u+=du; v+=dv; i+=di;
  }*/

	move.l	a0,-(sp)
	move.l	d1,a0
	move.l	w_l_mask_68K,d2
	move.l	w_l_wlog_68K,d3
	move.l	w_l_ltab,a6	
	move.l	d7,a1
	subq.w	#1,d0
	bmi.s		@IL_Done

@IL_Loop:
	move.l	d5,d1
	swap		d1
	ext.l		d1		 						// d1 = fix_fint(v)
	lsl.l		d3,d1							// d1 = (fix_fint(v)<<t_wlog
	and.l		d2,d1							// d1 = ((fix_fint(v)<<t_wlog))&t_mask

	move.l	a2,d7
	lsr.l		#8,d7
//	swap		d7
//	lsl.w		#8,d7
	move.b	(a5,d1.l),d7
	move.b	(a6,d7.w),(a4)
	
	add.l		a0,a4							// p_dest += gr_row;
	add.l		a1,d5							// v+=dv
	add.l		a3,a2							// i+=di
	dbra		d0,@IL_Loop

@IL_Done:
	move.l	(sp)+,a0
// ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
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
 	move.l	d0,0x48(a0)			// tli->right.u+=tli->right.du;
      
 	move.l	0x28(a0),d0
 	add.l		0x38(a0),d0	
 	move.l	d0,0x28(a0)			// tli->left.v+=tli->left.dv;
 	fix_mul_d2_d0(d5)				// v=fix_div((tli->left.v+=tli->left.dv),tli->w);
     
 	move.l	0x4C(a0),d0
 	add.l		0x5C(a0),d0
 	move.l	d0,0x4C(a0)			// tli->right.v+=tli->right.dv;
 	fix_mul_d2_d0(d7)				
	sub.l		d5,d7						// dv=fix_div(tli->right.v,tli->w)-v;
	
 	move.l	0x2C(a0),d0
 	add.l		0x3C(a0),d0	
 	move.l	d0,0x2C(a0)			// tli->left.i+=tli->left.di;
 	fix_mul_d2_d0(d3)				
  move.l	d3,96(sp)				// i=fix_div((tli->left.i+=tli->left.di),tli->w);
     
 	move.l	0x50(a0),d0
 	add.l		0x60(a0),d0
 	move.l	d0,0x50(a0)			// tli->right.i+=tli->right.di;
 	fix_mul_d2_d0(d3)				
 	sub.l		96(sp),d3
	move.l	d3,100(sp)			//  di=fix_div(tli->right.i,tli->w)-i;

// check for (di>=-256 && di<=256)
 	cmpi.l  #0xFFFFFF00,d3
	BLT.S   @NoAdd2
	CMPI.L  #0x00000100,d3
	BGT.S   @NoAdd2     
	ADDI.L  #0x00000200,96(sp)
@NoAdd2:
	
 	move.l	0x30(a0),d0			// tli->left.y+=tli->left.dy;
 	add.l		d0,0x20(a0)

 	move.l	0x54(a0),d0
 	add.l		d0,0x44(a0)			// tli->right.y+=tli->right.dy;
     
 	move.l	0x44(a0),d3
 	sub.l		0x20(a0),d3			// dy=tli->right.y-tli->left.y;

	addq.l	#1,4(a0)				// tli->x++;
  		 
	subq.l	#1,(a0)
	bgt 		@DoLoop					//	} while (--(tli->n) > 0);

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



void gri_opaque_lit_wall1d_umap_init(grs_tmap_loop_info *tli)
 {
// Wall1D is always log2
/*   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {*/
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_OPAQUE|GRL_LOG2;
/*   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_OPAQUE;
   }*/
   tli->loop_func=(void (*)()) gri_lit_wall_umap_loop_1D;
   tli->left_edge_func=(void (*)()) gri_uviwy_edge;
   tli->right_edge_func=(void (*)()) gri_uviwy_edge;
 }
 