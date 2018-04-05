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
 * $Source: r:/prj/lib/src/2d/RCS/fl8tp.c $
 * $Revision: 1.2 $
 * $Author: kevin $
 * $Date: 1994/08/16 12:57:22 $
 * 
 * full perspective texture mapper.
 * scanline processors.
 * 
*/

#include "grpix.h"
#include "pertyp.h"
#include "plytyp.h"
#include "cnvdat.h"
#include "fl8tmapdv.h"

// prototypes
void gri_trans_per_umap_hscan_scanline(grs_per_info *pi, grs_bitmap *bm);
void gri_trans_per_umap_vscan_scanline(grs_per_info *pi, grs_bitmap *bm);
void gri_trans_per_umap_hscan_init(grs_bitmap *bm, grs_per_setup *ps);
void gri_trans_per_umap_vscan_init(grs_bitmap *bm, grs_per_setup *ps);

// 68K stuff
#if !(defined(powerc) || defined(__powerc))	
asm void trans_per_hscan_68K_Loop(int dx, fix l_du, fix l_dv, fix *l_u, fix *l_v, uchar **p, fix *l_y_fix, int *y_cint);
asm void trans_per_vscan_68K_Loop(int dy, fix l_du, fix l_dv, fix *l_u, fix *l_v, uchar **p, fix *l_x_fix, int *x_cint);

// 68k globals
int 	t_l_u_mask,t_l_v_mask,t_l_v_shift;
uchar *t_bm_bits;
fix 	t_l_scan_slope;
int		t_gr_row;

#endif

