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
 * $Source: r:/prj/cit/src/RCS/target.c $
 * $Revision: 1.48 $
 * $Author: tjs $
 * $Date: 1994/11/09 01:03:30 $
 * 
 *
 */

#include <stdio.h>
#include <string.h>

#include "target.h"
#include "colors.h"
#include "newmfd.h"
#include "mfdext.h"
#include "mfddims.h"
#include "tools.h"
#include "objclass.h"
#include "objprop.h"
#include "objcrit.h"
#include "objsim.h"
#include "gamestrn.h"
#include "damage.h"
#include "ai.h"
#include "hudobj.h"
#include "combat.h"
#include "wares.h"
#include "strwrap.h"
#include "ai.h"
#include "aiflags.h"
#include "visible.h"
#include "fullscrn.h"
#include "gamescr.h"
#include "cybstrng.h"
#include "otrip.h"
#include "mfdart.h"
#include "cit2d.h"
#include "gr2ss.h"

#define sqr(x) ((x)*(x)) 

// -------------
//  PROTOTYPES
// -------------
void right_justify_num(char *num, int dlen);
bool iter_eligible_targets(ObjSpecID *sid);
void select_closest_target(void);
void toggle_current_target_backwards(void);
void mfd_targetware_expose(MFD* mfd, ubyte control);
bool mfd_targetware_handler(MFD *m, uiEvent *e);



// ==============================================
//               TARGET MFD CODE
// ==============================================
                               
#define LEFT_MARGIN    0
#define RNG_FIELD     30  
#define TOP_MARGIN     2
#define STATUS_TEXT_Y 35
#define MOOD_Y        27
#define Y_STEP         5

#define HP_BAR_MARGIN 0

#define TEXT_COLOR (RED_BASE+5)
#define TBUFSIZ 80
#define TARGET_FONT RES_tinyTechFont


// get the damage string for a critter subclass and damage estimate
#define DAMAGE_STRING_BASE REF_STR_MutantDmg
#define GET_DAMAGE_STRING(scl,dmg) (DAMAGE_STRING_BASE + (scl)*DAMAGE_DEGREES+(dmg))


extern bool full_game_3d;
extern int mfd_bmap_id(int triple);

#define PAGEBUTT_W 8
#define PAGEBUTT_H 11
#define BUTTON_Y (MFD_VIEW_HGT - PAGEBUTT_H - 2)
#define TEXT_RIGHT_X 42


#define LAST_TARGET(mfd) (player_struct.mfd_func_data[MFD_TARGET_FUNC][mfd])
#define LAST_MOOD(mfd)   (player_struct.mfd_func_data[MFD_TARGET_FUNC][mfd+2])
#define LAST_STATUS(mfd) (player_struct.mfd_func_data[MFD_TARGET_FUNC][mfd+4])

void right_justify_num(char *num, int dlen)
{
   int   len  = strlen(num);
   int   i;
   int   delta;

   if (len >= dlen)
      return;
   else
      delta = dlen-len;

   for (i=len;i>=0;i--)
      num[delta+i] = num[i];

   memset(num, '0', delta);
}


// ----------------------------------------------------------------------------
// mfd_target_expose()
//

