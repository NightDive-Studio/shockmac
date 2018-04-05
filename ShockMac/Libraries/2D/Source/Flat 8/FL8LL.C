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
 * $Source: r:/prj/lib/src/2d/RCS/genll.c $
 * $Revision: 1.3 $
 * $Author: kevin $
 * $Date: 1994/08/16 12:50:11 $
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
#include "scrdat.h"
#include "vtab.h"
#include "fl8tf.h"
#include "cnvdat.h"
#include "2dDiv.h"
#include "fl8tmapdv.h"

int gri_lit_lin_umap_loop(grs_tmap_loop_info *tli);

// 68K stuff
#if !(defined(powerc) || defined(__powerc))	
asm int Handle_Lit_68K_Loop(fix u, fix v, fix du, fix dv, fix dx,
														grs_tmap_loop_info *tli, uchar *start_pdest, uchar *t_bits, long gr_row,
														fix i, fix di);

// specific inner looops
asm void lin_lit_opaque(void);
asm void lin_lit_opaque_log2(void);
asm void lin_lit_trans(void);
asm void lin_lit_trans_log2(void);

// jump table for 68k inner loops
void (*lin_lit_in_loop[4])(void) = {lin_lit_opaque, lin_lit_trans, lin_lit_opaque_log2, lin_lit_trans_log2};
#endif

// globals used by 68K routines
long	*l_l_vtab_68K;
ulong	l_l_wlog_68K;
ulong	l_l_mask_68K;
uchar	*l_l_ltab;


