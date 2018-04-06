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
 * $Source: r:/prj/lib/src/2d/RCS/FL8F.c $
 * $Revision: 1.4 $
 * $Author: kevin $
 * $Date: 1994/08/16 12:50:13 $
 *
 * Routines to floor texture map a flat8 bitmap to a generic canvas.
 *
 * This file is part of the 2d library.
 *
 */

#include "tmapint.h"
#include "gente.h"
#include "grpix.h"
#include "poly.h"
#include "vtab.h"
#include "fl8tf.h"
#include "cnvdat.h"
#include "grnull.h"
#include "2dDiv.h"
#include "fl8tmapdv.h"


int gri_floor_umap_loop(grs_tmap_loop_info *tli);

// 68K stuff
#if !(defined(powerc) || defined(__powerc))	
// main loop routine
asm int Handle_Floor_68K_Loop(fix u, fix v, fix du, fix dv, fix dx,
															grs_tmap_loop_info *tli, uchar *start_pdest, uchar *t_bits, long gr_row);

// specific inner looops
asm void floor_opaque_log2(void);
asm void floor_opaque_clut_log2(void);
asm void floor_trans_log2(void);
asm void floor_trans_clut_log2(void);

// jump table for 68k inner loops
void (*floor_in_loop[8])(void) = {gr_not_imp, gr_not_imp, floor_opaque_log2, floor_trans_log2, 
																	gr_not_imp,gr_not_imp, floor_opaque_clut_log2,floor_trans_clut_log2};
#endif
																
// globals used by 68K routines
uchar *f_clut_68K;
ulong	f_wlog_68K;
ulong	f_mask_68K;


