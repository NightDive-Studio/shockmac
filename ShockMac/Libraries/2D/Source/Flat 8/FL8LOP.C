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
 * $Source: r:/prj/lib/src/2d/RCS/fl8lop.c $
 * $Revision: 1.2 $
 * $Author: kevin $
 * $Date: 1994/08/16 12:57:18 $
 * 
 * lit full perspective texture mapper.
 * scanline processors.
 * 
*/

#include "grpix.h"
#include "pertyp.h"
#include "scrdat.h"
#include "plytyp.h"
#include "cnvdat.h"
#include "tmapint.h"
#include "fl8tmapdv.h"

// prototypes
void gri_opaque_lit_per_umap_hscan_scanline(grs_per_info *pi, grs_bitmap *bm);
void gri_opaque_lit_per_umap_vscan_scanline(grs_per_info *pi, grs_bitmap *bm);
void gri_opaque_lit_per_umap_hscan_init(grs_bitmap *bm, grs_per_setup *ps);
void gri_opaque_lit_per_umap_vscan_init(grs_bitmap *bm, grs_per_setup *ps);


// 68K stuff
#if !(defined(powerc) || defined(__powerc))	
asm void opaque_lit_per_hscan_68K_Loop(int dx, fix l_du, fix l_dv, fix *l_u, fix *l_v,
																			 uchar **p, fix *l_y_fix, int *y_cint, fix l_di, fix *l_i);
asm void opaque_lit_per_vscan_68K_Loop(int dy, fix l_du, fix l_dv, fix *l_u, fix *l_v,
																			 uchar **p, fix *l_x_fix, int *x_cint, fix l_di, fix *l_i);

// 68k globals
int 	ol_l_u_mask,ol_l_v_mask,ol_l_v_shift;
uchar *ol_bm_bits;
fix 	ol_l_scan_slope;
int		ol_gr_row;
uchar *ol_ltab;
#endif

extern "C"
{
void opaque_lit_per_hscan_Loop_PPC(int dx, fix l_du, fix l_dv, fix *pl_u, fix *pl_v,
																	 uchar **pp, fix *pl_y_fix, int *py_cint, fix l_di, fix *pl_i,
																	 int l_u_mask, int l_v_shift, int l_v_mask, fix l_scan_slope,
		 														 	 uchar *bm_bits, int gr_row, uchar *ltab);
}
/*
void opaque_lit_per_hscan_Loop_C(int dx, fix l_du, fix l_dv, fix *pl_u, fix *pl_v,
																 uchar **pp, fix *pl_y_fix, int *py_cint, fix l_di, fix *pl_i,
																 int l_u_mask, int l_v_shift, int l_v_mask, fix l_scan_slope,
	 														 	 uchar *bm_bits, int gr_row, uchar *ltab)
 {
 	fix 	l_u,l_v,l_y_fix,l_i;
 	int		k,y_cint;
 	uchar *p;
 	
 	l_u = *pl_u;
 	l_v = *pl_v;
 	l_y_fix = *pl_y_fix;
 	l_i = *pl_i;
 	p = *pp;
 	y_cint = *py_cint;
 	
   for (;dx>0;dx--) 
    {
      k=((l_u>>16)&l_u_mask) + ((l_v>>l_v_shift)&l_v_mask);
      k=bm_bits[k];
      *(p++)=ltab[(fix_light(l_i))+k]; 

      k = y_cint; 
      y_cint = fix_int(l_y_fix+=l_scan_slope);
		  if (k!=y_cint)
		 	 	p+=gr_row;

      l_u+=l_du;
      l_v+=l_dv;
      l_i+=l_di;
    }
 	

 	*pl_u = l_u;
 	*pl_v = l_v;
 	*pl_y_fix = l_y_fix;
 	*pl_i = l_i;
 	*pp = p;
 	*py_cint = y_cint;
 }
*/															 


