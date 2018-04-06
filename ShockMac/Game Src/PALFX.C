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
 * $Source: r:/prj/cit/src/RCS/palfx.c $
 * $Revision: 1.10 $
 * $Author: minman $
 * $Date: 1994/08/27 04:34:07 $
 *
 */

#include <string.h>
#include "init.h"
#include "palfx.h"


byte pal_fade_id;
byte cyc_id0, cyc_id1, cyc_id2, cyc_id3, cyc_id4, cyc_id5;

#define FADE_DOWN_DELAY   3
#define FADE_DOWN_STEPS   30

#define FADE_UP_DELAY   0
#define FADE_UP_STEPS   30

// Internal Prototypes
void finish_pal_effect(byte id);
byte palfx_start_fade_up(uchar *new_pal);


//-------------------------------------
void finish_pal_effect(byte id)
{
   while (palette_query_effect(id) == ACTIVE) 
      palette_advance_effect(id, 1);
}


//-------------------------------------
void palfx_fade_down()
{
   byte id;
   static uchar blackp[768];
   static uchar savep[768];

   LG_memset(blackp, 0, sizeof(blackp));
   gr_get_pal(0, 256, savep);
   
   if ((num_installed_shifts >= 1) &&
       (pal_fade_id >= 0) &&
       (palette_query_effect(pal_fade_id) == ACTIVE))            
      { palette_remove_effect(pal_fade_id); }

   id = palette_install_fade(REAL_TIME, 0, 255, FADE_DOWN_DELAY, FADE_DOWN_STEPS, savep, blackp);
   finish_pal_effect(id);

   return;
}
 

//-------------------------------------
byte palfx_start_fade_up(uchar *new_pal)
{
   static uchar blackp[768];
   byte id;

   LG_memset(blackp, 0, sizeof(blackp));
   id = palette_install_fade(REAL_TIME, 0, 255, FADE_UP_DELAY, FADE_UP_STEPS, blackp, new_pal);
   palette_advance_effect(id, 1);
   return(id);
}


//-------------------------------------
void palfx_fade_up(bool do_now)
{
   // ppall is defined as the main shadow palette in init.c
   pal_fade_id=palfx_start_fade_up(ppall);

   if (do_now)
      finish_pal_effect(pal_fade_id);
}


//-------------------------------------
void palfx_init()
{      
   palette_initialize(8);     // 1 time unit per frame, 8 effects max
   palette_set_rate(1);
   
   cyc_id0 = palette_install_cbank(REAL_TIME, 0x03, 0x07, 68);	// 80
   cyc_id1 = palette_install_cbank(REAL_TIME, 0x0b, 0x0f, 40);	// 50
   cyc_id2 = palette_install_cbank(REAL_TIME, 0x10, 0x14, 20);	// 25
   cyc_id3 = palette_install_cbank(REAL_TIME, 0x15, 0x17, 108);	// 125
   cyc_id4 = palette_install_cbank(REAL_TIME, 0x18, 0x1a, 84);	// 100
   cyc_id5 = palette_install_cbank(REAL_TIME, 0x1b, 0x1f, 64);	// 75
} 
