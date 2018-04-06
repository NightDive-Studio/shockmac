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
// Implementation solid transparent mappers
//
// This file is part of the 2d library.
//


#include "tmapint.h"
#include "poly.h"
#include "grpix.h"
#include "gente.h"
#include "vtab.h"
#include "fl8tf.h"
#include "cnvdat.h"
#include "2dDiv.h"

// prototypes
int gri_trans_solid_lin_umap_loop(grs_tmap_loop_info *tli);
int gri_trans_solid_floor_umap_loop(grs_tmap_loop_info *tli);
int gri_solid_wall_umap_loop(grs_tmap_loop_info *tli);
void gri_trans_solid_per_umap_hscan_scanline(grs_per_info *pi, grs_bitmap *bm);
void gri_trans_solid_per_umap_vscan_scanline(grs_per_info *pi, grs_bitmap *bm);

// 68K stuff
#if !(defined(powerc) || defined(__powerc))	

// Linear Mapper stuff
asm int Handle_Solid_Lin_68K_Loop(fix u, fix v, fix du, fix dv, fix dx,grs_tmap_loop_info *tli, uchar *start_pdest, uchar *t_bits, long gr_row);
asm void solid_lin_trans(void);
asm void solid_lin_trans_log2(void);

// Floor Mapper stuff
asm int Handle_Solid_Floor_68K_Loop(fix u, fix v, fix du, fix dv, fix dx,grs_tmap_loop_info *tli, uchar *start_pdest, uchar *t_bits, long gr_row);
asm void solid_floor_trans_log2(void);

// Wall Mapper stuff
asm int Handle_Solid_Wall_68K_Loop(fix u, fix v, fix du, fix dv, fix dy,grs_tmap_loop_info *tli, uchar *start_pdest, uchar *t_bits, long gr_row);
asm void solid_wall_trans_log2(void);

// Perspective Mapper stuff
asm void opaque_solid_per_hscan_68K_Loop(int dx, fix l_du, fix l_dv, fix *l_u, fix *l_v, uchar **p, fix *l_y_fix, int *y_cint);
asm void opaque_solid_per_vscan_68K_Loop(int dy, fix l_du, fix l_dv, fix *l_u, fix *l_v, uchar **p, fix *l_x_fix, int *x_cint);
asm void solid_trans_per_hscan_68K_Loop(int dx, fix l_du, fix l_dv, fix *l_u, fix *l_v, uchar **p, fix *l_y_fix, int *y_cint);
asm void solid_trans_per_vscan_68K_Loop(int dx, fix l_du, fix l_dv, fix *l_u, fix *l_v, uchar **p, fix *l_y_fix, int *y_cint);
#endif


// globals used by 68K routines
ulong	s_wlog_68K;
ulong	s_mask_68K;
long	*s_vtab_68K;
int 	s_l_u_mask,s_l_v_mask,s_l_v_shift;
uchar *s_bm_bits;
fix 	s_l_scan_slope;
int		s_gr_row;
uchar	solid_color_68K;

