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
 * $Source: r:/prj/cit/src/RCS/ammomfd.c $
 * $Revision: 1.11 $
 * $Author: xemu $
 * $Date: 1994/10/13 15:38:10 $
 *
 * $Log: ammomfd.c $
 * Revision 1.11  1994/10/13  15:38:10  xemu
 * SVGA interface
 * 
 * Revision 1.10  1994/08/17  03:07:49  xemu
 * dont' be affected by font change
 * 
 * Revision 1.9  1994/06/02  03:39:54  mahk
 * Fixes due to dummymfd bug.
 * 
 * Revision 1.8  1994/05/31  18:38:24  tjs
 * Fixed running/out/of/rectangles problem.
 * 
 * Revision 1.7  1994/05/24  20:10:25  minman
 * got rid of warning message whenever ammo page is drawn
 * ,
 * 
 * Revision 1.6  1994/05/19  13:41:21  tjs
 * Ammo MFD interface revision.
 * 
 * Revision 1.5  1994/05/19  04:01:46  tjs
 * Eliminated separate ammo mfd func.
 * 
 * Revision 1.4  1994/05/12  16:11:38  tjs
 * use string_replace_char
 * use AMMO_TYPE_LETTER for consistency.
 * 
 * Revision 1.3  1994/05/12  11:53:45  tjs
 * Actually allocate enough space for "minibuf"
 * 
 * Revision 1.2  1994/05/12  11:51:48  tjs
 * Fixed RCS log.
 * 
 * Revision 1.1  1994/05/10  02:52:34  tjs
 * Initial revision
 * 
 */

#include <string.h>

#include "mfdint.h"
#include "mfdext.h"
#include "mfddims.h"
#include "weapons.h"
#include "objsim.h"
#include "objprop.h"
#include "objwpn.h"
#include "gamestrn.h"
#include "objclass.h"
#include "colors.h"
#include "gamescr.h"
#include "cybstrng.h"
#include "tools.h"
#include "fullscrn.h"
#include "gr2ss.h"


// ============================================================
//                   THE AMMO MFD
// ============================================================


// -------
// DEFINES
// -------

#define AMMO_LIST_X 3
#define AMMO_LIST_Y 5
#define AMMO_COUNT_X 73

// color for weapon not owned, weapon owned, selected weapon.
uchar ammo_line_colors[] = { GREEN_BASE+6, GREEN_BASE+2, GREEN_YELLOW_BASE+1 };
#define AMMO_TITLE_COLOR (RED_BASE+5)

#define PLAYER_HASNT 0
#define PLAYER_HAS   1
#define PLAYER_HAS_SELECTED 2

// ----------
//  PROTOTYPES
// ----------
uchar player_has_weapon(int trip);
void mfd_ammo_expose(ubyte control);
bool mfd_ammo_handler(MFD* m, uiEvent* ev);


// return 0 if player does not have weapon of this type.
// returns 1 if player does have one, but not selected
// returns 2 if player has one selected.
//
uchar player_has_weapon(int trip)
{
   int num;
   bool retval=PLAYER_HASNT;
   weapon_slot* wp=player_struct.weapons;

   for(num=0;num<NUM_WEAPON_SLOTS && wp[num].type!=EMPTY_WEAPON_SLOT; num++) {
      if(MAKETRIP(CLASS_GUN,wp[num].type,wp[num].subtype)==trip) {
         if(player_struct.actives[ACTIVE_WEAPON]==num)
            retval=PLAYER_HAS_SELECTED;
         else if(retval!=PLAYER_HAS_SELECTED)
            retval=PLAYER_HAS;
      }
   }
   return(retval);
}
                                    
