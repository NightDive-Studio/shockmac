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
// $Source: r:/prj/lib/src/2d/RCS/fl8s.asm $
// $Revision: 1.1 $
// $Author: kevin $
// $Date: 1994/08/16 12:34:32 $
//
// Inner loops of scaling and clut scaling primitives.
//
// This file is part of the 2d library.
//

#include "tmapint.h"
#include "flat8.h"
#include "cnvdat.h"
#include "grnull.h"
#include "gente.h"
#include "poly.h"
#include "grpix.h"

// globals
long	ADD_DEST_OFF;
long	ADD_DV_FRAC_OFF;
long	AND_BM_ROW_OFF;
long	ADD_SRC_OFF;
long	SET_REPS_OFF;
long	SET_OFFSET_OFF;
long	JMP_LOOP_MIDDLE_OFF;
 
#define unroll_num 4
#define unroll_log 2

// externs 
extern int gri_poly_loop (grs_tmap_loop_info *ti);
 
// internal prototypes
int gri_scale_umap_loop_PPC(grs_tmap_loop_info *tli);
int gri_scale_umap_loop_68K(grs_tmap_loop_info *tli);

// This file contains the scalers for both 68K and PowerPC
// First the routines that are generic to both, then the PowerPC routines, then 68K

// ------------------------------------------------------------------------
// Generic (68K & PowerPC) routines
// ------------------------------------------------------------------------

// ========================================================================
// opaque solid polygon scaler
int gri_opaque_solid_scale_umap_init(grs_tmap_loop_info *info, grs_vertex **vert)
 {
 	info->left_edge_func = (void (*)()) gri_scale_edge;
 	info->right_edge_func = (void (*)()) gr_null;
 	info->bm.hlog = 0;
 	info->bm.bits = info->clut;
 	info->loop_func = (void (*)()) gri_poly_loop;
 	info->d = ((uchar *) ((long) grd_canvas->bm.row * (long) info->y)); 	
 	info->d += (long)grd_canvas->bm.bits;
  return(0);
 } 

// ------------------------------------------------------------------------
// PowerPC routines
// ------------------------------------------------------------------------
#if defined(powerc) || defined(__powerc)

// ========================================================================
// transparent solid polygon scaler 
int gri_trans_solid_scale_umap_init(grs_tmap_loop_info *tli, grs_vertex **vert)
 {
	tli->bm.hlog=GRL_TRANS|GRL_SOLID;
	tli->loop_func=(void (*)()) gri_scale_umap_loop_PPC;
	tli->right_edge_func=gr_null;
	tli->left_edge_func=(void (*)()) gri_scale_edge;
 	return(0);
 }
 
// ========================================================================
// transparent bitmap scaler 
int gri_trans_scale_umap_init(grs_tmap_loop_info *tli, grs_vertex **vert)
 {
	tli->bm.hlog=GRL_TRANS;
	tli->loop_func=(void (*)()) gri_scale_umap_loop_PPC;
	tli->right_edge_func=gr_null;
	tli->left_edge_func=(void (*)()) gri_scale_edge;
 	return(0);
 }
 
// ========================================================================
// opaque bitmap scaler 
int gri_opaque_scale_umap_init(grs_tmap_loop_info *tli)
 {
	tli->bm.hlog=GRL_OPAQUE;
	tli->loop_func=(void (*)()) gri_scale_umap_loop_PPC;
	tli->right_edge_func=gr_null;
	tli->left_edge_func=(void (*)()) gri_scale_edge;
 	return(0);
 }

// ========================================================================
// transparent clut bitmap scaler 
int gri_trans_clut_scale_umap_init(grs_tmap_loop_info *tli)
 {
	tli->bm.hlog=GRL_TRANS|GRL_CLUT;
	tli->loop_func=(void (*)()) gri_scale_umap_loop_PPC;
	tli->right_edge_func=gr_null;
	tli->left_edge_func=(void (*)()) gri_scale_edge;
 	return(0);
 }