void gri_opaque_lit_per_umap_hscan_scanline(grs_per_info *pi, grs_bitmap *bm) {
   register uchar *ltab=grd_screen->ltab;
   register int 	l_x,y_cint,l_u_mask,l_v_mask,l_v_shift;
   register fix 	k,l_u,l_v,l_du,l_dv,l_i,l_di,l_y_fix,l_scan_slope;
	 register int		gr_row;
   register uchar *bm_bits,*p;

	 // locals used to speed PPC code
	 fix	test,l_dtl,l_dxl,l_dyl,l_dtr,l_dyr;
	 int	l_xl,l_xr,l_xr0;
	 
	 gr_row = grd_bm.row;
	 bm_bits = bm->bits;
	 l_xr0 = pi->xr0;
	 l_x = pi->x;
	 l_di = pi->di;
	 l_i = pi->i;
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
	 l_xl = pi->xl;
	 l_xr = pi->xr;
   l_u = pi->u;
   l_v = pi->v;
   l_du = pi->du;
   l_dv = pi->dv;

   l_y_fix=l_x*l_scan_slope+fix_make(pi->yp,0xffff);

 	 if (l_scan_slope<0) gr_row = -gr_row;

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
   p=grd_bm.bits+l_x+y_cint*grd_bm.row;
   if (l_x<l_xl) {
      test=l_x*l_dyl-y_cint*l_dxl+pi->cl;
      for (;l_x<l_xl;l_x++) {
         if (test<=0) {
            k=(l_u>>16)&l_u_mask;
            k+=(l_v>>l_v_shift)&l_v_mask;
            k=bm_bits[k];
            *p=ltab[(fix_light(l_i))+k]; 	// gr_fill_upixel(ltab[(fix_int(l_i)<<8)+k],l_x,y_cint);
         }

         k = y_cint; 
         y_cint = fix_int(l_y_fix+=l_scan_slope);
 				 if (k!=y_cint)
 				 	 {p+=gr_row; test+=l_dtl;}
 				 else
 				   test+=l_dyl;

         p++;
         l_u+=l_du;
         l_v+=l_dv;
         l_i+=l_di;
      }
   }

#if (defined(powerc) || defined(__powerc))	
	if (l_x<l_xr0)
	{
	 opaque_lit_per_hscan_Loop_PPC(l_xr0-l_x, l_du, l_dv, 
		 														 &l_u, &l_v, &p, &l_y_fix, 
		 														 &y_cint, l_di, &l_i,
		 														 l_u_mask, l_v_shift, l_v_mask, l_scan_slope,
		 														 bm_bits, gr_row, ltab);	
   l_x=l_xr0;
	}  
#else
	if (l_x<l_xr0)
	 {
	 	ol_l_v_shift = l_v_shift;
	 	ol_l_u_mask = l_u_mask;
		ol_l_v_mask = l_v_mask;
	  ol_bm_bits = bm->bits;
	  ol_l_scan_slope = l_scan_slope;
	  ol_gr_row = grd_bm.row;
	  ol_ltab = ltab;
		opaque_lit_per_hscan_68K_Loop(l_xr0-l_x, l_du, l_dv, &l_u, &l_v, &p, &l_y_fix, &y_cint, l_di, &l_i);

		l_x=l_xr0;
	 }
#endif
    
   
   if (l_x<l_xr) {
      test=l_x*l_dyr-y_cint*pi->dxr+pi->cr;
   		p=grd_bm.bits+l_x+y_cint*grd_bm.row;
      for (;l_x<l_xr;l_x++) {
         if (test>=0) {
            k=(l_u>>16)&l_u_mask;
            k+=(l_v>>l_v_shift)&l_v_mask;
            k=bm_bits[k];
            *p=ltab[(fix_light(l_i))+k]; 	// gr_fill_upixel(ltab[(fix_int(l_i)<<8)+k],l_x,y_cint);
         }

      	k = y_cint; 
      	y_cint = fix_int(l_y_fix+=l_scan_slope);
				if (k!=y_cint)
					{p+=gr_row; test+=l_dtr;}
				else
				  test+=l_dyr;
            
         p++;
         l_u+=l_du;
         l_v+=l_dv;
         l_i+=l_di;
      }
   }

	pi->y_fix = l_y_fix;
	pi->x = l_x;
	pi->u = l_u;
	pi->v = l_v;
	pi->du = l_du;
	pi->dv = l_dv;
	pi->di = l_di;
	pi->i = l_i;
}