void gri_trans_per_umap_hscan_scanline(grs_per_info *pi, grs_bitmap *bm) {
   register int k,y_cint;
   uchar *p,temp_pix;

	 // locals used to speed PPC code
	 fix	l_u,l_v,l_du,l_dv,l_y_fix,l_scan_slope,l_dtl,l_dxl,l_dyl,l_dtr,l_dyr;
	 int	l_x,l_xl,l_xr,l_xr0,l_u_mask,l_v_mask,l_v_shift;
	 int	gr_row,temp_y;
	 uchar *bm_bits;
	 
	 gr_row = grd_bm.row;
	 bm_bits = bm->bits;
	 l_dyr = pi->dyr;
	 l_dtr = pi->dtr;
	 l_dyl = pi->dyl;
	 l_dxl = pi->dxl;
	 l_dtl = pi->dtl;
	 l_scan_slope = pi->scan_slope;
	 l_y_fix = pi->y_fix;
	 l_v_shift = pi->v_shift;
	 l_v_mask = pi->v_mask;
	 l_u_mask = pi->u_mask;
	 l_xr0 = pi->xr0;
	 l_x = pi->x;
	 l_xl = pi->xl;
	 l_xr = pi->xr;
   l_u = pi->u;
   l_v = pi->v;
   l_du = pi->du;
   l_dv = pi->dv;
   
   l_y_fix=l_x*l_scan_slope+fix_make(pi->yp,0xffff);

#if InvDiv
	 k = fix_div(fix_make(1,0),pi->denom);
   l_u = pi->u0 + fix_mul_asm_safe(pi->unum,k);
   l_v = pi->v0 + fix_mul_asm_safe(pi->vnum,k);
   l_du = fix_mul_asm_safe(pi->dunum,k);
   l_dv = fix_mul_asm_safe(pi->dvnum,k);
#else
   l_u = pi->u0 + fix_div(pi->unum,pi->denom);
   l_v = pi->v0 + fix_div(pi->vnum,pi->denom);
   l_du = fix_div(pi->dunum,pi->denom);
   l_dv = fix_div(pi->dvnum,pi->denom);
#endif

   l_u += l_x*l_du;
   l_v += l_x*l_dv;

   y_cint=fix_int(l_y_fix);
	 if (l_scan_slope<0) gr_row = -gr_row;
   p=grd_bm.bits+l_x+y_cint*grd_bm.row;
   if (l_x<l_xl) {
      fix test=l_x*l_dyl-y_cint*l_dxl+pi->cl;
      for (;l_x<l_xl;l_x++) {
         if (test<=0) {
            int k=(l_u>>16)&l_u_mask;
            k+=(l_v>>l_v_shift)&l_v_mask;
            if (temp_pix=bm_bits[k]) *p=temp_pix;		// gr_fill_upixel(bm_bits[k],l_x,y_cint);
         }
         temp_y = y_cint; 
         y_cint = fix_int(l_y_fix+=l_scan_slope);
 				 if (temp_y!=y_cint)
 				 	 {p+=gr_row; test+=l_dtl;}
 				 else
 				   test+=l_dyl;

         p++;
         l_u+=l_du;
         l_v+=l_dv;
      }
   }
   
#if (defined(powerc) || defined(__powerc))	
   for (;l_x<l_xr0;l_x++) {
      int k=(l_u>>16)&l_u_mask;
      k+=(l_v>>l_v_shift)&l_v_mask;
      if (temp_pix=bm_bits[k]) *p=temp_pix;		// gr_fill_upixel(bm_bits[k],l_x,y_cint);
      temp_y = y_cint; 
      y_cint = fix_int(l_y_fix+=l_scan_slope);
		  if (temp_y!=y_cint)
		 	 	p+=gr_row;

			p++;
      l_u+=l_du;
      l_v+=l_dv;
   }
#else
	if (l_x<l_xr0)
	 {
	 	t_l_v_shift = l_v_shift;
	 	t_l_u_mask = l_u_mask;
		t_l_v_mask = l_v_mask;
	  t_bm_bits = bm->bits;
	  t_l_scan_slope = l_scan_slope;
	  t_gr_row = grd_bm.row;
		trans_per_hscan_68K_Loop(l_xr0-l_x, l_du, l_dv, &l_u, &l_v, &p, &l_y_fix, &y_cint);
		l_x=l_xr0;
	 }
#endif
   
   if (l_x<l_xr) {
      fix test=l_x*l_dyr-y_cint*pi->dxr+pi->cr;
   		p=grd_bm.bits+l_x+y_cint*grd_bm.row;
      for (;l_x<l_xr;l_x++) {
         if (test>=0) {
            int k=(l_u>>16)&l_u_mask;
            k+=(l_v>>l_v_shift)&l_v_mask;
            if (temp_pix=bm_bits[k]) *p=temp_pix;		// gr_fill_upixel(bm_bits[k],l_x,y_cint);
         }
      	temp_y = y_cint; 
      	y_cint = fix_int(l_y_fix+=l_scan_slope);
				if (temp_y!=y_cint)
					{p+=gr_row; test+=l_dtr;}
				else
				  test+=l_dyr;
            
         p++;
         l_u+=l_du;
         l_v+=l_dv;
      }
   }
   
	pi->y_fix = l_y_fix;
	pi->x = l_x;
	pi->u = l_u;
	pi->v = l_v;
	pi->du = l_du;
	pi->dv = l_dv;
}