void mfd_target_expose(MFD *m, ubyte control)
{
   int version = player_struct.hardwarez[CPTRIP(TARG_GOG_TRIPLE)];
   bool full = control & MFD_EXPOSE_FULL;
   ObjSpecID target = objs[player_struct.curr_target].specID;

   if (version < 1)
   {
      target = OBJ_SPEC_NULL;
      player_struct.curr_target = OBJ_NULL;
   }
   if (player_struct.curr_target != LAST_TARGET(m->id)) full = TRUE;

   if (control & MFD_EXPOSE) {

      // If we just turned to this page, select a target if none currently
      // selected, because, hey, this is just a goofy version anyway.
//      if ((control & MFD_EXPOSE_FULL) && (target == OBJ_SPEC_NULL))
//         { toggle_current_target(); target = objs[player_struct.curr_target].specID; }

      gr_push_canvas(pmfd_canvas);
      ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
      mfd_clear_rects();

      if (!full_game_3d)
//KLC - chg for new art         ss_bitmap(&mfd_background, 0, 0);
         gr_bitmap(&mfd_background, 0, 0);

      if (full && target == OBJ_SPEC_NULL)
      {
         char buf[80];
         short w,h;
         draw_res_bm(MKREF(RES_mfdArtOverlays,MFD_ART_TRIOP), 0, 0);
         gr_set_font((grs_font*)ResLock(TARGET_FONT));
         get_string((version > 0) ? REF_STR_NoTarget : REF_STR_NoTargetWare,buf,sizeof(buf));
         wrap_text(buf,MFD_VIEW_WID);
         mfd_string_wrap = FALSE;
         gr_string_size(buf,&w,&h);
         mfd_full_draw_string(buf, (MFD_VIEW_WID - w)/2, MFD_VIEW_HGT/2-((version>0)?h:(h/2)), TEXT_COLOR, TARGET_FONT, TRUE, TRUE);
         mfd_string_wrap = TRUE;
         unwrap_text(buf);
         if(version>0) {
#ifdef REF_STR_NumKills
            get_string(REF_STR_NumKills,buf,sizeof(buf));
#else
            strcpy(buf,"Kills: ");
#endif
            numtostring(player_struct.num_victories,buf+strlen(buf));
            mfd_string_wrap = FALSE;
            gr_string_size(buf,&w,&h);
            mfd_full_draw_string(buf, (MFD_VIEW_WID - w)/2, MFD_VIEW_HGT/2, TEXT_COLOR, TARGET_FONT, TRUE, TRUE);
            mfd_string_wrap = TRUE;
            unwrap_text(buf);
         }
         if(version>0) {
            draw_raw_resource_bm(REF_IMG_PrevPage,0,BUTTON_Y);
            draw_raw_resource_bm(REF_IMG_NextPage,TEXT_RIGHT_X-PAGEBUTT_W,BUTTON_Y);
            draw_raw_resource_bm(REF_IMG_Near,(TEXT_RIGHT_X+LEFT_MARGIN - res_bm_width(REF_IMG_Near))/2,BUTTON_Y);
         }
         mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
         ResUnlock(TARGET_FONT);
      }
      else
      {
         LGPoint siz;
         short x; 
         short y;
         int triple = ID2TRIP(objCritters[target].id);
         char buf[TBUFSIZ];
         int dmg;
         int id = mfd_bmap_id(triple);

         // Aha! Herein lies the meat of the targeting display.

         // draw the creature mfd bitmap
#ifdef PLAYTEST
         if (RefIndexValid((RefTable*)ResGet(REFID(id)),REFINDEX(id)))
#endif
         {
//KLC - chg for new art
            x = SCONV_X(MFD_VIEW_WID) - res_bm_width(id) - SCONV_X(HP_BAR_MARGIN);
            y = (SCONV_Y(MFD_VIEW_HGT) - res_bm_height(id))/2;
//            draw_raw_res_bm_extract(id,x,y,MFD_EXTRACT_BUF);
            draw_hires_resource_bm(id, x, y);
         }
         if (full)
            mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
         y = TOP_MARGIN;

         mfd_string_shadow = MFD_SHADOW_ALWAYS;
         {
            int len,sc;
            char small_buf[15];

            /// Draw target id.
            sc=objs[player_struct.curr_target].subclass;
            get_string(REF_STR_TargetID,buf,TBUFSIZ);
            len=strlen(buf);
            get_string(REF_STR_CritClasses+sc,buf+len,TBUFSIZ-len);
            len=strlen(buf);
            buf[len]='-';

            // it's edward - do him as cyborg 1
            if(sc==CRITTER_SUBCLASS_ROBOBABE)
            {
               numtostring(objs[player_struct.curr_target].info.type,small_buf);
               right_justify_num(small_buf, 5);
               strcpy(buf+len+1, small_buf);
            }
            else
            {
               // let's start with the level
               *(buf+len+1) = (player_struct.level < 10) ? '0' : player_struct.level/10;
               *(buf+len+2) = player_struct.level % 10 + '0';
               // add the subclass
               *(buf+len+3) = sc + '0';
               // and lastly, add the SpecID
               numtostring(target+1,small_buf);
               right_justify_num(small_buf, 2);
               strcpy(buf+len+4, small_buf);
            }
            siz = mfd_full_draw_string(buf, LEFT_MARGIN, y, TEXT_COLOR, TARGET_FONT, TRUE, TRUE);
            y += siz.y;

            // draw target name. 
            get_object_long_name(triple,buf,TBUFSIZ);
            string_replace_char(buf,'\n',CHAR_SOFTSP);
            siz = mfd_full_draw_string(buf, LEFT_MARGIN, y, TEXT_COLOR, TARGET_FONT, TRUE, TRUE);
            y += siz.y;
            if(version>0) {
               // draw buttons
               draw_raw_resource_bm(REF_IMG_PrevPage,0,BUTTON_Y);
               draw_raw_resource_bm(REF_IMG_NextPage,TEXT_RIGHT_X-PAGEBUTT_W,BUTTON_Y);
               draw_raw_resource_bm(REF_IMG_Near,(TEXT_RIGHT_X+LEFT_MARGIN - res_bm_width(REF_IMG_Near))/2,BUTTON_Y);
            }
         }
         if (!full)
            mfd_clear_rects();

         // draw target range
         {
            LGRect r;
            short x = TEXT_RIGHT_X;
            short lx;
            char rstr[10];
            ObjLoc loc = objs[player_struct.curr_target].loc;
            ObjLoc ploc = objs[PLAYER_OBJ].loc;
            long mapdist;
            fix dist;
            
            mapdist = long_fast_pyth_dist(loc.x - ploc.x,loc.y - ploc.y);
            mapdist = mapdist*100*8*12/(39*256); // convert to centimeters.
            dist=fix_mul(FIX_UNIT/100,fix_make(mapdist,0)); // fix meters, 2d
            dist=fix_fast_pyth_dist(dist,fix_from_obj_height(PLAYER_OBJ)
                                        -fix_from_obj_height(player_struct.curr_target));
            gr_set_fcolor(TEXT_COLOR);
            gr_set_font((grs_font*)ResLock(TARGET_FONT));
//            if (full)
               siz = mfd_full_draw_string(get_temp_string(REF_STR_TargRange), LEFT_MARGIN, y, TEXT_COLOR, TARGET_FONT, TRUE, TRUE);
//            else
//               gr_string_size(get_temp_string(REF_STR_TargRange),&siz.x,&siz.y);
            lg_sprintf(rstr,"%2.2fm",dist);
            // lx used as dummy variable here, before we really need it.
            gr_string_size(rstr,&x,&lx);
            x=LEFT_MARGIN+siz.x+RNG_FIELD-x;
            draw_shadowed_string(rstr,x,y,TRUE);
            lx = LEFT_MARGIN + siz.x;
            if (lx < x)
            {
               r.ul = MakePoint(lx,y);
               r.lr = MakePoint(x,y+siz.y);
               mfd_partial_clear(&r);
            }
            mfd_add_rect(x,y,TEXT_RIGHT_X,y+siz.y);
            mfd_notify_func(MFD_TARGET_FUNC,MFD_TARGET_SLOT,FALSE,MFD_ACTIVE,FALSE);
            ResUnlock(TARGET_FONT);
         }         

         // Draw the target's mood
         if (version >= 2)
         {
            ubyte clr = TEXT_COLOR;
            ubyte mood = objCritters[target].mood;
            if (full || mood != LAST_MOOD(m->id))
            {
               // check if critter is asleep - only order we care about
               if (objCritters[target].orders == AI_ORDERS_SLEEP)
                  mood = NUM_AI_MOODS;
               else if (objCritters[target].flags & AI_FLAG_TRANQ)
                  mood = NUM_AI_MOODS+1;
               else if (objCritters[target].flags & AI_FLAG_CONFUSED)
                  mood = NUM_AI_MOODS+2;

               get_string(REF_STR_CritMoods + mood,buf,TBUFSIZ);
               if (mood == AI_MOOD_HOSTILE) clr = RED_BASE+1;
               siz = mfd_full_draw_string(buf, LEFT_MARGIN, MOOD_Y, clr, TARGET_FONT, TRUE, TRUE);
               mfd_add_rect(LEFT_MARGIN,MOOD_Y,MFD_VIEW_WID,MOOD_Y+siz.y);
               LAST_MOOD(m->id) = mood;
            }
         }

         if (version >= 2)
         {
            // draw target status 
            y = STATUS_TEXT_Y;
            dmg = get_damage_estimate(target);
            if (dmg < DAMAGE_MIN || dmg > DAMAGE_MAX)
               dmg = DAMAGE_CRITICAL;
            if (full || dmg != LAST_STATUS(m->id))
            {
               get_string(GET_DAMAGE_STRING(TRIP2SC(triple),dmg),buf,TBUFSIZ);            

//             siz = mfd_draw_string(get_temp_string(REF_STR_Condition), LEFT_MARGIN, y, TEXT_COLOR, TRUE);
//             y += siz.y + 1;
               siz = mfd_full_draw_string(buf, LEFT_MARGIN, y, TEXT_COLOR, TARGET_FONT, TRUE, TRUE);
               y += siz.y + 1;
               LAST_STATUS(m->id) = dmg;
            }
         }
         mfd_string_shadow = MFD_SHADOW_FULLSCREEN;
      }

      gr_pop_canvas();
      mfd_update_rects(m);
   }
   LAST_TARGET(m->id) = player_struct.curr_target;


   return;
}


