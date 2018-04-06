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
 * $Source: n:/project/lib/src/3d/RCS/3dinterp.h $
 * $Revision: 1.2 $
 * $Author: kaboom $
 * $Date: 1993/10/02 09:19:46 $
 *
 * includes for the 3d interpreter
 *
 * $Log: 3dinterp.h $
 * Revision 1.2  1993/10/02  09:19:46  kaboom
 * New include for g3_set_vtext().
 * 
 * Revision 1.1  1993/08/04  00:48:56  dc
 * Initial revision
 */

// actual inline code
extern uchar	_vcolor_tab[]; 
#define g3_set_vcolor(vcolor_id, color_val) _vcolor_tab[vcolor_id] = (color_val);

/*void g3_set_vcolor(int vcolor_id, int color_val);
#pragma aux g3_set_vcolor = \
   "add eax, OFFSET vcolor_tab" \
   "mov [eax], bl" \
   parm [eax] [ebx] modify exact [eax]*/

extern g3s_point *_vpoint_tab[];
#define g3_set_vpoint(vpoint_id, point_ptr) _vpoint_tab[vpoint_id] = (g3s_point *) (point_ptr);

/*
void g3_set_vpoint(int vpoint_id, void *point_ptr);
#pragma aux g3_set_vpoint = \
   "mov vpoint_tab[eax*4], ebx" \
   parm [eax] [ebx] modify exact [eax]*/

extern grs_bitmap *_vtext_tab[];
#define g3_set_vtext(vtext_id, text_ptr) _vtext_tab[vtext_id] = (grs_bitmap *) (text_ptr);
/*
void g3_set_vtext(int vtext_id, void *text_ptr);
#pragma aux g3_set_vtext = \
   "mov vtext_tab[eax*4], ebx" \
   parm [eax] [ebx] modify exact [eax]*/
   

// flags for polygon draw type
extern uchar itrp_gour_flg, itrp_wire_flag, itrp_check_flg;

#define g3_set_gour_flag(x)   itrp_gour_flag  = x
#define g3_set_wire_flag(x)   itrp_wire_flag  = x
#define g3_set_check_flag(x)  itrp_check_flag = x
