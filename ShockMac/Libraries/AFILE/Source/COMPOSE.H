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
//		COMPOSEW.H		WIDE8 Compose buffering
//		Rex E. Bradford (REX)
//
//		These routines implement a buffer for composing images.  You should
//		use one of the following types when defining such a compose buffer:
//
//		BMT_FLAT8:	Compose 8-bit images, don't need difference info
//		BMT_FLAT24:	Compose 24-bit images, don't need difference info

/*
* $Header: r:/prj/lib/src/afile/RCS/compose.h 1.1 1994/07/22 13:21:13 rex Exp $
* $Log: compose.h $
 * Revision 1.1  1994/07/22  13:21:13  rex
 * Initial revision
 * 
*/

#ifndef __COMPOSEW_H
#define __COMPOSEW_H

#ifndef __2D_H
#include "2d.h"
#endif
//#ifndef __RSD24_H
//#include <rsd24.h>
//#endif

//	Prototypes

void ComposeInit(grs_bitmap *pcompose, int bmtype, int w, int h);
void ComposeAdd(grs_bitmap *pcompose, grs_bitmap *pbm);
long ComposeDiff(grs_bitmap *pcompose, grs_bitmap *pbmNew,
	grs_bitmap *pbmDiff);
long ComposeConvert(grs_bitmap *pcompose, grs_bitmap *pbm);
void ComposeFree(grs_bitmap *pcompose);

//	Specific compose routines (type is compose buffer type, not bm!)

void ComposeFlat8Add(grs_bitmap *pcompose, grs_bitmap *pbm);
//void ComposeFlat24Add(grs_bitmap *pcompose, grs_bitmap *pbm);

//	Specific diff routines (type is compose buffer type, not bm!)

long ComposeFlat8Diff(grs_bitmap *pcompose, grs_bitmap *pbmNew,
	grs_bitmap *pbmDiff);
//long ComposeFlat24Diff(grs_bitmap *pcompose, grs_bitmap *pbmNew,
//	grs_bitmap *pbmDiff);

//	Specific convert routines (type is compose buffer type, not bm!)

long ComposeFlat8Convert(grs_bitmap *pcompose, grs_bitmap *pbm);
//long ComposeFlat24Convert(grs_bitmap *pcompose, grs_bitmap *pbm);

#endif