int gri_floor_umap_loop(grs_tmap_loop_info *tli) {
   fix u,v,du,dv,dx,d;

	// locals used to store copies of tli-> stuff, so its in registers on the PPC
	uchar	t_wlog;
	ulong	t_mask;
	int 	x,k;
	uchar *t_bits;
	uchar *p_dest;
	fix		inv;
	uchar *t_clut;
	uchar temp_pix;
	long	*t_vtab;
	
#if InvDiv
  inv = fix_div(fix_make(1,0),tli->w);
	u=fix_mul_asm_safe(tli->left.u,inv);
	du=fix_mul_asm_safe(tli->right.u,inv)-u;
	v=fix_mul_asm_safe(tli->left.v,inv); 
	dv=fix_mul_asm_safe(tli->right.v,inv)-v;
#else
	u=fix_div(tli->left.u,tli->w);
	du=fix_div(tli->right.u,tli->w)-u;
	v=fix_div(tli->left.v,tli->w); 
	dv=fix_div(tli->right.v,tli->w)-v;
#endif

	dx=tli->right.x-tli->left.x;

	f_clut_68K = t_clut = tli->clut;
	f_mask_68K = t_mask = tli->mask;
	f_wlog_68K = t_wlog = tli->bm.wlog;
	t_vtab = tli->vtab;
	t_bits = tli->bm.bits;

// handle PowerPC loop
#if (defined(powerc) || defined(__powerc))	
   do {
      if ((d = fix_ceil(tli->right.x)-fix_ceil(tli->left.x)) > 0) {
         d =fix_ceil(tli->left.x)-tli->left.x;

#if InvDiv
  			 inv = fix_div(fix_make(1,0),dx);
         du=fix_mul_asm_safe(du,inv);	
         dv=fix_mul_asm_safe(dv,inv);	
#else
         du=fix_div(du,dx);	
         dv=fix_div(dv,dx);	
#endif

         u+=fix_mul(du,d);
         v+=fix_mul(dv,d);

			   // copy out tli-> stuff into locals
				 p_dest = grd_bm.bits + (grd_bm.row*tli->y) + fix_cint(tli->left.x);
			   x = fix_cint(tli->right.x) - fix_cint(tli->left.x);

         switch (tli->bm.hlog) {
         case GRL_OPAQUE:
            for (; x>0; x--) {
               k=t_vtab[fix_fint(v)]+fix_fint(u);
               *(p_dest++) = t_bits[k];		// gr_fill_upixel(t_bits[k],x,t_y);
               u+=du; v+=dv;
            }
            break;
         case GRL_TRANS:
            for (; x>0; x--) {
               k=t_vtab[fix_fint(v)]+fix_fint(u);
	           	 if (temp_pix=t_bits[k]) *p_dest = temp_pix;		// gr_fill_upixel(t_bits[k],x,t_y);
               p_dest++; u+=du; v+=dv;
            }
            break;
         case GRL_OPAQUE|GRL_LOG2:
            for (; x>0; x--) {
               k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
               *(p_dest++) = t_bits[k];		// gr_fill_upixel(t_bits[k],x,t_y);
               u+=du; v+=dv;
            }
            break;
         case GRL_TRANS|GRL_LOG2:
            for (; x>0; x--) {
               k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
	           	 if (temp_pix=t_bits[k]) *p_dest = temp_pix;		// gr_fill_upixel(t_bits[k],x,t_y);
               p_dest++; u+=du; v+=dv;
            }
            break;
         case GRL_OPAQUE|GRL_CLUT:
            for (; x>0; x--) {
               k=t_vtab[fix_fint(v)]+fix_fint(u);
	           	 *(p_dest++) = t_clut[t_bits[k]];		// gr_fill_upixel(tli->clut[t_bits[k]],x,t_y);
               u+=du; v+=dv;
            }
            break;
         case GRL_TRANS|GRL_CLUT:
            for (; x>0; x--) {
               k=t_vtab[fix_fint(v)]+fix_fint(u);
	           	 if (k=t_bits[k]) *p_dest = t_clut[k];		// gr_fill_upixel(tli->clut[k],x,t_y);
               p_dest++; u+=du; v+=dv;
            }
            break;
         case GRL_OPAQUE|GRL_LOG2|GRL_CLUT:
         			 while ((long) p_dest & 3 != 0)
         			  {
	               k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
		           	 *(p_dest++) = t_clut[t_bits[k]];		// gr_fill_upixel(tli->clut[t_bits[k]],x,t_y);
	               u+=du; v+=dv;
	               x--;
         			  }
         			  
            	 while (x>=4)
            	  {
	               k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
		           	 inv = t_clut[t_bits[k]];		// gr_fill_upixel(tli->clut[t_bits[k]],x,t_y);
	               u+=du; v+=dv;
								 inv <<=8;
								 
	               k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
		           	 inv |= t_clut[t_bits[k]];		// gr_fill_upixel(tli->clut[t_bits[k]],x,t_y);
	               u+=du; v+=dv;
								 inv <<=8;

	               k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
		           	 inv |= t_clut[t_bits[k]];		// gr_fill_upixel(tli->clut[t_bits[k]],x,t_y);
	               u+=du; v+=dv;
								 inv <<=8;

	               k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
		           	 inv |= t_clut[t_bits[k]];		// gr_fill_upixel(tli->clut[t_bits[k]],x,t_y);
	               u+=du; v+=dv;

								 * (long *) p_dest = inv;
								 x -= 4;
								 p_dest += 4;
            	  }
            	 
            for (; x>0; x--) {
               k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
	           	 *(p_dest++) = t_clut[t_bits[k]];		// gr_fill_upixel(tli->clut[t_bits[k]],x,t_y);
               u+=du; v+=dv;
            }
            break;
         case GRL_TRANS|GRL_LOG2|GRL_CLUT:
            for (; x>0; x--) {
               k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
	           	 if (k=t_bits[k]) *p_dest = t_clut[k];		// gr_fill_upixel(tli->clut[k],x,t_y);
               p_dest++; u+=du; v+=dv;
            }
         }
      } else if (d<0) return TRUE; /* punt this tmap */
      
      tli->w+=tli->dw;

#if InvDiv
  		inv = fix_div(fix_make(1,0),tli->w);
      u=fix_mul_asm_safe((tli->left.u+=tli->left.du),inv); 
      tli->right.u+=tli->right.du;
      du=fix_mul_asm_safe(tli->right.u,inv)-u;
      v=fix_mul_asm_safe((tli->left.v+=tli->left.dv),inv); 
      tli->right.v+=tli->right.dv;
      dv=fix_mul_asm_safe(tli->right.v,inv)-v;
#else
      u=fix_div((tli->left.u+=tli->left.du),tli->w); 
      tli->right.u+=tli->right.du;
      du=fix_div(tli->right.u,tli->w)-u;
      v=fix_div((tli->left.v+=tli->left.dv),tli->w); 
      tli->right.v+=tli->right.dv;
      dv=fix_div(tli->right.v,tli->w)-v;
#endif
            
      tli->left.x+=tli->left.dx;
      tli->right.x+=tli->right.dx;
      dx=tli->right.x-tli->left.x;
      tli->y++;
   } while (--(tli->n) > 0);
   return FALSE; /* tmap OK */
// handle 68K loops
#else
	return(Handle_Floor_68K_Loop(u,v,du,dv,dx,tli,grd_bm.bits,t_bits,grd_bm.row));
#endif
}
	