#if !(defined(powerc) || defined(__powerc))	
asm void trans_per_hscan_68K_Loop(int dx, fix l_du, fix l_dv, fix *l_u, fix *l_v, uchar **p, fix *l_y_fix, int *y_cint)
 { 
/*   for (;l_x<l_xr0;l_x++) {
      int k=(l_u>>16)&l_u_mask;
      k+=(l_v>>l_v_shift)&l_v_mask;
      if (temp_pix=bm_bits[k]) *p=temp_pix;		// gr_fill_upixel(bm_bits[k],l_x,y_cint);
      temp_y = y_cint; 
      y_cint = fix_int(l_y_fix+=l_scan_slope);
      if (temp_y!=y_cint)		// y_cint=fix_int((l_y_fix+=l_scan_slope));
       {
     		temp_y -= y_cint;
     		if (temp_y>0)
     			while (temp_y--) p-=gr_row;
     		else
       		while (temp_y++<0) p+=gr_row;
       }
      l_u+=l_du;
      l_v+=l_dv;
   }*/

  movem.l	d0-d7/a0-a6,-(sp)    
	
	move.l	64(sp),d0		// dx
	subq.w	#1,d0				// for dbra		
	move.l	68(sp),a4		// l_du
	move.l	72(sp),a5		// l_dv
	move.l	76(sp),a3		// *l_u
	move.l	(a3),d1			// l_u
	move.l	80(sp),a2		// *l_v
	move.l	(a2),d2			// l_v
	move.l	84(sp),a1		// *p
	move.l	88(sp),a0
	move.l	(a0),a6			// l_y_fix
	move.l	92(sp),a0
	move.l	(a0),d4			// y_cint
	move.l	t_l_v_shift,d3
	move.l	(a1),a0
	move.l	t_bm_bits,a1
	move.l	t_gr_row,d5
	move.l	t_l_scan_slope,a2
	
@Loop:
	move.l	d1,d6
	swap		d6
	ext.l		d6							// l_u>>16
	and.l		t_l_u_mask,d6		// k = (l_u>>16)&l_u_mask
	move.l	d2,d7
	asr.l		d3,d7						// l_v>>l_v_shift
	and.l		t_l_v_mask,d7		// (l_v>>l_v_shift)&l_v_mask
	add.l		d7,d6						// k+=(l_v>>l_v_shift)&l_v_mask;
	move.b	(a1,d6.l),d7
	beq.s		@skippix				// if (temp_pix=bm_bits[k])
	move.b	d7,(a0)					//		*p=bm_bits[k];
@skippix:
	
	move.l	d4,d6							// temp_y = y_cint
	add.l		a2,a6							// l_y_fix+=l_scan_slope
	move.l	a6,d4							// y_cint = l_y_fix
	swap		d4
	ext.l		d4
	cmp.l		d4,d6
	beq			@skip
	
	sub.l		d4,d6	 					// temp_y -= y_cint
	bmi.s		@neg

	subq.w	#1,d6
@pos:
	sub.l		d5,a0
	dbra		d6,@pos
	bra.s		@skip

@neg:
	neg.w		d6
	subq.w	#1,d6
@neg2:
	add.l		d5,a0
	dbra		d6,@neg2
	
@skip:
	addq.w	#1,a0			// p++
	add.l		a4,d1
	add.l		a5,d2
	dbra		d0,@Loop
			
//	move.l	76(sp),a3		// *l_u
	move.l	d1,(a3)		// save l_u
	move.l	80(sp),a2		// *l_v
	move.l	d2,(a2)		// save l_v

	move.l	84(sp),a1		
	move.l	a0,(a1)			// save p
	move.l	88(sp),a0
	move.l	a6,(a0)			// save l_y_fix
	move.l	92(sp),a0
	move.l	d4,(a0)			// save y_cint
						
  movem.l	(sp)+,d0-d7/a0-a6 
	rts
 }
#endif