// ========================================================================
// opaque clut bitmap scaler 
int gri_opaque_clut_scale_umap_init(grs_tmap_loop_info *tli)
 {
	tli->bm.hlog=GRL_OPAQUE|GRL_CLUT;
	tli->loop_func=(void (*)()) gri_scale_umap_loop_PPC;
	tli->right_edge_func=gr_null;
	tli->left_edge_func=(void (*)()) gri_scale_edge;
 	return(0);
 }

// ========================================================================
// main inside loop for PPC scalers
int gri_scale_umap_loop_PPC(grs_tmap_loop_info *tli) {
   fix u,ul,du;
   int x;
   uchar k;
   fix xl,xr,dx,d;
   uchar *p_src,*p_dest;

   xl=fix_cint(tli->left.x);
   xr=fix_cint(tli->right.x);
   if (xr<=xl) return TRUE;
   ul=tli->left.u;
   dx=tli->right.x-tli->left.x;
   du=fix_div(tli->right.u-ul,dx);
   d =fix_ceil(tli->left.x)-tli->left.x;
   ul+=fix_mul(du,d);
   
   do {
      p_src=tli->bm.bits+tli->bm.row*fix_int(tli->left.v);
      p_dest = grd_bm.bits + (grd_bm.row*tli->y) + xl;
      switch (tli->bm.hlog) {
      case GRL_OPAQUE:
         for (x=xl,u=ul; x<xr; x++) {
            *(p_dest++) = p_src[fix_fint(u)];		// gr_fill_upixel(k,x,tli->y);
            u+=du;
         }
         break;
      case GRL_TRANS:
         for (x=xl,u=ul; x<xr; x++) {
            if (k=p_src[fix_fint(u)]) *p_dest = k;		// gr_fill_upixel(k,x,tli->y);
            u+=du;
            p_dest++;
         }
         break;
      case GRL_OPAQUE|GRL_CLUT:
         for (x=xl,u=ul; x<xr; x++) {
            *(p_dest++) = tli->clut[p_src[fix_fint(u)]];	// gr_fill_upixel(tli->clut[k],x,tli->y);
            u+=du;
         }
         break;
      case GRL_TRANS|GRL_CLUT:
         for (x=xl,u=ul; x<xr; x++) {
            if (k=p_src[fix_fint(u)]) *p_dest = tli->clut[k];	// gr_fill_upixel(tli->clut[k],x,tli->y);
            u+=du;
            p_dest++;
         }
         break;
      case GRL_TRANS|GRL_SOLID:
         for (x=xl,u=ul; x<xr; x++) {
            if (k=p_src[fix_fint(u)]) *p_dest = (uchar )(tli->clut);	// gr_fill_upixel((uchar )(tli->clut),x,tli->y);
            u+=du;
            p_dest++;
         }
         break;
      }
      tli->left.v+=tli->left.dv;
      tli->y++;
   } while (--(tli->n) > 0);
   
   return FALSE; /* tmap OK */
}
 
// ------------------------------------------------------------------------
// 68K routines
// ------------------------------------------------------------------------
#else

// ========================================================================
// transparent solid polygon scaler 
int gri_trans_solid_scale_umap_init(grs_tmap_loop_info *tli, grs_vertex **vert)
 {
	tli->bm.hlog=GRL_TRANS|GRL_SOLID;
	tli->loop_func=(void (*)()) gri_scale_umap_loop_68K;
	tli->right_edge_func=gr_null;
	tli->left_edge_func=(void (*)()) gri_scale_edge;
 	return(0);
 }
 
// ========================================================================
// transparent bitmap scaler 
int gri_trans_scale_umap_init(grs_tmap_loop_info *tli, grs_vertex **vert)
 {
	tli->bm.hlog=GRL_TRANS;
	tli->loop_func=(void (*)()) gri_scale_umap_loop_68K;
	tli->right_edge_func=gr_null;
	tli->left_edge_func=(void (*)()) gri_scale_edge;
 	return(0);
 }
 