// ==============================================
//               TARGET HARDWARE CODE
// ==============================================

#define NUM_TARG_FRAMES 5
extern ubyte targ_frame;

void select_current_target(ObjID id, bool force_mfd)
{
#ifdef ANNOY_PLAYERS_TRYING_TO_TARGET_THINGS
   extern errtype change_current(ObjRefID new_current_ref);
#endif
   if ((player_struct.hardwarez[CPTRIP(TARG_GOG_TRIPLE)] == 0) || (id == PLAYER_OBJ))
      return;
   if (objs[id].info.current_hp <= 0)
      return;

   hudobj_set_id(player_struct.curr_target,FALSE); 
   player_struct.curr_target = id; 
   hudobj_set_id(id,TRUE);
   targ_frame = NUM_TARG_FRAMES;
   if (force_mfd)
   {
      int m = NUM_MFDS;
      if(id != OBJ_NULL && !mfd_yield_func(MFD_TARGET_FUNC,&m)) {
         use_ware(WARE_HARD,HARDWARE_TARGET);
         mfd_yield_func(MFD_TARGET_FUNC,&m);
      }
      if(m!=NUM_MFDS)
         full_visible |= visible_mask(m);
   }
   mfd_notify_func(MFD_TARGET_FUNC,MFD_TARGET_SLOT,FALSE,MFD_ACTIVE,TRUE);
#ifdef ANNOY_PLAYERS_TRYING_TO_TARGET_THINGS
   change_current(objs[id].ref);
#endif
} 