void gri_trans_per_umap_vscan_scanline(grs_per_info *pi, grs_bitmap *bm) {
   register int k,x_cint;

	 // locals used to speed PPC code
	 fix	l_dxr,l_x_fix,l_u,l_v,l_du,l_dv,l_scan_slope,l_dtl,l_dxl,l_dyl,l_dtr,l_dyr;
	 int	l_yl,l_yr0,l_yr,l_y,l_u_mask,l_v_mask,l_v_shift;
	 int	gr_row,temp_x;
	 uchar *bm_bits;
	 uchar *p,temp_pix;

	 gr_row = grd_bm.row;
	 bm_bits = bm->bits;
   l_dxr = pi->dxr;
   l_x_fix = pi->x_fix;
	 l_y = pi->y;
	 l_yr = pi->yr;
	 l_yr0 = pi->yr0;
	 l_yl = pi->yl;
	 l_dyr = pi->dyr;
	 l_dtr = pi->dtr;
	 l_dyl = pi->dyl;
	 l_dxl = pi->dxl;
	 l_dtl = pi->dtl;
	 l_scan_slope = pi->scan_slope;
	 l_v_shift = pi->v_shift;
	 l_v_mask = pi->v_mask;
	 l_u_mask = pi->u_mask;
   l_u = pi->u;
   l_v = pi->v;
   l_du = pi->du;
   l_dv = pi->dv;

   l_x_fix=l_y*l_scan_slope+fix_make(pi->xp,0xffff);

#if InvDiv
	 k = fix_div(fix_make(1,0),pi->denom);
   l_u = pi->u0 + fix_mul_asm_safe(pi->unum,k);
   l_v = pi->v0 + fix_mul_asm_safe(pi->vnum,k);
   l_du = fix_mul_asm_safe(pi->dunum,k);
   l_dv = fix_mul_asm_safe(pi->dvnum,k);
#else
   l_u = pi->u0 + fix_div(pi->unum,pi->denom);
   l_v = pi->v0 + fix_div(pi->vnum,pi->denom);
   l_du = fix_div(pi->dunum,pi->denom);
   l_dv = fix_div(pi->dvnum,pi->denom);
#endif

   l_u += l_y*l_du;
   l_v += l_y*l_dv;

   x_cint=fix_int(l_x_fix);
   p=grd_bm.bits+x_cint+l_y*gr_row;
   if (l_y<l_yl) {
      fix test=l_y*l_dxl-x_cint*l_dyl+pi->cl;
      for (;l_y<l_yl;l_y++) {
         if (test<=0) {
            int k=(l_u>>16)&l_u_mask;
            k+=(l_v>>l_v_shift)&l_v_mask;
            if (temp_pix=bm_bits[k]) *p=temp_pix;		// gr_fill_upixel(bm_bits[k],x_cint,l_y);
         }
         temp_x = x_cint;
         x_cint = fix_int(l_x_fix+=l_scan_slope);
         if (temp_x!=x_cint)
          {
            test+=l_dtl;
         		p -= (temp_x-x_cint);
          }
         else
            test+=l_dxl;
          
         p+=gr_row;
         l_u+=l_du;
         l_v+=l_dv;
      }
   }
   
#if (defined(powerc) || defined(__powerc))	
   for (;l_y<l_yr0;l_y++) {
      int k=(l_u>>16)&l_u_mask;
      k+=(l_v>>l_v_shift)&l_v_mask;
      if (temp_pix=bm_bits[k]) *p=temp_pix;		// gr_fill_upixel(bm_bits[k],x_cint,l_y);

      temp_x = x_cint;
      x_cint=fix_int(l_x_fix+=l_scan_slope);
      if (temp_x!=x_cint)
      	p -= (temp_x-x_cint);

      p+=gr_row;
      l_u+=l_du;
      l_v+=l_dv;
   }
#else
	if (l_y<l_yr0)
	 {
	 	t_l_v_shift = l_v_shift;
	 	t_l_u_mask = l_u_mask;
		t_l_v_mask = l_v_mask;
	  t_bm_bits = bm->bits;
	  t_l_scan_slope = l_scan_slope;
	  t_gr_row = gr_row;
	 	trans_per_vscan_68K_Loop(l_yr0-l_y, l_du, l_dv, &l_u, &l_v, &p, &l_x_fix, &x_cint);
	 	l_y  = l_yr0;
	 }
#endif
   
   if (l_y<l_yr) {
      fix test=l_y*l_dxr-x_cint*l_dyr+pi->cr;
   		p=grd_bm.bits+x_cint+l_y*gr_row;
      for (;l_y<l_yr;l_y++) {
         if (test>=0) {
            int k=(l_u>>16)&l_u_mask;
            k+=(l_v>>l_v_shift)&l_v_mask;
            if (temp_pix=bm_bits[k]) *p=temp_pix;		// gr_fill_upixel(bm_bits[k],x_cint,l_y);
         }

         temp_x = x_cint;
         x_cint = fix_int(l_x_fix+=l_scan_slope);
         if (temp_x!=x_cint)
          {
            test+=l_dtr;
      			p -= (temp_x-x_cint);
          }
         else
            test+=l_dxr;
					
      	 p+=gr_row;
         l_u+=l_du;
         l_v+=l_dv;
      }
   }

	pi->x_fix = l_x_fix;
	pi->y = l_y;
	pi->u = l_u;
	pi->v = l_v;
	pi->du = l_du;
	pi->dv = l_dv;
}