// Main 68K handler loop
#if !(defined(powerc) || defined(__powerc))	
asm int Handle_Floor_68K_Loop(fix u, fix v, fix du, fix dv, fix dx,
												grs_tmap_loop_info *tli, uchar *start_pdest, uchar *t_bits, long gr_row)
 {
  movem.l	d0-d7/a0-a6,-(sp)
	
// load up vars
	move.l	64(sp),d4			// u
	move.l	68(sp),d5			// v
	move.l	72(sp),d6			// du
	move.l	76(sp),d7			// dv
	move.l	80(sp),d3			// dx
 	move.l	84(sp),a0			// *tli
	
@DoLoop: 	
//	  if ((d = fix_ceil(tli->right.x)-fix_ceil(tli->left.x)) > 0) 
 	move.l	RightX(a0),d0
 	add.l		#0x0000FFFF,d0
 	clr.w		d0									// fix_ceil(tli->right.x)
 	move.l	LeftX(a0),d1
 	add.l		#0x0000FFFF,d1
 	clr.w		d1									// fix_ceil(tli->left.x)
 	sub.l		d1,d0								// fix_ceil(tli->right.x)-fix_ceil(tli->left.x))
 	bmi 		@Err								// d<0, punt
 	beq 		@Skip								// d==0, skip this map

//   d =fix_ceil(tli->left.x)-tli->left.x;
//   du=fix_div(du,dx);
//   dv=fix_div(dv,dx);
//   u+=fix_mul(du,d);
//   v+=fix_mul(dv,d);
 
	move.l	LeftX(a0),d2		// tli->left.x
 	move.l	d2,d1
 	add.l		#0x0000FFFF,d2
 	clr.w		d2
 	sub.l		d1,d2							// d =fix_ceil(tli->left.x)-tli->left.x;

	fix_div_68k_d3(d6)			// inline function, returns result in d0
	move.l	d0,d6						//  du=fix_div(du,dx);
	
	fix_div_68k_d3(d7)			// inline function, returns result in d0
	move.l	d0,d7						//  dv=fix_div(dv,dx);
	
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
	 	
// t_xl = fix_cint(tli->left.x);
// t_xr = fix_cint(tli->right.x);
 	move.l	LeftX(a0),d1
 	add.l		#0x0000FFFF,d1
 	clr.w		d1
 	swap		d1

 	move.l	RightX(a0),d0
 	add.l		#0x0000FFFF,d0
 	clr.w		d0
 	swap		d0
 	sub.l		d1,d0			// t_xr-t_xl

// p_dest = grd_bm.bits + (gr_row*t_yl) + t_x;
	move.l	T_Y(a0),d2
	dc.w		0x4C2F,0x2000,0x0060   //  MULU.L    $0060(A7),D2
	add.l		88(sp),d2
	move.l	d2,a4
	add.l		d1,a4
 	
	// when we call the asm inner loop, a5 = t_bits, a4 = p_dest, 
	// d0 = t_xr-t_xl, u,v,du,dv = d4-d7
	//
	move.l	92(sp),a5
	lea			floor_in_loop,a1
	moveq		#0,d1
	move.b	HLog(a0),d1
	dc.l		0x22711400      //  MOVEA.L   $00(A1,D1.W*4),A1
	jsr			(a1)
@Skip:

//      tli->w+=tli->dw;
//      u=fix_div((tli->left.u+=tli->left.du),tli->w);
//      tli->right.u+=tli->right.du;
//      du=fix_div(tli->right.u,tli->w)-u;
//      v=fix_div((tli->left.v+=tli->left.dv),tli->w);
//      tli->right.v+=tli->right.dv;
//      dv=fix_div(tli->right.v,tli->w)-v;
//      tli->left.x+=tli->left.dx;
//      tli->right.x+=tli->right.dx;
//      dx=tli->right.x-tli->left.x;
//      tli->y++;

	move.l	T_W(a0),d2
  add.l		T_DW(a0),d2			// tli->w+=tli->dw;
  move.l	d2,T_W(a0)
  
 	move.l	LeftDU(a0),d0			// tli->left.du
 	add.l	  LeftU(a0),d0			// tli->left.u
 	move.l	d0,LeftU(a0)
 	fix_div_68k_d2_d0(d4)		// inline function, returns result in d0
	move.l	d0,d4						// u=fix_div((tli->left.u+=tli->left.du),tli->w);
	
 	move.l	RightU(a0),d0
 	add.l		RightDU(a0),d0
 	move.l	d0,RightU(a0)		// tli->right.u+=tli->right.du;
 	fix_div_68k_d2_d0(d6)		// inline function, returns result in d0
  sub.l		d4,d0
  move.l	d0,d6						//  du=fix_div(tli->right.u,tli->w)-u;
      
 	move.l	LeftV(a0),d0
 	add.l		LeftDV(a0),d0	
 	move.l	d0,LeftV(a0)	// tli->left.v+=tli->left.dv;
 	fix_div_68k_d2_d0(d5)	// inline function, returns result in d0
  move.l	d0,d5					// v=fix_div((tli->left.v+=tli->left.dv),tli->w);
     
 	move.l	RightV(a0),d0
 	add.l		RightDV(a0),d0
 	move.l	d0,RightV(a0)		// tli->right.v+=tli->right.dv;
 	fix_div_68k_d2_d0(d7)		// inline function, returns result in d0
 	sub.l		d5,d0	
	move.l	d0,d7						//  dv=fix_div(tli->right.v,tli->w)-v;
	
 	move.l	LeftDX(a0),d0		// tli->left.x+=tli->left.dx;
 	add.l		d0,LeftX(a0)

	move.l	RightX(a0),d0
 	add.l		RightDX(a0),d0
 	move.l	d0,RightX(a0)		// tli->right.x+=tli->right.dx;
     
 	move.l	d0,d3
 	sub.l		LeftX(a0),d3		// dx=tli->right.x-tli->left.x;

	addq.l	#1,T_Y(a0)			// tli->y++;
  		 
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
 
// handle inner loop for floor opaque (width log2) mode
asm void floor_opaque_log2(void)
 {
/*  for (x=t_xl; x<t_xr; x++) {
     int k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
     *(p_dest++) = t_bits[k];		// gr_fill_upixel(t_bits[k],x,t_y);
     u+=du; v+=dv;
  }*/
  
	move.l	f_mask_68K,d2
	move.l	f_wlog_68K,d3
	move.l	d7,a3
	move.l	d6,a2
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
	move.b	(a5,d1.l),(a4)+		// *(p_dest++) = t_bits[k];
	add.l		a2,d4							// u+=du
	add.l		a3,d5							// v+=dv
	dbra		d0,@Loop
@Done:		
	rts	   
 }
 
asm void floor_opaque_clut_log2(void)
 {
/*
	for (x=t_xl; x<t_xr; x++) {
	   int k=t_vtab[fix_fint(v)]+fix_fint(u);
	   *(p_dest++) = t_clut[t_bits[k]];		// gr_fill_upixel(tli->clut[t_bits[k]],x,y);
	   u+=du; v+=dv;
	}*/
	
	move.l	f_clut_68K,a1
	move.l	f_mask_68K,d2
	move.l	f_wlog_68K,d3
	move.l	d7,a3
	move.l	d6,a2
	move.l	d4,a6
	subq.w	#1,d0
	bmi.s		@Done

	moveq		#16,d6
	moveq		#0,d4
@Loop:
	move.l	d5,d1
	asr.l		d6,d1 						// d1 = fix_fint(v)
	lsl.l		d3,d1							// d1 = (fix_fint(v)<<t_wlog
	move.l	a6,d7
	asr.l		d6,d7 						// d7 = fix_fint(u)
	add.l		d7,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))
	and.l		d2,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask
	move.b	(a5,d1.l),d4
	move.b	(a1,d4.w),(a4)+		// *(p_dest++) = t_bits[k];
	add.l		a2,a6							// u+=du
	add.l		a3,d5							// v+=dv
	dbra		d0,@Loop