#if !(defined(powerc) || defined(__powerc))	
asm void opaque_lit_per_hscan_68K_Loop(int dx, fix l_du, fix l_dv, fix *l_u, fix *l_v, 
																			 uchar **p, fix *l_y_fix, int *y_cint, fix l_di, fix *l_i)
 { 
/*   for (;l_x<l_xr0;l_x++) {
      int k=(l_u>>16)&l_u_mask;
      k+=(l_v>>l_v_shift)&l_v_mask;
      k=bm_bits[k];
      *(p++)=ltab[(fix_int(l_i)<<8)+k]; 	// gr_fill_upixel(ltab[(fix_int(l_i)<<8)+k],l_x,y_cint);

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
      l_i+=l_di;
   }
*/

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
	move.l	100(sp),a0
	move.l	(a0),d5			// l_i
	move.l	(a1),a0
	move.l	ol_bm_bits,a1
	move.l	ol_ltab, a2
	move.l	ol_l_v_shift,d3
	move.l	ol_l_scan_slope,a3
	
@Loop:
	move.l	d1,d6
	swap		d6
	ext.l		d6							// l_u>>16
	and.l		ol_l_u_mask,d6	// k = (l_u>>16)&l_u_mask
	move.l	d2,d7
	asr.l		d3,d7						// l_v>>l_v_shift
	and.l		ol_l_v_mask,d7		// (l_v>>l_v_shift)&l_v_mask
	add.l		d7,d6						// k+=(l_v>>l_v_shift)&l_v_mask;
	move.l	d5,d7
	lsr.l		#8,d7
//	swap		d7							// fix_int(l_i)
//	lsl.w		#8,d7
	move.b	(a1,d6.l),d7		// temp = (fix_int(l_i)<<8)+k;
	move.b	(a2,d7.w),(a0)+	// *(p++)=ltab[temp];
	
	move.l	d4,d6								// temp_y = y_cint
	add.l		a3,a6						// l_y_fix+=l_scan_slope
	move.l	a6,d4								// y_cint = l_y_fix
	swap		d4
	ext.l		d4
	cmp.l		d4,d6
	beq			@skip
	move.l	ol_gr_row,d7
	
	sub.l		d4,d6	 					// temp_y -= y_cint
	bmi.s		@neg

	subq.w	#1,d6
@pos:
	sub.l		d7,a0
	dbra		d6,@pos
	bra.s		@skip

@neg:
	neg.w		d6
	subq.w	#1,d6
@neg2:
	add.l		d7,a0
	dbra		d6,@neg2
	
@skip:	
	add.l	 	96(sp),d5		// l_i += di
	add.l		a4,d1				// l_u+=l_du
	add.l		a5,d2				// l_v+=l_dv
	dbra		d0,@Loop
			
	move.l	76(sp),a3		// *l_u
	move.l	d1,(a3)			// save l_u
	move.l	80(sp),a2		// *l_v
	move.l	d2,(a2)			// save l_v
	move.l	100(sp),a2	// *l_i
	move.l	d5,(a2)			// save l_i
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


extern "C"
{
void opaque_lit_per_vscan_Loop_PPC(int dy, fix l_du, fix l_dv, fix *pl_u, fix *pl_v,
																	 uchar **pp, fix *pl_y_fix, int *py_cint, fix l_di, fix *pl_i,
																	 int l_u_mask, int l_v_shift, int l_v_mask, fix l_scan_slope,
		 														 	 uchar *bm_bits, int gr_row, uchar *ltab);
}
/*
void opaque_lit_per_vscan_Loop_C(int dy, fix l_du, fix l_dv, fix *pl_u, fix *pl_v,
																 uchar **pp, fix *pl_x_fix, int *px_cint, fix l_di, fix *pl_i,
																 int l_u_mask, int l_v_shift, int l_v_mask, fix l_scan_slope,
	 														 	 uchar *bm_bits, int gr_row, uchar *ltab)
 {
 	fix 	l_u,l_v,l_x_fix,l_i;
 	int		k,x_cint;
 	uchar *p;
 	
 	l_u = *pl_u;
 	l_v = *pl_v;
 	l_x_fix = *pl_x_fix;
 	l_i = *pl_i;
 	p = *pp;
 	x_cint = *px_cint;
 	
   for (;dy>0;dy--) 
    {
      k=(l_u>>16)&l_u_mask;
      k+=(l_v>>l_v_shift)&l_v_mask;
      k=bm_bits[k];
      *p=ltab[(fix_light(l_i))+k];	

      k = x_cint;
      x_cint=fix_int(l_x_fix+=l_scan_slope);
      if (k!=x_cint)
      	p-=(k-x_cint);

      p+=gr_row;
      l_u+=l_du;
      l_v+=l_dv;
      l_i+=l_di;
    }

 	*pl_u = l_u;
 	*pl_v = l_v;
 	*pl_x_fix = l_x_fix;
 	*pl_i = l_i;
 	*pp = p;
 	*px_cint = x_cint;
 }
*/

void gri_opaque_lit_per_umap_vscan_scanline(grs_per_info *pi, grs_bitmap *bm) {
	 register int 	 k,x_cint;
   register uchar *ltab=grd_screen->ltab;
   
	 // locals used to speed PPC code
	 fix	l_dxr,l_x_fix,l_u,l_v,l_du,l_dv,l_scan_slope,l_dtl,l_dxl,l_dyl,l_dtr,l_dyr,l_i,l_di;
	 int	l_yl,l_yr0,l_yr,l_y,l_u_mask,l_v_mask,l_v_shift;
	 int	gr_row,temp_x;
	 uchar *bm_bits;
	 uchar *p;

	 gr_row = grd_bm.row;
	 bm_bits = bm->bits;
	 l_di = pi->di;
	 l_i = pi->i;
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
            k=bm_bits[k];
            *p=ltab[(fix_light(l_i))+k];		// gr_fill_upixel(ltab[(fix_int(l_i)<<8)+k],x_cint,l_y);
         }

         temp_x = x_cint;
         x_cint = fix_int(l_x_fix+=l_scan_slope);
         if (temp_x!=x_cint)
          {
            test+=l_dtl;
         		p-=temp_x-x_cint;
          }
         else
          {
            test+=l_dxl;
         		x_cint=fix_int(l_x_fix);
          }
          
         p+=gr_row;
         l_u+=l_du;
         l_v+=l_dv;
         l_i+=l_di;
      }
   }

