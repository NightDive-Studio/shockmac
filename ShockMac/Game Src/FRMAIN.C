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
 * FrMain.c
 *
 * $Source: r:/prj/cit/src/RCS/frmain.c $
 * $Revision: 1.6 $
 * $Author: xemu $
 * $Date: 1994/10/08 04:02:08 $
 *
 * Citadel Renderer
 *  main routines to render and interact with high level renderer data
 *  
 * $Log: frmain.c $
 * Revision 1.6  1994/10/08  04:02:08  xemu
 * audiolog updates
 * 
 * Revision 1.5  1994/09/08  04:08:50  kevin
 * Turn off blending in pickup mode.
 * 
 * Revision 1.4  1994/09/05  06:43:36  dc
 * call pipe_go_3, add dummy test_me code
 * 
 * Revision 1.3  1994/03/09  03:01:56  xemu
 * c:\bin\more.exe frequent updating
 * 
 * Revision 1.2  1994/01/23  14:02:55  dc
 * static and other solid effects
 * 
 * Revision 1.1  1994/01/02  17:11:53  dc
 * Initial revision
 * 
 */

#define __FRMAIN_SRC
#include "frtypes.h"
#include "frintern.h"
#include "frparams.h"
#include "frflags.h"

int fr_pipe_go_2(void);
int fr_pipe_go_3(void);

// for synchronous update...
#ifdef AUDIOLOGS
extern errtype audiolog_loop_callback();
#endif

extern "C"
{
  void ClearCache (unsigned char* theAddress, unsigned long numBlocks);
}

int fr_rend(frc *view)
{
   fr_prepare_view(view);              /* init _fr, load flags, so on */
   if (!fr_start_view()) return -1;    /* broken broken - but what to really return */
   if (_fr_curflags&(FR_NORENDR_MASK|FR_SOLIDFR_MASK))  /* dont really render, call a game thing */
    { if (_fr->render_call) _fr->render_call(&_fr->draw_canvas.bm, _fr_curflags); }
   else
   {                                   /* actually do the 3d thang */
      extern bool _g3d_enable_blend;
      extern Boolean DoubleSize;
      bool save_blend_flag;

      if ((_fr_curflags&FR_PICKUPM_MASK) || DoubleSize) {
         save_blend_flag=_g3d_enable_blend;
         _g3d_enable_blend=FALSE;
      }

      // MLA - does nothing!  
      // synchronous_update();            // Make sure our time-sensitive updater gets run
#ifdef AUDIOLOGS
      audiolog_loop_callback();
#endif
      fr_pipe_start(-1);               /* set environment up */
      fr_clip_cone();                  /* generate basic spans */
      fr_clip_tile();                  /* clipping and obj sort pass */
      // MLA - does nothing!  
      // synchronous_update();            // One more time
//    	ClearCache(_fr->draw_canvas.bm.bits, (_fr->draw_canvas.bm.row >> 5) * _fr->ywid);
     	fr_pipe_go_3();                  /* actually render the stuff */
      fr_pipe_end();                   /* clean environment up */
      // MLA - does nothing!  
      // synchronous_update();            // And one for the road.
#ifdef AUDIOLOGS
      audiolog_loop_callback();
#endif

      if ((_fr_curflags&FR_PICKUPM_MASK) || DoubleSize) {
         _g3d_enable_blend=save_blend_flag;
      }
   }
   fr_send_view();                     /* send it, whether it came from 3d or special */
   if ((_fr->flags & FR_CURVIEW_MASK) == FR_CURVIEW_STRT)
	   _frp.time.last_frame_cnt++;
   return 1;
}

#ifdef DEBUG_STUFF_FOR_LATER
void    fr_show_stats(frc *view)
#endif
