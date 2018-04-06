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
 * $Source: r:/prj/lib/src/2d/RCS/blend.c $
 * $Revision: 1.2 $
 * $Author: kevin $
 * $Date: 1994/09/08 21:57:25 $
 *
 * Support for creation and maintenance of blend tables
 *
 * This file is part of the 2d library.
 */

#include "grs.h"
#include "blncon.h"
#include "grmalloc.h"
#include "rgb.h"
#include "scrdat.h"

// prototypes
void gri_build_blend(uchar *base_addr, int blend_fac);
int gr_free_blend(void);
bool gr_init_blend(int log_blend_levels);


// points to blend_tabs-1 tables, each 64k
uchar *grd_blend=NULL;
uchar *grd_half_blend=NULL;
int grd_log_blend_levels=0;

// blend fac is 0-256, where 0 is all 0, 256 is all 1
void gri_build_blend(uchar *base_addr, int blend_fac)
{
   uchar *c=grd_ipal, *cur_addr=base_addr, cols[2][3];
   int offs, i, j, k;                  /* offset from ipal for data, loop controls */
   int blend_bar=GR_BLEND_TABLE_RES-blend_fac;        /* remaining blend frac */

   for (i=0; i<256; i++)
   {
      gr_split_rgb(grd_bpal[i],&cols[0][0],&cols[0][1],&cols[0][2]);
      for (j=0; j<256; j++)
      {
         if ((i==0)||(i==j)||(cols[0][0]+cols[0][1]+cols[0][2]==0))
            *cur_addr++=i;                    // transparency and self and black are themselves, for zaniness w/shifts
         else
         {
		      gr_split_rgb(grd_bpal[j],&cols[1][0],&cols[1][1],&cols[1][2]);
            if ((j==0)||(cols[1][0]+cols[1][1]+cols[1][2]==0))
               *cur_addr++=j;
            else
            {
			      for (offs=0, k=2; k>=0; k--)      // go do the blends
		            offs=(offs<<5)+((((cols[0][k]*blend_bar)+(cols[1][k]*blend_fac))>>GR_BLEND_TABLE_RES_LOG)>>3);
		         *cur_addr++=*(c+offs);
            }
         }
      }
   }
}

/* frees the blending table. returns 0 if ok, nonzero if error. */
int gr_free_blend(void)
{
   if (grd_blend==NULL)
      return 1;
   DisposePtr((Ptr) grd_blend);	// was gr_free
   grd_blend=NULL;
   grd_log_blend_levels=0;
   return 0;
}

// at the moment, log_blend_levels = 0 deallocates the blend, ie. runs free_blend
bool gr_init_blend(int log_blend_levels)
{
   if (log_blend_levels>0)
   {
      int fac=GR_BLEND_TABLE_RES>>log_blend_levels;   /* base blend factor*/
      int tab_cnt=(1<<log_blend_levels)-1, i;         /* number of tables, loop control */ 

      if (grd_blend!=NULL) if (!gr_free_blend()) return FALSE; /* something went horribly wrong */
//	   if ((grd_blend=(uchar *) gr_malloc(tab_cnt*GR_BLEND_TABLE_SIZE))==NULL) return FALSE; /* x 64k tables */
	   if ((grd_blend=(uchar *) NewPtr(tab_cnt*GR_BLEND_TABLE_SIZE))==NULL) return FALSE; /* x 64k tables */
	   for (i=0; i<tab_cnt; i++)
	      gri_build_blend(grd_blend+(i*GR_BLEND_TABLE_SIZE),fac*(i+1));
      grd_log_blend_levels=log_blend_levels;
      grd_half_blend=grd_blend+(tab_cnt>>1)*GR_BLEND_TABLE_SIZE;
      return TRUE;
   }
   else return gr_free_blend();
}