int gri_trans_solid_lin_umap_loop(grs_tmap_loop_info *tli) {
	fix u,v,du,dv,dx,d;

	// locals used to store copies of tli-> stuff, so its in registers on the PPC
	int 	x;
	uchar	solid_color;
	int		t_xl,t_xr;	
	uchar *p_dest;
	long	*t_vtab;
	uchar *t_bits;
	uchar *t_clut;
	uchar	t_wlog;
	ulong	t_mask;
	long	gr_row;
	uchar *start_pdest;
							
	solid_color_68K = solid_color = (uchar) tli->clut;
	u=tli->left.u;
	du=tli->right.u-u;
	v=tli->left.v;
	dv=tli->right.v-v;
	dx=tli->right.x-tli->left.x;

	s_vtab_68K = t_vtab = tli->vtab;
	s_mask_68K = t_mask = tli->mask;
	s_wlog_68K = t_wlog = tli->bm.wlog;

	t_bits = tli->bm.bits;
	gr_row = grd_bm.row;
	start_pdest = grd_bm.bits + (gr_row*(tli->y));

// handle PowerPC loop
#if (defined(powerc) || defined(__powerc))	
	do {
	  if ((d = fix_ceil(tli->right.x)-fix_ceil(tli->left.x)) > 0) 
	    {
	     d =fix_ceil(tli->left.x)-tli->left.x;
	     du=fix_div(du,dx);
	     dv=fix_div(dv,dx);
	     u+=fix_mul(du,d);
	     v+=fix_mul(dv,d);
		
		   // copy out tli-> stuff into locals
			 t_xl = fix_cint(tli->left.x);
			 t_xr = fix_cint(tli->right.x);
			 p_dest = start_pdest + t_xl;

			 if (tli->bm.hlog==GRL_TRANS) 
			   {
	        for (x=t_xl; x<t_xr; x++) 
	         {
	           if (t_bits[t_vtab[fix_fint(v)]+fix_fint(u)]) *p_dest = solid_color;		// gr_fill_upixel(t_bits[k],x,y);
             p_dest++; u+=du; v+=dv;
	         }
	       }
	      else
	       {
	        for (x=t_xl; x<t_xr; x++) 
	         {
	           if (t_bits[((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask]) *p_dest = solid_color;		// gr_fill_upixel(t_bits[k],x,y);
             p_dest++; u+=du; v+=dv;
	         }
	       }
	     }
	    else if (d<0) return TRUE; /* punt this tmap */
	  
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
	return(Handle_Solid_Lin_68K_Loop(u,v,du,dv,dx,tli,start_pdest,t_bits,gr_row));
#endif
}
 
// Main 68K handler loop
#if !(defined(powerc) || defined(__powerc))	
asm int Handle_Solid_Lin_68K_Loop(fix u, fix v, fix du, fix dv, fix dx,
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
 	move.l	0x40(a0),d0
 	add.l		#0x0000FFFF,d0
 	clr.w		d0									// fix_ceil(tli->right.x)
 	move.l	0x1C(a0),d1
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
 
	move.l	0x1c(a0),d2		// tli->left.x
	
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
 	move.l	0x1C(a0),d1
 	add.l		#0x0000FFFF,d1
 	clr.w		d1
 	swap		d1

 	move.l	0x40(a0),d0
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
	move.b	23(a0),d1
	cmp.b		#1,d1
	beq.s		@trans
	jsr			solid_lin_trans_log2
	bra.s		@Skip
@trans:
	jsr			solid_lin_trans
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
 	
 	move.l	0x34(a0),d0
 	move.l	0x24(a0),d1
 	add.l		d0,d1
 	move.l	d1,d4				  // *u=(tli->left.u+=tli->left.du);
 	
 	move.l	0x48(a0),d0
 	add.l		0x58(a0),d0
 	move.l	d0,0x48(a0)		// tli->right.u+=tli->right.du;
 	sub.l		d1,d0
 	move.l	d0,d6					// *du=tli->right.u-*u;
 	
 	move.l	0x28(a0),d1
 	add.l		0x38(a0),d1
 	move.l	d1,0x28(a0)
 	move.l	d1,d5					// *v=(tli->left.v+=tli->left.dv);
 	
 	move.l	0x4C(a0),d0
 	add.l		0x5C(a0),d0
 	move.l	d0,0x4C(a0)		// tli->right.v+=tli->right.dv;
 	sub.l		d1,d0
 	move.l	d0,d7					// *dv=tli->right.v-*v;
 	
 	move.l	0x30(a0),d0		// tli->left.x+=tli->left.dx;
 	add.l		d0,0x1C(a0)
 	
 	move.l	0x54(a0),d0
 	add.l		d0,0x40(a0)		// tli->right.x+=tli->right.dx;
 	
 	move.l	0x40(a0),d0
 	sub.l		0x1C(a0),d0
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

// handle inner loop for transparent (non log2) mode
asm void solid_lin_trans(void)
 {
/*  for (x=t_xl; x<t_xr; x++) {
     int k=t_vtab[fix_fint(v)]+fix_fint(u);
     if (temp_pix=t_bits[k]) *p_dest = temp_pix;		// gr_fill_upixel(t_bits[k],x,y);
     p_dest++; u+=du; v+=dv;
  }*/
	
	move.b	solid_color_68K,d2
	move.l	s_vtab_68K,a6
	move.l	d7,d3
	move.l	d6,a1
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
	move.b	d2,(a4)+					// *(p_dest++) = t_bits[k];
	add.l		a1,d4							// u+=du
	add.l		d3,d5							// v+=dv
	dbra		d0,@Loop
	bra.s		@End

@skippix:
	addq.w	#1,a4
	add.l		a1,d4							// u+=du
	add.l		d3,d5							// v+=dv
	dbra		d0,@Loop

@End:
@Done:		
	rts	   
 }
 

// handle inner loop for transparent (width log2) mode
asm void solid_lin_trans_log2(void)
 {
/* 
  for (x=t_xl; x<t_xr; x++) {
     int k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
     if (temp_pix=t_bits[k]) *p_dest = temp_pix;		// gr_fill_upixel(t_bits[k],x,y);
     p_dest++; u+=du; v+=dv;
  }*/
	
	move.l	d5,a6
	move.b	solid_color_68K,d5
	move.l	s_mask_68K,d2
	move.l	s_wlog_68K,d3
	move.l	d7,a3
	move.l	d6,a2
	subq.w	#1,d0
	bmi.s		@Done

	moveq		#16,d6
@Loop:
	move.l	a6,d1
	asr.l		d6,d1 						// d1 = fix_fint(v)
	lsl.l		d3,d1							// d1 = (fix_fint(v)<<t_wlog
	move.l	d4,d7
	asr.l		d6,d7 						// d7 = fix_fint(u)
	add.l		d7,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))
	and.l		d2,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask
	move.b	(a5,d1.l),d7
	beq.s		@skippix
	move.b	d5,(a4)+					// *(p_dest++) = t_bits[k];
	add.l		a2,d4							// u+=du
	add.l		a3,a6							// v+=dv
	dbra		d0,@Loop
	bra.s		@End

@skippix:
	addq.w	#1,a4
	add.l		a2,d4							// u+=du
	add.l		a3,a6							// v+=dv
	dbra		d0,@Loop

@End:
@Done:		
	rts	   
 }
#endif
		 
void gri_trans_solid_lin_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_TRANS|GRL_LOG2;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_TRANS;
   }
   tli->loop_func=(void (*)()) gri_trans_solid_lin_umap_loop;
   tli->right_edge_func=(void (*)()) gri_uvx_edge;
   tli->left_edge_func=(void (*)()) gri_uvx_edge;
}