#if !(defined(powerc) || defined(__powerc))	
asm void trans_per_vscan_68K_Loop(int dy, fix l_du, fix l_dv, fix *l_u, fix *l_v, uchar **p, fix *l_x_fix, int *x_cint)
 {
/*    for (;l_y<l_yr0;l_y++) {
      int k=(l_u>>16)&l_u_mask;
      k+=(l_v>>l_v_shift)&l_v_mask;
      *p=bm_bits[k];		// gr_fill_upixel(bm_bits[k],x_cint,l_y);

      temp_x = x_cint;
      x_cint=fix_int(l_x_fix+=l_scan_slope);
      if (temp_x!=x_cint)
      	p -= (temp_x-x_cint);

      p+=gr_row;
      l_u+=l_du;
      l_v+=l_dv;
*/
  movem.l	d0-d7/a0-a6,-(sp)    

	move.l	64(sp),d0		// dy
	subq.w	#1,d0				// for dbra		
	move.l	68(sp),a4		// l_du
	move.l	72(sp),a5		// l_dv
	move.l	76(sp),a3		// *l_u
	move.l	(a3),d1			// l_u
	move.l	80(sp),a2		// *l_v
	move.l	(a2),d2			// l_v
	move.l	84(sp),a1		// *p
	move.l	88(sp),a0
	move.l	(a0),a6			// l_y_fix
	move.l	92(sp),a0
	move.l	(a0),d4			// y_cint
	move.l	t_l_v_shift,d3
	move.l	(a1),a0			// P
	move.l	t_bm_bits,a1
	move.l	t_gr_row,d5
	move.l	t_l_scan_slope,a2
	
@Loop:
	move.l	d1,d6
	swap		d6
	ext.l		d6							// l_u>>16
	and.l		t_l_u_mask,d6		// k = (l_u>>16)&l_u_mask
	move.l	d2,d7
	asr.l		d3,d7						// l_v>>l_v_shift
	and.l		t_l_v_mask,d7		// (l_v>>l_v_shift)&l_v_mask
	add.l		d7,d6						// k+=(l_v>>l_v_shift)&l_v_mask;
	move.b	(a1,d6.l),d7
	beq.s		@skippix
	move.b	d7,(a0)					// *p=bm_bits[k];
@skippix:

	move.w	d4,d6							// temp_y = y_cint
	add.l		a2,a6						// l_x_fix+=l_scan_slope
	move.l	a6,d4							// x_cint=fix_int(l_x_fix+=l_scan_slope);
	swap		d4
	ext.l		d4
	sub.w		d4,d6
	sub.w		d6,a0							// p -= (temp_x-x_cint);
	
	add.l		d5,a0					// p+=gr_row;
	add.l		a4,d1					// l_u+=l_du;
	add.l		a5,d2					// l_v+=l_dv;
	dbra		d0,@Loop

//	move.l	76(sp),a3			// *l_u
	move.l	d1,(a3)				// save l_u
	move.l	80(sp),a2			// *l_v
	move.l	d2,(a2)				// save l_v

	move.l	84(sp),a1		
	move.l	a0,(a1)				// save p
	move.l	88(sp),a0
	move.l	a6,(a0)				// save l_y_fix
	move.l	92(sp),a0
	move.l	d4,(a0)				// save y_cint

  movem.l	(sp)+,d0-d7/a0-a6 
	rts
 }
#endif


extern void gri_per_umap_hscan(grs_bitmap *bm, int n, grs_vertex **vpl, grs_per_setup *ps);
extern void gri_per_umap_vscan(grs_bitmap *bm, int n, grs_vertex **vpl, grs_per_setup *ps);

void gri_trans_per_umap_hscan_init(grs_bitmap *bm, grs_per_setup *ps) {
   ps->shell_func=(void (*)()) gri_per_umap_hscan;
   ps->scanline_func=(void (*)()) gri_trans_per_umap_hscan_scanline;
}

void gri_trans_per_umap_vscan_init(grs_bitmap *bm, grs_per_setup *ps) {
   ps->shell_func=(void (*)()) gri_per_umap_vscan;
   ps->scanline_func=(void (*)()) gri_trans_per_umap_vscan_scanline;
}