@Done:		
	rts	   
 }
 

asm void floor_trans_log2(void)
 {
/* 
  for (x=t_xl; x<t_xr; x++) {
     int k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
     if (temp_pix=t_bits[k]) *p_dest = temp_pix;		// gr_fill_upixel(t_bits[k],x,y);
     p_dest++; u+=du; v+=dv;
  }*/
	
	move.l	f_mask_68K,d2
	move.l	f_wlog_68K,d3
	move.l	d7,a3
	move.l	d6,a2
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
	move.b	(a5,d1.l),d7
	beq.s		@skippix
	move.b	d7,(a4)+					// *(p_dest++) = t_bits[k];
	add.l		a2,d4							// u+=du
	add.l		a3,d5							// v+=dv
	dbra		d0,@Loop
	bra.s		@End

@skippix:
	addq.w	#1,a4
	add.l		a2,d4							// u+=du
	add.l		a3,d5							// v+=dv
	dbra		d0,@Loop

@End:
@Done:		
	rts	   
 }
 

asm void floor_trans_clut_log2(void)
 {
/* 
  for (x=t_xl; x<t_xr; x++) {
     int k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
     if (k=t_bits[k]) *p_dest = t_clut[k];		// gr_fill_upixel(tli->clut[k],x,y);
     p_dest++; u+=du; v+=dv;
  }*/
	
	move.l	f_mask_68K,d2
	move.l	f_wlog_68K,d3
	move.l	f_clut_68K,a6
	move.l	d7,a3
	move.l	d6,a2
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
	move.b	(a6,d7.w),(a4)+		// *(p_dest++) = t_clut[t_bits[k]];
	add.l		a2,d4							// u+=du
	add.l		a3,d5							// v+=dv
	dbra		d0,@Loop
	bra.s		@End

@skippix:
	addq.w	#1,a4
	add.l		a2,d4							// u+=du
	add.l		a3,d5							// v+=dv
	dbra		d0,@Loop

@End:
@Done:		
	rts	   
 }
 
