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
 * $Source: r:/prj/lib/src/2d/RCS/genrsdtm.c $
 * $Revision: 1.4 $
 * $Author: kevin $
 * $Date: 1994/08/16 13:08:05 $
 * 
 * Generic routines for texture mapping rsd bitmaps.
 *
 * This file is part of the 2d library.
 *
 * $Log: genrsdtm.c $
 * Revision 1.4  1994/08/16  13:08:05  kevin
 * Changed to accomodate new function table organization.
 * 
 * Revision 1.3  1994/07/28  01:29:57  kevin
 * New general purpose chaining procedures for new texture mapping regime.
 * 
 * Revision 1.2  1993/12/28  16:30:18  kevin
 * changed name of gr_unpack_rsd to gr_rsd8_convert.
 * 
 * Revision 1.1  1993/12/06  13:16:17  kevin
 * Initial revision
 * 
 */

#include "bitmap.h"
#include "grs.h"
#include "ifcn.h"
#include "pertyp.h"
#include "rsdunpck.h"
#include "tmapint.h"
#include "tmaptab.h"
#include "gentf.h"

void rsd8_tm_init(grs_tmap_loop_info *ti)
{
   if (grd_unpack_buf!=NULL) {
      grs_bitmap tbm;
      if (gr_rsd8_convert(&(ti->bm),&tbm)==GR_UNPACK_RSD8_OK) {
         ti->bm.bits=tbm.bits;
         ti->n+=(tbm.type-BMT_RSD8)*GRD_FUNCS;
         ((void (*)(grs_tmap_loop_info *))(grd_tmap_init_table[ti->n]))(ti);
      }
   }
}

void rsd8_pm_init(grs_bitmap *bm, grs_per_setup *ps)
{
   if (grd_unpack_buf!=NULL) {
      grs_bitmap tbm;
      if (gr_rsd8_convert(bm,&tbm)==GR_UNPACK_RSD8_OK) {
         bm->bits=tbm.bits;
         ps->dp+=(tbm.type-BMT_RSD8)*GRD_FUNCS;
         ((void (*)(grs_bitmap *, grs_per_setup *))(grd_tmap_init_table[ps->dp]))(bm,ps);
      }
   }
}
