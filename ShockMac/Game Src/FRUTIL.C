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
 * $Source: r:/prj/cit/src/RCS/frutil.c $
 * $Revision: 1.12 $
 * $Author: dc $
 * $Date: 1994/11/25 16:57:41 $
 *
 * utilities for use in the basic indoor terrain rendering of tiles system
 */
#define __FRUTIL_SRC

#include <stdlib.h>
#include <string.h>

//#include <mprintf.h>

#include "faketime.h"

#include "frtypes.h"
#include "frintern.h"
#include "frparams.h"
#include "frflags.h"

uchar  fr_cur_obj_col;
ushort fr_col_to_obj[256];

static char fr_str[15];

#if 0 // KLC
char *fr_get_frame_rate(void)
{
   fr_str[0]='\0';
   if (_frp.time.last_frame_cnt>0)
   {
      if (_frp.time.last_chk_time!=0)
      {
         long num=(*tmd_ticks-_frp.time.last_chk_time);
         char mod;

         if (_frp.time.last_frame_cnt>1)
            num/=_frp.time.last_frame_cnt;
         num=28000/num;
         itoa(num,fr_str,10);
         mod=strlen(fr_str);
         fr_str[mod+1]=fr_str[mod];
         fr_str[mod]=fr_str[mod-1];
         fr_str[mod-1]=fr_str[mod-2];
         fr_str[mod-2]='.';
         _frp.time.last_frame_len=num/100;
      } 
      _frp.time.last_frame_cnt=0;
   }
   _frp.time.last_chk_time=*tmd_ticks;
   return fr_str;
}
#endif // KLC

// look, vainly i try an reuse code from uw2
//  and, amazingly, it works... wow
#define SEARCH_DIAM 10
/* uses xwid implicitly, as well as xhgt to determine bounds. note width is really xwid+2
 * checks within search_rad of x,y in the array pointed at by base
 * looks for an object...
 */
uchar check_around(uchar *base, int x, int y)
{
	uchar curval;
	int extloop, inloop, clen;
	int dvec[2]={0,1};

	for (clen=1; clen<SEARCH_DIAM; clen++) /* for each radius */
		for (extloop=0; extloop<2; extloop++) /* two of each */
		{
			for (inloop=0; inloop<clen; inloop++)
			{
				x+=dvec[0]; y+=dvec[1];
				if (((x>=0)&&(x<_fr->draw_canvas.bm.w))&&((y>=0)&&(y<_fr->draw_canvas.bm.h)))
				{
					curval=*(base+(y*_fr->draw_canvas.bm.row)+x);
					if ((curval>=FR_CUR_OBJ_BASE)&&(curval<fr_cur_obj_col))
						return curval;
				}
			}
			if (dvec[0]!=0) { dvec[1]= -dvec[0]; dvec[0]=0; }
			else 				 { dvec[0]= dvec[1] ; dvec[1]=0; }
		}
	return 0;
}

// is transp is set, then the get at is done with transparency on, being able to look through gratings, etc.
// if it is false, then transparency is not used
ushort fr_get_real(fauxrend_context *cur_fr, int x, int y)
{
   int col, tmpcol;
   if ((_fr_glob_flags|_fr->flags)&(FR_NORENDR_MASK|FR_SOLIDFR_MASK))  /* dont really render, call a game thing */
      return 0;
   col=(int)(*((cur_fr->draw_canvas.bm.bits)+(y*cur_fr->draw_canvas.bm.row)+(x)));
// mprintf("Color %d, obj %d, max c %d at %d %d\n",col,(col>=FR_CUR_OBJ_BASE)?fr_col_to_obj[col-FR_CUR_OBJ_BASE]:0,fr_cur_obj_col,x,y);
   if ((col>=FR_CUR_OBJ_BASE)&&(col<fr_cur_obj_col))         // if we are actually exactly over an object
      return (ushort)fr_col_to_obj[col-FR_CUR_OBJ_BASE];     //  actual obj_id
   if (tmpcol=check_around(cur_fr->draw_canvas.bm.bits,x,y)) // if we found an object nearby (what to do about transparent doors)
      return (ushort)fr_col_to_obj[tmpcol-FR_CUR_OBJ_BASE];  //  actual obj_id
   else                                                      // its a wall folks, just a wall
	   return ((ushort)0)-((ushort)col);                      //  return a tmap as - (tmapid+1), or nothing as 0
}

int         fr_cspace_idx(void)              {gr_set_fill_parm(1); return 1;}

ushort fr_get_at(frc *fr, int x, int y, bool transp)
{
   int (*fr_ptr_idx)(void)=fr_get_idx, of=_fr_glob_flags;

   _fr_top(fr);
   _fr_glob_flags|=FR_PICKUPM_MASK;
   if (!transp) _fr_glob_flags|=FR_NOTRANS_MASK;
   fr_cur_obj_col=FR_CUR_OBJ_BASE;
   if (_frp.faces.cyber)
	   fr_get_idx=fr_cspace_idx;
   else
	   fr_get_idx=fr_pickup_idx;
   fr_rend(fr);
   fr_get_idx=fr_ptr_idx;
   _fr_glob_flags=of;
   return fr_get_real(_fr,x,y);
}

ushort fr_get_again(frc *fr, int x, int y)
{
   _fr_top(fr);
   return fr_get_real(_fr,x,y);
}
