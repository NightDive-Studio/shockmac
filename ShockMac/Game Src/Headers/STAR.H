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
 * $Source: r:/prj/lib/src/star/RCS/star.h $
 * $Revision: 1.1 $
 * $Author: jaemz $
 * $Date: 1994/10/24 23:27:39 $
 *
 * Star library header
 *
 * $Log: star.h $
 * Revision 1.1  1994/10/24  23:27:39  jaemz
 * Initial revision
 * 
*/

#ifndef __STAR_H
#define __STAR_H

// small vector structure, all elements are 15 bit
// with top bit of sign.  Shift left when unpacking
// to a fix, since sts_vecs are normalized they can
// be this size
typedef struct {
   short x,y,z;
} sts_vec;

extern   int std_size;

// sets global pointers in the star library
// to the number of stars, their positions, their colors
void star_set(int n,sts_vec *vlist,uchar *clist);

// allocates the necessary space for stars using alloc
// returns neg 1 if problem
int star_alloc(int n);

// frees star space using free
void star_free(void);

// stuffs random vectors and colors into the set areas
// randomly assigning a color range to them
void star_rand(uchar col,uchar range);

// render a starry polygon
void star_poly(int n,g3s_phandle *vp);

// render a starry polygon
void star_empty(int n,g3s_phandle *vp);

// Render to an empty sky, you'll have
// to blacken it for us to color 0
void star_sky(void);

// renders star field in the polygon defined by the vertex list
// uses your 3d context, so make sure that's been set
// call this before doing a frame end.  You can put it in
// an object frame if you'd like to rotate them and such
void star_render(void);

// transform star point frugally, only doing z if possible against
// half plane, then projecting if in viewing pyramid
g3s_phandle star_transform_point(g3s_vector *v);
//#pragma aux star_transform_point "*" parm [esi] value [edi] modify [eax ebx ecx edx esi edi];


#endif








