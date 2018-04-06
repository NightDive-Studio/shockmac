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
 *  $Source: r:/prj/lib/src/2d/RCS/linfcn.h $
 *  $Revision: 1.1 $
 *  $Author: kevin $
 *  $Date: 1994/09/06 02:24:49 $
 */

#ifndef __LINFCN_H
#define __LINFCN_H

#include "plytyp.h"
/* functions living in the tables */

/* all - means that it is a dispatcher */
extern void gri_all_uiline_fill (long, long, grs_vertex *, grs_vertex *);

/* gen is also bank8, bank24 and modex */
extern void gri_gen_uline_fill (long, long, grs_vertex *, grs_vertex *);
extern void gri_gen_uhline_fill (short, short, short, long, long);
extern void gri_gen_uvline_fill (short, short, short, long, long);
extern void gri_gen_usline_fill (long, long, grs_vertex *, grs_vertex *);
extern void gri_gen_ucline_fill (long, long, grs_vertex *, grs_vertex *);
extern void gri_gen_wire_poly_uline (long, long, grs_vertex *, grs_vertex *);
extern void gri_gen_wire_poly_usline (long, long, grs_vertex *, grs_vertex *);
extern void gri_gen_wire_poly_ucline (long, long, grs_vertex *, grs_vertex *);

/* flat8  -- for each line type and fill type */
extern void gri_flat8_uline_ns (long, long, grs_vertex *, grs_vertex *);
extern void gri_flat8_uline_clut (long, long, grs_vertex *, grs_vertex *);
extern void gri_flat8_uline_xor (long, long, grs_vertex *, grs_vertex *);
extern void gri_flat8_uline_blend (long, long, grs_vertex *, grs_vertex *);

extern void gri_flat8_uhline_ns (short, short, short, long, long);
extern void gri_flat8_uhline_clut (short, short, short, long, long);
extern void gri_flat8_uhline_xor (short, short, short, long, long);
extern void gri_flat8_uhline_blend (short, short, short, long, long);

extern void gri_flat8_uvline_ns (short, short, short, long, long);
extern void gri_flat8_uvline_clut (short, short, short, long, long);
extern void gri_flat8_uvline_xor (short, short, short, long, long);
extern void gri_flat8_uvline_blend (short, short, short, long, long);

extern void gri_flat8_ucline_norm (long, long, grs_vertex *, grs_vertex *);
extern void gri_flat8_ucline_clut (long, long, grs_vertex *, grs_vertex *);
extern void gri_flat8_ucline_xor (long, long, grs_vertex *, grs_vertex *);
extern void gri_flat8_ucline_blend (long, long, grs_vertex *, grs_vertex *);

extern void gri_flat8_usline_norm (long, long, grs_vertex *, grs_vertex *);
extern void gri_flat8_usline_clut (long, long, grs_vertex *, grs_vertex *);
extern void gri_flat8_usline_xor (long, long, grs_vertex *, grs_vertex *);
extern void gri_flat8_usline_blend (long, long, grs_vertex *, grs_vertex *);

extern void gri_flat8_wire_poly_uline (long, long, grs_vertex *, grs_vertex *);
extern void gri_flat8_wire_poly_uline_xor (long, long, grs_vertex *, grs_vertex *);
extern void gri_flat8_wire_poly_uline_blend (long, long, grs_vertex *, grs_vertex *);

extern void gri_flat8_wire_poly_usline_norm (long, long, grs_vertex *, grs_vertex *);
extern void gri_flat8_wire_poly_usline_clut (long, long, grs_vertex *, grs_vertex *);
extern void gri_flat8_wire_poly_usline_xor (long, long, grs_vertex *, grs_vertex *);
extern void gri_flat8_wire_poly_usline_blend (long, long, grs_vertex *, grs_vertex *);

extern void gri_flat8_wire_poly_ucline_norm (long, long, grs_vertex *, grs_vertex *);
extern void gri_flat8_wire_poly_ucline_clut (long, long, grs_vertex *, grs_vertex *);
extern void gri_flat8_wire_poly_ucline_xor (long, long, grs_vertex *, grs_vertex *);
extern void gri_flat8_wire_poly_ucline_blend (long, long, grs_vertex *, grs_vertex *);
/* bank8 and modex have their own hlines only */

extern void gri_modex_uhline_ns (short, short, short, long, long);
extern void gri_modex_uhline_clut (short, short, short, long, long);
extern void gri_modex_uhline_xor (short, short, short, long, long);
extern void gri_modex_uhline_blend (short, short, short, long, long); 

extern void gri_bank8_uhline_ns (short, short, short, long, long);
extern void gri_bank8_uhline_clut (short, short, short, long, long);
extern void gri_bank8_uhline_xor (short, short, short, long, long);
extern void gri_bank8_uhline_blend (short, short, short, long, long);

#endif