int gri_trans_solid_floor_umap_loop(grs_tmap_loop_info *tli) {
   fix u,v,du,dv,dx,d;
	 uchar	solid_color;
	 int		x;
	// locals used to store copies of tli-> stuff, so its in registers on the PPC
	int		t_xl,t_xr,t_y,gr_row;	
	long	*t_vtab;
	uchar *t_bits;
	uchar *p_dest;
	uchar temp_pix;
	uchar	t_wlog;
	ulong	t_mask;

	solid_color_68K = solid_color = (uchar) tli->clut;
	u=fix_div(tli->left.u,tli->w);
	du=fix_div(tli->right.u,tli->w)-u;
	v=fix_div(tli->left.v,tli->w);
	dv=fix_div(tli->right.v,tli->w)-v;
	dx=tli->right.x-tli->left.x;

	s_mask_68K = t_mask = tli->mask;
	s_wlog_68K = t_wlog = tli->bm.wlog;
	s_vtab_68K = t_vtab = tli->vtab;
	t_bits = tli->bm.bits;
	gr_row = grd_bm.row;

#if (defined(powerc) || defined(__powerc))	
   do {
      if ((d = fix_ceil(tli->right.x)-fix_ceil(tli->left.x)) > 0) {
         d =fix_ceil(tli->left.x)-tli->left.x;
         du=fix_div(du,dx);
         u+=fix_mul(du,d);
         dv=fix_div(dv,dx);
         v+=fix_mul(dv,d);

			   // copy out tli-> stuff into locals
				 t_xl = fix_cint(tli->left.x);
				 t_xr = fix_cint(tli->right.x);
				 t_y = tli->y;
				 p_dest = grd_bm.bits + (grd_bm.row*t_y) + t_xl;

				 if (tli->bm.hlog==GRL_TRANS)
				  {
            for (x=t_xl; x<t_xr; x++) 
             {
               int k=t_vtab[fix_fint(v)]+fix_fint(u);
	           	 if (t_bits[k]) *p_dest = solid_color;		// gr_fill_upixel(t_bits[k],x,t_y);
               p_dest++; u+=du; v+=dv;
             }
				  }
				 else
				  {
            for (x=t_xl; x<t_xr; x++) 
             {
               int k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
	           	 if (t_bits[k]) *p_dest = solid_color;		// gr_fill_upixel(t_bits[k],x,t_y);
               p_dest++; u+=du; v+=dv;
             }
				  }
      } else if (d<0) return TRUE; /* punt this tmap */
      tli->w+=tli->dw;
      u=fix_div((tli->left.u+=tli->left.du),tli->w);
      tli->right.u+=tli->right.du;
      du=fix_div(tli->right.u,tli->w)-u;
      v=fix_div((tli->left.v+=tli->left.dv),tli->w);
      tli->right.v+=tli->right.dv;
      dv=fix_div(tli->right.v,tli->w)-v;
      tli->left.x+=tli->left.dx;
      tli->right.x+=tli->right.dx;
      dx=tli->right.x-tli->left.x;
      tli->y++;
   } while (--(tli->n) > 0);
   return FALSE; /* tmap OK */
// handle 68K loops
#else
	return(Handle_Solid_Floor_68K_Loop(u,v,du,dv,dx,tli,grd_bm.bits,t_bits,gr_row));
#endif
}

