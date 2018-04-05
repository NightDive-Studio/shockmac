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
 * $Source: r:/prj/lib/src/2d/RCS/genl.c $
 * $Revision: 1.4 $
 * $Author: kevin $
 * $Date: 1994/08/16 12:48:26 $
 *
 * Routines to linearly texture map a flat8 bitmap to a generic canvas.
 *
 * This file is part of the 2d library.
 *
 */

#include "tmapint.h"
#include "poly.h"
#include "grpix.h"
#include "gente.h"
#include "vtab.h"
#include "fl8tf.h"
#include "cnvdat.h"
#include "2dDiv.h"
#include "fl8tmapdv.h"

// prototypes
int gri_lin_umap_loop(grs_tmap_loop_info *tli);

// 68K stuff
#if !(defined(powerc) || defined(__powerc))	
// main loop routine
asm int Handle_68K_Loop(fix u, fix v, fix du, fix dv, fix dx,
												grs_tmap_loop_info *tli, uchar *start_pdest, uchar *t_bits, long gr_row);

// specific inner looops
asm void lin_opaque(void);
asm void lin_opaque_log2(void);
asm void lin_opaque_clut(void);
asm void lin_opaque_clut_log2(void);
asm void lin_trans(void);
asm void lin_trans_log2(void);
asm void lin_trans_clut(void);
asm void lin_trans_clut_log2(void);

// jump table for 68k inner loops
void (*lin_in_loop[8])(void) = {lin_opaque, lin_trans, lin_opaque_log2, lin_trans_log2, 
																lin_opaque_clut,lin_trans_clut, lin_opaque_clut_log2,lin_trans_clut_log2};
#endif
																
// globals used by 68K routines
long	*l_vtab_68K;
uchar *l_clut_68K;
ulong	l_wlog_68K;
ulong	l_mask_68K;

extern "C"
{
int Handle_LinClut_Loop_PPC(fix u, fix v, fix du, fix dv, fix dx,
														grs_tmap_loop_info *tli, uchar *start_pdest, uchar *t_bits, long gr_row, uchar *t_clut,
														uchar	t_wlog, ulong t_mask);
}
/*
int Handle_LinClut_Loop_C(fix u, fix v, fix du, fix dv, fix dx,
													grs_tmap_loop_info *tli, uchar *start_pdest, uchar *t_bits, long gr_row, uchar *t_clut,
													uchar	t_wlog, ulong t_mask)
 {
	register int 	x,k;
	uchar 				*p_dest;
	register fix	rx,lx;
	
	rx = tli->right.x;
	lx = tli->left.x;
	tli->y += tli->n;
	
	do {
	  if ((x = fix_ceil(rx)-fix_ceil(lx)) > 0) 
	    {
	     x =fix_ceil(lx)-lx;
	     
       k = fix_div(fix_make(1,0),dx);
       du=fix_mul_asm_safe(du,k);
       dv=fix_mul_asm_safe(dv,k);

	     u+=fix_mul(du,x);
	     v+=fix_mul(dv,x);
		
		   // copy out tli-> stuff into locals
			 p_dest = start_pdest + fix_cint(lx);
			 x = fix_cint(rx) - fix_cint(lx);
			 
      for (; x>0; x--) 
       {
         *(p_dest++) = t_clut[t_bits[((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask]];		
         u+=du; v+=dv;
       }
	  } else if (x<0) return TRUE; // punt this tmap 
	  
	  u=(tli->left.u+=tli->left.du);
	  tli->right.u+=tli->right.du;
	  du=tli->right.u-u;
	  v=(tli->left.v+=tli->left.dv);
	  tli->right.v+=tli->right.dv;
	  dv=tli->right.v-v;
	  lx+=tli->left.dx;
	  rx+=tli->right.dx;
	  dx=rx-lx;
		start_pdest += gr_row;
	} while (--(tli->n) > 0);

	tli->right.x = rx;
	tli->left.x = lx;

	return FALSE; // tmap OK 
 }*/
								

