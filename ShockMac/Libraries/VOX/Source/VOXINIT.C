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
 * $Source: n:/project/lib/src/vox/RCS/voxinit.c $
 * $Revision: 1.2 $
 * $Author: jaemz $
 * $Date: 1994/04/21 12:00:59 $
 *
 * Voxel initialization routines
 * This file is part of the vox library
 *
 * $Log: voxinit.c $
 * Revision 1.2  1994/04/21  12:00:59  jaemz
 * Added vxd_maxd to facilitate bounds checking in debug version
 * 
 * Revision 1.1  1994/04/21  10:52:54  jaemz
 * Initial revision
 * 
 */

#include "lg.h"
#include "vox.h"

// pointers to arrays for multiplication tables
fix *zdxdz;
fix *zdydz;

#ifdef DBG_ON
// maximal dimension use for bounds checking
int vxd_maxd;
#endif

// Startup for the voxel system, it needs to allocate
// space for the incremental multiplication tables
// pass it the maximum pixel dimension of any of the
// voxel objects you anticipate drawing
// returns TRUE for success, FALSE if unable to allocate
bool vx_init(int max_depth)
{
   zdxdz = (fix *)NewPtr(2 * max_depth * sizeof(fix));
   zdydz = zdxdz + max_depth;

   #ifdef DBG_ON
   vxd_maxd = max_depth;
   #endif

   if (zdxdz == NULL) return FALSE;
   return TRUE;
}

void vx_close()
{
   DisposePtr((Ptr)zdxdz);
}

// Der, this could be a macro, and
// maybe should be
void vx_init_vox(vxs_vox *v,fix pix_dist,fix pix_size,int depth,grs_bitmap *col,grs_bitmap *ht)
{
   v->pix_dist = pix_dist;
   v->pix_size = pix_size;

   v->col = col;
   v->ht = ht;

   v->w = col->w;
   v->h = col->h;
   v->d = depth;
}