#if (defined(powerc) || defined(__powerc))	
	if (l_y<l_yr0)
	 {
		 opaque_lit_per_vscan_Loop_PPC(l_yr0-l_y, l_du, l_dv, 
			 														 &l_u, &l_v, &p, &l_x_fix, 
			 														 &x_cint, l_di, &l_i,
			 														 l_u_mask, l_v_shift, l_v_mask, l_scan_slope,
			 														 bm_bits, gr_row, ltab);	
	   l_y=l_yr0;
   }
 #else
	if (l_y<l_yr0)
	 {
	 	ol_l_v_shift = l_v_shift;
	 	ol_l_u_mask = l_u_mask;
		ol_l_v_mask = l_v_mask;
	  ol_bm_bits = bm->bits;
	  ol_l_scan_slope = l_scan_slope;
	  ol_gr_row = gr_row;
	  ol_ltab = ltab;
	 	opaque_lit_per_vscan_68K_Loop(l_yr0-l_y, l_du, l_dv, &l_u, &l_v, &p, &l_x_fix, &x_cint, l_di, &l_i);
	 	l_y = l_yr0;
	 }
#endif
  
   if (l_y<l_yr) {
      fix test=l_y*l_dxr-x_cint*l_dyr+pi->cr;
   		p=grd_bm.bits+x_cint+l_y*gr_row;
      for (;l_y<l_yr;l_y++) {
         if (test>=0) {
            int k=(l_u>>16)&l_u_mask;
            k+=(l_v>>l_v_shift)&l_v_mask;
            k=bm_bits[k];
            *p=ltab[(fix_light(l_i))+k];		// gr_fill_upixel(ltab[(fix_int(l_i)<<8)+k],x_cint,l_y);
         }

         temp_x = x_cint;
         x_cint = fix_int(l_x_fix+=l_scan_slope);
         if (temp_x!=x_cint)
          {
            test+=l_dtr;
         		p-=temp_x-x_cint;
          }
         else
          {
            test+=l_dxr;
         		x_cint=fix_int(l_x_fix);
					}
      	 p+=gr_row;
         l_u+=l_du;
         l_v+=l_dv;
         l_i+=l_di;
      }
   }

	pi->x_fix = l_x_fix;
	pi->y = l_y;
	pi->u = l_u;
	pi->v = l_v;
	pi->du = l_du;
	pi->dv = l_dv;
	pi->di = l_di;
	pi->i = l_i;
}