#if !(defined(powerc) || defined(__powerc))	
asm int Handle_Solid_Floor_68K_Loop(fix u, fix v, fix du, fix dv, fix dx,
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
	jsr			solid_floor_trans_log2
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
  move.l	d0,d6					//  du=fix_div(tli->right.u,tli->w)-u;
      
 	move.l	LeftV(a0),d0
 	add.l		LeftDV(a0),d0	
 	move.l	d0,LeftV(a0)		// tli->left.v+=tli->left.dv;
 	fix_div_68k_d2_d0(d5)		// inline function, returns result in d0
  move.l	d0,d5					// v=fix_div((tli->left.v+=tli->left.dv),tli->w);
     
 	move.l	RightV(a0),d0
 	add.l		RightDV(a0),d0
 	move.l	d0,RightV(a0)		// tli->right.v+=tli->right.dv;
 	fix_div_68k_d2_d0(d7)		// inline function, returns result in d0
 	sub.l		d5,d0
	move.l	d0,d7					//  dv=fix_div(tli->right.v,tli->w)-v;
	
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
  

asm void solid_floor_trans_log2(void)
 {
/* 
  for (x=t_xl; x<t_xr; x++) {
     int k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
     if (temp_pix=t_bits[k]) *p_dest = temp_pix;		// gr_fill_upixel(t_bits[k],x,y);
     p_dest++; u+=du; v+=dv;
  }*/
	
	move.l	d4,a1
	move.b	solid_color_68K,d4
	move.l	s_mask_68K,d2
	move.l	s_wlog_68K,d3
	move.l	d7,a3
	move.l	d6,a2
	subq.w	#1,d0
	bmi.s		@Done

	moveq		#16,d6
@Loop:
	move.l	d5,d1
	asr.l		d6,d1 						// d1 = fix_fint(v)
	lsl.l		d3,d1							// d1 = (fix_fint(v)<<t_wlog
	move.l	a1,d7
	asr.l		d6,d7 						// d7 = fix_fint(u)
	add.l		d7,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))
	and.l		d2,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask
	move.b	(a5,d1.l),d7
	beq.s		@skippix
	move.b	d4,(a4)+					// *(p_dest++) = t_bits[k];
	add.l		a2,a1							// u+=du
	add.l		a3,d5							// v+=dv
	dbra		d0,@Loop
	bra.s		@End

