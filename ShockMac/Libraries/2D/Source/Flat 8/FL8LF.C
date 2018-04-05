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
 * $Source: r:/prj/lib/src/2d/RCS/genlf.c $
 * $Revision: 1.3 $
 * $Author: kevin $
 * $Date: 1994/08/16 12:50:14 $
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
#include "scrmac.h"
#include "vtab.h"
#include "fl8tf.h"
#include "cnvdat.h"
#include "2dDiv.h"
#include "fl8tmapdv.h"

int gri_lit_floor_umap_loop(grs_tmap_loop_info *tli);

// 68K stuff
#if !(defined(powerc) || defined(__powerc))	
asm int Handle_Floor_Lit_68K_Loop(fix u, fix v, fix du, fix dv, fix dx,
														 		 grs_tmap_loop_info *tli, uchar *start_pdest, uchar *t_bits, long gr_row,
														 		 fix i, fix di);

// specific inner looops
asm void floor_lit_opaque_log2(void);
asm void floor_lit_trans_log2(void);
#endif

// globals used by 68K routines
ulong	f_l_wlog_68K;
ulong	f_l_mask_68K;
uchar	*f_l_ltab;

extern "C"
{
extern int HandleFloorLoop_PPC(grs_tmap_loop_info *tli,
															 fix u, fix v, fix du, fix dv, fix dx, fix i, fix di,
													 		 uchar t_wlog, ulong	t_mask, uchar *t_bits, uchar *g_ltab);
}
/*
int HandleFloorLoop_C(grs_tmap_loop_info *tli,
										  fix u, fix v, fix du, fix dv, fix dx, fix i, fix di,
								 		  uchar t_wlog, ulong	t_mask, uchar *t_bits, uchar *g_ltab)
 {
	uchar *p_dest;
  int		x;
	fix		inv;

   do
    {
      if ((x = fix_ceil(tli->right.x)-fix_ceil(tli->left.x)) > 0) 
       {
         x =fix_ceil(tli->left.x)-tli->left.x;

  			 inv = fix_div(fix_make(1,0)<<8,dx);
         di=fix_mul_asm_safe_light(di,inv);	
				 inv>>=8;
         du=fix_mul_asm_safe(du,inv);	
         dv=fix_mul_asm_safe(dv,inv);	
         
         u+=fix_mul(du,x);
         v+=fix_mul(dv,x);
         i+=fix_mul(di,x);

			   // copy out tli-> stuff into locals
				 p_dest = grd_bm.bits + (grd_bm.row*tli->y) + fix_cint(tli->left.x);
			   x = fix_cint(tli->right.x) - fix_cint(tli->left.x);

					while ((long) p_dest & 3 != 0)
					 {
	         	 *(p_dest++) = g_ltab[t_bits[((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask]+fix_light(i)];		
	         	 u+=du; v+=dv; i+=di;
	         	 x--;
					 }
					 
	      	 while (x>=4) 
	      	  {
	         	 inv = g_ltab[t_bits[((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask]+fix_light(i)];		
	         	 u+=du; v+=dv; i+=di;
	         	 inv <<= 8;
	         	 
	         	 inv |= g_ltab[t_bits[((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask]+fix_light(i)];		
	         	 u+=du; v+=dv; i+=di;
	         	 inv <<= 8;
	
	         	 inv |= g_ltab[t_bits[((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask]+fix_light(i)];	
	         	 u+=du; v+=dv; i+=di;
	         	 inv <<= 8;
	
	         	 inv |= g_ltab[t_bits[((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask]+fix_light(i)];	
	         	 u+=du; v+=dv; i+=di;
	
						 * (long *) p_dest = inv;
						 x-=4;
						 p_dest += 4;
	      	  }
	         
	       for (; x>0; x--) 
	        {
	       	 *(p_dest++) = g_ltab[t_bits[((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask]+fix_light(i)];		
	       	 u+=du; v+=dv; i+=di;
	        }
      } else if (x<0) return TRUE; // punt this tmap 
      
      tli->w+=tli->dw;

  		inv = fix_div(fix_make(1,0),tli->w);
      u=fix_mul_asm_safe((tli->left.u+=tli->left.du),inv); 
      tli->right.u+=tli->right.du;
      du=fix_mul_asm_safe(tli->right.u,inv)-u;
      v=fix_mul_asm_safe((tli->left.v+=tli->left.dv),inv);
      tli->right.v+=tli->right.dv;
      dv=fix_mul_asm_safe(tli->right.v,inv)-v;
      i=fix_mul_asm_safe((tli->left.i+=tli->left.di),inv); 
      tli->right.i+=tli->right.di;
      di=fix_mul_asm_safe(tli->right.i,inv)-i;
      
      tli->left.x+=tli->left.dx;
      tli->right.x+=tli->right.dx;
      dx=tli->right.x-tli->left.x;
      tli->y++;
   } while (--(tli->n) > 0);
   
   return FALSE; // tmap OK 
 }
 */

