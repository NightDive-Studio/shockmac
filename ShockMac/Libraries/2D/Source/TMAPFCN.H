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
 * $Source: r:/prj/lib/src/2d/RCS/tmapfcn.h $
 * $Revision: 1.13 $
 * $Author: kevin $
 * $Date: 1994/07/29 12:02:37 $
 *
 * Public texture mapping procedures.
 *
 * This file is part of the 2d library.
 *
 */

#ifndef __TMAPFCN_H
#define __TMAPFCN_H

#include "grs.h"
#include "plytyp.h"
#include "tmaps.h"

extern void per_umap(grs_bitmap *bm, int n, grs_vertex **vpl, grs_tmap_info *ti);
extern void h_umap(grs_bitmap *bm, int n, grs_vertex **vpl, grs_tmap_info *ti);
extern void v_umap(grs_bitmap *bm, int n, grs_vertex **vpl, grs_tmap_info *ti);
extern int per_map(grs_bitmap *bm, int n, grs_vertex **vpl, grs_tmap_info *ti);
extern int h_map(grs_bitmap *bm, int n, grs_vertex **vpl, grs_tmap_info *ti);
extern int v_map(grs_bitmap *bm, int n, grs_vertex **vpl, grs_tmap_info *ti);

#endif /* __TMAPFCN_H */