// ========================================================================
// opaque bitmap scaler 
int gri_opaque_scale_umap_init(grs_tmap_loop_info *tli)
 {
	tli->bm.hlog=GRL_OPAQUE;
	tli->loop_func=(void (*)()) gri_scale_umap_loop_68K;
	tli->right_edge_func=gr_null;
	tli->left_edge_func=(void (*)()) gri_scale_edge;
 	return(0);
 }

// ========================================================================
// transparent clut bitmap scaler 
int gri_trans_clut_scale_umap_init(grs_tmap_loop_info *tli)
 {
	tli->bm.hlog=GRL_TRANS|GRL_CLUT;
	tli->loop_func=(void (*)()) gri_scale_umap_loop_68K;
	tli->right_edge_func=gr_null;
	tli->left_edge_func=(void (*)()) gri_scale_edge;
 	return(0);
 }

// ========================================================================
// opaque clut bitmap scaler 
int gri_opaque_clut_scale_umap_init(grs_tmap_loop_info *tli)
 {
	tli->bm.hlog=GRL_OPAQUE|GRL_CLUT;
	tli->loop_func=(void (*)()) gri_scale_umap_loop_68K;
	tli->right_edge_func=gr_null;
	tli->left_edge_func=(void (*)()) gri_scale_edge;
 	return(0);
 }

asm void ILoop68k(int count, fix ul, fix du, uchar *p_src, uchar *p_dest, grs_tmap_loop_info *tli);

// ========================================================================
// main inside loop for 68K scalers
int gri_scale_umap_loop_68K(grs_tmap_loop_info *tli) {
   fix u,ul,du;
   fix xl,xr,dx,d;
   uchar *p_src,*p_dest;

   xl=fix_cint(tli->left.x);
   xr=fix_cint(tli->right.x);
   if (xr<=xl) return TRUE;
   ul=tli->left.u;
   dx=tli->right.x-tli->left.x;
   du=fix_div(tli->right.u-ul,dx);
   d =fix_ceil(tli->left.x)-tli->left.x;
   ul+=fix_mul(du,d);
   
   p_src=tli->bm.bits+tli->bm.row*fix_int(tli->left.v);
   p_dest = grd_bm.bits + (grd_bm.row*tli->y) + xl;
   ILoop68k(xr-xl, ul, du, p_src, p_dest, tli);
   return FALSE; /* tmap OK */
}