#define ELIGIBLE_TARGET_RANGE 20 
#define ELIGIBLE_TARGET_RANGE_SQUARED (ELIGIBLE_TARGET_RANGE*ELIGIBLE_TARGET_RANGE)
// ---------------------------------------------------
// iter_eligible_targets() 
// takes a pointer to an objSpecId for a creature.  If there is a later creature in the level's creature 
// list that is a valid target, modifies the specid to point to that target and returns TRUE, otherwise, 
// returns FALSE, setting the specID to OBJ_SPEC_NULL.  If *sid is OBJ_SPEC_NULL, sets *sid to the first 
// eligible critter, or returns FALSE if none exists. 

bool iter_eligible_targets(ObjSpecID *sid) 
{
   ObjLoc ploc = objs[PLAYER_OBJ].loc;
   LGPoint plr;
   plr.x = OBJ_LOC_BIN_X(ploc);
   plr.y = OBJ_LOC_BIN_Y(ploc);

   if (*sid == OBJ_SPEC_NULL) *sid = objCritters[0].id;
   else *sid = objCritters[*sid].next;
   for(;*sid != OBJ_SPEC_NULL; *sid = objCritters[*sid].next)
   {
      ObjID oid = objCritters[*sid].id;
      ObjLoc loc = objs[oid].loc;
      int dsq = sqr(OBJ_LOC_BIN_X(loc) - plr.x) +
                sqr(OBJ_LOC_BIN_Y(loc) - plr.y);

      // you cannot target yourself - well - not this game....
      // cause then you'll have nobody to take your tasks...
      if (oid == PLAYER_OBJ)
         continue;

      if (get_crit_posture(*sid) == DEATH_CRITTER_POSTURE || 
            objs[PLAYER_OBJ].specID == *sid) continue;
      if (dsq > ELIGIBLE_TARGET_RANGE_SQUARED)
         continue;
   	if (ray_cast_objects(PLAYER_OBJ, oid, VISIBLE_MASS, VISIBLE_SIZE,
         VISIBLE_SPEED, fix_make(ELIGIBLE_TARGET_RANGE,0)) != oid)
         continue;
      return TRUE;
   }
   return FALSE;

}