// PPC specific optimized routines
extern "C"
{
int Handle_Lit_Lin_Loop_PPC(fix u, fix v, fix du, fix dv, fix dx,
														grs_tmap_loop_info *tli, uchar *start_pdest, uchar *t_bits, long gr_row,
														fix i, fix di, uchar *g_ltab, uchar	t_wlog, ulong	t_mask);
															
int Handle_TLit_Lin_Loop2_PPC(fix u, fix v, fix du, fix dv, fix dx,
															grs_tmap_loop_info *tli, uchar *start_pdest, uchar *t_bits, long gr_row,
															fix i, fix di, uchar *g_ltab, uchar	t_wlog, ulong	t_mask);
}
/*

int Handle_Lit_Lin_Loop_C(fix u, fix v, fix du, fix dv, fix dx,
														grs_tmap_loop_info *tli, uchar *start_pdest, uchar *t_bits, long gr_row,
														fix i, fix di, uchar *g_ltab, uchar	t_wlog, ulong	t_mask)
 {
	int		x,t_xl,t_xr,inv;	
	uchar *p_dest;

	tli->y+=tli->n;

   do {
      if ((x = fix_ceil(tli->right.x)-fix_ceil(tli->left.x)) > 0) 
       {         
         x = fix_div(fix_make(1,0)<<8,dx);
         di=fix_mul_asm_safe_light(di,x);
				 x>>=8;
         du=fix_mul_asm_safe(du,x);
         dv=fix_mul_asm_safe(dv,x);

         x =fix_ceil(tli->left.x)-tli->left.x;
         u+=fix_mul(du,x);
         v+=fix_mul(dv,x);
         i+=fix_mul(di,x);
         
	 		   // copy out tli-> stuff into locals
				 t_xl = fix_cint(tli->left.x);
				 t_xr = fix_cint(tli->right.x);
				 p_dest = start_pdest + t_xl;
				 x = t_xr - t_xl;

	       for (; x>0; x--) 
	        {
           *(p_dest++) = g_ltab[t_bits[((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask]+fix_light(i)];		
           u+=du; v+=dv; i+=di;
	        }
      } else if (x<0) return TRUE; // punt this tmap 
      
      u=(tli->left.u+=tli->left.du);
      tli->right.u+=tli->right.du;
      du=tli->right.u-u;
      v=(tli->left.v+=tli->left.dv);
      tli->right.v+=tli->right.dv;
      dv=tli->right.v-v;
      i=(tli->left.i+=tli->left.di);
      tli->right.i+=tli->right.di;
      di=tli->right.i-i;
      tli->left.x+=tli->left.dx;
      tli->right.x+=tli->right.dx;
      dx=tli->right.x-tli->left.x;
			start_pdest += gr_row;
   } while (--(tli->n) > 0);
   return FALSE; // tmap OK 
 }														


 															
															
int Handle_TLit_Lin_Loop2_C(fix u, fix v, fix du, fix dv, fix dx,
															grs_tmap_loop_info *tli, uchar *start_pdest, uchar *t_bits, long gr_row,
															fix i, fix di, uchar *g_ltab, uchar	t_wlog, ulong	t_mask)
 {
 	int		x,k;
 	uchar *p_dest;
	int		t_xl,t_xr;	
	int		lx,rx;
	
	lx = tli->left.x;
	rx = tli->right.x;
	
   tli->y+=tli->n;
   do {
      if ((x = fix_ceil(rx)-fix_ceil(lx)) > 0) 
       {
         x =fix_ceil(lx)-lx;
         
         k = fix_div(fix_make(1,0)<<8,dx);
         di=fix_mul_asm_safe_light(di,k);
				 k>>=8;
         du=fix_mul_asm_safe(du,k);
         dv=fix_mul_asm_safe(dv,k);

         u+=fix_mul(du,x);
         v+=fix_mul(dv,x);
         i+=fix_mul(di,x);
         
	 		   // copy out tli-> stuff into locals
				 t_xl = fix_cint(lx);
				 t_xr = fix_cint(rx);
				 p_dest = start_pdest + t_xl;
				 x = t_xr - t_xl;

         for (; x>0; x--) 
          {
           // assume pixel in transparent first
           k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
           k=t_bits[k];
           if (k)	// not transparent, move to assuming opaque
           	 *p_dest = g_ltab[k+fix_light(i)];
             
           p_dest++; u+=du; v+=dv; i+=di;
          }
      } else if (x<0) return TRUE; // punt this tmap
      
      u=(tli->left.u+=tli->left.du);
      tli->right.u+=tli->right.du;
      du=tli->right.u-u;
      v=(tli->left.v+=tli->left.dv);
      tli->right.v+=tli->right.dv;
      dv=tli->right.v-v;
      i=(tli->left.i+=tli->left.di);
      tli->right.i+=tli->right.di;
      di=tli->right.i-i;
      lx+=tli->left.dx;
      rx+=tli->right.dx;
      dx=rx-lx;
			start_pdest += gr_row;
   } while (--(tli->n) > 0);

	tli->left.x = lx;
	tli->right.x = rx;

   return FALSE; // tmap OK 
 }
*/

