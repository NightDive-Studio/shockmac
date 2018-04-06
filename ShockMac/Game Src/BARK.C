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
 * $Source: u://RCS/bark.c $
 * $Revision: 1.33 $
 * $Author: xemu $
 * $Date: 1994/10/27 04:52:50 $
 * 
 *
 */

// Includes for example mfd.
#include <stdlib.h>

#include "bark.h"
#include "mfdext.h"
#include "mfddims.h"
#include "tools.h"
#include "gamestrn.h"
#include "objects.h"
#include "mfdart.h"
#include "gamescr.h"
#include "shodan.h"
#include "sfxlist.h"
#include "musicai.h"
#include "fullscrn.h"
#include "cit2d.h"
#include "citres.h"
#include "fullscrn.h"
#include "audiolog.h"
#include "gr2ss.h"


// ============================================================
//                   MFD BARK 
// ============================================================

#define BARK_MARGIN 2

void mfd_bark_expose(MFD* mfd, ubyte control)
{
   bool full = control & MFD_EXPOSE_FULL;
   if (control == 0)  // MFD is drawing stuff
   {
      // Do unexpose stuff here.  
   }
   if (control & MFD_EXPOSE) // Time to draw stuff
   {
      // clear update rects
      mfd_clear_rects();
      // set up canvas
      gr_push_canvas(pmfd_canvas);
      ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);

      // Clear the canvas by drawing the background bitmap
      if (!full_game_3d)
//KLC - chg for new art         ss_bitmap(&mfd_background, 0, 0);
         gr_bitmap(&mfd_background, 0, 0);

      if (mfd_bark_mug>0)
      {
         grs_bitmap bm;
         int mug=REF_IMG_EmailMugShotBase+mfd_bark_mug;

         bm.bits = NULL;

         extract_temp_res_bitmap(&bm,mug);
//KLC - chg for new art         ss_bitmap(&bm,(MFD_VIEW_WID-bm.w)/2,(MFD_VIEW_HGT-bm.h)/2);
         gr_bitmap(&bm,(SCONV_X(MFD_VIEW_WID)-bm.w)/2, (SCONV_Y(MFD_VIEW_HGT)-bm.h)/2);
      }
      else if (!full_game_3d)
      {
         draw_raw_resource_bm(MKREF(RES_mfdArtOverlays,MFD_ART_TRIOP),0,0);
      }
      if (full && global_fullmap->cyber && mfd->id == MFD_RIGHT
         && (full_visible & visible_mask(mfd->id)) == 0)
      {
#ifdef STEREO_SUPPORT
         if (convert_use_mode == 5)
            full_visible = visible_mask(mfd->id);
         else
#endif
            full_visible |= visible_mask(mfd->id);
         mfd_notify_func(MFD_BARK_FUNC,MFD_INFO_SLOT,FALSE,MFD_ACTIVE,TRUE);
      }

      if (full && mfd_bark_string != REF_STR_Null)
      {
         extern int hyphenated_wrap_text(char*, char*, short);
         char buf[256];
         short x,y;
         short w,h;
         RefTable *prt = (RefTable *)ResLock(REFID(mfd_bark_string));
//            ResReadRefTable(REFID(mfd_bark_string));
          
         if (RefIndexValid(prt,REFINDEX(mfd_bark_string)))
         {
            gr_set_font((grs_font*)ResLock(MFD_FONT));
            hyphenated_wrap_text(get_temp_string(mfd_bark_string),buf,MFD_VIEW_WID-2);
            gr_string_size(buf,&w,&h);
            gr_set_fcolor(mfd_bark_color);
            x = (MFD_VIEW_WID-w)/2;
            if(mfd_bark_mug>0)
               y = BARK_MARGIN;
            else
               y = (MFD_VIEW_HGT-h)/2;
            draw_shadowed_string(buf,x,y,mfd_bark_mug>0 || full_game_3d);
            ResUnlock(MFD_FONT);
         }
         ResUnlock(REFID(mfd_bark_string));
//         ResFreeRefTable(prt);
      }
      if (full)
         mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);

      // Pop the canvas
      gr_pop_canvas();
      // Now that we've popped the canvas, we can send the 
      // updated mfd to screen
      mfd_update_rects(mfd);

   }
  
}


void long_bark(ObjID speaker_id, uchar mug_id, int string_id, ubyte color)
{
   short mfd_id = mfd_grab_func(MFD_BARK_FUNC,MFD_INFO_SLOT);
#ifdef AUDIOLOGS
   errtype alog_rv = ERR_NOEFFECT;
#endif

   mfd_bark_string = string_id;
   mfd_bark_speaker = speaker_id;
   mfd_bark_color = color;
   mfd_bark_mug = mug_id;
#ifdef AUDIOLOGS
   if ((audiolog_setting) && (REFID(string_id) == RES_traps))
      alog_rv = audiolog_bark_play(string_id - REF_STR_TrapZeroMessage);
#else
   if ((mug_id >= FIRST_SHODAN_BARK) && (mug_id <= FIRST_SHODAN_BARK + NUM_SHODAN_MUGS - 1))
      play_digi_fx(SFX_SHODAN_BARK, 1);
#endif

#ifdef AUDIOLOGS
   if ((alog_rv != OK) || (audiolog_setting == 2))
#endif
   {
      mfd_notify_func(MFD_BARK_FUNC,MFD_INFO_SLOT,TRUE,MFD_ACTIVE,TRUE);
      if (speaker_id>0)
      {
         save_mfd_slot(mfd_id);
         player_struct.panel_ref = speaker_id;
      }
      mfd_change_slot(mfd_id,MFD_INFO_SLOT);
   }
}