@skippix:
	addq.w	#1,a4
	add.l		a2,a1							// u+=du
	add.l		a3,d5							// v+=dv
	dbra		d0,@Loop

@End:
@Done:		
	rts	   
 }
#endif

void gri_trans_solid_floor_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_TRANS|GRL_LOG2;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_TRANS;
   }
   tli->loop_func=(void (*)()) gri_trans_solid_floor_umap_loop;
   tli->left_edge_func=(void (*)()) gri_uvwx_edge;
   tli->right_edge_func=(void (*)()) gri_uvwx_edge;
}


int gri_solid_wall_umap_loop(grs_tmap_loop_info *tli) {
   fix u,v,du,dv,dy,d;
	 uchar	solid_color;

	 // locals used to store copies of tli-> stuff, so its in registers on the PPC
	 int		t_yl,t_yr;	
	 long		*t_vtab;
	 uchar 	*t_bits;
	 uchar 	*p_dest;
	 uchar *t_clut;
	 uchar	t_wlog;
	 ulong	t_mask;
	 long		gr_row;
	 int		y;
	 
	 solid_color_68K = solid_color = (uchar) tli->clut;
   u=fix_div(tli->left.u,tli->w);
   du=fix_div(tli->right.u,tli->w)-u;
   v=fix_div(tli->left.v,tli->w);
   dv=fix_div(tli->right.v,tli->w)-v;
   dy=tli->right.y-tli->left.y;

	 t_bits = tli->bm.bits;
	 s_vtab_68K = t_vtab = tli->vtab;
	 s_mask_68K = t_mask = tli->mask;
	 s_wlog_68K = t_wlog = tli->bm.wlog;

	 gr_row = grd_bm.row;

// handle PowerPC loop
#if (defined(powerc) || defined(__powerc))	
   do {
      if ((d = fix_ceil(tli->right.y)-fix_ceil(tli->left.y)) > 0) {
 
         d =fix_ceil(tli->left.y)-tli->left.y;
         du=fix_div(du,dy);
         dv=fix_div(dv,dy);
         u+=fix_mul(du,d);
         v+=fix_mul(dv,d);

			   t_yl = fix_cint(tli->left.y);
			   t_yr = fix_cint(tli->right.y);
			 	 p_dest = grd_bm.bits + (gr_row*t_yl) + tli->x; 

         if (tli->bm.hlog==GRL_TRANS)
          {
            for (y=t_yl; y<t_yr; y++) {
               int k=t_vtab[fix_fint(v)]+fix_fint(u);
               if (t_bits[k]) 
                 *p_dest = solid_color;			// gr_fill_upixel(t_bits[k],t_x,y);
               p_dest += gr_row; u+=du; v+=dv;
            }
				  }
				 else
				  {
            for (y=t_yl; y<t_yr; y++) {
               int k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
               if (t_bits[k]) 
                 *p_dest = solid_color;		// gr_fill_upixel(t_bits[k],t_x,y);
               p_dest += gr_row; u+=du; v+=dv;
            }
          }
      } else if (d<0) return TRUE; /* punt this tmap */
      
			
      tli->w+=tli->dw;
      u=fix_div((tli->left.u+=tli->left.du),tli->w);
      tli->right.u+=tli->right.du;
      du=fix_div(tli->right.u,tli->w)-u;
      v=fix_div((tli->left.v+=tli->left.dv),tli->w);
      tli->right.v+=tli->right.dv;
      dv=fix_div(tli->right.v,tli->w)-v;
      tli->left.y+=tli->left.dy;
      tli->right.y+=tli->right.dy;
      dy=tli->right.y-tli->left.y;
      tli->x++;
         
   } while (--(tli->n) > 0);

	return FALSE;
// handle 68K loops
#else
	return(Handle_Solid_Wall_68K_Loop(u,v,du,dv,dy,tli,grd_bm.bits,t_bits,gr_row)); 
#endif
}