int gri_lin_umap_loop(grs_tmap_loop_info *tli) {
	fix u,v,du,dv,dx,d;

	// locals used to store copies of tli-> stuff, so its in registers on the PPC
	register int 	x,k;
	uchar *p_dest;
	uchar temp_pix;
	long	*t_vtab;
	uchar *t_bits;
	uchar *t_clut;
	uchar	t_wlog;
	ulong	t_mask;
	long	gr_row;
	uchar *start_pdest;
	long	inv;
								
	u=tli->left.u;
	du=tli->right.u-u;
	v=tli->left.v;
	dv=tli->right.v-v;
	dx=tli->right.x-tli->left.x;

	l_vtab_68K = t_vtab = tli->vtab;
	l_clut_68K = t_clut = tli->clut;
	l_mask_68K = t_mask = tli->mask;
	l_wlog_68K = t_wlog = tli->bm.wlog;

	t_bits = tli->bm.bits;
	gr_row = grd_bm.row;
	start_pdest = grd_bm.bits + (gr_row*(tli->y));

// handle PowerPC loop
#if (defined(powerc) || defined(__powerc))	
	if (tli->bm.hlog == (GRL_OPAQUE|GRL_LOG2|GRL_CLUT))
		return(Handle_LinClut_Loop_PPC(u,v,du,dv,dx,tli,start_pdest,t_bits,gr_row,t_clut,t_wlog,t_mask));

	do {
	  if ((d = fix_ceil(tli->right.x)-fix_ceil(tli->left.x)) > 0) 
	    {
	     d =fix_ceil(tli->left.x)-tli->left.x;
	     
#if InvDiv
       k = fix_div(fix_make(1,0),dx);
       du=fix_mul_asm_safe(du,k);
       dv=fix_mul_asm_safe(dv,k);
#else
       du=fix_div(du,dx);
       dv=fix_div(dv,dx);
#endif
	     u+=fix_mul(du,d);
	     v+=fix_mul(dv,d);
		
		   // copy out tli-> stuff into locals
			 p_dest = start_pdest + fix_cint(tli->left.x);
			 x = fix_cint(tli->right.x) - fix_cint(tli->left.x);
			 
	     switch (tli->bm.hlog) {
	     case GRL_OPAQUE:
	        for (; x>0; x--) {
	           k=t_vtab[fix_fint(v)]+fix_fint(u);
	           *(p_dest++) = t_bits[k];		// gr_fill_upixel(t_bits[k],x,y);
	           u+=du; v+=dv;
	        }
	        break;
	     case GRL_TRANS:
	        for (; x>0; x--) {
	           k=t_vtab[fix_fint(v)]+fix_fint(u);
	           if (temp_pix=t_bits[k]) *p_dest = temp_pix;		// gr_fill_upixel(t_bits[k],x,y);
             p_dest++; u+=du; v+=dv;
	        }
	        break;
	     case GRL_OPAQUE|GRL_LOG2:
	        for (; x>0; x--) {
	           k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
	           *(p_dest++) = t_bits[k];		// gr_fill_upixel(t_bits[k],x,y);
	           u+=du; v+=dv;
	        }
	        break;
	     case GRL_TRANS|GRL_LOG2:
	        for (; x>0; x--) {
	           k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
	           if (temp_pix=t_bits[k]) *p_dest = temp_pix;		// gr_fill_upixel(t_bits[k],x,y);
             p_dest++; u+=du; v+=dv;
	        }
	        break;
	     case GRL_OPAQUE|GRL_CLUT:
	        for (; x>0; x--) {
	           k=t_vtab[fix_fint(v)]+fix_fint(u);
	           *(p_dest++) = t_clut[t_bits[k]];		// gr_fill_upixel(tli->clut[t_bits[k]],x,y);
	           u+=du; v+=dv;
	        }
	        break;
	     case GRL_TRANS|GRL_CLUT:
	        for (; x>0; x--) {
	           k=t_vtab[fix_fint(v)]+fix_fint(u);
	           if (k=t_bits[k]) *p_dest = t_clut[k];		// gr_fill_upixel(tli->clut[k],x,y);
             p_dest++; u+=du; v+=dv;
	        }
	        break;
// handled in special case now
	/*     case GRL_OPAQUE|GRL_LOG2|GRL_CLUT:
	        for (; x>0; x--) {
	           k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
	           *(p_dest++) = t_clut[t_bits[k]];		// gr_fill_upixel(tli->clut[t_bits[k]],x,y);
	           u+=du; v+=dv;
	        }
	        break;*/
	     case GRL_TRANS|GRL_LOG2|GRL_CLUT:
	        for (; x>0; x--) {
	           k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
	           if (k=t_bits[k]) *p_dest = t_clut[k];		// gr_fill_upixel(tli->clut[k],x,y);
             p_dest++; u+=du; v+=dv;
	        }
	        break;
	     }
	  } else if (d<0) return TRUE; /* punt this tmap */
	  
	  u=(tli->left.u+=tli->left.du);
	  tli->right.u+=tli->right.du;
	  du=tli->right.u-u;
	  v=(tli->left.v+=tli->left.dv);
	  tli->right.v+=tli->right.dv;
	  dv=tli->right.v-v;
	  tli->left.x+=tli->left.dx;
	  tli->right.x+=tli->right.dx;
	  dx=tli->right.x-tli->left.x;
	  tli->y++;
		start_pdest += gr_row;
	} while (--(tli->n) > 0);
	return FALSE; /* tmap OK */

// handle 68K loops
#else
	return(Handle_68K_Loop(u,v,du,dv,dx,tli,start_pdest,t_bits,gr_row));
#endif
}
 