// ---------------------------------------------------------------------------
// select_closest_target()
//
// finds the closest eligible target and selects it.  Duh

void select_closest_target(void)
{
   ObjLoc ploc = objs[PLAYER_OBJ].loc;
   LGPoint plr;
   ObjSpecID sid = OBJ_SPEC_NULL;
   ObjID bestid = OBJ_NULL;
   uint bestdist = 0xFFFFFFFF;
   plr.x = OBJ_LOC_BIN_X(ploc);
   plr.y = OBJ_LOC_BIN_Y(ploc);
   while(iter_eligible_targets(&sid))
   {
      ObjID oid = objCritters[sid].id;
      ObjLoc loc = objs[oid].loc;
      int dsq = sqr(OBJ_LOC_BIN_X(loc) - plr.x) +
                sqr(OBJ_LOC_BIN_Y(loc) - plr.y);
      if (dsq < bestdist)
      {
         bestid = oid;
         bestdist = dsq;
      }
   }
   select_current_target(bestid,TRUE);
   if (player_struct.curr_target == OBJ_NULL)
      string_message_info(REF_STR_NoTargetsAround);
}

// ---------------------------------------------------------------------------
// toggle_current_target_backwards()
//
// finds the closest eligible target and selects it.  Duh

void toggle_current_target_backwards(void)
{
   ObjSpecID sid = OBJ_SPEC_NULL;
   ObjID bestid = OBJ_NULL;
   if (player_struct.curr_target != OBJ_NULL)
      while(iter_eligible_targets(&sid))
      {
         ObjID oid = objCritters[sid].id;
         if (oid == player_struct.curr_target)
            break;
         bestid = oid;
      }
   if (player_struct.curr_target == OBJ_NULL || bestid == OBJ_NULL)
   {
      while(iter_eligible_targets(&sid))
      {
         ObjID oid = objCritters[sid].id;
         if (sid == OBJ_SPEC_NULL)
            break;
         bestid = oid;
      }
   }
   select_current_target(bestid,TRUE);
   if (player_struct.curr_target == OBJ_NULL)
      string_message_info(REF_STR_NoTargetsAround);
}


// ---------------------------------------------------------------------------
// toggle_current_target()
//
// Doofy code to cycle through all extant critters and pop 'em up on targeting