// Main 68K handler loop
#if !(defined(powerc) || defined(__powerc))	
asm int Handle_Solid_Wall_68K_Loop(fix u, fix v, fix du, fix dv, fix dy,
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
	jsr			solid_wall_trans_log2
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
 	fix_div_68k_d2_d0(d7)		// inline function, returns result in d0
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
 
// handle inner loop for wall transparent (width log2) mode
asm void solid_wall_trans_log2(void)
 {
/*  for (y=t_yl; y<t_yr; y++) {
     int k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
     if (temp_pix = t_bits[k]) 
       *p_dest = temp_pix;		// gr_fill_upixel(t_bits[k],t_x,y);
     p_dest += gr_row; u+=du; v+=dv;*/
  
  move.l	d5,a6
  move.b 	solid_color_68K,d5
	move.l	s_mask_68K,d2
	move.l	s_wlog_68K,d3
	move.l	d7,a3
	move.l	d6,a1
	subq.w	#1,d0
	bmi.s		@Done

	moveq		#16,d6
@Loop:
	move.l	a6,d1
	asr.l		d6,d1 						// d1 = fix_fint(v)
	lsl.l		d3,d1							// d1 = (fix_fint(v)<<t_wlog
	move.l	d4,d7
	asr.l		d6,d7 						// d7 = fix_fint(u)
	add.l		d7,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))
	and.l		d2,d1							// d1 = ((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask
	move.b	(a5,d1.l),d7			// *p_dest = t_bits[k];
	beq.s		@skippix
	move.b	d5,(a4)
@skippix:
	add.l		a2,a4							// p_dest += gr_row;
	add.l		a1,d4							// u+=du
	add.l		a3,a6							// v+=dv
	dbra		d0,@Loop

@Done:		
	rts	   
 }
#endif


void gri_trans_solid_wall_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_TRANS|GRL_LOG2;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_TRANS;
   }
   tli->loop_func=(void (*)()) gri_solid_wall_umap_loop;
   tli->left_edge_func=(void (*)()) gri_uvwy_edge;
   tli->right_edge_func=(void (*)()) gri_uvwy_edge;
}





void gri_trans_solid_per_umap_hscan_scanline(grs_per_info *pi, grs_bitmap *bm) {
   int y_cint;
   uchar *p;
	 uchar	solid_color;

	 // locals used to speed PPC code
	 fix	l_u,l_v,l_du,l_dv,l_y_fix,l_scan_slope,l_dtl,l_dxl,l_dyl,l_dtr,l_dyr;
	 int	l_x,l_xl,l_xr,l_xr0,l_u_mask,l_v_mask,l_v_shift;
	 int	gr_row,temp_y;
	 uchar *bm_bits;
	 
	 solid_color_68K = solid_color = (uchar) pi->clut;
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
   l_u = pi->u0 + fix_div(pi->unum,pi->denom);
   l_v = pi->v0 + fix_div(pi->vnum,pi->denom);
   l_du = fix_div(pi->dunum,pi->denom);
   l_dv = fix_div(pi->dvnum,pi->denom);
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
            if (bm_bits[k]) *p=solid_color;		// gr_fill_upixel(bm_bits[k],l_x,y_cint);
         }
         temp_y = y_cint; 
         y_cint = fix_int(l_y_fix+=l_scan_slope);
         if (temp_y!=y_cint)
          { 
         		test+=l_dtl;
         		p+=gr_row;
          }
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
      if (bm_bits[k]) *p=solid_color;		// gr_fill_upixel(bm_bits[k],l_x,y_cint);
      temp_y = y_cint; 
      y_cint = fix_int(l_y_fix+=l_scan_slope);
      if (temp_y!=y_cint)		// y_cint=fix_int((l_y_fix+=l_scan_slope));
       {
     		temp_y -= y_cint;
     		p+=gr_row;
       }

			p++;
      l_u+=l_du;
      l_v+=l_dv;
   }