#if !(defined(powerc) || defined(__powerc))	
asm void opaque_lit_per_vscan_68K_Loop(int dy, fix l_du, fix l_dv, fix *l_u, fix *l_v,
																			 uchar **p, fix *l_x_fix, int *x_cint, fix l_di, fix *l_i)
 {
/*
   for (;l_y<l_yr0;l_y++) {
      int k=(l_u>>16)&l_u_mask;
      k+=(l_v>>l_v_shift)&l_v_mask;
      k=bm_bits[k];
      *p=ltab[(fix_int(l_i)<<8)+k];		// gr_fill_upixel(ltab[(fix_int(l_i)<<8)+k],x_cint,l_y);

      temp_x = x_cint;
      x_cint=fix_int(l_x_fix+=l_scan_slope);
      if (temp_x!=x_cint)
      	p -= (temp_x-x_cint);

      p+=gr_row;
      l_u+=l_du;
      l_v+=l_dv;
      l_i+=l_di;
   }
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
	move.l	(a0),a6			// l_x_fix
	move.l	92(sp),a0
	move.l	(a0),d4			// x_cint
	move.l	100(sp),a0
	move.l	(a0),d5			// l_i
	move.l	(a1),a0			// P
	move.l	ol_bm_bits,a1
	move.l	ol_l_v_shift,d3
	move.l	ol_ltab, a2
	move.l	ol_l_scan_slope,a3
	
@Loop:
	move.l	d1,d6
	swap		d6
	ext.l		d6							// l_u>>16
	and.l		ol_l_u_mask,d6		// k = (l_u>>16)&l_u_mask
	move.l	d2,d7
	asr.l		d3,d7						// l_v>>l_v_shift
	and.l		ol_l_v_mask,d7	// (l_v>>l_v_shift)&l_v_mask
	add.l		d7,d6						// k+=(l_v>>l_v_shift)&l_v_mask;
	move.l	d5,d7
	lsr.l		#8,d7
//	swap		d7							// fix_int(l_i)
//	lsl.w		#8,d7
	move.b	(a1,d6.l),d7		// temp = (fix_int(l_i)<<8)+k;
	move.b	(a2,d7.w),(a0)	// *(p++)=ltab[temp];

	move.w	d4,d6							// temp_y = y_cint
	add.l		a3,a6							// l_x_fix+=l_scan_slope
	move.l	a6,d4							// x_cint=fix_int(l_x_fix+=l_scan_slope);
	swap		d4
	ext.l		d4
	sub.w		d4,d6
	sub.w		d6,a0							// p -= (temp_x-x_cint);
	
	add.l		ol_gr_row,a0	// p+=gr_row;
	add.l		a4,d1					// l_u+=l_du;
	add.l		a5,d2					// l_v+=l_dv;
	add.l	 	96(sp),d5			// l_i += di
	dbra		d0,@Loop

	move.l	76(sp),a3			// *l_u
	move.l	d1,(a3)				// save l_u
	move.l	80(sp),a2			// *l_v
	move.l	d2,(a2)				// save l_v
	move.l	100(sp),a2			// *l_v
	move.l	d5,(a2)				// save l_i

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


extern void gri_lit_per_umap_hscan(grs_bitmap *bm, int n, grs_vertex **vpl, grs_per_setup *ps);
extern void gri_lit_per_umap_vscan(grs_bitmap *bm, int n, grs_vertex **vpl, grs_per_setup *ps);

void gri_opaque_lit_per_umap_hscan_init(grs_bitmap *bm, grs_per_setup *ps) {
   ps->shell_func=(void (*)()) gri_lit_per_umap_hscan;
   ps->scanline_func=(void (*)()) gri_opaque_lit_per_umap_hscan_scanline;
}

void gri_opaque_lit_per_umap_vscan_init(grs_bitmap *bm, grs_per_setup *ps) {
   ps->shell_func=(void (*)()) gri_lit_per_umap_vscan;
   ps->scanline_func=(void (*)()) gri_opaque_lit_per_umap_vscan_scanline;
}