// Main 68K handler loop
#if !(defined(powerc) || defined(__powerc))	
asm int Handle_68K_Loop(fix u, fix v, fix du, fix dv, fix dx,
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

// p_dest = start_pdest + t_xl;
	move.l	88(sp),a4
	add.l		d1,a4	
 	
	// when we call the asm inner loop, a5 = t_bits, a4 = p_dest, 
	// d0 = t_xr-t_xl, u,v,du,dv = d4-d7
	//
	move.l	92(sp),a5
	lea			lin_in_loop,a1
	moveq		#0,d1
	move.b	23(a0),d1
	dc.l		0x22711400      //  MOVEA.L   $00(A1,D1.W*4),A1
	jsr			(a1)
@Skip:

//	  u=(tli->left.u+=tli->left.du);
//	  tli->right.u+=tli->right.du;
//	  du=tli->right.u-u;
//	  v=(tli->left.v+=tli->left.dv);
//	  tli->right.v+=tli->right.dv;
//	  dv=tli->right.v-v;
//	  tli->left.x+=tli->left.dx;
//	  tli->right.x+=tli->right.dx;
//	  dx=tli->right.x-tli->left.x;
//	  tli->y++;

// now dx = d3, u,v,du,dv = d4-d7, a0= *tli
 	
 	move.l	LeftDU(a0),d0
 	move.l	LeftU(a0),d1
 	add.l		d0,d1
 	move.l	d1,LeftU(a0)
 	move.l	d1,d4				  // *u=(tli->left.u+=tli->left.du);
 	
 	move.l	RightU(a0),d0
 	add.l		RightDU(a0),d0
 	move.l	d0,RightU(a0)		// tli->right.u+=tli->right.du;
 	sub.l		d1,d0
 	move.l	d0,d6					// *du=tli->right.u-*u;
 	
 	move.l	LeftV(a0),d1
 	add.l		LeftDV(a0),d1
 	move.l	d1,LeftV(a0)
 	move.l	d1,d5					// *v=(tli->left.v+=tli->left.dv);
 	
 	move.l	RightV(a0),d0
 	add.l		RightDV(a0),d0
 	move.l	d0,RightV(a0)		// tli->right.v+=tli->right.dv;
 	sub.l		d1,d0
 	move.l	d0,d7					// *dv=tli->right.v-*v;
 	
 	move.l	LeftDY(a0),d0		// tli->left.x+=tli->left.dx;
 	add.l		d0,LeftX(a0)
 	
 	move.l	RightDX(a0),d0
 	add.l		d0,RightX(a0)		// tli->right.x+=tli->right.dx;
 	
 	move.l	RightX(a0),d0
 	sub.l		LeftX(a0),d0
 	move.l	d0,d3					// *dx=tli->right.x-tli->left.x;
 	
// 	tli->y++; 	
	addq.l	#1,4(a0)			

//		start_pdest += gr_row;
	move.l	96(sp),d0
	add.l		d0,88(sp)
	 
//	} while (--(tli->n) > 0);
	subq.l	#1,(a0)
	bgt 		@DoLoop

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
// d0 = t_xr-t_xl,u,v,du,dv = d4-d7
// can trash all other regs except the stack pointer (a7)
 
// handle inner loop for opaque (non log2) mode
asm void lin_opaque(void)
 {
/*
  for (x=t_xl; x<t_xr; x++) {
     int k=t_vtab[fix_fint(v)]+fix_fint(u);
     *(p_dest++) = t_bits[k];		// gr_fill_upixel(t_bits[k],x,y);
     u+=du; v+=dv;
  }*/

	move.l	l_vtab_68K,a6
	move.l	d7,d3
	move.l	d6,d2
	subq.w	#1,d0
	bmi.s		@Done

	moveq		#16,d6
@Loop:
	move.l	d5,d1
	asr.l		d6,d1 						// d1 = fix_fint(v)
	dc.l		0x22361C00 // MOVE.L  $00(A6,D1.L*4),D1		// t_vtab[fix_fint(v)]
	move.l	d4,d7
	asr.l		d6,d7 						// d7 = fix_fint(u)
	add.l		d7,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))
	move.b	(a5,d1.l),(a4)+		// *(p_dest++) = t_bits[k];
	add.l		d2,d4							// u+=du
	add.l		d3,d5							// v+=dv
	dbra		d0,@Loop