// ========================================================================
// 68K inner loop for scalers
asm void ILoop68k(int count, fix ul, fix du, uchar *p_src, uchar *p_dest, grs_tmap_loop_info *tli)
 {
 	movem.l	d3-d7/a2-a4,-(sp)		// save regs
 	movem.l	36(sp),d0-d2/a0-a2	// get parms
 	 
 	move.l	a1,d4		// save p_dest
 	subq.w	#1,d0		// -1 for dbra
	move.w	d0,a4		// save

	move.l	d2,d5
	swap		d5			// high word of du in d5
	
	move.b	0x17(a2),d0
	bne.s		@Not_OPAQUE	// check type

// --------------------------------------------------
@OPAQUE:	
	move.w	a4,d0		// restore count
	move.l	d1,d3		// d3 = u = ul
	move.l	d3,d7
 	swap		d7			// high word of u in d7
	 	
@O_loop:
	move.b	(a0,d7.w),(a1)+	// move pixel
	add.w		d2,d3		// add low word
	addx.w	d5,d7		// add high word with carry
 	dbra		d0,@O_loop			// next pixel
 
@O_done:
	move.w	0x28(a2),d0
	move.l	0x38(a2),d3
	add.l		d3,0x28(a2)					//    tli->left.v+=tli->left.dv;
	add.l		#1,4(a2)						//    tli->y++;
	
  move.l	grd_canvas,a3									
	move.w	0x0c(a3),a1
	add.l		d4,a1								// get new dest
	move.l	a1,d4 
	
	sub.w		0x28(a2),d0					// move src ptr
	neg.w		d0
	beq.s		@O_skipit
	subq.w	#1,d0
	move.w	0x14(a2),a3
	
@O_srcloop:
	add.l		a3,a0								// p_src += tli->bm.row
	dbra		d0,@O_srcloop	
	
@O_skipit:	
	sub.l		#1,(a2)
	bne.s		@OPAQUE							//   } while (--(tli->n) > 0);
	bra			@exit
		
// --------------------------------------------------
@Not_OPAQUE:
	cmp.b		#GRL_TRANS,d0
	bne.s		@Not_TRANS

@TRANS:
	move.w	a4,d0		// restore count
	move.l	d1,d3		// d3 = u = ul
	move.l	d3,d7
 	swap		d7			// high word of u in d7
	 	
@T_loop:
	move.b	(a0,d7.w),d6	// get pixel
	beq.s		@T_skip				// transparent?
	move.b	d6,(a1)+			// copy it
	add.w		d2,d3		// add low word
	addx.w	d5,d7		// add high word with carry
 	dbra		d0,@T_loop			// next pixel
	bra.s		@T_done
	
@T_skip:									// don't copy
	addq.w	#1,a1
	add.w		d2,d3		// add low word
	addx.w	d5,d7		// add high word with carry
 	dbra		d0,@T_loop			// next pixel
 
@T_done:
	move.w	0x28(a2),d0
	move.l	0x38(a2),d3
	add.l		d3,0x28(a2)					//    tli->left.v+=tli->left.dv;
	add.l		#1,4(a2)						//    tli->y++;
	
  move.l	grd_canvas,a3									
	move.w	0x0c(a3),a1
	add.l		d4,a1								// get new dest
	move.l	a1,d4 
	
	sub.w		0x28(a2),d0					// move src ptr
	neg.w		d0
	beq.s		@T_skipit
	subq.w	#1,d0
	move.w	0x14(a2),a3
	
@T_srcloop:
	add.l		a3,a0								// p_src += tli->bm.row
	dbra		d0,@T_srcloop	
	
@T_skipit:	
	sub.l		#1,(a2)
	bne.s		@TRANS							//   } while (--(tli->n) > 0);
	bra			@exit

// --------------------------------------------------
@Not_TRANS:
	cmp.b		#(GRL_OPAQUE|GRL_CLUT),d0
	bne.s		@Not_OPAQUE_CLUT

	moveq		#0,d6
@OPAQUE_CLUT:	
	move.l	0x70(a2),a3			// clut ptr in a3
	move.w	a4,d0						// restore count
	move.l	d1,d3						// d3 = u = ul
 	swap		d3
	 	
@OC_loop:
	move.b	(a0,d3.w),d6
	move.b	(a3,d6.w),(a1)+	// move pixel
	swap		d3
	add.l		d2,d3
	swap		d3 
 	dbra		d0,@OC_loop			// next pixel
 
@OC_done:
	move.w	0x28(a2),d0
	move.l	0x38(a2),d3
	add.l		d3,0x28(a2)					//    tli->left.v+=tli->left.dv;
	add.l		#1,4(a2)						//    tli->y++;
	
  move.l	grd_canvas,a3									
	move.w	0x0c(a3),a1
	add.l		d4,a1								// get new dest
	move.l	a1,d4 
	
	sub.w		0x28(a2),d0					// move src ptr
	neg.w		d0
	beq.s		@OC_skipit
	subq.w	#1,d0
	move.w	0x14(a2),a3
	
@OC_srcloop:
	add.l		a3,a0								// p_src += tli->bm.row
	dbra		d0,@OC_srcloop	
	
@OC_skipit:	
	sub.l		#1,(a2)
	bne.s		@OPAQUE_CLUT				//   } while (--(tli->n) > 0);
	bra			@exit

// --------------------------------------------------
@Not_OPAQUE_CLUT:
	cmp.b		#(GRL_TRANS|GRL_CLUT),d0
	bne.s		@Not_TRANS_CLUT

	move		#0,d6
@TRANS_CLUT:
	move.l	0x70(a2),a3			// clut ptr in a3
	move.w	a4,d0						// restore count
	move.l	d1,d3						// d3 = u = ul
 	swap		d3
	 	
@TC_loop:
	move.b	(a0,d3.w),d6		// get pixel
	beq.s		@TC_skip				// transparent?
	move.b	(a3,d6.w),(a1)+	// move pixel
	swap		d3
	add.l		d2,d3
	swap		d3
 	dbra		d0,@TC_loop			// next pixel
	bra.s		@TC_done
	
@TC_skip:									// don't copy
	addq.w	#1,a1
	swap		d3
	add.l		d2,d3
	swap		d3 
 	dbra		d0,@TC_loop			// next pixel
 
@TC_done:
	move.w	0x28(a2),d0
	move.l	0x38(a2),d3
	add.l		d3,0x28(a2)					//    tli->left.v+=tli->left.dv;
	add.l		#1,4(a2)						//    tli->y++;
	
  move.l	grd_canvas,a3									
	move.w	0x0c(a3),a1
	add.l		d4,a1								// get new dest
	move.l	a1,d4 
	
	sub.w		0x28(a2),d0					// move src ptr
	neg.w		d0
	beq.s		@TC_skipit
	subq.w	#1,d0
	move.w	0x14(a2),a3
	
@TC_srcloop:
	add.l		a3,a0								// p_src += tli->bm.row
	dbra		d0,@TC_srcloop	
	
@TC_skipit:	
	sub.l		#1,(a2)
	bne.s		@TRANS_CLUT					//   } while (--(tli->n) > 0);
	bra			@exit

// --------------------------------------------------
@Not_TRANS_CLUT:
	cmp.b		#(GRL_TRANS|GRL_SOLID),d0
	bne.s		@exit

	move.b	0x70(a2),d6			// solid color in d6
@TRANS_SOLID:
	move.w	a4,d0						// restore count
	move.l	d1,d3						// d3 = u = ul
 	swap		d3
	 	
@TS_loop:
	tst.b		(a0,d3.w)				// get pixel
	beq.s		@TS_skip				// transparent?
	move.b	d6,(a1)+				// put pixel
	swap		d3
	add.l		d2,d3
	swap		d3
 	dbra		d0,@TS_loop			// next pixel
	bra.s		@TS_done
	
@TS_skip:									// don't copy
	addq.w	#1,a1
	swap		d3
	add.l		d2,d3
	swap		d3 
 	dbra		d0,@TS_loop			// next pixel
 
@TS_done:
	move.w	0x28(a2),d0
	move.l	0x38(a2),d3
	add.l		d3,0x28(a2)					//    tli->left.v+=tli->left.dv;
	add.l		#1,4(a2)						//    tli->y++;
	
  move.l	grd_canvas,a3									
	move.w	0x0c(a3),a1
	add.l		d4,a1								// get new dest
	move.l	a1,d4 
	
	sub.w		0x28(a2),d0					// move src ptr
	neg.w		d0
	beq.s		@TS_skipit
	subq.w	#1,d0
	move.w	0x14(a2),a3
	
@TS_srcloop:
	add.l		a3,a0								// p_src += tli->bm.row
	dbra		d0,@TS_srcloop	
	
@TS_skipit:	
	sub.l		#1,(a2)
	bne.s		@TRANS_SOLID				//   } while (--(tli->n) > 0);

// --------------------------------------------------
@exit:
 	movem.l	(sp)+,d3-d7/a2-a4		// restore regs
 	rts
 }

 
#endif
