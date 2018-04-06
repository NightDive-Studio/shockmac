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
 * $Source: r:/prj/lib/src/2d/RCS/fl8nl.c $
 * $Revision: 1.3 $
 * $Author: kevin $
 * $Date: 1994/08/16 13:12:41 $
 *
 * Routines to linearly texture map a flat8 bitmap to a generic canvas.
 *
 * This file is part of the 2d library.
 *
 */

#include "plytyp.h"
#include "cnvdat.h"
#include "tmapint.h"
#include "gente.h"
#include "poly.h"
#include "scrdat.h"
#include "tlucdat.h"
#include "vtab.h"
#include "fl8tf.h"
#include "cnvdat.h"
#include "fl8tmapdv.h"

// prototypes
int gri_tluc8_lin_umap_loop(grs_tmap_loop_info *tli);


int gri_tluc8_lin_umap_loop(grs_tmap_loop_info *tli) {
   fix u,v,du,dv,dx,d;

	// locals used to store copies of tli-> stuff, so its in registers on the PPC
	long	*t_vtab;
	uchar *t_bits;
	uchar *t_clut;
	uchar temp_pix;
	uchar	t_wlog;
	ulong	t_mask;
	int		k;
	
   u=tli->left.u;
   du=tli->right.u-u;
   v=tli->left.v;
   dv=tli->right.v-v;
   dx=tli->right.x-tli->left.x;

	t_vtab = tli->vtab;
	t_bits = tli->bm.bits;
	t_clut = tli->clut;
	t_mask = tli->mask;
	t_wlog = tli->bm.wlog;

   do {
      if ((d = fix_ceil(tli->right.x)-fix_ceil(tli->left.x)) > 0) {
         uchar *p=tli->d+fix_cint(tli->left.x);
         uchar *p_final=tli->d+fix_cint(tli->right.x);
         d =fix_ceil(tli->left.x)-tli->left.x;
         
#if InvDiv
         k = fix_div(fix_make(1,0),dx);
	       du=fix_mul_asm_safe(du,k);
	       dv=fix_mul_asm_safe(dv,k);
#else
	       du=fix_div(du,dx);
	       dv=fix_div(dv,dx);
#endif
       //  u+=fix_mul(du,d);
       //  v+=fix_mul(dv,d);

         switch (tli->bm.hlog) {
         case GRL_OPAQUE:
            for (;p < p_final; p++) {
               k = t_vtab[fix_fint(v)]+fix_fint(u);
               k = t_bits[k];
               if (tluc8tab[k]!=NULL) *p=tluc8tab[k][*p];
               else *p=k;
               u+=du; v+=dv;
            }
            break;
         case GRL_TRANS:
            for (;p < p_final; p++) {
               k=t_vtab[fix_fint(v)]+fix_fint(u);
               if (k = t_bits[k]) {
                  if (tluc8tab[k]!=NULL) *p=tluc8tab[k][*p];
                  else *p=k;
               }
               u+=du; v+=dv;
            }
            break;
         case GRL_OPAQUE|GRL_LOG2:
            for (;p < p_final; p++) {
               k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
               k = t_bits[k];
               if (tluc8tab[k]!=NULL) *p=tluc8tab[k][*p];
               else *p=k;
               u+=du; v+=dv;
            }
            break;
         case GRL_TRANS|GRL_LOG2:
            for (;p < p_final; p++) {
               k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
               if (k = t_bits[k]) {
                  if (tluc8tab[k]!=NULL) *p=tluc8tab[k][*p];
                  else *p=k;
               }
               u+=du; v+=dv;
            }
            break;
         case GRL_OPAQUE|GRL_CLUT:
            for (;p < p_final; p++) {
               k=t_vtab[fix_fint(v)]+fix_fint(u);
               k = t_bits[k];
               if (tluc8tab[k]!=NULL) *p=t_clut[tluc8tab[k][*p]];
               else *p=t_clut[k];
               u+=du; v+=dv;
            }
            break;
         case GRL_TRANS|GRL_CLUT:
            for (;p < p_final; p++) {
               k=t_vtab[fix_fint(v)]+fix_fint(u);
               if (k = t_bits[k]) {
                  if (tluc8tab[k]!=NULL) *p=t_clut[tluc8tab[k][*p]];
                  else *p=t_clut[k];
               }
               u+=du; v+=dv;
            }
            break;
         case GRL_OPAQUE|GRL_LOG2|GRL_CLUT:
            for (;p < p_final; p++) {
               k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
               k = t_bits[k];
               if (tluc8tab[k]!=NULL) *p=t_clut[tluc8tab[k][*p]];
               else *p=t_clut[k];
               u+=du; v+=dv;
            }
            break;
         case GRL_TRANS|GRL_LOG2|GRL_CLUT:
            for (;p < p_final; p++) {
               k=((fix_fint(v)<<t_wlog)+fix_fint(u))&t_mask;
               u+=du; v+=dv;
               if (k = t_bits[k]) {
                  if (tluc8tab[k]!=NULL) *p=t_clut[tluc8tab[k][*p]];
                  else *p=t_clut[k];
               }
               u+=du; v+=dv;
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
      tli->d+=grd_bm.row;
   } while (--(tli->n) > 0);
   return FALSE; /* tmap OK */
}

void gri_tluc8_trans_lin_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_TRANS|GRL_LOG2;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_TRANS;
   }
   tli->d=grd_bm.bits+grd_bm.row*tli->y;
   tli->loop_func=(void (*)()) gri_tluc8_lin_umap_loop;
   tli->right_edge_func=(void (*)()) gri_uvx_edge;
   tli->left_edge_func=(void (*)()) gri_uvx_edge;
}

void gri_tluc8_opaque_lin_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_OPAQUE|GRL_LOG2;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_OPAQUE;
   }
   tli->d=grd_bm.bits+grd_bm.row*tli->y;
   tli->loop_func=(void (*)()) gri_tluc8_lin_umap_loop;
   tli->right_edge_func=(void (*)()) gri_uvx_edge;
   tli->left_edge_func=(void (*)()) gri_uvx_edge;
}

void gri_tluc8_trans_clut_lin_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_TRANS|GRL_LOG2|GRL_CLUT;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_TRANS|GRL_CLUT;
   }
   tli->d=grd_bm.bits+grd_bm.row*tli->y;
   tli->loop_func=(void (*)()) gri_tluc8_lin_umap_loop;
   tli->right_edge_func=(void (*)()) gri_uvx_edge;
   tli->left_edge_func=(void (*)()) gri_uvx_edge;
}

void gri_tluc8_opaque_clut_lin_umap_init(grs_tmap_loop_info *tli) {
   if ((tli->bm.row==(1<<tli->bm.wlog)) &&
            (tli->bm.h==(1<<tli->bm.hlog))) {
      tli->mask=(1<<(tli->bm.hlog+tli->bm.wlog))-1;
      tli->bm.hlog=GRL_OPAQUE|GRL_LOG2|GRL_CLUT;
   } else {
      tli->vtab=gr_make_vtab(&(tli->bm));
      tli->bm.hlog=GRL_OPAQUE|GRL_CLUT;
   }
   tli->d=grd_bm.bits+grd_bm.row*tli->y;
   tli->loop_func=(void (*)()) gri_tluc8_lin_umap_loop;
   tli->right_edge_func=(void (*)()) gri_uvx_edge;
   tli->left_edge_func=(void (*)()) gri_uvx_edge;
}
