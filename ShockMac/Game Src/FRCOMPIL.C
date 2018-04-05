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
 * FrCompil.c
 *
 * $Source: n:/project/cit/src/RCS/frcompil.c $
 * $Revision: 1.8 $
 * $Author: dc $
 * $Date: 1994/04/23 09:21:25 $
 *
 * Citadel Renderer
 *  various clippers for terrain, including basic cone clip and the later day
 *   tile based clipper
 *  
 * $Log: frcompil.c $
 * Revision 1.8  1994/04/23  09:21:25  dc
 * stuff for map clear state
 * 
 * Revision 1.7  1994/01/02  17:11:37  dc
 * Initial revision
 * 
 */
#define __FRCOMPIL_SRC
#include "frintern.h"
#include "map.h"
#include "mapflags.h"

void fr_compile_rect(fmp *fmptr, int llx, int lly, int ulx, int uly, bool seen_bits)
{
   FullMap *fm = (FullMap *)fmptr;
   int x,y;
   MapElem *mptr;

   if (lly>0) lly--; else lly=0;
   if (llx>0) llx--; else llx=0;
   if (uly<fm_y_sz(fm)-1) uly++; else uly=fm_y_sz(fm)-1;
   if (ulx<fm_x_sz(fm)-1) ulx++; else ulx=fm_x_sz(fm)-1;
   y=lly; x=llx;
   for (; y<=uly; y++)
	{
      x=llx;
		mptr=FULLMAP_GET_XY(fm,x,y);
      for (; x<=ulx; x++, mptr++)
      {
         me_clearsolid_set(mptr,0);  // we know nothing
         if (seen_bits) me_bits_seen_clear(mptr);
      }
   }
}

void fr_compile_restart(fmp *fmptr)
{
	FullMap *fm = (FullMap *)fmptr;
	fr_pipe_resize(fm_x_sz(fm),fm_y_sz(fm),fm_z_shft(fm),fm_map(fm));
	fr_compile_rect(fm,0,0,fm_x_sz(fm),fm_y_sz(fm),FALSE);
}