// handle inner loop for opaque (width log2) mode
#endif


void gri_trans_floor_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_TRANS|GRL_LOG2;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_TRANS;
   }
   tli->loop_func=(void (*)()) gri_floor_umap_loop;
   tli->left_edge_func=(void (*)()) gri_uvwx_edge;
   tli->right_edge_func=(void (*)()) gri_uvwx_edge;
}

void gri_opaque_floor_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_OPAQUE|GRL_LOG2;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_OPAQUE;
   }
   tli->loop_func=(void (*)()) gri_floor_umap_loop;
   tli->left_edge_func=(void (*)()) gri_uvwx_edge;
   tli->right_edge_func=(void (*)()) gri_uvwx_edge;
}

void gri_trans_clut_floor_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_TRANS|GRL_LOG2|GRL_CLUT;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_TRANS|GRL_CLUT;
   }
   tli->loop_func=(void (*)()) gri_floor_umap_loop;
   tli->left_edge_func=(void (*)()) gri_uvwx_edge;
   tli->right_edge_func=(void (*)()) gri_uvwx_edge;
}

void gri_opaque_clut_floor_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_OPAQUE|GRL_LOG2|GRL_CLUT;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_OPAQUE|GRL_CLUT;
   }
   tli->loop_func=(void (*)()) gri_floor_umap_loop;
   tli->left_edge_func=(void (*)()) gri_uvwx_edge;
   tli->right_edge_func=(void (*)()) gri_uvwx_edge;
}