void toggle_current_target()
{
   ObjSpecID oldsid, osid;
   ObjID old;

   old = player_struct.curr_target;
   if (!objs[old].active) old = OBJ_NULL;

   if (old != OBJ_NULL)
   {
      osid = oldsid = objs[old].specID;
      iter_eligible_targets(&osid);
   }
   else osid = oldsid = OBJ_SPEC_NULL;

   if (osid == OBJ_SPEC_NULL)
      iter_eligible_targets(&osid);

   if (osid != OBJ_SPEC_NULL && objCritters[osid].id != PLAYER_OBJ)
   {
      select_current_target(objCritters[osid].id,TRUE);
   }
   else if (oldsid != OBJ_SPEC_NULL)
   {
      select_current_target(objCritters[oldsid].id,TRUE);
   }
   else player_struct.curr_target = OBJ_NULL;

   if (player_struct.curr_target != old)
   {
      if (player_struct.curr_target == OBJ_NULL)
         mfd_notify_func(MFD_TARGET_FUNC, MFD_TARGET_SLOT, FALSE, MFD_ACTIVE, TRUE);
      else
         mfd_notify_func(MFD_TARGET_FUNC, MFD_TARGET_SLOT, FALSE, MFD_FLASH, TRUE);
   }
   if (player_struct.curr_target == OBJ_NULL)
      string_message_info(REF_STR_NoTargetsAround);
   return;
}


// ---------------------------------------------------------------------------
// mfd_target_handler()
//
// Iterates through all possible targets

bool mfd_target_handler(MFD *m, uiEvent *e)
{
   LGPoint pos;
   uiMouseEvent *ratbert = (uiMouseEvent*)e;

   pos.x = e->pos.x - m->rect.ul.x;
   pos.y = e->pos.y - m->rect.ul.y;

   if(player_struct.hardwarez[CPTRIP(TARG_GOG_TRIPLE)]==0)
      return FALSE;

   if (pos.y < BUTTON_Y || pos.x > TEXT_RIGHT_X)
      return FALSE;

   if (!(ratbert->action & MOUSE_LDOWN)) return TRUE;
   if (pos.x >= TEXT_RIGHT_X - PAGEBUTT_W)
      toggle_current_target();
   else if (pos.x <= PAGEBUTT_W)
      toggle_current_target_backwards();
   else
      select_closest_target();
   return TRUE;
}


// ----------------------------------------------------------
//                 THE TARGET WARE ITEM MFD
// ----------------------------------------------------------

extern void mfd_item_micro_hires_expose(bool full, int triple);
extern void draw_mfd_item_spew(Ref id, int n);
#define STRINGS_PER_WARE (REF_STR_wareSpew1 - REF_STR_wareSpew0)

void mfd_targetware_expose(MFD* mfd, ubyte control)
{
   uchar n = CPTRIP(TARG_GOG_TRIPLE);
   uchar v = player_struct.hardwarez[n];
   bool full = control & MFD_EXPOSE_FULL;
   if (control == 0) return;
   gr_push_canvas(pmfd_canvas);
   mfd_clear_rects();
//KLC - chg for new art   mfd_item_micro_expose(full,TARG_GOG_TRIPLE);
   mfd_item_micro_hires_expose(full,TARG_GOG_TRIPLE);
   draw_mfd_item_spew(REF_STR_wareSpew0 + STRINGS_PER_WARE*n,v);
   if (full)
   {
      draw_raw_resource_bm(REF_IMG_TargetButton,(MFD_VIEW_WID-res_bm_width(REF_IMG_TargetButton))/2,BUTTON_Y);
      mfd_add_rect(0,0,MFD_VIEW_WID-1,MFD_VIEW_HGT-1);
   }
   gr_pop_canvas();
   mfd_update_rects(mfd);
}

#define BUTTON_SIZE 30

bool mfd_targetware_handler(MFD *m, uiEvent *e)
{
   LGPoint pos;
   uiMouseEvent *ratbert = (uiMouseEvent*)e;

   pos.x = e->pos.x - m->rect.ul.x;
   pos.y = e->pos.y - m->rect.ul.y;


   if (pos.y < BUTTON_Y
      || abs(2*pos.x - MFD_VIEW_WID) > BUTTON_SIZE
      || e->type != UI_EVENT_MOUSE)
      return FALSE;

   if (!(ratbert->action & MOUSE_LDOWN)) return TRUE;
   if (player_struct.curr_target == OBJ_NULL)
      select_closest_target();
   mfd_change_slot(m->id,MFD_TARGET_SLOT);

   return TRUE;
}
