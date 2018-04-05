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
 *  $Source: n:/project/lib/src/fix/RCS/fix_pow.c $
 *  $Revision: 1.1 $
 *  $Author: lmfeeney $
 *  $Date: 1994/06/18 03:48:48 $
*/

#include <fix.h>

// returns the fixed point x ^ y, read em and weep
// this can easily overflow so chill
fix fix_pow(fix x,fix y)
{
   int i;
   fix ans;
   fix rh, rl;
   ushort yh,yl;

   ans = FIX_UNIT;
   yh = fix_int(y);
   yl = fix_frac(y);
   rh = rl = x;

   // calculate hi part, leave when done
   for (i=0;i<16;++i) {
      if (yh & 1) ans = fix_mul(ans,rh);
      if (yh!=0) rh = fix_mul(rh,rh);
      yh = yh >> 1;
      if (yl!=0) rl = fix_sqrt(rl);
      if (yl & 0x8000) ans = fix_mul(ans,rl);
      yl = yl << 1;
   }
   return ans;
}

