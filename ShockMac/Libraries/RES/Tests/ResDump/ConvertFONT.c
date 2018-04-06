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
//=======================================================================
//
// ConvertFONT  - Code to convert a PC FONT resource to the Mac.
//              	by Ken Cobb.
//
//=======================================================================

#include "lg_types.h"
#include "res.h"
#include "rect.h"

typedef struct
{
	ushort id;
	char dummy1[34];
	short min;
	short max;
	char dummy2[32];
	long cotptr;
	long buf;
	short w;
	short h;
	short off_tab[1];
} grs_font;

//-----------------------------------------------------------------------
void ConvertFONT(Ptr dataPtr, long size)
{
	short	i;
	
	grs_font	*fp = (grs_font *)dataPtr;

	fp->id = SwapShortBytes(fp->id);
	fp->min = SwapShortBytes(fp->min);
	fp->max = SwapShortBytes(fp->max);
	fp->w = SwapShortBytes(fp->w);
	fp->h = SwapShortBytes(fp->h);

	fp->cotptr = SwapLongBytes(fp->cotptr);
	fp->buf = SwapLongBytes(fp->buf);
	
	for(i = 0; i <= (fp->max - fp->min + 1); i++)
		fp->off_tab[i] = SwapShortBytes(fp->off_tab[i]);
}