@Done:		
	rts	   
 }

// handle inner loop for opaque (width log2) mode
asm void lin_opaque_log2(void)
 {
/* 
  for (x=t_xl; x<t_xr; x++) {
     int k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
     *(p_dest++) = t_bits[k];		// gr_fill_upixel(t_bits[k],x,y);
     u+=du; v+=dv;
  }*/
	
	move.l	l_mask_68K,d2
	move.l	l_wlog_68K,d3
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

// handle inner loop for opaque clut (non log2) mode
asm void lin_opaque_clut(void)
 {
 /* for (x=t_xl; x<t_xr; x++) {
     int k=t_vtab[fix_fint(v)]+fix_fint(u);
     *(p_dest++) = t_clut[t_bits[k]];		// gr_fill_upixel(tli->clut[t_bits[k]],x,y);
     u+=du; v+=dv;
  }*/
	
	move.l	l_vtab_68K,a6
	move.l	l_clut_68K,a1
	move.l	d7,d3
	move.l	d6,d2
	move.l	d4,a2
	subq.w	#1,d0
	bmi.s		@Done

	moveq		#16,d6
	moveq		#0,d4
@Loop:
	move.l	d5,d1
	asr.l		d6,d1 						// d1 = fix_fint(v)
	dc.l		0x22361C00 // MOVE.L  $00(A6,D1.L*4),D1		// t_vtab[fix_fint(v)]
	move.l	a2,d7
	asr.l		d6,d7 						// d7 = fix_fint(u)
	add.l		d7,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))
	move.b	(a5,d1.l),d4
	move.b	(a1,d4.w),(a4)+		// *(p_dest++) = t_bits[k];
	add.l		d2,a2							// u+=du
	add.l		d3,d5							// v+=dv
	dbra		d0,@Loop

@Done:		
	rts	   
 }
 
// handle inner loop for opaque clut (width log2) mode
asm void lin_opaque_clut_log2(void)
 {
/*
	for (x=t_xl; x<t_xr; x++) {
	   int k=t_vtab[fix_fint(v)]+fix_fint(u);
	   *(p_dest++) = t_clut[t_bits[k]];		// gr_fill_upixel(tli->clut[t_bits[k]],x,y);
	   u+=du; v+=dv;
	}*/
	
	move.l	l_clut_68K,a1
	move.l	l_mask_68K,d2
	move.l	l_wlog_68K,d3
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
 

// handle inner loop for transparent (non log2) mode
asm void lin_trans(void)
 {
/*  for (x=t_xl; x<t_xr; x++) {
     int k=t_vtab[fix_fint(v)]+fix_fint(u);
     if (temp_pix=t_bits[k]) *p_dest = temp_pix;		// gr_fill_upixel(t_bits[k],x,y);
     p_dest++; u+=du; v+=dv;
  }*/
	
	move.l	l_vtab_68K,a6
	move.l	d7,d3
	move.l	d6,d2
	subq.w	#1,d0
	bmi.s		@Done

	moveq		#16,d6
@Loop:
	move.l	d5,d1
	asr.l		d6,d1 						// d1 = fix_fint(v)
	dc.l		0x22361C00 // MOVE.L  $00(A6,D1.L*4),D1		// t_vtab[fix_fint(v)]
	move.l	d4,d7
	asr.l		d6,d7 						// d7 = fix_fint(u)
	add.l		d7,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))
	move.b	(a5,d1.l),d7
	beq.s		@skippix
	move.b	d7,(a4)+					// *(p_dest++) = t_bits[k];
	add.l		d2,d4							// u+=du
	add.l		d3,d5							// v+=dv
	dbra		d0,@Loop
	bra.s		@End