#else
	if (l_x<l_xr0)
	 {
	 	s_l_v_shift = l_v_shift;
	 	s_l_u_mask = l_u_mask;
		s_l_v_mask = l_v_mask;
	  s_bm_bits = bm->bits;
	  s_l_scan_slope = l_scan_slope;
	  s_gr_row = grd_bm.row;
		solid_trans_per_hscan_68K_Loop(l_xr0-l_x, l_du, l_dv, &l_u, &l_v, &p, &l_y_fix, &y_cint);
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
            if (bm_bits[k]) *p=solid_color;		// gr_fill_upixel(bm_bits[k],l_x,y_cint);
         }
      	temp_y = y_cint; 
      	y_cint = fix_int(l_y_fix+=l_scan_slope);
         if (temp_y!=y_cint)
          {
         		test+=l_dtr;
         		p+=gr_row;
         	}
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
asm void solid_trans_per_hscan_68K_Loop(int dx, fix l_du, fix l_dv, fix *l_u, fix *l_v, uchar **p, fix *l_y_fix, int *y_cint)
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
	move.l	(a3),a3			// l_u
	move.l	80(sp),a2		// *l_v
	move.l	(a2),d2			// l_v
	move.l	84(sp),a1		// *p
	move.l	88(sp),a0
	move.l	(a0),a6			// l_y_fix
	move.l	92(sp),a0
	move.l	(a0),d4			// y_cint
	move.l	s_l_v_shift,d3
	move.l	(a1),a0
	move.l	s_bm_bits,a1
	move.l	s_gr_row,d5
	move.l	s_l_scan_slope,a2
	move.b	solid_color_68K,d1
	
@Loop:
	move.l	a3,d6
	swap		d6
	ext.l		d6							// l_u>>16
	and.l		s_l_u_mask,d6		// k = (l_u>>16)&l_u_mask
	move.l	d2,d7
	asr.l		d3,d7						// l_v>>l_v_shift
	and.l		s_l_v_mask,d7		// (l_v>>l_v_shift)&l_v_mask
	add.l		d7,d6						// k+=(l_v>>l_v_shift)&l_v_mask;
	move.b	(a1,d6.l),d7
	beq.s		@skippix				// if (temp_pix=bm_bits[k])
	move.b	d1,(a0)					//		*p=bm_bits[k];
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
	add.l		a4,a3
	add.l		a5,d2
	dbra		d0,@Loop
			
	move.l	76(sp),a2		// *l_u
	move.l	a3,(a2)			// save l_u
	move.l	80(sp),a2		// *l_v
	move.l	d2,(a2)			// save l_v
	
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

