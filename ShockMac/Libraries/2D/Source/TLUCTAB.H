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
 * $Source: n:/project/lib/src/2d/RCS/tluctab.h $
 * $Revision: 1.9 $
 * $Author: baf $
 * $Date: 1994/01/31 14:46:54 $
 *
 * Declarations and macros for translucency
 * table generation.
 *
 * This file is part of the 2d library.
 *
 * $Log: tluctab.h $
 * Revision 1.9  1994/01/31  14:46:54  baf
 * Fixed a typo.
 * 
 * Revision 1.8  1994/01/17  22:13:22  baf
 * Redid tluc8 spolys (again).
 * 
 * Revision 1.7  1994/01/14  12:41:13  baf
 * Lit translucency reform.
 * 
 * Revision 1.6  1993/12/09  17:39:36  baf
 * Added lighting
 * 
 * Revision 1.5  1993/12/01  21:20:50  baf
 * Added gr_set_tluc8_table.
 * 
 *
 */

#ifndef _TLUCTAB
#define _TLUCTAB

#include "fix.h"
#include "grs.h"
#include "grmalloc.h"
#include "tlucdat.h"

extern uchar *gr_init_translucency_table(uchar *p, fix opacity, fix purity, grs_rgb color);
extern uchar *gr_init_lit_translucency_table(uchar *p, fix opacity, fix purity, grs_rgb color, grs_rgb light);
extern uchar *gr_init_lit_translucency_tables(uchar *p, fix opacity, fix purity, grs_rgb color, int n);

extern int gr_dump_tluc8_table(uchar *buf, int nlit);
extern void gr_read_tluc8_table(uchar *buf);

#define gr_alloc_translucency_table(n) \
   ((uchar *)NewPtr(n*256))
#define gr_free_translucency_table(tab) (DisposePtr((Ptr) tab))

#define gr_make_translucency_table(op, pu, co) \
  (gr_init_translucency_table(gr_alloc_translucency_table(1), op, pu, co))
#define gr_make_lit_translucency_table(op, pu, co, li) \
  (gr_init_translucency_table(gr_alloc_translucency_table(1), op, pu, co, li))
#define gr_make_lit_translucency_tables(op, pu, co, lnum) \
  (gr_init_lit_translucency_tables(gr_alloc_translucency_table(lnum), op, pu, co, lnum))

#define gr_make_tluc8_table(num, op, pu, co) \
   (tluc8tab[num]=gr_make_translucency_table(op, pu, co))
#define gr_make_lit_tluc8_table(num, op, pu, co, li) \
   (tluc8ltab[num]=gr_make_lit_translucency_tables(op, pu, co, li), \
    gr_make_tluc8_table(num, op, pu, co))
#define gr_alloc_tluc8_spoly_table(num) \
   (tluc8nstab=num, tluc8stab=gr_alloc_translucency_table(num))
#define gr_init_tluc8_spoly_table(num, op, pu, co, li) \
   (gr_init_lit_translucency_table(tluc8stab+(256*num), op, pu, co, li))
#define gr_init_tluc8_spoly_tables(num, op, pu, co, li) \
   (gr_init_lit_translucency_tables(tluc8stab+(256*num), op, pu, co, li))
#define gr_bind_tluc8_table(num, p) (tluc8tab[num]=p)
#define gr_bind_lit_tluc8_table(num, p) (tluc8ltab[num]=p)
#define gr_bind_tluc8_spoly_table(p) (tluc8stab=p)

#endif