@skippix:
	addq.w	#1,a4
	add.l		d2,d4							// u+=du
	add.l		d3,d5							// v+=dv
	dbra		d0,@Loop

@End:
@Done:		
	rts	   
 }
 

// handle inner loop for transparent (width log2) mode
asm void lin_trans_log2(void)
 {
/* 
  for (x=t_xl; x<t_xr; x++) {
     int k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
     if (temp_pix=t_bits[k]) *p_dest = temp_pix;		// gr_fill_upixel(t_bits[k],x,y);
     p_dest++; u+=du; v+=dv;
  }*/
	
	move.l	l_mask_68K,d2
	move.l	l_wlog_68K,d3
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
 

// handle inner loop for transparent clut (width log2) mode
asm void lin_trans_clut(void)
 {
/* for (x=t_xl; x<t_xr; x++) {
     int k=t_vtab[fix_fint(v)]+fix_fint(u);
     if (k=t_bits[k]) *p_dest = t_clut[k];		// gr_fill_upixel(tli->clut[k],x,y);
     p_dest++; u+=du; v+=dv;
    } */
	move.l	l_vtab_68K,a6
	move.l	l_clut_68K,a2
	move.l	d7,d3
	move.l	d6,d2
	subq.w	#1,d0
	bmi.s		@Done

	moveq		#16,d6
@Loop:
	move.l	d5,d1
	asr.l		d6,d1 						// d1 = fix_fint(v)
	dc.l		0x22361C00 // MOVE.L  $00(A6,D1.L*4),D1		// t_vtab[fix_fint(v)]
	move.l	d4,d7
	asr.l		d6,d7 						// d7 = fix_fint(u)
	add.l		d7,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))
	moveq		#0,d7
	move.b	(a5,d1.l),d7
	beq.s		@skippix
	move.b	(a2,d7.w),(a4)+		// *(p_dest++) = t_clut[t_bits[k]];
	add.l		d2,d4							// u+=du
	add.l		d3,d5							// v+=dv
	dbra		d0,@Loop
	bra.s		@End

@skippix:
	addq.w	#1,a4
	add.l		d2,d4							// u+=du
	add.l		d3,d5							// v+=dv
	dbra		d0,@Loop

@End:
@Done:		
	rts	   
 }
 

// handle inner loop for transparent clut (width log2) mode
asm void lin_trans_clut_log2(void)
 {
/* 
  for (x=t_xl; x<t_xr; x++) {
     int k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
     if (k=t_bits[k]) *p_dest = t_clut[k];		// gr_fill_upixel(tli->clut[k],x,y);
     p_dest++; u+=du; v+=dv;
  }*/
	
	move.l	l_mask_68K,d2
	move.l	l_wlog_68K,d3
	move.l	l_clut_68K,a6
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
#endif

void gri_trans_lin_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_TRANS|GRL_LOG2;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_TRANS;
   }
   tli->loop_func=(void (*)()) gri_lin_umap_loop;
   tli->right_edge_func=(void (*)()) gri_uvx_edge;
   tli->left_edge_func=(void (*)()) gri_uvx_edge;
}

void gri_opaque_lin_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_OPAQUE|GRL_LOG2;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_OPAQUE;
   }
   tli->loop_func=(void (*)()) gri_lin_umap_loop;
   tli->right_edge_func=(void (*)()) gri_uvx_edge;
   tli->left_edge_func=(void (*)()) gri_uvx_edge;
}

void gri_trans_clut_lin_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_TRANS|GRL_LOG2|GRL_CLUT;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_TRANS|GRL_CLUT;
   }
   tli->loop_func=(void (*)()) gri_lin_umap_loop;
   tli->right_edge_func=(void (*)()) gri_uvx_edge;
   tli->left_edge_func=(void (*)()) gri_uvx_edge;
}

void gri_opaque_clut_lin_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_OPAQUE|GRL_LOG2|GRL_CLUT;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_OPAQUE|GRL_CLUT;
   }
   tli->loop_func=(void (*)()) gri_lin_umap_loop;
   tli->right_edge_func=(void (*)()) gri_uvx_edge;
   tli->left_edge_func=(void (*)()) gri_uvx_edge;
}