int gri_lit_lin_umap_loop(grs_tmap_loop_info *tli) {
   fix u,v,i,du,dv,di,dx,d;

	// locals used to store copies of tli-> stuff, so its in registers on the PPC
  register int x,k;
	int		t_xl,t_xr,inv;	
	long	*t_vtab;
	uchar *t_bits;
	uchar *p_dest;
	uchar temp_pix;
	uchar	t_wlog;
	ulong	t_mask;
  uchar *g_ltab;
	long	gr_row;
	uchar *start_pdest;
								
	u=tli->left.u;
	du=tli->right.u-u;
	v=tli->left.v;
	dv=tli->right.v-v;
	i=tli->left.i;
	di=tli->right.i-i;
	dx=tli->right.x-tli->left.x;

	l_l_vtab_68K = t_vtab = tli->vtab;
	l_l_mask_68K = t_mask = tli->mask;
	l_l_wlog_68K = t_wlog = tli->bm.wlog;
  l_l_ltab = g_ltab = grd_screen->ltab;

	t_bits = tli->bm.bits;
	gr_row = grd_bm.row;
	start_pdest = grd_bm.bits + (gr_row*(tli->y));

// handle PowerPC loop
#if (defined(powerc) || defined(__powerc))	
	// handle optimized cases first
	if (tli->bm.hlog == (GRL_OPAQUE|GRL_LOG2))
		return(Handle_Lit_Lin_Loop_PPC(u,v,du,dv,dx,tli,start_pdest,t_bits,gr_row,i,di,g_ltab,t_wlog,t_mask));
	if (tli->bm.hlog == (GRL_TRANS|GRL_LOG2))
		return(Handle_TLit_Lin_Loop2_PPC(u,v,du,dv,dx,tli,start_pdest,t_bits,gr_row,i,di,g_ltab,t_wlog,t_mask));
		
   do {
      if ((d = fix_ceil(tli->right.x)-fix_ceil(tli->left.x)) > 0) 
       {
         d =fix_ceil(tli->left.x)-tli->left.x;
         
#if InvDiv
         k = fix_div(fix_make(1,0)<<8,dx);
         di=fix_mul_asm_safe_light(di,k);
				 k>>=8;
         du=fix_mul_asm_safe(du,k);
         dv=fix_mul_asm_safe(dv,k);
#else
         du=fix_div(du,dx);
         dv=fix_div(dv,dx);
         di=fix_div(di,dx);
#endif 

         u+=fix_mul(du,d);
         v+=fix_mul(dv,d);
         i+=fix_mul(di,d);
         
	 		   // copy out tli-> stuff into locals
				 t_xl = fix_cint(tli->left.x);
				 t_xr = fix_cint(tli->right.x);
				 p_dest = start_pdest + t_xl;
				 x = t_xr - t_xl;

				switch (tli->bm.hlog)
				 {
         case GRL_OPAQUE:
	          for (; x>0; x--) {
	             k=t_vtab[fix_fint(v)]+fix_fint(u);
	             *(p_dest++) = g_ltab[t_bits[k]+fix_light(i)];		// gr_fill_upixel(g_ltab[t_bits[k]+fix_light(i)],x,t_y);
	             u+=du; v+=dv; i+=di;
	          }
            break;
         case GRL_TRANS:
            for (; x>0; x--) {
               k=t_vtab[fix_fint(v)]+fix_fint(u);
               if (k=t_bits[k]) *p_dest = g_ltab[k+fix_light(i)];		// gr_fill_upixel(g_ltab[k+fix_light(i)],x,t_y);
               p_dest++; u+=du; v+=dv; i+=di;
            }
            break;
         // handled in special case code
         case GRL_OPAQUE|GRL_LOG2:
            for (; x>0; x--) {
               k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
               *(p_dest++) = g_ltab[t_bits[k]+fix_light(i)];		// gr_fill_upixel(g_ltab[t_bits[k]+fix_light(i)],x,t_y);
               u+=du; v+=dv; i+=di;
            }
            break;
         case GRL_TRANS|GRL_LOG2:
            for (; x>0; x--) {
               k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
               if (k=t_bits[k]) *p_dest = g_ltab[k+fix_light(i)];		// gr_fill_upixel(g_ltab[k+fix_light(i)],x,t_y);
               p_dest++; u+=du; v+=dv; i+=di;
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
      i=(tli->left.i+=tli->left.di);
      tli->right.i+=tli->right.di;
      di=tli->right.i-i;
      tli->left.x+=tli->left.dx;
      tli->right.x+=tli->right.dx;
      dx=tli->right.x-tli->left.x;
      tli->y++;
			start_pdest += gr_row;
   } while (--(tli->n) > 0);
   return FALSE; /* tmap OK */

// handle 68K loops
#else
	return(Handle_Lit_68K_Loop(u,v,du,dv,dx,tli,start_pdest,t_bits,gr_row,i,di));
#endif
}

// Main 68K handler loop
#if !(defined(powerc) || defined(__powerc))	
asm int Handle_Lit_68K_Loop(fix u, fix v, fix du, fix dv, fix dx,
														grs_tmap_loop_info *tli, uchar *start_pdest, uchar *t_bits, long gr_row,
														fix i, fix di)
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
//   di=fix_div(di,dx);
//   u+=fix_mul(du,d);
//   v+=fix_mul(dv,d);
//   i+=fix_mul(di,d);

	move.l	LeftX(a0),d2		// tli->left.x
	
 	move.l	d2,d1
 	add.l		#0x0000FFFF,d2
 	clr.w		d2
 	sub.l		d1,d2							// d =fix_ceil(tli->left.x)-tli->left.x;

	fix_div_68k_d3(d6)			// inline function, returns result in d0
	move.l	d0,d6						//  du=fix_div(du,dx);
	
	fix_div_68k_d3(d7)			// inline function, returns result in d0
	move.l	d0,d7						//  dv=fix_div(dv,dx);

	move.l	d7,a4						// save d7
	move.l	104(sp),d7
	fix_div_68k_d3(d7)			// inline function, returns result in d0
	move.l	d0,104(sp)			//  di=fix_div(di,dx);
	move.l	a4,d7						// restore d7

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
	beq.s		@Skip
	
// p_dest = start_pdest + t_xl;
	move.l	88(sp),a4
	add.l		d1,a4	
 	
	// when we call the asm inner loop, a5 = t_bits, a4 = p_dest, 
	// d0 = t_xr-t_xl, u,v,du,dv = d4-d7, a2 = *i, a3 = *di
	moveq		#0,d1
	move.b	23(a0),d1	
	move.l	92(sp),a5
	move.l	100(sp),a2
	move.l	104(sp),a3
	lea			lin_lit_in_loop,a1
	dc.l		0x22711400     //   MOVEA.L   $00(A1,D1.W*4),A1
	jsr			(a1)
@Skip:

//	  u=(tli->left.u+=tli->left.du);
//	  tli->right.u+=tli->right.du;
//	  du=tli->right.u-u;
//	  v=(tli->left.v+=tli->left.dv);
//	  tli->right.v+=tli->right.dv;
//	  dv=tli->right.v-v;
//    i=(tli->left.i+=tli->left.di);
//    tli->right.i+=tli->right.di;
//    di=tli->right.i-i;
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
 
 	move.l	LeftI(a0),d1
 	add.l		LeftDI(a0),d1
 	move.l	d1,LeftI(a0)
 	move.l	d1,100(sp)		// i=(tli->left.i+=tli->left.di);
 	
 	move.l	RightI(a0),d0
 	add.l		RightDI(a0),d0
 	move.l	d0,RightI(a0)		// tli->right.i+=tli->right.di;
 	sub.l		d1,d0
 	move.l	d0,104(sp)		// di=tli->right.i-i;
 
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
// d0 = t_xr-t_xl,u,v,du,dv = d4-d7, a2 = i, a3 = di
// can trash all other regs except the stack pointer (a7)
 
// handle inner loop for lit opaque (non log2) mode
asm void lin_lit_opaque(void)
 {
/*  for (x=t_xl; x<t_xr; x++) {
     int k=t_vtab[fix_fint(v)]+fix_fint(u);
     *(p_dest++) = g_ltab[t_bits[k]+fix_light(i)];		// gr_fill_upixel(g_ltab[t_bits[k]+fix_light(i)],x,t_y);
     u+=du; v+=dv; i+=di;
  }*/

	move.l	l_l_ltab,a1
	move.l	l_l_vtab_68K,a6
	subq.w	#1,d0

@Loop:
	move.l	d5,d1
	swap		d1
	ext.l		d1	 						// d1 = fix_fint(v)
	dc.l		0x22361C00 // MOVE.L  $00(A6,D1.L*4),D1		// t_vtab[fix_fint(v)]
	move.l	d4,d2
	swap		d2
	ext.l		d2		 						// d7 = fix_fint(u)
	add.l		d2,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))

	move.l	a2,d2
	swap		d2
	lsl.w		#8,d2
	move.b	(a5,d1.l),d2
	move.b	(a1,d2.w),(a4)+

	add.l		d6,d4							// u+=du
	add.l		d7,d5							// v+=dv
	add.l		a3,a2							// i+=di
	dbra		d0,@Loop
	rts	   
 }


asm void lin_lit_opaque_log2(void)
 {
/*  for (x=t_xl; x<t_xr; x++) {
     int k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
     *(p_dest++) = g_ltab[t_bits[k]+fix_light(i)];		// gr_fill_upixel(g_ltab[t_bits[k]+fix_light(i)],x,t_y);
     u+=du; v+=dv; i+=di;
  }*/
  
	move.l	l_l_mask_68K,d2
	move.l	l_l_wlog_68K,d3
	move.l	l_l_ltab,a6	
	move.l	d7,a1
	subq.w	#1,d0
	
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
	move.b	(a6,d7.w),(a4)+
	
	add.l		d6,d4							// u+=du
	add.l		a1,d5							// v+=dv
	add.l		a3,a2							// i+=di
	dbra		d0,@Loop

	rts	   
 } 

// handle inner loop for lit transparent (non log2) mode
asm void lin_lit_trans(void)
 {
/**  for (x=t_xl; x<t_xr; x++) {
     int k=t_vtab[fix_fint(v)]+fix_fint(u);
     if (k=t_bits[k]) *p_dest = g_ltab[t_bits[k]+fix_light(i)];		// gr_fill_upixel(g_ltab[k+fix_light(i)],x,t_y);
     p_dest++; u+=du; v+=dv; i+=di;
  }*/

	move.l	l_l_ltab,a1
	move.l	l_l_vtab_68K,a6
	subq.w	#1,d0

@Loop:
	move.l	d5,d1
	swap		d1
	ext.l		d1	 						// d1 = fix_fint(v)
	dc.l		0x22361C00 // MOVE.L  $00(A6,D1.L*4),D1		// t_vtab[fix_fint(v)]
	move.l	d4,d2
	swap		d2
	ext.l		d2		 						// d7 = fix_fint(u)
	add.l		d2,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))

	move.l	a2,d2
	swap		d2
	lsl.w		#8,d2
	move.b	(a5,d1.l),d2
	tst.b		d2
	beq.s		@skip
	move.b	(a1,d2.w),(a4)+

	add.l		d6,d4							// u+=du
	add.l		d7,d5							// v+=dv
	add.l		a3,a2							// i+=di
	dbra		d0,@Loop
	bra.s		@Done
	
