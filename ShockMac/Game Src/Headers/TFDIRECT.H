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
 * $Source: r:/prj/cit/src/inc/RCS/tfdirect.h $
 * $Revision: 1.4 $
 * $Author: dc $
 * $Date: 1994/08/30 05:10:47 $
 */

// with tommorrow, track 13, till i can gain (18)

#ifndef __TFDIRECT_H
#define __TFDIRECT_H

// some useful constants, i hope
#define fix_0         fix_make(0,0)
#define fix_1         fix_make(1,0)
#define fix_root2     fix_make(1,27146)   // 1.414
#define fix_inv_root2 fix_make(0,46341)   // 0.707

// globals
extern fix (*tf_vert_2d)[2];  // 4 elements
extern char  tf_norm_hnts[4]; // hint for each if used
extern fix  *tf_pt;           // 3 elements: first 2 in plane, 3 is distance from plane
extern fix   tf_loc_pt[3];
extern fix   tf_raw_pt[3];     // raw world location of object
extern int   tf_ph;            // current physics handle
extern fix   tf_rad;            // current physics handle

// masks and hints
#define FACELET_MASK_N (1<<0)
#define FACELET_MASK_E (1<<1)
#define FACELET_MASK_S (1<<2)
#define FACELET_MASK_W (1<<3)
#define FACELET_MASK_F (1<<4)
#define FACELET_MASK_C (1<<5)
#define FACELET_MASK_I (1<<6)

#define NO_NORM_HINT (127)

// prototypes....
bool tf_solve_aligned_face(fix pt[3], fix walls[4][2], int flags, fix *norm);
bool tf_solve_remetriced_face(fix pt[3], fix walls[4][2], int flags, fix norm[3], fix metric);
bool tf_solve_cylinder(fix pt[3], fix rad, fix height);
void tf_global_bcd_add(int flg, int param);

// for now, really this will go soon....
// tfutil stupidity till physics really deals
#define FCE_NO_PRIM  (113)
void facelet_clear(void);
void facelet_add(int which, fix norm[3], fix atten, fix comp, int prim);
void facelet_send(void);

#endif