int gri_lit_floor_umap_loop(grs_tmap_loop_info *tli) {
   fix u,v,i,du,dv,di,dx,d;
   
	// locals used to store copies of tli-> stuff, so its in registers on the PPC
  int		x,k;
	uchar	t_wlog;
	ulong	t_mask;
	uchar *t_bits;
	uchar *p_dest;
  uchar *g_ltab;
	fix		inv;
	long	*t_vtab;
  
  
#if InvDiv
  inv = fix_div(fix_make(1,0),tli->w);
	u=fix_mul_asm_safe(tli->left.u,inv); 
	du=fix_mul_asm_safe(tli->right.u,inv)-u;
	v=fix_mul_asm_safe(tli->left.v,inv); 
	dv=fix_mul_asm_safe(tli->right.v,inv)-v;
	i=fix_mul_asm_safe(tli->left.i,inv); 
	di=fix_mul_asm_safe(tli->right.i,inv)-i;
#else
	u=fix_div(tli->left.u,tli->w); 
	du=fix_div(tli->right.u,tli->w)-u;
	v=fix_div(tli->left.v,tli->w); 
	dv=fix_div(tli->right.v,tli->w)-v;
	i=fix_div(tli->left.i,tli->w); 
	di=fix_div(tli->right.i,tli->w)-i;
#endif

	dx=tli->right.x-tli->left.x;
	
	f_l_mask_68K = t_mask = tli->mask;
	f_l_wlog_68K = t_wlog = tli->bm.wlog;
	f_l_ltab = g_ltab = grd_screen->ltab;
	
	t_vtab = tli->vtab;
	t_bits = tli->bm.bits;
   
// handle PowerPC loop
#if (defined(powerc) || defined(__powerc))	
	if (tli->bm.hlog==(GRL_OPAQUE|GRL_LOG2))
	  return HandleFloorLoop_PPC(tli, u, v, du, dv, dx, i, di, t_wlog, t_mask, t_bits, g_ltab);

   do {
      if ((d = fix_ceil(tli->right.x)-fix_ceil(tli->left.x)) > 0) {
         d =fix_ceil(tli->left.x)-tli->left.x;

#if InvDiv
  			 inv = fix_div(fix_make(1,0)<<8,dx);
         di=fix_mul_asm_safe_light(di,inv);	
				 inv>>=8;
         du=fix_mul_asm_safe(du,inv);	
         dv=fix_mul_asm_safe(dv,inv);	
#else
         du=fix_div(du,dx);	
         dv=fix_div(dv,dx);	
         di=fix_div(di,dx);	
#endif
         u+=fix_mul(du,d);
         v+=fix_mul(dv,d);
         i+=fix_mul(di,d);

			   // copy out tli-> stuff into locals
				 p_dest = grd_bm.bits + (grd_bm.row*tli->y) + fix_cint(tli->left.x);
			   x = fix_cint(tli->right.x) - fix_cint(tli->left.x);

         switch (tli->bm.hlog) {
         case GRL_OPAQUE:
            for (; x>0; x--) {
               k=t_vtab[fix_fint(v)]+fix_fint(u);
               *(p_dest++) = g_ltab[t_bits[k]+fix_light(i)];		// gr_fill_upixel(g_ltab[t_bits[k]+fix_light(i)],x,t_y);
            }
            break;
         case GRL_TRANS:
            for (; x>0; x--) {
               k=t_vtab[fix_fint(v)]+fix_fint(u);
               if (k=t_bits[k]) *p_dest = g_ltab[k+fix_light(i)];		// gr_fill_upixel(g_ltab[k+fix_light(i)],x,t_y);
               p_dest++; u+=du; v+=dv; i+=di;
            }
            break;
   // special case handles this
   /*      case GRL_OPAQUE|GRL_LOG2:
             for (; x>0; x--) {
               k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
             	 *(p_dest++) = g_ltab[t_bits[k]+fix_light(i)];		// gr_fill_upixel(g_ltab[t_bits[k]+fix_light(i)],x,t_y);
             	 u+=du; v+=dv; i+=di;
            }
            break;*/
         case GRL_TRANS|GRL_LOG2:
            for (; x>0; x--) {
               k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
               if (k=t_bits[k]) *p_dest = g_ltab[k+fix_light(i)];		// gr_fill_upixel(g_ltab[k+fix_light(i)],x,t_y);
               p_dest++; u+=du; v+=dv; i+=di;
            }
            break;
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
      i=fix_mul_asm_safe((tli->left.i+=tli->left.di),inv); 
      tli->right.i+=tli->right.di;
      di=fix_mul_asm_safe(tli->right.i,inv)-i;
#else
      u=fix_div((tli->left.u+=tli->left.du),tli->w);
      tli->right.u+=tli->right.du;
      du=fix_div(tli->right.u,tli->w)-u;
      v=fix_div((tli->left.v+=tli->left.dv),tli->w);
      tli->right.v+=tli->right.dv;
      dv=fix_div(tli->right.v,tli->w)-v;
      i=fix_div((tli->left.i+=tli->left.di),tli->w);
      tli->right.i+=tli->right.di;
      di=fix_div(tli->right.i,tli->w)-i;
#endif
      
      tli->left.x+=tli->left.dx;
      tli->right.x+=tli->right.dx;
      dx=tli->right.x-tli->left.x;
      tli->y++;
   } while (--(tli->n) > 0);
   return FALSE; /* tmap OK */

// handle 68K loops
#else
	return(Handle_Floor_Lit_68K_Loop(u,v,du,dv,dx,tli,grd_bm.bits,t_bits,grd_bm.row, i, di));
#endif
}

// Main 68K handler loop
#if !(defined(powerc) || defined(__powerc))	
asm int Handle_Floor_Lit_68K_Loop(fix u, fix v, fix du, fix dv, fix dx,
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
//    if ((d = fix_ceil(tli->right.x)-fix_ceil(tli->left.x)) > 0) {
 	move.l	RightX(a0),d0
 	add.l		#0x0000FFFF,d0
 	clr.w		d0									// fix_ceil(tli->right.y)
 	move.l	LeftX(a0),d1
 	add.l		#0x0000FFFF,d1
 	clr.w		d1									// fix_ceil(tli->left.y)
 	sub.l		d1,d0								// fix_ceil(tli->right.y)-fix_ceil(tli->left.y))
 	beq 		@Skip								// d==0, skip this map
 	bmi 		@Err								// d<0, punt


//         d =fix_ceil(tli->left.x)-tli->left.x;
//         du=fix_div(du,dx);
//         u+=fix_mul(du,d);
//         dv=fix_div(dv,dx);
//         v+=fix_mul(dv,d);
//         di=fix_div(di,dx);
//         i+=fix_mul(di,d);
 
 	move.l	LeftX(a0),d0				// tli->left.x
 	move.l	d0,d2
 	add.l		#0x0000FFFF,d2
 	clr.w		d2
 	sub.l		d0,d2							// d =fix_ceil(tli->left.x)-tli->left.x;
	
	fix_div_68k_d3(d6)	// inline function, returns result in d0
	move.l	d0,d6						//  du=fix_div(du,dy);
	
	fix_div_68k_d3(d7)	// inline function, returns result in d0
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

	move.l	104(sp),d1
	cmp.l		#256,d1
	bgt.s		@skipfixup
	cmp.l		#-256,d1
	blt.s		@skipfixup			// if (di>=-256 && di<=256) i+=1024;
   
	add.l		#1024,100(sp)
@skipfixup:

//			 t_xl = fix_cint(tli->left.x);
//			 t_xr = fix_cint(tli->right.x);
 	move.l	LeftX(a0),d1
 	add.l		#0x0000FFFF,d1
 	clr.w		d1
 	swap		d1
	move.l	d1,d2	// save for later
	
 	move.l	RightX(a0),d0
 	add.l		#0x0000FFFF,d0
 	clr.w		d0
 	swap		d0
 	sub.l		d1,d0			// t_xr-t_xl

// p_dest = grd_bm.bits + (grd_bm.row*t_y) + t_xl;
	move.l	T_Y(a0),d1
	dc.w		0x4C2F,0x1000,0x0060   //  MULU.L    $0060(A7),D1
	add.l		88(sp),d1				// + grd_bm.bits
	add.l		d2,d1						// + t_xl
	move.l	d1,a4

	// when we call the asm inner loop, a5 = t_bits, a4 = p_dest
	// d0 = t_xr-t_xl, u,v,du,dv = d4-d7, a2 = i, a3 = di
	move.l	92(sp),a5
	move.l	100(sp),a2
	move.l	104(sp),a3
	
	cmp.b		#(GRL_OPAQUE|GRL_LOG2),HLog(a0)		
	beq.s		@Opq
	jsr			floor_lit_trans_log2
	bra.s		@Skip
@Opq:
	jsr			floor_lit_opaque_log2
@Skip:

	move.l	T_W(a0),d2
  add.l		T_DW(a0),d2			// tli->w+=tli->dw;
  move.l	d2,T_W(a0)
  
 	move.l	LeftDU(a0),d0			// tli->left.du
 	add.l	  LeftU(a0),d0			// tli->left.u
 	move.l	d0,LeftU(a0)
 	fix_div_68k_d2_d0(d4)	// inline function, returns result in d0
	move.l	d0,d4						// u=fix_div((tli->left.u+=tli->left.du),tli->w);
	
 	move.l	RightU(a0),d0
 	add.l		RightDU(a0),d0
 	move.l	d0,RightU(a0)		// tli->right.u+=tli->right.du;
 	fix_div_68k_d2_d0(d6)	// inline function, returns result in d0
  sub.l		d4,d0
  move.l	d0,d6					//  du=fix_div(tli->right.u,tli->w)-u;
      
 	move.l	LeftV(a0),d0
 	add.l		LeftDV(a0),d0	
 	move.l	d0,LeftV(a0)		// tli->left.v+=tli->left.dv;
 	fix_div_68k_d2_d0(d5)	// inline function, returns result in d0
  move.l	d0,d5					// v=fix_div((tli->left.v+=tli->left.dv),tli->w);
     
 	move.l	RightV(a0),d0
 	add.l		RightDV(a0),d0
 	move.l	d0,RightV(a0)		// tli->right.v+=tli->right.dv;
 	fix_div_68k_d2_d0(d7)	// inline function, returns result in d0
 	sub.l		d5,d0
	move.l	d0,d7					//  dv=fix_div(tli->right.v,tli->w)-v;
	
 	move.l	LeftI(a0),d0
 	add.l		LeftDI(a0),d0	
 	move.l	d0,LeftI(a0)		// tli->left.i+=tli->left.di;
 	fix_div_68k_d2_d0(d3)		// inline function, returns result in d0
  move.l	d0,100(sp)			// i=fix_div((tli->left.i+=tli->left.di),tli->w);
     
 	move.l	RightI(a0),d0
 	add.l		RightDI(a0),d0
 	move.l	d0,RightI(a0)			// tli->right.i+=tli->right.di;
 	fix_div_68k_d2_d0(d3)			// inline function, returns result in d0
 	sub.l		100(sp),d0
	move.l	d0,104(sp)			//  di=fix_div(tli->right.i,tli->w)-i;
	
 	move.l	LeftDX(a0),d0		// tli->left.x+=tli->left.dx;
 	add.l		d0,LeftX(a0)

 	move.l	RightDX(a0),d0
 	add.l		d0,RightX(a0)		// tli->right.x+=tli->right.dx;
     
 	move.l	RightX(a0),d3
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


// ========================================================================
// when we call the asm inner loops, a5 = t_bits, a4 = p_dest, 
// d0 = t_yr-t_yl,u,v,du,dv = d4-d7, a2 = i, a3 = di
// can trash all other regs except the stack pointer (a7)

// handle inner loop for floor lit opaque (log2) mode
asm void floor_lit_opaque_log2(void)
 {
/*  for (x=t_xl; x<t_xr; x++) {
     int k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
     *(p_dest++) = g_ltab[t_bits[k]+fix_light(i)];		// gr_fill_upixel(g_ltab[t_bits[k]+fix_light(i)],x,t_y);
     u+=du; v+=dv; i+=di;
  }*/
  
	move.l	f_l_mask_68K,d2
	move.l	f_l_wlog_68K,d3
	move.l	f_l_ltab,a6	
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
	move.b	(a6,d7.w),(a4)+
	
	add.l		d6,d4							// u+=du
	add.l		a1,d5							// v+=dv
	add.l		a3,a2							// i+=di
	dbra		d0,@Loop

@Done:
	rts	   
 }


// handle inner loop for floor lit transparent (log2) mode
asm void floor_lit_trans_log2(void)
 {
 /* for (x=t_xl; x<t_xr; x++) {
     int k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
     if (k=t_bits[k]) *p_dest = g_ltab[k+fix_light(i)];		// gr_fill_upixel(g_ltab[k+fix_light(i)],x,t_y);
     p_dest++; u+=du; v+=dv; i+=di;
  }*/
  
	move.l	f_l_mask_68K,d2
	move.l	f_l_wlog_68K,d3
	move.l	f_l_ltab,a6	
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
	move.b	(a6,d7.w),(a4)+
	add.l		d6,d4							// u+=du
	add.l		a1,d5							// v+=dv
	add.l		a3,a2							// i+=di
	dbra		d0,@Loop
	bra.s		@Done
	
@skippix:		
	addq.w	#1,a4
	add.l		d6,d4							// u+=du
	add.l		a1,d5							// v+=dv
	add.l		a3,a2							// i+=di
	dbra		d0,@Loop

@Done:
	rts	   
 }
#endif


void gri_trans_lit_floor_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_TRANS|GRL_LOG2;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_TRANS;
   }
   tli->loop_func=(void (*)()) gri_lit_floor_umap_loop;
   tli->left_edge_func=(void (*)()) gri_uviwx_edge;
   tli->right_edge_func=(void (*)()) gri_uviwx_edge;
}

void gri_opaque_lit_floor_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_OPAQUE|GRL_LOG2;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_OPAQUE;
   }
   tli->loop_func=(void (*)()) gri_lit_floor_umap_loop;
   tli->left_edge_func=(void (*)()) gri_uviwx_edge;
   tli->right_edge_func=(void (*)()) gri_uviwx_edge;
}