#define HARDWIRED_HEIGHT_CONSTANT   5
void mfd_ammo_expose(ubyte control)
{
   bool full = control & MFD_EXPOSE_FULL;
   if (control & MFD_EXPOSE) // Time to draw stuff
   {
      gr_set_font((grs_font*)ResGet(MFD_FONT));

      // Clear the canvas by drawing the background bitmap
      if (!full_game_3d)
//KLC - chg for new art         ss_bitmap(&mfd_background, 0, 0);
         gr_bitmap(&mfd_background, 0, 0);

      {
         int num, guntrip, typemask, type, subc, ammonum, count;
         short w,h,ypos=0,y_list;
         uchar col,has;
         bool gotammo;
         char ammoline[30], weapline[30], minibuf[3]=" /", *title;

         if(full) {
            title=get_temp_string(REF_STR_AmmoMFDWeaps);
            mfd_draw_string(title,AMMO_LIST_X,AMMO_LIST_Y+ypos,AMMO_TITLE_COLOR,TRUE);
            title=get_temp_string(REF_STR_AmmoMFDClips);
            gr_string_size(title,&w,&h);
            mfd_draw_string(title,AMMO_COUNT_X-w,AMMO_LIST_Y+ypos,AMMO_TITLE_COLOR,TRUE);
         }
         else {
            gr_string_size(minibuf,&w,&h);
         }
         ypos+=HARDWIRED_HEIGHT_CONSTANT;
         y_list = ypos+HARDWIRED_HEIGHT_CONSTANT;
 
         // iterate through gun types
         for (num=0;num<NUM_GUN;num++) {
            guntrip=get_triple_from_class_nth_item(CLASS_GUN,num);
            typemask=GunProps[CPTRIP(guntrip)].useable_ammo_type;
            subc=AMMOTYPE_SUBCLASS(typemask);
            typemask=AMMOTYPE_TYPE(typemask); // type type type

            if(typemask!=0) {
               gotammo=FALSE;
               ammoline[0]='\0';
               for(type=0;typemask!=0;typemask=typemask>>1,type++) {
                  if(typemask & 0x1) { // that's 1 HEX, mind you
                     ammonum = get_nth_from_triple(MAKETRIP(CLASS_AMMO,subc,type));
                     count=player_struct.cartridges[ammonum];
                     gotammo = gotammo || count;
                     numtostring(count,ammoline+strlen(ammoline));
                     minibuf[0]=AMMO_TYPE_LETTER(ammonum);
                     strcat(ammoline,minibuf[0]==' '?minibuf+1:minibuf);
                  }
               }

               has=player_has_weapon(guntrip);
               if(has||gotammo) {
                  get_object_short_name(guntrip,weapline,sizeof(weapline));
                  col=ammo_line_colors[has];
                  mfd_draw_string(weapline,AMMO_LIST_X,AMMO_LIST_Y+ypos,col,TRUE);

                  // get rid of trailing slash
                  ammoline[strlen(ammoline)-1]='\0';
                  // shamefully, replace '1' with 'I' to save space onscreen
                  string_replace_char(ammoline,'1','I');
                  gr_string_size(ammoline,&w,&h);
                  mfd_draw_string(ammoline,AMMO_COUNT_X-w,AMMO_LIST_Y+ypos,col,TRUE);
                  // glue together ammo rects so as not to
                  // run out of mfd rects, goddamnit.
                  mfd_add_rect(AMMO_COUNT_X-1,y_list,AMMO_COUNT_X,AMMO_LIST_Y+ypos);
                  ypos+=HARDWIRED_HEIGHT_CONSTANT;
               }

            }
         }
      }

      // on a full expose, make sure to draw everything
 
      if (full)
         mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);

   }
  
}

bool mfd_ammo_handler(MFD* m, uiEvent* ev)
{
   LGPoint pos;
   short w,h,line;
   int guntrip;
   uchar has;

   if(ev->type != UI_EVENT_MOUSE || !(ev->subtype & MOUSE_LDOWN)) return FALSE;

   pos = MakePoint(ev->pos.x - m->rect.ul.x, ev->pos.y - m->rect.ul.y);
   gr_font_char_size((grs_font*)ResGet(MFD_FONT),'X',&w,&h);

   line=(pos.y-AMMO_LIST_Y)/h;
   if(line<=0) return FALSE;
   guntrip=get_triple_from_class_nth_item(CLASS_GUN,line-1);
   if(guntrip<0) return FALSE;
   has=player_has_weapon(guntrip);
   if(has==PLAYER_HAS_SELECTED) {
      mfd_change_slot(m->id,MFD_WEAPON_SLOT);
      return TRUE;
   }
   return FALSE;
}
   
      