@skip:
	addq.w	#1,a4
	add.l		d6,d4							// u+=du
	add.l		d7,d5							// v+=dv
	add.l		a3,a2							// i+=di
	dbra		d0,@Loop

@Done:
	rts	   
 }
 

// handle inner loop for lit transparent (width log2) mode
asm void lin_lit_trans_log2(void)
 {
/* for (x=t_xl; x<t_xr; x++) {
     int k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
     if (k=t_bits[k]) *p_dest = g_ltab[t_bits[k]+fix_light(i)];		// gr_fill_upixel(g_ltab[k+fix_light(i)],x,t_y);
     p_dest++; u+=du; v+=dv; i+=di;
  }*/
    
	move.l	l_l_mask_68K,d2
	move.l	l_l_wlog_68K,d3
	move.l	l_l_ltab,a6	
	move.l	d7,a1
	subq.w	#1,d0

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
	beq.s		@skip
	move.b	(a6,d7.w),(a4)+
	
	add.l		d6,d4							// u+=du
	add.l		a1,d5							// v+=dv
	add.l		a3,a2							// i+=di
	dbra		d0,@Loop
	bra.s		@Done

@skip:
	addq.w	#1,a4
	add.l		d6,d4							// u+=du
	add.l		a1,d5							// v+=dv
	add.l		a3,a2							// i+=di
	dbra		d0,@Loop

@Done:
	rts	   
 }
 
#endif


void gri_trans_lit_lin_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_TRANS|GRL_LOG2;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_TRANS;
   }
   tli->loop_func=(void (*)()) gri_lit_lin_umap_loop;
   tli->right_edge_func=(void (*)()) gri_uvix_edge;
   tli->left_edge_func=(void (*)()) gri_uvix_edge;
}

void gri_opaque_lit_lin_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_OPAQUE|GRL_LOG2;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_OPAQUE;
   }

   tli->loop_func=(void (*)()) gri_lit_lin_umap_loop;
   tli->right_edge_func=(void (*)()) gri_uvix_edge;
   tli->left_edge_func=(void (*)()) gri_uvix_edge;
}