void gri_trans_solid_per_umap_vscan_scanline(grs_per_info *pi, grs_bitmap *bm) {
   int x_cint;
	 uchar	solid_color;

	 // locals used to speed PPC code
	 fix	l_dxr,l_x_fix,l_u,l_v,l_du,l_dv,l_scan_slope,l_dtl,l_dxl,l_dyl,l_dtr,l_dyr;
	 int	l_yl,l_yr0,l_yr,l_y,l_u_mask,l_v_mask,l_v_shift;
	 int	gr_row,temp_x;
	 uchar *bm_bits;
	 uchar *p;

	 solid_color_68K = solid_color = (uchar) pi->clut;
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

   l_u = pi->u0 + fix_div(pi->unum,pi->denom);
   l_v = pi->v0 + fix_div(pi->vnum,pi->denom);
   l_du = fix_div(pi->dunum,pi->denom);
   l_dv = fix_div(pi->dvnum,pi->denom);
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
            if (bm_bits[k]) *p=solid_color;		// gr_fill_upixel(bm_bits[k],x_cint,l_y);
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
      if (bm_bits[k]) *p=solid_color;		// gr_fill_upixel(bm_bits[k],x_cint,l_y);

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
	 	s_l_v_shift = l_v_shift;
	 	s_l_u_mask = l_u_mask;
		s_l_v_mask = l_v_mask;
	  s_bm_bits = bm->bits;
	  s_l_scan_slope = l_scan_slope;
	  s_gr_row = gr_row;
		solid_trans_per_vscan_68K_Loop(l_yr0-l_y, l_du, l_dv, &l_u, &l_v, &p, &l_x_fix, &x_cint);
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
            if (bm_bits[k]) *p=solid_color;		// gr_fill_upixel(bm_bits[k],x_cint,l_y);
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
asm void solid_trans_per_vscan_68K_Loop(int dy, fix l_du, fix l_dv, fix *l_u, fix *l_v, uchar **p, fix *l_x_fix, int *x_cint)
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
	move.l	(a3),a3			// l_u
	move.l	80(sp),a2		// *l_v
	move.l	(a2),d2			// l_v
	move.l	84(sp),a1		// *p
	move.l	88(sp),a0
	move.l	(a0),a6			// l_y_fix
	move.l	92(sp),a0
	move.l	(a0),d4			// y_cint
	move.l	s_l_v_shift,d3
	move.l	(a1),a0			// P
	move.l	s_bm_bits,a1
	move.l	s_gr_row,d5
	move.l	s_l_scan_slope,a2
	move.b	solid_color_68K,d1
	
@Loop:
	move.l	a3,d6
	swap		d6
	ext.l		d6							// l_u>>16
	and.l		s_l_u_mask,d6		// k = (l_u>>16)&l_u_mask
	move.l	d2,d7
	asr.l		d3,d7						// l_v>>l_v_shift
	and.l		s_l_v_mask,d7		// (l_v>>l_v_shift)&l_v_mask
	add.l		d7,d6						// k+=(l_v>>l_v_shift)&l_v_mask;
	move.b	(a1,d6.l),d7
	beq.s		@skippix
	move.b	d1,(a0)					// *p=bm_bits[k];
@skippix:

	move.w	d4,d6							// temp_y = y_cint
	add.l		a2,a6						// l_x_fix+=l_scan_slope
	move.l	a6,d4							// x_cint=fix_int(l_x_fix+=l_scan_slope);
	swap		d4
	ext.l		d4
	sub.w		d4,d6
	sub.w		d6,a0							// p -= (temp_x-x_cint);
	
	add.l		d5,a0					// p+=gr_row;
	add.l		a4,a3					// l_u+=l_du;
	add.l		a5,d2					// l_v+=l_dv;
	dbra		d0,@Loop

	move.l	76(sp),a2			// *l_u
	move.l	a3,(a2)				// save l_u
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

void gri_trans_solid_per_umap_hscan_init(grs_bitmap *bm, grs_per_setup *ps) {
   ps->shell_func=(void (*)()) gri_per_umap_hscan;
   ps->scanline_func=(void (*)()) gri_trans_solid_per_umap_hscan_scanline;
}

void gri_trans_solid_per_umap_vscan_init(grs_bitmap *bm, grs_per_setup *ps) {
   ps->shell_func=(void (*)()) gri_per_umap_vscan;
   ps->scanline_func=(void (*)()) gri_trans_solid_per_umap_vscan_scanline;
}




