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
 * $Source: r:/prj/cit/src/RCS/trigger.c $
 * $Revision: 1.161 $
 * $Author: xemu $
 * $Date: 1994/11/25 18:18:30 $
 *
 */

#define __TRIGGER_SRC

#include <stdlib.h>
#include <string.h>

#include "Prefs.h"

#include "ai.h"
#include "audiolog.h"
#include "criterr.h"
#include "cybstrng.h"
#include "damage.h"
#include "diffq.h"
#include "doorparm.h"
#include "effect.h"
#include "faketime.h"
#include "frflags.h"
#include "frprotox.h"
#include "gameloop.h"   // for VITALS_UPDATE
#include "gamerend.h"
#include "gamescr.h"
#include "gamestrn.h"
#include "invent.h"
#include "mainloop.h"   // for flag setting stuff
#include "map.h"
#include "mapflags.h"
#include "mfdext.h"
#include "musicai.h"
#include "objbit.h"
#include "objcrit.h"
#include "objgame.h"
#include "objsim.h"
#include "objstuff.h"  
#include "objuse.h"
#include "olhext.h"
#include "otrip.h"
#include "physics.h"
#include "player.h"
#include "saveload.h"
#include "schedule.h"
#include "sfxlist.h"
#include "shodan.h"
#include "tilename.h"
#include "tools.h"
#include "trigger.h"
#include "hkeyfunc.h"
#include "cyber.h"
#include "colors.h"
#include "grenades.h"
#include "bark.h"
#include "view360.h"
#include "objload.h"
#include "rendfx.h"
#include "statics.h"
#include "citres.h"


#ifdef OLD_TELEPORT_BETWEEN_LEVELS
#include <gamewrap.h>
#include <gamerend.h>
#include <render.h>
#endif



// As far as I can tell, these NEVER GET USED.  So I thought I'd move them out of the file 
// to remove a dependency problem. 

#define TRAP_NULL_CODE        0
#define TRAP_TELEPORT_CODE    1
#define TRAP_DAMAGE_CODE      2
#define TRAP_CREATE_OBJ_CODE  3
#define TRAP_QUESTBIT_CODE    4
#define TRAP_ENDGAME_CODE     5
#define TRAP_MULTI_CODE       6
#define TRAP_LIGHT_CODE       7
#define TRAP_SFX_CODE         8
#define TRAP_HEIGHT_CODE      9
#define TRAP_TERRAIN_CODE     10
#define TRAP_SCHEDULER_CODE   11
#define TRAP_ALT_SPLIT_CODE   12
#define TRAP_DESTROY_OBJ_CODE 13
#define TRAP_PLOT_CLOCK_CODE  14
#define TRAP_EMAIL_CODE       15
#define TRAP_EXPOSE_CODE      16
#define TRAP_INSTANCE_CODE    17
#define TRAP_ANIMATE_CODE     18
#define TRAP_HACK_CODE        19
#define TRAP_TEXTURE_CODE     20
#define TRAP_AI_CODE          21
#define TRAP_BARK_CODE        22
#define TRAP_MONSTER_CODE     23
#define TRAP_TRANSMOGRIFY_CODE     24

// Note that this will always give you boolean 0 or 1, as opposed to
// things like (p2 & 0x10000) that certain people once used which is
// always 0 when cast to a bool.

#define BIT_SET(val,bit) (((val)&(1<<bit))==(1<<bit))

#define REACTOR_BOOM_QB 0x14
errtype do_special_reactor_hack();

errtype do_destroy(int victim_data);

ObjID current_trap;
bool _tr_use_message;
#define trap_use_message (&_tr_use_message)

short qdata_get(short qdata);
errtype qdata_set(short qdata, short new_val);
bool comparator_check(int comparator, ObjID obj, uchar *special_code);
errtype set_trap_data(ObjID id, char num_param, int new_val);
errtype do_timed_multi_stuff(int p);

errtype trap_null_func(int p1, int p2, int p3, int p4);
errtype trap_transmogrify_func(int p1, int p2, int p3, int p4);
bool player_facing_square(LGPoint sq);
errtype trap_monster_func(int p1, int p2, int p3, int p4);
errtype do_ai_trap(ObjSpecID osid, int p1, int p3, int p4);
errtype trap_ai_func(int p1, int p2, int p3, int p4);
errtype trap_alternating_splitter_func(int p1, int p2, int p3, int p4);
errtype trap_main_light_func(int p1, int p2, int p3, int p4);
errtype trap_terrain_func(int p1, int p2, int p3, int p4);
errtype trap_height_func(int p1, int p2, int p3, int p4);
errtype real_instance_func(int p1, int p2, int p3, int p4);
errtype trap_instance_func(int p1, int p2, int p3, int p4);
void animate_callback_func(ObjID id, void *user_data);
errtype real_animate_func(ObjID id, int p2, int p3, int p4);
errtype trap_animate_func(int p1, int p2, int p3, int p4);
void hack_shodan_conquer_func(char bonus_fun);
void hack_armageddon_func(int otrip, int x0, int y0, int r);
void hack_multi_trans(int trip, int newtype);
void hack_change_comparator(int p2, int p3);
void hack_taunt_diego(int p2, int p3);
errtype trap_hack_func(int p1, int p2, int p3, int p4);
errtype trap_multi_func(int p1, int p2, int p3, int p4);
errtype trap_destroy_object_func(int p1, int p2, int p3, int p4);
errtype trap_plot_clock_func(int p1, int p2, int p3, int p4);
errtype trap_email_func(int mung, int time, int p3, int p4);
errtype trap_texture_func(int p1, int p2, int p3, int p4);
errtype trap_expose_func(int dmg, int dtype, int tsecs, int dummy);
errtype trap_bark_func(int speaker, int strnum, int color, int hud_bark);

errtype grind_trap(char type, int p1, int p2, int p3, int p4, ubyte *destroy_count_ptr, ObjID id);
errtype do_level_entry_triggers();
errtype do_shodan_triggers();
errtype do_ecology_triggers();

#define SHODOMETER_QVAR_BASE  0x10


short qdata_get(short qdata)
{
   short contents = qdata & 0xFFF;
   if (qdata & 0x1000)
   {
      if ((contents >= FIRST_SHODAN_QV) && (contents <= FIRST_SHODAN_QV + MAX_SHODOMETER_LEVEL))
      {
         short retval;
         if (QUESTVAR_GET(MISSION_DIFF_QVAR) <= 1)
            return(0);
         retval = QUESTVAR_GET(contents) * 255 / player_struct.initial_shodan_vals[contents - FIRST_SHODAN_QV];
         if (retval > 255)
            retval = 255;
         return(retval);
      }
      else
         return(QUESTVAR_GET(contents));
   }
   else if (qdata & 0x2000)
      return(QUESTBIT_GET(contents));
   else
      return(contents);
}

errtype qdata_set(short qdata, short new_val)
{
   if (qdata & 0x1000)
      QUESTVAR_SET(qdata & 0xFFF, new_val);
   else if (qdata & 0x2000)
   {
      if ((new_val > 1) || (new_val < -1))
      {
         if (qdata_get(qdata))
            new_val = 0;
         else
            new_val = 1;
      }
      if (new_val)
         QUESTBIT_ON(qdata & 0xFFF);
      else
         QUESTBIT_OFF(qdata & 0xFFF);
   }

   mfd_notify_func(MFD_PLOTWARE_FUNC,MFD_ITEM_SLOT,FALSE,MFD_ACTIVE,TRUE);

   return(OK);
}


bool comparator_check(int comparator, ObjID obj, uchar *special_code)
{
   short cval, compval;
   uchar fail_code;
   short fail_amt =0;
   bool truthval = FALSE;
   // shodo_qvar is the questvariable for the shodometer on the current levelle
   char shodo_qvar = SHODOMETER_QVAR_BASE + player_struct.level;

   *special_code = 0;
   fail_code = comparator >> 24;
   comparator = comparator & 0xFFFFFF;
   if (comparator == 0)
      return(TRUE);
   compval = comparator >> 16;
   if (comparator & 0x1000)
   {
      cval = qdata_get(0x1000 | (comparator & 0xFFF)); // QUESTVAR_GET(comparator & 0xFFF);
   }
   else 
      cval = (QUESTBIT_GET(comparator & 0xFFF) != 0);
   switch ((comparator & 0xE000) >> 13)
   {
      case 0: truthval = (cval == compval);  fail_amt = abs(cval-compval); break;
      case 1: truthval = (cval < compval);   fail_amt = cval - compval; break;
      case 2: truthval = (cval <= compval);  fail_amt = cval - compval; break;
      case 3: truthval = (cval > compval);   fail_amt = compval - cval; break;
      case 4: truthval = (cval >= compval);  fail_amt = compval - cval; break;
      case 5: truthval = (cval != compval);  fail_amt = abs(cval-compval);  break;
      case 6: truthval = (compval > (rand()%255)); break;
   }
   if (!truthval && fail_code)
   {
      if ((comparator & 0xFFF) == shodo_qvar)
         *special_code = fail_amt;
      if ((*special_code != 0) && (fail_code == SPECIAL_SHODAN_FAIL_CODE))
      {
         short shodan_amt;
         shodan_amt = min(NUM_SHODAN_MUGS-1, *special_code >> SHODAN_INTERVAL_SHIFT);
         long_bark(obj, FIRST_SHODAN_MUG + shodan_amt, SHODAN_FAILURE_STRING, 0x4c);
      }
      else
      {
         string_message_info(REF_STR_TrapZeroMessage + fail_code);
#ifdef AUDIOLOGS
         audiolog_bark_play(fail_code);
#endif
      }
   }
   
   return(truthval);
}


errtype set_trap_data(ObjID id, char num_param, int new_val)
{
   uint* pbase=NULL;
   ObjSpecID osid=objs[id].specID;

   if(num_param<1 || num_param>4) return(ERR_NOEFFECT);

   switch(objs[id].obclass) {
      case CLASS_FIXTURE:
         pbase=&(objFixtures[osid].p1);
         break;
      case CLASS_TRAP:
         pbase=&(objTraps[osid].p1);
         break;
   }

   if(pbase==NULL)
      return(ERR_NOEFFECT);

   *(pbase+(num_param-1))=new_val;

   return(OK);
}


errtype trigger_check_destroyed(ObjID id)
{
   errtype retval = OK;
   if ((objs[id].obclass == CLASS_FIXTURE) && (objs[id].subclass == FIXTURE_SUBCLASS_CYBER))
      retval = trap_activate(id, trap_use_message);
   return(retval);             
}   

errtype location_trigger_activate(ObjID id)
{
   errtype retval = ERR_NOEFFECT;

   if (objs[id].info.type == ENTRY_TRIGGER_TYPE)
      retval = trap_activate(id, trap_use_message);
   if (objs[id].info.type == FLOOR_TRIGGER_TYPE)
   {
      if (fix_from_obj_height(PLAYER_OBJ) <
       (STANDARD_SIZE + (fix_from_map_height(me_height_flr(MAP_GET_XY(PLAYER_BIN_X, PLAYER_BIN_Y))))))
    retval = trap_activate(id, trap_use_message);
   }

   return(retval);
}


errtype trap_null_func(int, int, int, int)
{
   return(OK);
}

#define MAX_BRIDGE_FRAME   32

errtype trap_transmogrify_func(int p1, int p2, int, int)
{
   extern void slam_posture_meter_state(void);
   int source,dest,t1,t2;
   
   if (!objs[p1].active)
   {
      return(ERR_NOEFFECT);
   }
   t1=p2&0xFFFF;

   // okay, so this is kind of a dopey way to do things, but it's backwards-
   // compatible with our initial transmog spec.  For FC, we should not do
   // this ... hopefully following ifdef trick will make sure we change it.

#ifdef FIRST_SHODAN_MUG
   t2=t1^(p2>>16);
#else
   hey, look at me, I don't compile.  Look at me!
   t2=(p2>>16);
#endif

   if ( ((t1>t2)?t1:t2) >= num_types(objs[p1].obclass, objs[p1].subclass))
   {
      return(ERR_NOEFFECT);
   }
   source  = ID2TRIP(p1);
   if(objs[p1].info.type == t1)
      objs[p1].info.type = t2;
   else
      objs[p1].info.type = t1;
   dest = ID2TRIP(p1);
//   if (p3 > 0)
   {
      switch (source)
      {
         case NON_BRIDGE_TRIPLE:
            if ((source == NON_BRIDGE_TRIPLE) && ((dest == FORCE_BRIJ_TRIPLE) || (dest == FORCE_BRIJ2_TRIPLE)))
            {
               remove_obj_from_animlist(p1);
               objs[p1].info.current_frame = MAX_BRIDGE_FRAME;
               objBigstuffs[objs[p1].specID].cosmetic_value = MAX_BRIDGE_FRAME;
               // counting backwards from zero
               add_obj_to_animlist(p1, FALSE, TRUE, FALSE, 16, NULL, NULL, 0);
               if (source == NON_BRIDGE_TRIPLE)
               {
                  play_digi_fx_obj(SFX_FORCE_BRIDGE,1,p1);
               }
            }
            break;
#ifdef TWO_WAY_TRANSMOGGING
         case FORCE_BRIJ_TRIPLE:
         case FORCE_BRIJ2_TRIPLE:
            if (dest == NON_BRIDGE_TRIPLE)
            {
               remove_obj_from_animlist(p1);
               objs[p1].info.current_frame = 0;
               objBigstuffs[objs[p1].specID].cosmetic_value = MAX_BRIDGE_FRAME;
               add_obj_to_animlist(p1, FALSE, FALSE, FALSE, 16, NULL, NULL, 0);
            }
#endif
      }
   }
   slam_posture_meter_state();
   obj_physics_refresh_area(OBJ_LOC_BIN_X(objs[p1].loc), OBJ_LOC_BIN_Y(objs[p1].loc),FALSE);
   return(OK);
}

// Above this search size, just go through list of objCritters, otherwise
// look at all the objects
#define SEARCH_AREA_THRESHOLD 81

#define MIN_MONSTER_DISTANCE  7
#define NOLOOK_MONSTER_DISTANCE 14

#define CONSERVATIVE_OCTANT_ARC 0x2500

// Okay, this is stupid and uses fix_atan but WTF, I wanted to
// write something really quickly for alpha.

// cast to signed-ness before taking abs
#define fixang_abs(x) abs((short)(x))

bool player_facing_square(LGPoint sq)
{
   fixang plrh, sqh, delta;
   plrh = fixang_from_phys_angle(phys_angle_from_obj(objs[PLAYER_OBJ].loc.h));
   // reflect x,y around the line x=y to sneakily convert fixang in regular-
   // guy coordinates to fixang in north-is-zero-and-clockwise coordinates
   // so, anyway, I'm swapping x and y for a reason here.
   sqh = fix_atan2(fix_make(sq.x - PLAYER_BIN_X,0),fix_make(sq.y - PLAYER_BIN_Y, 0));
   delta = sqh - plrh;
   if (fixang_abs(delta) < CONSERVATIVE_OCTANT_ARC)
      return(TRUE);
   // note that we're working in a left-handed, clockwise world
   if (view360_active_contexts[LEFT_CONTEXT] &&
       fixang_abs(delta + 0x4000) < CONSERVATIVE_OCTANT_ARC)
      return(TRUE);
   if (view360_active_contexts[RIGHT_CONTEXT] &&
       fixang_abs(delta - 0x4000) < CONSERVATIVE_OCTANT_ARC)
      return(TRUE);
   if (view360_active_contexts[MID_CONTEXT] &&
       fixang_abs(delta + 0x8000) < CONSERVATIVE_OCTANT_ARC)
      return(TRUE);
   return(FALSE);
}

// p1 = triple
// p2 = area of effect
// p3 = quantity of object to create
errtype trap_monster_func(int p1, int p2, int p3, int p4)
{
   ObjID new_id, id1, id2;
   ObjSpecID osid;
   ObjRefID oref;
   char minx,miny,sizex,sizey, quan, failures=0, num_gen = 0;
   LGPoint sq;
   extern errtype obj_load_art(bool flush_all);
   bool okay = FALSE;
   char monster_count;
   MapElem *pme;

   quan = qdata_get(p3);
   switch(player_struct.difficulty[COMBAT_DIFF_INDEX]) {
      case 0: quan=0; break;
      case 3: quan++;
   }
   while (quan > 0)
   {
      id1 = p2 & 0xFFFF;
      id2 = p2 >> 16;
      if (id2 != OBJ_NULL)
      {
         minx = min(OBJ_LOC_BIN_X(objs[id1].loc), OBJ_LOC_BIN_X(objs[id2].loc));
         miny = min(OBJ_LOC_BIN_Y(objs[id1].loc), OBJ_LOC_BIN_Y(objs[id2].loc));
         sizex = max(OBJ_LOC_BIN_X(objs[id1].loc), OBJ_LOC_BIN_X(objs[id2].loc)) - minx;
         sizey = max(OBJ_LOC_BIN_Y(objs[id1].loc), OBJ_LOC_BIN_Y(objs[id2].loc)) - miny;
      }
      else
      {
         sizex = sizey = id1 * 2;
         minx = OBJ_LOC_BIN_X(objs[current_trap].loc) - sizex;
         miny = OBJ_LOC_BIN_Y(objs[current_trap].loc) - sizey;
      }
      okay = FALSE;
      monster_count = 0;
      while (!okay && (monster_count < 100))
      {
         int tiletype, room;

         okay = TRUE;
         sq.x = minx + rand()%(sizex+1);
         sq.y = miny + rand()%(sizey+1);
         pme = MAP_GET_XY(sq.x,sq.y);

         tiletype=me_tiletype(pme);
         room=MAP_HEIGHTS-me_height_ceil(pme)-me_height_flr(pme);
         
         if(tiletype>=TILE_SLOPEUP_N && tiletype<=TILE_SLOPECV_SW) {
            if(me_bits_mirror(pme)==MAP_FFLAT)
               room-=me_param(pme);
            else
               room=0;
         }
         else if(tiletype!=TILE_OPEN)
            room=0;
         room=(room<<MAP_ZSHF)/8;
         if(room>=8) {
            oref=me_objref(pme);
            while (okay && (oref != OBJ_REF_NULL))
            {
               if (objs[objRefs[oref].obj].obclass == CLASS_CRITTER)
               {
                  okay = FALSE;
               }
               oref = objRefs[oref].next;
            }
         }
         else {
            okay = FALSE;
            break;
         }
         if(p4 & 0x8) okay=TRUE;
         if (okay)
         {
            char d;
            d = abs(PLAYER_BIN_X - sq.x) + abs(PLAYER_BIN_Y - sq.y);

            // This should be in order of harshness
            if ((p4 & 0x2) && player_facing_square(sq) && (d < NOLOOK_MONSTER_DISTANCE))
            {
               okay = FALSE;
            }
            if ((p4 & 0x1) && (d < MIN_MONSTER_DISTANCE))
            {
               okay = FALSE;
            }
            if (!(p4 & 0x4) && (me_bits_music(pme) == ELEVATOR_ZONE))
            {
               okay = FALSE;
            }
         }
         monster_count++;
      }
      if (okay)
      {
         new_id = object_place(p1,sq);
         if (new_id == OBJ_NULL)
         {
            quan = 0;
         }
         else
         {
            num_gen++;
            // Set some default instance data
            osid = objs[new_id].specID;
            switch(objs[new_id].obclass)
            {
               case CLASS_CRITTER:
                  objCritters[osid].mood = AI_MOOD_NEUTRAL;
                  objCritters[osid].orders = AI_ORDERS_ROAM;
                  break;
            }
            quan--;
         }
      }
      else {
         failures++;
         if(failures>quan)
            quan = 0;
      }
   }
   if (num_gen > 0)
      obj_load_art(FALSE);
   return(OK);
}

errtype do_ai_trap(ObjSpecID osid, int p1, int p3, int p4)
{
   ObjID oid;
   oid = objCritters[osid].id;
   if ((p1 < 0) ||
      ((objs[oid].subclass == (p1 & 0xFF00) >> 8) &&
       (objs[oid].info.type == (p1 & 0xFF))))
   {
      // Now look at the solo bit
      if ((p1 == -1) ||  // we are already OK
          (p1 & 0x20000) || // or we don't care
          ((p1 & 0x10000) && (objs[oid].info.inst_flags & CLASS_INST_FLAG)) || // or if we want only loners, and we're a loner
          ((!(p1 & 0x10000)) && (!(objs[oid].info.inst_flags & CLASS_INST_FLAG)))) // or we want only non-loners, and we're a non-loner
      {
         if ((p3 < 0x1000) && ((QUESTVAR_GET(COMBAT_DIFF_QVAR) > 0) || (qdata_get(p3) == AI_MOOD_FRIENDLY)))
            objCritters[osid].mood = qdata_get(p3);
         if (p4 < 0x1000)
            objCritters[osid].orders = qdata_get(p4);
      }
   }
   return(OK);
}

errtype trap_ai_func(int p1, int p2, int p3, int p4)
{
   int x1,x2,y1,y2,i,j;
   ObjSpecID osid;
   ObjRefID oref;
   ObjID oid,o1,o2;
                             
   if (p1 & 0x40000)
   {
      if ((objs[p1 & 0xFFFF].active) && (objs[p1 & 0xFFFF].obclass == CLASS_CRITTER))
         do_ai_trap(objs[p1 & 0xFFFF].specID,-1,p3,p4);
      return(OK);
   }
   if ((p2 & 0xFFFF) == 0)
   {
      // Radius or total AOE
      if ((p2 >> 16) == 0)
      {
         x1 = 0;
         y1 = 0;
         x2 = global_fullmap->x_size;
         y2 = global_fullmap->y_size;
      }
      else
      {
         x1 = OBJ_LOC_BIN_X(objs[current_trap].loc) - (p2 >> 16);
         y1 = OBJ_LOC_BIN_Y(objs[current_trap].loc) - (p2 >> 16);
         x2 = x1 + (2 * (p2 >> 16));
         y2 = y1 + (2 * (p2 >> 16));
      }
   }
   else
   {
      // Object-demarked rectangle AOE
      o1 = p2 & 0xFFFF;
      o2 = p2 >> 16;
      if ((o1 == OBJ_NULL) || (o2 == OBJ_NULL))
      {
         return(ERR_NOEFFECT);
      }
      else
      {
         x1 = min(OBJ_LOC_BIN_X(objs[o1].loc), OBJ_LOC_BIN_X(objs[o2].loc));
         x2 = max(OBJ_LOC_BIN_X(objs[o1].loc), OBJ_LOC_BIN_X(objs[o2].loc));
         y1 = min(OBJ_LOC_BIN_Y(objs[o1].loc), OBJ_LOC_BIN_Y(objs[o2].loc));
         y2 = max(OBJ_LOC_BIN_Y(objs[o1].loc), OBJ_LOC_BIN_Y(objs[o2].loc));
      }
   }

   // If the area to search is greater than a certain threshold, use
   // one search method, otherwise use another.
   if ((x2 - x1) * (y2 - y1) > SEARCH_AREA_THRESHOLD)
   {
      osid = objCritters[0].id;
      while (osid != OBJ_SPEC_NULL)
      {
         oid = objCritters[osid].id;
         if ((OBJ_LOC_BIN_X(objs[oid].loc) >= x1) &&
             (OBJ_LOC_BIN_X(objs[oid].loc) <= x2) &&
             (OBJ_LOC_BIN_Y(objs[oid].loc) >= y1) &&
             (OBJ_LOC_BIN_Y(objs[oid].loc) <= y2))
         {
            do_ai_trap(osid, p1, p3, p4);
         }
         osid = objCritters[osid].next;
      }
   }
   else
   {
      for (j=y1; j<=y2; j++)
      {
         for (i=x1; i<=x2; i++)
         {
            oref = me_objref(MAP_GET_XY(i,j));
            while (oref != OBJ_REF_NULL)
            {
               // Make sure we only do this to the "true" ref
               // on each obj
               oid = objRefs[oref].obj;
               if ((objs[oid].ref == oref) && (objs[oid].obclass == CLASS_CRITTER))
               {
                  if (p1 & 0x80000)
                     do_ai_trap(objs[oid].specID, -2, p3, p4);
                  else
                     do_ai_trap(objs[oid].specID, p1, p3, p4);
               }
               oref = objRefs[oref].next;
            }
         }
      }
   }
   return(OK);
}

#define TRAP_TIME_UNIT 10 // how many time-setting units in a second

errtype trap_scheduler_func(int p1, int p2, int p3, int p4)
{
   TrapSchedEvent new_event;
   uint *p;

   if ((p3 >= 0xFFFF) || 
       ((p3 > 0x1000) && (QUESTBIT_GET(p3 & 0xFFF))) ||
       ((p3 < 0x1000) && (p3 > 0)))
   {
      switch (objs[current_trap].obclass)
      {
         case CLASS_TRAP: p = &(objTraps[objs[current_trap].specID].p3);
         case CLASS_FIXTURE: p = &(objFixtures[objs[current_trap].specID].p3);
      }
      if ((p3 < 0x1000) && (p3 > 0))
         (*p)--;
      new_event.timestamp = TICKS2TSTAMP(player_struct.game_time + (CIT_CYCLE * (ushort)qdata_get(p2))/TRAP_TIME_UNIT) + 1;
      if (qdata_get(p4) != 0)
         new_event.timestamp += rand()%qdata_get(p4);
      new_event.type = TRAP_SCHED_EVENT;
      new_event.target_id = qdata_get(p1);
      new_event.source_id = current_trap;
      return(schedule_event(&(global_fullmap->sched[MAP_SCHEDULE_GAMETIME]), (SchedEvent *)&new_event));
   }
   return(OK);
}

errtype trap_alternating_splitter_func(int p1, int p2, int p3, int p4)
{
   bool found_new = FALSE;
   char loop_count = 0;
   int n = p4;
   ObjID tr = current_trap;

   while (!found_new)
   {
      switch(n)
      {
    case 0:  do_timed_multi_stuff(qdata_get(p1)); found_new = TRUE; break;
    case 1:  do_timed_multi_stuff(qdata_get(p2)); found_new = TRUE; break;
    case 2:  do_timed_multi_stuff(qdata_get(p3)); found_new = TRUE; break;
      }
      n += 1;
      set_trap_data(tr, 4, n);
      if ((n > 2) || ((n == 2) && (p3 == 0)))
      {
    set_trap_data(tr,4,0);
      }
      loop_count++;
      if (loop_count > 10)
      {
    found_new = TRUE;
      }
   }
   return(OK);
}

errtype trap_lighting_func(bool floor, int p1, int p2, int p3, int p4);

// NUM_LIGHT_STEPS * LIGHT_TICKS should equal the total time of a transition, in this case .5 seconds
#define NUM_LIGHT_STEPS    8
#define LIGHT_TICKS        (CIT_CYCLE >> 4)

errtype trap_main_light_func(int p1, int p2, int p3, int p4)
{
   uint *p;
   if ((p3 & 0x10000) || (p3 & 0x20000))
      trap_lighting_func(FALSE,p1,p2,p3&0xffff,p4);
   if (!(p3 & 0x10000))
      trap_lighting_func(TRUE, p1,p2,p3&0xffff,p4);

   // Do transition rescheduling & incrementing
   if (p2 & 0xFFFF)
   {
      TrapSchedEvent new_event;
      int num_steps = (p2 & 0xFFF0000) >> 16;
      switch (objs[current_trap].obclass)
      {
         case CLASS_TRAP:  p = &(objTraps[objs[current_trap].specID].p2); break;
         case CLASS_FIXTURE:  p = &(objFixtures[objs[current_trap].specID].p2); break;
      }
      *p &= 0xF000FFFF;
      if (num_steps < NUM_LIGHT_STEPS)
      {
         // Now increment & re-schedule
         num_steps++;
         *p |= (num_steps << 16);
         new_event.timestamp = TICKS2TSTAMP(player_struct.game_time + LIGHT_TICKS) +1;
         new_event.type = TRAP_SCHED_EVENT;
         new_event.target_id = OBJ_NULL;
         new_event.source_id = current_trap;
         schedule_event(&(global_fullmap->sched[MAP_SCHEDULE_GAMETIME]), (SchedEvent *)&new_event);
      }
   }
   return(OK);
}

#define MAX_LIGHT_VAL   15


#define LIGHT_PLAIN_ALT 0
#define LIGHT_EW_SMOOTH 1
#define LIGHT_NS_SMOOTH 2
#define LIGHT_RADIAL    3

errtype trap_lighting_func(bool floor, int p1, int p2, int p3, int p4)
{
   int i,j;
   int targ1, targ2, setme,delta;
   int otarg1,otarg2;
   int x1,x2,y1,y2;
   fix rad_delt, rdx, rdy;
   short num_steps = -1;
   char trans_type;
   ObjID o1,o2;
   MapElem *pme;
   char v[4];
   char light_state;
   uint *p;

   o1 = qdata_get(p1 & 0xFFFF);
   o2 = qdata_get(p1 >> 16);

   // p2 will eventually be used for lighting transition types (interp or area staggered)
   // although, the high bits of it are used for maintaining state.

   trans_type = p2 & 0xFFFF;
   if (trans_type != 0)
   {
      num_steps = (p2 & 0xFFF0000) >> 16;
   }

   if (p3 == LIGHT_RADIAL)
   {
      // compute the bounding box of the radius
      x1 = OBJ_LOC_BIN_X(objs[current_trap].loc) - o1;
      x2 = x1 + (2 * o1);
      y1 = OBJ_LOC_BIN_Y(objs[current_trap].loc) - o1;
      y2 = y1 + (2 * o1);
   }
   else
   {
      if ((o1 == OBJ_NULL) || (o2 == OBJ_NULL) || (!objs[o1].active) || (!objs[o2].active))
      {
         return(ERR_NOEFFECT);
      }

      x1 = min(OBJ_LOC_BIN_X(objs[o1].loc), OBJ_LOC_BIN_X(objs[o2].loc));
      x2 = max(OBJ_LOC_BIN_X(objs[o1].loc), OBJ_LOC_BIN_X(objs[o2].loc));
      y1 = min(OBJ_LOC_BIN_Y(objs[o1].loc), OBJ_LOC_BIN_Y(objs[o2].loc));
      y2 = max(OBJ_LOC_BIN_Y(objs[o1].loc), OBJ_LOC_BIN_Y(objs[o2].loc));
   }

      v[0] = p4 & 0xFF;
      v[1] = (p4 & 0xFF00) >> 8;
      v[2] = (p4 & 0xFF0000) >> 16;
      v[3] = p4 >> 24;
      // radial light doesn't actually necessarily set any of its neighboring
      // points to its lighting values ... lighting from trap may fall from
      // an illegal value into the legal range due to distance from trap to
      // floor vertex.
      if (p3!=LIGHT_RADIAL) {
         for (i=0; i < 4; i++)
         {
            if (v[i] > MAX_LIGHT_VAL)
            {
               return(ERR_RANGE);
            }
         }
      }

      // Compare against it and figure out which set of lighting
      // values to use.  Plain lighting cares about 0 vs 1, everyone
      // else is 0 & 1 vs 2 & 3.
      light_state = p2 >> 28;
      if (p3 == LIGHT_PLAIN_ALT)
      {
         if (light_state)
         {
            targ1 = v[0];
            otarg1 = v[1];
         }
         else
         {
            targ1 = v[1];
            otarg1 = v[0];
         }
      }
      else
      {
         if (light_state)
         {
            targ1=v[0];    otarg1=v[2];
            targ2=v[1];    otarg2=v[3];
         }
         else
         {
            targ1=v[2];    otarg1=v[0];
            targ2=v[3];    otarg2=v[1];
         }
      }

      // Toggle state if done transiting (or not transiting)
      switch (objs[current_trap].obclass)
      {
         case CLASS_TRAP:  p = &(objTraps[objs[current_trap].specID].p2); break;
         case CLASS_FIXTURE:  p = &(objFixtures[objs[current_trap].specID].p2); break;
      }
      if ((num_steps == -1) || (num_steps == NUM_LIGHT_STEPS))
      {
         if (!light_state)
            *p |= 0x10000000;    // turn on state
         else
            *p &= 0xFFFFFFF;     // turn off highest bits
      }
      else // otherwise, tone down the destination appropriately.
      {
         targ1 -= ((targ1 - otarg1) * (NUM_LIGHT_STEPS - num_steps) / NUM_LIGHT_STEPS);
         if (p3 != LIGHT_PLAIN_ALT)
            targ2 -= ((targ2 - otarg2) * (NUM_LIGHT_STEPS - num_steps) / NUM_LIGHT_STEPS);
     }
 
      // Now go and crank through it...
      switch (p3)
      {
         case LIGHT_PLAIN_ALT:
            delta = 0;
            break;
         case LIGHT_EW_SMOOTH:
            delta = (targ2 - targ1) / (x2 - x1);
            break;
         case LIGHT_NS_SMOOTH:
            delta = (targ2 - targ1) / (y2 - y1);
            break;
      }
#ifdef OLD_LIGHT
      setme = targ1 - otarg1;
#endif
      setme = targ1;
      for (j = y1; j <= y2; j++)
      {
         for (i = x1; i <= x2; i++)
         {
            pme = MAP_GET_XY(i,j);
            // This code now does lighting deltas
            // Note, however, that it is still confused by light values getting "pegged"

  #define FIX_HALF (FIX_UNIT>>1)
            if (p3==LIGHT_RADIAL) {
               rdx = fix_make(i,0) - (objs[current_trap].loc.x<<8);
               rdy = fix_make(j,0) - (objs[current_trap].loc.y<<8);
               rad_delt = fix_fast_pyth_dist(rdx,rdy);
               if(rad_delt<=fix_make(o1,0))
                  setme = targ1 + fix_int((rad_delt/o1)*(targ2-targ1));
               else
                  setme=me_templight_flr(pme);
               if(setme>MAX_LIGHT_VAL) setme=MAX_LIGHT_VAL;
            }

            if (floor)
            {
#ifdef OLD_LIGHT
//               if ((setme + me_templight_flr(pme) > 0xF) || (me_templight_flr(pme) - setme < 0))
//                  Spew(DSRC_GAMESYS_Traps, ("pegged lights at 0x%x, 0x%x -- %d + %d = %d\n",
//                     i,j,setme,me_templight_flr(pme),setme+me_templight_flr(pme)));
               new_val = me_templight_flr(pme) + setme;
               if (newval > 0xF)
                  newval = 0xF;
               else if (newval < 0)
                  newval = 0;
               me_templight_flr_set(pme, newval);
#endif
               me_templight_flr_set(pme,setme);
            }
            else
            {
#ifdef OLD_LIGHT
//               if ((setme + me_templight_ceil(pme) > 0xF) || (me_templight_ceil(pme) - setme < 0))
//                  Spew(DSRC_GAMESYS_Traps, ("pegged lights at 0x%x, 0x%x -- %d + %d = %d\n",
//                     i,j,setme,me_templight_ceil(pme),setme+me_templight_ceil(pme)));
               new_val = me_templight_ceil(pme) + setme;
               if (newval > 0xF)
                  newval = 0xF;
               else if (newval < 0)
                  newval = 0;
               me_templight_ceil_set(pme, newval);
#endif
               me_templight_ceil_set(pme, setme);   
            }
            if (p3==LIGHT_EW_SMOOTH) {
               if (i == x2-1)
                  setme = targ2 ;
               else
                  setme += delta;
            }
         }

         // Now set values for next time around
         switch (p3)
         {
            case LIGHT_EW_SMOOTH:
               setme = targ1;
               break;
            case LIGHT_NS_SMOOTH:
               if (j == y2-1)
                  setme = targ2;
               else
                  setme += delta;
               break;
         }
      }

   return(OK);
}

errtype trap_damage_func(int p1, int p2, int p3, int p4)
{
   short dval;

   // Can't be inverted!
   dval = qdata_get(p1);
   if (dval > 0)
      damage_object(PLAYER_OBJ, EXPLOSION_TYPE, dval, 0);
  
   dval = qdata_get(p2 & 0xFFFF);
   if (!(p2 & 0x10000))
      damage_object(PLAYER_OBJ, dval, p2 >> 24, 0x01);
   else 
      player_struct.hit_points = min((short)player_struct.hit_points + dval, PLAYER_MAX_HP);

   dval = qdata_get(p3 & 0xFFFF);
   if (p3 < 0x10000)
      player_struct.energy -= dval;
   else
      player_struct.energy += dval;

   dval = qdata_get(p4 & 0xFFFF);
   if (p4 < 0x10000)
      player_struct.fatigue += dval;
   else
      player_struct.fatigue -= dval;

   chg_set_flg(VITALS_UPDATE);
   return(OK);
}

bool fake_endgame = FALSE;
#define ENDGAME_TICKS   CIT_CYCLE * 2

errtype trap_sfx_func(int p1, int p2, int p3, int p4)
{
   extern short fr_solidfr_time;
   extern short fr_sfx_time;
   extern short fr_surge_time;
   extern char surg_fx_frame;
   short scr_fx, sfx_time,sound_fx;
   short wacky, wacky_sev;
   extern short surge_duration;
   extern ulong player_death_time;

   sound_fx = qdata_get(p1 & 0xFFFF);
   scr_fx = qdata_get(p3);
   sfx_time = qdata_get(p4);
   wacky = qdata_get(p2 & 0xFFFF);
   wacky_sev = qdata_get(p2 >> 16);

   if (sound_fx)
      play_digi_fx(sound_fx,qdata_get(p1 >> 16));

   switch(wacky)
   {
      // Power Surge effect 
      case 1:
         if (fr_surge_time == 0)
         {
            fr_surge_time = surge_duration;
            surg_fx_frame = 0;
            play_digi_fx(SFX_SURGE, 1);
         }
         break;

      // Shake that booty (or head)
      case 2:
//         if (!sound_fx)
         play_digi_fx(SFX_RUMBLE, 2);
         fr_global_mod_flag(FR_SFX_SHAKE, FR_SFX_MASK);
         fr_sfx_time = CIT_CYCLE * 4;  // 4 seconds of shake
         break;

      // Fake endgame 
      case 3:
         {
            extern void physics_zero_all_controls();
            extern ulong secret_sfx_time;
            physics_zero_all_controls();
            secret_render_fx=FAKEWIN_REND_SFX;
            secret_sfx_time = *tmd_ticks;
            fr_surge_time = surge_duration;
            chg_set_sta(GL_CHG_2);
         }
         break;

      // Teleport special effect
      case 4:
         fr_global_mod_flag(FR_SFX_TELEPORT, FR_SFX_MASK);
         fr_sfx_time = CIT_CYCLE;  // 1 second of teleport effect
         break;

      case 5:
         {
            // let's do some damage static!
            extern void set_dmg_percentage(int which, ubyte percent);
            set_dmg_percentage(DMG_BLOOD, 100);    // 100 is the amount of static (100/255) is the percent of static
         }
         break;
   }
   switch(scr_fx)
   {
   case 1: fr_global_mod_flag(FR_SOLIDFR_SLDCLR, FR_SOLIDFR_MASK); fr_solidfr_color=GRENADE_COLOR; break;
   case 2: fr_global_mod_flag(FR_SOLIDFR_SLDCLR, FR_SOLIDFR_MASK); fr_solidfr_color=RED_BASE;      break;
   case 3: fr_global_mod_flag(FR_SOLIDFR_STATIC, FR_SOLIDFR_MASK);                                 break;
   case 4:
      {
         extern short vhold_shift;
         fr_global_mod_flag(FR_SFX_VHOLD, FR_SFX_MASK);
         vhold_shift = 0;
         break;
      }
   }
   if (scr_fx)
   {
      if (scr_fx >= 4)
          fr_sfx_time = sfx_time;
      else
          fr_solidfr_time = sfx_time;
   }
   return(OK);
}


errtype trap_create_obj_func(int p1, int p2, int p3, int p4)
{
   ObjID new_id, oid;
   ObjLoc new_loc;

   if ((p1 & 0xFFFF) == OBJ_NULL)
   {
      return(ERR_NOEFFECT);
   }

   // use questvar if asked for
   oid = qdata_get(p1 & 0xFFFF);

   if (!objs[oid].active)
   {
      return(ERR_NOEFFECT);
   }
   
   // We are okay to go, so clone the darned thing
   if ((p1 > 0xFFFF) && !(p1 & 0x10000000))
      new_id = oid;
   else
   {
      new_id = obj_create_clone(oid);

      // set the QV, if appropriate
      qdata_set(p1 >> 16, new_id);
   }

   // Now move it to where the trap says
   new_loc = objs[oid].loc;
   if (p2 < 0x4000)
   {
      p2 = qdata_get(p2);
      new_loc.x = (p2 << 8) | (new_loc.x & 0xFF);
   }
   if (p3 < 0x4000)
   {
      p3 = qdata_get(p3);
      new_loc.y = (p3 << 8) | (new_loc.y & 0xFF);
   }
   if (p4 < 0x4000)
   {
      new_loc.z = qdata_get(p4);
   }
   obj_move_to(new_id, &new_loc, TRUE);
   obj_physics_refresh_area(OBJ_LOC_BIN_X(new_loc), OBJ_LOC_BIN_Y(new_loc),TRUE);
   return(OK);
}

errtype trap_questbit_func(int p1, int p2, int p3, int p4)
{
   char message_buf[100];
   short qarg, mod;
   qarg = p2 & 0xFFFF;
   mod = p2 >> 16;

   if ((p1 & 0xF000) == 0)
      p1 |= 0x2000;
   
   if (p1 == 0x2091)					// KLC - special hack for auto shutoff of on-line help.
   {
       olh_active = FALSE;				// KLC - this is kept in a global now.
       
       gShockPrefs.goOnScreenHelp = FALSE;		// Yeah, got to update this one too and
       SavePrefs(kPrefsResID);						// save the prefs out to disk.
       return (OK);
   }
   
   if (p1 & 0x2000)
   {
      switch(qarg)
      {
         case 0: qdata_set(p1,FALSE); break;
         case 1: qdata_set(p1,TRUE); break;
         default:
            if (qdata_get(p1))
               qdata_set(p1,FALSE);
            else
               qdata_set(p1,TRUE);
            break;
      }
      if (((p1 & 0xFFF == REACTOR_BOOM_QB)) && (qdata_get(p1)))
         do_special_reactor_hack();
   }
   else
   {
      switch (mod)
      {
         case 0: qdata_set(p1, qdata_get(qarg)); break;
         case 1: qdata_set(p1, qdata_get(p1) + qdata_get(qarg)); break;
         case 2: qdata_set(p1, qdata_get(p1) - qdata_get(qarg)); break;
         case 3: qdata_set(p1, qdata_get(p1) * qdata_get(qarg)); break;
         case 4: qdata_set(p1, qdata_get(p1) / qdata_get(qarg)); break;
         case 5: qdata_set(p1, qdata_get(p1) % qdata_get(qarg)); break;
      }
   }
   if (qdata_get(p1))
   {
      if (qdata_get(p3))
      {
         message_info(get_string(REF_STR_TrapZeroMessage + qdata_get(p3), message_buf, 80));
#ifdef AUDIOLOGS
         audiolog_bark_play(qdata_get(p3));
#endif
         *trap_use_message = TRUE;
      }
   }
   else
   {
      if (qdata_get(p4))
      {
         message_info(get_string(REF_STR_TrapZeroMessage + qdata_get(p4), message_buf, 80));
#ifdef AUDIOLOGS
         audiolog_bark_play(qdata_get(p4));
#endif
         *trap_use_message = TRUE;
      }
   }

   return(OK);
}

extern bool alternate_death;

extern Boolean	gPlayingGame;
extern Boolean	gDeadPlayerQuit;
extern Boolean	gGameCompletedQuit;

errtype trap_cutscene_func(int p1, int p2, int, int)
{
	short		cs = qdata_get(p1);
//	if (qdata_get(p1) == 0)							// KLC - if we are to play the endgame cutscene
//	{
		gGameCompletedQuit = TRUE;
		gPlayingGame = FALSE;													// Hop out of the game loop.
//KLC   play_cutscene(qdata_get(p1), qdata_get(p2));
//	}
	alternate_death = (qdata_get(p2) != 0);
	return(OK);
}

errtype trap_terrain_func(int p1, int p2, int p3, int p4)
{
   MapElem *pme;
   bool reprocess = FALSE;
   extern void rendedit_process_tilemap(FullMap* map,LGRect* r,bool newMap);
   LGRect bounds;

   pme = MAP_GET_XY(qdata_get(p1),qdata_get(p2));
   if (pme == NULL)
      return(ERR_NOEFFECT);
   if (p3 < 0x4000)
   {
      me_tiletype_set(pme,qdata_get(p3));
      reprocess =TRUE;
   }
   if (p4 < 0x4000)
   {
      me_param_set(pme,qdata_get(p4));
      reprocess = TRUE;
   }
   if (reprocess)
   {
      bounds.ul.x = bounds.lr.x = qdata_get(p1);
      bounds.ul.y = bounds.lr.y = qdata_get(p2);
      rendedit_process_tilemap(global_fullmap, &bounds, FALSE);
   }
   obj_physics_refresh_area(qdata_get(p1),qdata_get(p2),TRUE);
   return(OK);
}

errtype trap_height_func(int p1, int p2, int p3, int p4)
{
   MapElem *pme;
   HeightSchedEvent hse;
   ushort use_val;
   uchar x,y;
   char steps;
   char ht;
   bool did_sfx = FALSE;
   extern bool register_h_event(uchar x, uchar y, bool floor, char* sem, char* key, bool no_sfx);

   x = qdata_get(p1);
   y = qdata_get(p2);
   pme = MAP_GET_XY(x,y);

   hse.timestamp = TICKS2TSTAMP(player_struct.game_time + (CIT_CYCLE * HEIGHT_STEP_TIME)/HEIGHT_TIME_UNIT)+1;
   use_val = qdata_get(p3 & 0xFFFF);
   if (use_val < 0x100)
   {
      if (me_height_flr(pme) != use_val)
      {
         hse.type = FLOOR_SCHED_EVENT;

         steps = use_val - me_height_flr(pme);
         hse.steps_remaining = steps;
         hse.sfx_code = 0;
         if (p4 >> 16)
            did_sfx = TRUE;
         if(register_h_event(x,y,TRUE,&hse.semaphor,&hse.key,p4 >> 16)) {
            schedule_event(&(global_fullmap->sched[MAP_SCHEDULE_GAMETIME]), (SchedEvent *)&hse);
         }
      }
   }
   use_val = qdata_get(p3 >> 16);
   if (use_val < 0x100)
   {
      // convert to make life easier on Erik
      use_val = 32 - use_val;

      if (me_height_ceil(pme) != use_val)
      {
         hse.type = CEIL_SCHED_EVENT;
         ht = me_height_ceil(pme);
         steps = use_val - ht;
         hse.steps_remaining = steps;
         hse.sfx_code = p4 >> 16;
         if(register_h_event(x,y,FALSE,&hse.semaphor,&hse.key,(did_sfx) ? FALSE : (p4 >> 16))) {
            schedule_event(&(global_fullmap->sched[MAP_SCHEDULE_GAMETIME]), (SchedEvent *)&hse);
         }
      }
   }
   obj_physics_refresh_area(x, y,TRUE);
   return(OK);
}


errtype real_instance_func(int p1, int p2, int p3, int p4)
{
   ObjSpecID osid;
   if (p1 == 0)
      return(OK);
   if (!objs[p1].active)
   {
      return(ERR_NOEFFECT);
   }
   osid = objs[p1].specID;
   switch(objs[p1].obclass)
   {
      case CLASS_BIGSTUFF:
         if (p2 != -1) objBigstuffs[osid].cosmetic_value = p2;
         if (p3 != -1) objBigstuffs[osid].data1 = p3;
         if (p4 != -1) objBigstuffs[osid].data2 = p4;
         break;
      case CLASS_DOOR:
         if(p2 != -1) objDoors[osid].locked = p2;
         if(p3 != -1) {
            objDoors[osid].stringnum = p3 >> 8;
            objDoors[osid].cosmetic_value = p3 & 0xFF;
         }
         if(p4 != -1) {
            objDoors[osid].access_level = p4 >> 8;
            objDoors[osid].autoclose_time = p4 & 0xFF;
         }
         break;
      case CLASS_FIXTURE:
      case CLASS_TRAP:
         set_trap_data(p1,p2,p3);
         break;
   }
   return(OK);
}

errtype trap_instance_func(int p1, int p2, int p3, int p4)
{
   real_instance_func(qdata_get(p1 & 0xFFFF),p2,p3,p4);
   real_instance_func(qdata_get(p1 >> 16), p2,p3,p4);
   return(OK);
}

void animate_callback_func(ObjID id, void *user_data)
{
   int p3;
   
   p3 = (int)user_data;

   if(BIT_SET(p3,17)) 
      do_multi_stuff(p3&0x7FFF);
   else
      real_animate_func(id,0,p3,0);
}

errtype real_animate_func(ObjID id, int p2, int p3, int p4)
{
   errtype retval = OK;
   int frames;
   bool reverse;

   if (id == 0)
      return(OK);
   if (!objs[id].active)
   {
      return(ERR_NOEFFECT);
   }

   // Don't allow animation updates of "destroyed" screens
   if (objs[id].info.current_hp == 0)
      return(OK);

   frames=objBigstuffs[objs[id].specID].cosmetic_value;
   if(frames==0) frames=4;

   if (p4 != 0)
      remove_obj_from_animlist(id);
   if (p2 == 0)
   {
      reverse=BIT_SET(p2,15);
      if(p3&0xF0000000) frames=p3>>28;
      real_instance_func(id,frames,-1,p3 & 0x7FFF);
      remove_obj_from_animlist(id);
      objs[id].info.current_frame = reverse?frames-1:0;
      add_obj_to_animlist(id,TRUE,BIT_SET(p3,15),BIT_SET(p3,16), 0,NULL,NULL,0);
   }
   else
   {
      reverse=BIT_SET(p2,15);
      if(p2&0xF0000000) frames=p2>>28;
      real_instance_func(id,frames,-1,p2 & 0x7FFF);
      objs[id].info.current_frame = reverse?frames-1:0;
      if (p3 != 0)
         retval = add_obj_to_animlist(id,FALSE,BIT_SET(p2,15),BIT_SET(p2,16), 0,6,(void *)p3,ANIMCB_REMOVE);
      else
         retval = add_obj_to_animlist(id,FALSE,BIT_SET(p2,15),BIT_SET(p2,16), 0,NULL,NULL,0);
   }
   return(retval);
}

errtype trap_animate_func(int p1, int p2, int p3, int p4)
{
   real_animate_func(qdata_get(p1 & 0xFFFF),p2,p3,p4);
   real_animate_func(qdata_get(p1 >> 16),p2,p3,p4);
   return(OK);
}

// Note that this blows away the usual shodan time countdown,
// so can only be used in endgame!
uchar *shodan_bitmask = NULL;
grs_bitmap shodan_draw_fs;
grs_bitmap shodan_draw_normal;

void hack_shodan_conquer_func(char)
{
   extern void begin_shodan_conquer_fx(bool begin);
   extern ulong time_until_shodan_avatar;
   extern char thresh_fail;
   extern bool shodan_phase_in(uchar *bitmask, short x, short y, short w, short h, short num, bool dir);
   shodan_bitmask = tmap_static_mem;
   LG_memset(shodan_bitmask, 0, SHODAN_BITMASK_SIZE / 8);
   shodan_draw_fs.bits = tmap_static_mem + (SHODAN_BITMASK_SIZE / 8);
   shodan_draw_normal.bits = shodan_draw_fs.bits + (320 * 200);
   load_res_bitmap(&shodan_draw_fs, SHODAN_FULLSCRN_CONQUER_REF, FALSE);
   load_res_bitmap(&shodan_draw_normal, SHODAN_CONQUER_REF, FALSE);
   thresh_fail = 0;
   time_until_shodan_avatar = player_struct.game_time + SHODAN_INTERVAL;

   begin_shodan_conquer_fx(TRUE);
}

void hack_armageddon_func(int otrip, int x0, int y0, int r)
{
   int ulx, uly, lrx, lry;
   int i,j;
   ObjRefID oref;
   ObjID oid;

   ulx=x0-r;
   uly=y0-r;
   lrx=x0+r;
   lry=y0+r;

   for (j=uly; j<=lry; j++) {
      for (i=ulx; i<=lrx; i++) {
         oref = me_objref(MAP_GET_XY(i,j));
         while (oref != OBJ_REF_NULL) {
            // Make sure we only do this to the "true" ref
            // on each obj
            oid = objRefs[oref].obj;
            if ((objs[oid].ref == oref) && (objs[oid].obclass == CLASS_CRITTER)) {
               if(ID2TRIP(oid)==otrip) 
                  ADD_DESTROYED_OBJECT(oid);
            }
            oref = objRefs[oref].next;
         }
      }
   }
}

/*���
void hack_area_spew(int p2,int p3,int p4)
{
   int ulx,uly,lrx,lry;
   int x,y;
   MapElem* pme;
   bool state;
   ObjID obj;

   ulx = min(OBJ_LOC_BIN_X(objs[current_trap].loc), OBJ_LOC_BIN_X(objs[(ObjID)p2].loc));
   lrx = max(OBJ_LOC_BIN_X(objs[current_trap].loc), OBJ_LOC_BIN_X(objs[(ObjID)p2].loc));
   uly = min(OBJ_LOC_BIN_Y(objs[current_trap].loc), OBJ_LOC_BIN_Y(objs[(ObjID)p2].loc));
   lry = max(OBJ_LOC_BIN_Y(objs[current_trap].loc), OBJ_LOC_BIN_Y(objs[(ObjID)p2].loc));

   obj=(ObjID)(p3&0xFFFF);
   if(obj==OBJ_NULL)
      state=0;
   else
      state=(objs[obj].info.current_frame==0);
   state=state^((p3>>16)!=0);

   for(x=ulx;x<=lrx;x++) {
      for(y=uly;y<=lry;y++) {
         pme = MAP_GET_XY(x,y);
         switch(p4) {
            case 0:
               me_hazard_rad_set(pme,state);
               break;
         }
      }
   }
}
*/

void hack_multi_trans(int trip, int newtype)
{
   ObjSpecID osid;
   ObjID oid;

   osid = objCritters[0].id;
   while (osid != OBJ_SPEC_NULL)
   {
      oid = objCritters[osid].id;
      if(ID2TRIP(oid)==trip) {
         if(newtype>0xF)
            do_destroy(oid);
         else
            trap_transmogrify_func(oid,newtype,0,0);
      }
      osid = objCritters[osid].next;
   }
}

void hack_change_comparator(int p2, int p3)
{
   ObjSpecID osid;

   osid=objs[p2].specID;
   switch(objs[p2].obclass) {
      case CLASS_TRAP:
         objTraps[osid].comparator=p3;
         return;
      case CLASS_FIXTURE:
         objFixtures[osid].comparator=p3;
         return;
      default:
         return;
   }
}

// Wow.  Pretty non-general here.
// Diego taunts the player.
#define FIXANG_OCT (FIXANG_PI/4)
void hack_taunt_diego(int p2, int p3)
{
   fixang plrh;

   // triggers trap p3 if and only if player's facing is within
   // an octant either way of fixang p2.
   plrh = fixang_from_phys_angle(phys_angle_from_obj(objs[PLAYER_OBJ].loc.h));
   plrh -= p2;
   if((plrh<FIXANG_OCT)||(plrh>=((fixang)-FIXANG_OCT)))
      do_multi_stuff(p3);
}

#define REPULSOR_TOGGLE_HACK 0x1
#define REPULSOR_UP 0
#define REPULSOR_DOWN 1

#define REACTOR_DIGIT_HACK 0x2
#define REACTOR_COMBO_QVAR 0x1f
#define SCREEN_DIGIT_0 (REFINDEX(REF_STR_ScreenZero+0x34))

#define REACTOR_KEYPAD_HACK 0x3

#define FIXTURE_FRAME_HACK 0x4

#define DOOR_HACK 0x5

#define GAME_OVER_HACK 0x6

#define TURN_OBJECT_HACK 0x7

#define ARMAGEDDON_HACK 0x8

#define SHODAN_CONQUER_HACK   0x9

#define COMPARATOR_HACK 0xA

#define PLOTWARE_HACK 0xB

#define AREASPEW_HACK 0xC

#define DIEGO_HACK 0xD

#define PANEL_REF_HACK 0xE

#define EARTH_DESTROYED_HACK 0xF

#define MULTI_TRANSMOG_HACK 0x10

errtype trap_hack_func(int p1, int p2, int p3, int p4)
{
   bool door_moving(ObjID door, bool dir);
   void plotware_showpage(uchar page);
   void check_panel_ref(bool puntme);
   void email_slam_hack(short which);

   // As we need hacks in the game, just add new
   // cases here to do your particular hack.
   switch(p1)
   {
      case REPULSOR_TOGGLE_HACK:
         if(ID2TRIP(p2)==REPULSOR_TRIPLE) {
            Obj* repul=&objs[p2];
            ObjRefID oref;
            ObjID oid;
            uint *upness=&(objTraps[repul->specID].p4);
            int nu_tmap, old_tmap, nu_frame;
            MapElem* pme;
            ObjLoc where=repul->loc;
            extern errtype obj_physics_refresh(short x, short y, bool use_floor);

            switch(p4) {
               case 0:
                  *upness=(*upness==REPULSOR_UP)?REPULSOR_DOWN:REPULSOR_UP;
                  break;
               case 1:
                  *upness=REPULSOR_UP;
                  break;
               case 2:
                  *upness=REPULSOR_DOWN;
                  break;
            }

            if(*upness==REPULSOR_UP) {
               nu_tmap=(p3>>8)&0x1F;
               old_tmap=p3&0x1F;
               nu_frame=0;
            }
            else {
               old_tmap=(p3>>8)&0x1F;
               nu_tmap=p3&0x1F;
               nu_frame=1;
            }

            pme=MAP_GET_XY(OBJ_LOC_BIN_X(where),OBJ_LOC_BIN_Y(where));
            if(nu_tmap!=old_tmap) {
               if(me_tmap_flr(pme)==old_tmap)
                  me_tmap_flr_set(pme,nu_tmap);
               if(me_tmap_ceil(pme)==old_tmap)
                  me_tmap_ceil_set(pme,nu_tmap);
            }
            oref=me_objref(pme);
            while(oref!=OBJ_REF_NULL) {
               oid=objRefs[oref].obj;
               if((objs[oid].ref==oref) && ID2TRIP(oid)==REPULSWALL_TRIPLE)
                  objs[oid].info.current_frame=nu_frame;
               oref=objRefs[oref].next;
            }
            obj_physics_refresh(OBJ_LOC_BIN_X(where),OBJ_LOC_BIN_Y(where),FALSE);
         }
         break;
      case REACTOR_DIGIT_HACK:
         if(objs[p2].obclass==CLASS_BIGSTUFF) {
            uint combo;
            
            combo=(QUESTVAR_GET(REACTOR_COMBO_QVAR)<<12)|
               (QUESTVAR_GET(REACTOR_COMBO_QVAR+1));

            objBigstuffs[ID2SPEC(p2)].data2=
               0x100 | (SCREEN_DIGIT_0+(0xF&(combo>>(4*(6-p3)))));
         }
         break;
      case REACTOR_KEYPAD_HACK:
         if(objs[p2].obclass==CLASS_FIXTURE) {
            uint* field = &(objFixtures[ID2SPEC(p2)].p1);

            field+=(p3-1);
            *field = (*field&~0xFFFF)|QUESTVAR_GET(p4);
         }
         break;
      case FIXTURE_FRAME_HACK:
         objs[p2].info.current_frame=p3;
         if(p4)
            mfd_notify_func(MFD_FIXTURE_FUNC,MFD_INFO_SLOT,FALSE,MFD_ACTIVE,TRUE);
         break;
      case DOOR_HACK:
         if(objs[p2].obclass==CLASS_DOOR) {

            bool closed;

            // p3 indicates operation on door:
            // 0=null; 1=open; 2=close; 3=toggle; 4=disable autoclose

            if(p3<4) {
               closed=door_moving(p2,TRUE)||DOOR_REALLY_CLOSED(p2);

               if(p3&&((p3==3)||((p3==1)==closed)))
                  do_multi_stuff(p2);
            }
            else {
               objDoors[objs[p2].specID].autoclose_time=NEVER_AUTOCLOSE_COOKIE;
               remove_obj_from_animlist(p2);
            }
         }
         break;
      case GAME_OVER_HACK:
      {
         extern void player_dead(void);
         extern int curr_alog;
         extern char secret_pending_hack;
         if (curr_alog != -1)
            secret_pending_hack = 1;
         else
         {
	     	gDeadPlayerQuit = TRUE;					// The player is dead.
			gPlayingGame = FALSE;					// Hop out of the game loop.
		}
         break;
      }
      case TURN_OBJECT_HACK:
      {
         short head,lo,hi,phb;

         phb=(p3>>24)&0xF;
         switch(phb) {
            case 1: head=objs[p2].loc.p; break;
            case 2: head=objs[p2].loc.b; break;
            default: head=objs[p2].loc.h; break;
         }
         hi=p4&0xFF; if(hi==0) hi=255;
         lo=(p4>>8)&0xFF;
         if(p3&0xFF==0) p3|=0x10;
         head+=(p3&0xFF0000)?-(p3&0xFF):(p3&0xFF);
         if((p3&0xFF00)==0) {
            head=lo+(head-lo)%(hi-lo);
         }
         else {
            if(head>hi) {
               head=hi;
               p3=p3^0x10000;
            }
            else if(head<lo) {
               head=lo;
               p3=p3^0x10000;
            }
         }
         switch(phb) {
            case 1: objs[p2].loc.p=head; break;
            case 2: objs[p2].loc.b=head; break;
            default: objs[p2].loc.h=head; break;
         }
         objTraps[objs[current_trap].specID].p3=p3;
         break;
      }
      case ARMAGEDDON_HACK:
         hack_armageddon_func(p2,OBJ_LOC_BIN_X(objs[current_trap].loc),OBJ_LOC_BIN_Y(objs[current_trap].loc),p3);
         break;
      case SHODAN_CONQUER_HACK:
         hack_shodan_conquer_func(p2);
         break;
      case COMPARATOR_HACK:
         hack_change_comparator(p2,p3);
         break;
      case PLOTWARE_HACK:
         // show page p2
        plotware_showpage(p2);
         break;
      case AREASPEW_HACK:
//���         hack_area_spew(p2,p3,p4);
         break;
      case DIEGO_HACK:
         hack_taunt_diego(p2,p3);
         break;
      case PANEL_REF_HACK:
         if(player_struct.panel_ref==p2)
            check_panel_ref(TRUE);
         break;
      case EARTH_DESTROYED_HACK:
         email_slam_hack(0x01D);
         break;
      case MULTI_TRANSMOG_HACK:
         hack_multi_trans(p2,p3);
         break;
   }
   return(OK);
}

errtype do_multi_stuff(ObjID id)
{
   ObjID other;

   if (id != OBJ_NULL)
   {
      if (objs[id].obclass == CLASS_TRAP)
        trap_activate(id, trap_use_message);
      else
      {
         switch(objs[id].obclass)
         {
            case CLASS_DOOR:
               objDoors[objs[id].specID].locked = 0;
               objDoors[objs[id].specID].access_level = 0;
               other = objDoors[objs[id].specID].other_half;
               if((other) && objs[other].obclass == CLASS_DOOR)
               {
                  // door has a door other half.  Unlock it too.
                  objDoors[objs[other].specID].locked = 0;
                  objDoors[objs[other].specID].access_level = 0;
               }
               break;
         }
         object_use(id, FALSE, OBJ_NULL);
      }
   }
   return(OK);
}


// number of multi delay units in a second
#define MULTI_TIME_UNIT 10

errtype do_timed_multi_stuff(int p)
{
   TrapSchedEvent new_event;

   if (p >> 16)
   {
      // Time delay
      new_event.timestamp = TICKS2TSTAMP(player_struct.game_time + (ushort)(CIT_CYCLE * (p >> 16)) / MULTI_TIME_UNIT) + 1;
      new_event.type = TRAP_SCHED_EVENT;
      new_event.target_id = qdata_get(p & 0xFFFF);
      new_event.source_id = -1;
      schedule_event(&(global_fullmap->sched[MAP_SCHEDULE_GAMETIME]), (SchedEvent *)&new_event);
   }
   else
   {
      // Do immediately
      do_multi_stuff(qdata_get(p & 0xFFFF));
   }
   return(OK);
}

errtype trap_multi_func(int p1, int p2, int p3, int p4)
{
   do_timed_multi_stuff(p1);
   do_timed_multi_stuff(p2);
   do_timed_multi_stuff(p3);
   do_timed_multi_stuff(p4);
   return(OK);
}

errtype do_destroy(int victim_data)
{
   ObjID victim;

   victim = qdata_get(victim_data);
         
   if (victim != OBJ_NULL)
      ADD_DESTROYED_OBJECT(victim);
   return(OK);
}

errtype trap_destroy_object_func(int p1, int p2, int p3, int p4)
{
   char message_buf[100];

   do_destroy(p1);
   do_destroy(p2);
   do_destroy(p3);
   if (p4 > 0)
   {
      message_info(get_string(REF_STR_TrapZeroMessage + p4, message_buf, 80));
#ifdef AUDIOLOGS
      audiolog_bark_play(p4);
#endif
      *trap_use_message = TRUE;
   }
   return(OK);
}

errtype trap_plot_clock_func(int, int, int, int)
{
   return(OK);
}

errtype trap_email_func(int mung, int time, int, int)
{
   void add_email_datamunge(short mung,bool select);

#ifdef DOOM_EMULATION_MODE
   if (QUESTVAR_GET(MISSION_DIFF_QVAR) == 0)
      return(OK);
#endif
   if (time == 0)
   {
      add_email_datamunge(mung,TRUE);
      *trap_use_message = TRUE;
   }
   else
   {
      EmailSchedEvent ev;
      ev.type = EMAIL_SCHED_EVENT;
      ev.timestamp = TICKS2TSTAMP((time << APPROX_CIT_CYCLE_SHFT) + player_struct.game_time);
      ev.datamunge = mung;
      schedule_event(&game_seconds_schedule,(SchedEvent*)&ev);
   }
   return(OK);
}

errtype trap_texture_func(int p1, int p2, int p3, int p4)
{
   ObjID id1,id2;
   short cx,cy,minx,maxx,miny,maxy;
   int i, src[3], dest[3];
   MapElem *pme;

   id2 = qdata_get(p1 >> 16);
   if (id2 != OBJ_NULL)
   {
      id1 = qdata_get(p1 & 0xFFFF);
      if (!objs[id1].active || !objs[id2].active)
      {
         return(OK);
      }
      minx = min(OBJ_LOC_BIN_X(objs[id1].loc), OBJ_LOC_BIN_X(objs[id2].loc));
      maxx = max(OBJ_LOC_BIN_X(objs[id1].loc), OBJ_LOC_BIN_X(objs[id2].loc));
      miny = min(OBJ_LOC_BIN_Y(objs[id1].loc), OBJ_LOC_BIN_Y(objs[id2].loc));
      maxy = max(OBJ_LOC_BIN_Y(objs[id1].loc), OBJ_LOC_BIN_Y(objs[id2].loc));
   }
   else
   {
      minx = maxx = (p1 & 0xFF00) >> 8;
      miny = maxy = (p1 & 0xFF);
   }

   src[0] = p2 >> 16;
   src[1] = p3 >> 16;
   src[2] = p4 >> 16;
   dest[0] = p2 & 0xFFFF;
   dest[1] = p3 & 0xFFFF;
   dest[2] = p4 & 0xFFFF;

   for (cx = minx; cx <= maxx; cx++)
   {
      for (cy = miny; cy <= maxy; cy++)
      {
         pme = MAP_GET_XY(cx,cy);
         for (i = 0; i < 3; i++)
         {
            if ((src[i] >= 0x1000) || (src[i] == me_tmap(pme,i)))
            {
               if (dest[i] < 0x1000)
               {
                  me_tmap_set(pme,i,dest[i]);
               }
            }
         }
      }
   }
   return(OK);
}

errtype trap_teleport_func(int targ_x, int targ_y, int targ_z, int targlevel)
{
   ObjLoc newloc;
   errtype errcode = OK;
   bool to_cyber = FALSE;
   
   if (targlevel >= 0x1000)
      targlevel = player_struct.level;

   // If going between cyber and real, static out the screen during the load
   if (targlevel != player_struct.level)
   {
      to_cyber = go_to_different_level(targlevel);
   }
   if (errcode == OK) 
   {
      newloc = objs[PLAYER_OBJ].loc;
      if (targ_x < 0x4000)
      {
         targ_x = qdata_get(targ_x);
         newloc.x = (targ_x << 8) + 0x80;
      }
      if (targ_y < 0x4000)
      {
         targ_y = qdata_get(targ_y);
          newloc.y = (targ_y << 8) + 0x80;
      }
      if (targ_z < 0x4000)
      {
         targ_z = qdata_get(targ_z);
          newloc.z = targ_z;
      }
      obj_move_to(PLAYER_OBJ, &newloc, TRUE);
      if (to_cyber)
      {
         recall_objloc = newloc;
//KLC         if (music_on)
//KLC            start_music();
      }
   }
   else
   {
      return(ERR_FOPEN);
   }
   return(OK);
}


errtype trap_expose_func(int dmg, int dtype, int tsecs, int)
{
   extern void expose_player(byte damage, ubyte type, ushort tsecs);
   short damage = qdata_get(dmg & 0xFFFF);

   if (dmg & 0x10000)
   {
      damage &= 0xFFFF;
      if (dtype == RADIATION_TYPE)
      {
         if (damage <= player_struct.rad_post_expose)
         {
            player_struct.rad_post_expose -= damage;
            return OK;
         }
         damage -= player_struct.rad_post_expose;
         player_struct.rad_post_expose = 0;
      }
      if (dtype == BIO_TYPE)
      {
         if (damage <= player_struct.bio_post_expose)
         {
            player_struct.bio_post_expose -= damage;
            return OK;
         }
         damage -= player_struct.bio_post_expose;
         player_struct.bio_post_expose = 0;
      }
      damage = max(- damage,-128);
   }
   expose_player((byte)damage,(ubyte)qdata_get(dtype),(ushort)qdata_get(tsecs));
   return OK;
}

errtype trap_bark_func(int speaker, int strnum, int color, int hud_bark)
{
   int string_id = REF_STR_TrapZeroMessage+qdata_get(strnum);
   BarkSchedEvent new_event;
   ushort special;
   uint timeout=0, len;

   special=(speaker<0)?-speaker:0;

   if (hud_bark)
   {
      // just message_info for now
      string_message_info(string_id);
#ifdef AUDIOLOGS
      audiolog_bark_play(string_id - REF_STR_TrapZeroMessage);
#endif
   }
   else if(special) {
      int mug;

      mug=(-special==SHODAN_BARK_CODE)?SHODAN_MUG:DIEGO_MUG;
      long_bark(PLAYER_OBJ,mug,string_id,(ubyte)color);
      timeout=SHODAN_BARK_TIMEOUT;
   }
   else if((ObjID)speaker==OBJ_NULL) {
      long_bark(PLAYER_OBJ,0,string_id,(ubyte)color);
      timeout=NULL_BARK_TIMEOUT;
   }
   else {
//      long_bark(speaker,0,string_id,(ubyte)color);
      long_bark(PLAYER_OBJ,0,string_id,(ubyte)color);
      timeout=NULL_BARK_TIMEOUT;
   }
   if(timeout>0) {
      // convert to sixteenths of a second
      timeout=(timeout*16)/10;

      len=strlen(get_temp_string(string_id));
      timeout *= 10+len;
      new_event.timestamp = TICKS2TSTAMP(player_struct.game_time) + timeout;
      new_event.type = BARK_SCHED_EVENT;
      schedule_event(&game_seconds_schedule,(SchedEvent*) &new_event);
   }
   return OK;
}


errtype (*trap_functions[])(int, int, int, int) = {
   trap_null_func, trap_teleport_func, trap_damage_func, trap_create_obj_func,
   trap_questbit_func, trap_cutscene_func, trap_multi_func, trap_main_light_func,
   trap_sfx_func, trap_height_func, trap_terrain_func, trap_scheduler_func,
   trap_alternating_splitter_func, trap_destroy_object_func, trap_plot_clock_func,
   trap_email_func,trap_expose_func, trap_instance_func, trap_animate_func,
   trap_hack_func, trap_texture_func, trap_ai_func, trap_bark_func, trap_monster_func,
   trap_transmogrify_func } ;

ubyte num_trap_types = (sizeof(trap_functions)/sizeof(trap_functions[0]));

errtype grind_trap(char type, int p1, int p2, int p3, int p4, ubyte *destroy_count_ptr, ObjID id)
{
   trap_functions[type](p1,p2,p3,p4);
   if (*destroy_count_ptr > 0)
   {
      (*destroy_count_ptr) = (*destroy_count_ptr) - 1;
      if (*destroy_count_ptr == 0)
         ADD_DESTROYED_OBJECT(id);
   }
   return(OK);
}


bool trap_activate(ObjID id, bool *use_message)
{
   bool retval = FALSE;
   ubyte traptype;
   int comparator;
   int p1,p2,p3,p4;
   ubyte *destroy_count_ptr;
   uchar special;

   *trap_use_message = *use_message;
   current_trap = id;

   switch(objs[id].obclass)
   {
   case CLASS_TRAP:
      traptype = objTraps[objs[id].specID].trap_type;
      destroy_count_ptr = &(objTraps[objs[id].specID].destroy_count);
      if (objs[id].subclass == TRAP_SUBCLASS_TRIGGER)
      {
         // Triggers that overwrite their comparator have their comparator
         // set to zero to avoid problems in interpretation.
         switch (objs[id].info.type)
         {
         case DEATHWATCH_TRIGGER_TYPE:
         case AREA_ENTRY_TRIGGER_TYPE:
         case AREA_CONTINUOUS_TRIGGER_TYPE:
            comparator = 0;
         break;
         default:
           comparator = objTraps[objs[id].specID].comparator;
           break;
         }
      }
      p1 = objTraps[objs[id].specID].p1;
      p2 = objTraps[objs[id].specID].p2;
      p3 = objTraps[objs[id].specID].p3;
      p4 = objTraps[objs[id].specID].p4;
      break;                         
   case CLASS_FIXTURE:
      traptype = objFixtures[objs[id].specID].trap_type;
      destroy_count_ptr = &(objFixtures[objs[id].specID].destroy_count);
      if ((objs[id].subclass == FIXTURE_SUBCLASS_RECEPTACLE) || (objs[id].subclass == FIXTURE_SUBCLASS_VENDING))
         comparator = 0;
      else
         comparator = objFixtures[objs[id].specID].comparator;
      p1 = objFixtures[objs[id].specID].p1;
      p2 = objFixtures[objs[id].specID].p2;
      p3 = objFixtures[objs[id].specID].p3;
      p4 = objFixtures[objs[id].specID].p4;
      break;
   default:
      retval = FALSE;
      goto out;
   }

   if (comparator_check(comparator, id, &special))
   {
      grind_trap(traptype,p1,p2,p3,p4, destroy_count_ptr,id);
      retval = TRUE;
   }
out:
   *use_message = *trap_use_message;
   return(retval);
}


// Look through all deathwatch triggers on the level
// if lots of goofy things like explosions are being destroyed here, we may want to
// use some sort of heuristic to speed things up (don't search if destroyed thing
// was an anim, etc.)
// also, we might want to be able to override this behavior if destroying objects
// via the editor is causing unwanted trap problems....

errtype check_deathwatch_triggers(ObjID id, bool really_dead)
{
   ObjSpecID osid, nextid;
   int comp;
   bool dummy;

   if (id == NULL)
      return(ERR_NOEFFECT);

   osid = objTraps[0].id;

   while (osid != OBJ_SPEC_NULL)
   {
      nextid=objTraps[osid].next;
      if (objs[objTraps[osid].id].info.type == DEATHWATCH_TRIGGER_TYPE)
      {
         comp = objTraps[osid].comparator;

      // Does this particular trigger care about us?
         if(really_dead==!(comp&0x2000000)) {
            if (((comp & 0x1000000) && ((comp & 0xFFFF) == id)) || (ID2TRIP(id) == (comp & 0xFFFFFF))) {
               trap_activate(objTraps[osid].id, &dummy);
            }
         }
      }
      osid = nextid;
   }
   return(OK);
}


#define SQ(x) (x * x)
#define in_bin_radius(x1,y1,x2,y2,r) (SQ(r) > SQ((x1 - x2)) + SQ((y1 - y2)))

errtype check_entrance_triggers(uchar old_x, uchar old_y, uchar new_x, uchar new_y)
{
   ObjID id;
   ObjSpecID osid;
   uchar trap_x, trap_y;
   uchar rad;
   bool invert, in_rad_before, in_rad_now;
  
   osid = objTraps[0].id;
   while (osid != OBJ_SPEC_NULL)
   {
      id = objTraps[osid].id;

      switch(objs[id].subclass)
      {
    case TRAP_SUBCLASS_TRIGGER:
       trap_x = OBJ_LOC_BIN_X(objs[id].loc);
       trap_y = OBJ_LOC_BIN_Y(objs[id].loc);
       switch(objs[id].info.type)
       {
          // Yer basic entry & floor trigger -- this will be used for tripbeams in the near future too
          case FLOOR_TRIGGER_TYPE:
          case ENTRY_TRIGGER_TYPE:
        if ((trap_x == new_x) && (trap_y == new_y))
           location_trigger_activate(id);
        break;
          case AREA_ENTRY_TRIGGER_TYPE:
          case AREA_CONTINUOUS_TRIGGER_TYPE:
        rad = abs(objTraps[osid].comparator);
        invert = (objTraps[osid].comparator >= 0x1000);
        in_rad_now = in_bin_radius(trap_x, trap_y, new_x, new_y, rad);
        if (objs[id].info.type == AREA_CONTINUOUS_TRIGGER_TYPE)
        {
           if ((in_rad_now && !invert) || (!in_rad_now && invert))
            trap_activate(id, trap_use_message);
        }
        else
        {
           in_rad_before = in_bin_radius(trap_x, trap_y, old_x, old_y, rad);
           if (in_rad_before != in_rad_now)
           {
         if ((in_rad_now && !invert) || (!in_rad_now && invert))
            trap_activate(id, trap_use_message);
           }
        }
        break;
       }
       break;
      }
      osid = objTraps[osid].next;
   }
   return(OK);   
}


#define REACTOR_BOOM_QB 0x14

errtype do_special_reactor_hack()
{
   ObjSpecID osid;

   if (!qdata_get(REACTOR_BOOM_QB | 0x2000))
      return(OK);

   // Secret reactor alarm hack
   osid = objAnimatings[0].id;
   while (osid != OBJ_SPEC_NULL)
   {
      if ((ID2TRIP(objAnimatings[osid].id) == ALERT_PANEL_OFF_TRIPLE) ||
          (ID2TRIP(objAnimatings[osid].id) == HORZ_KLAXOFF_TRIPLE))
      {
         objs[objAnimatings[osid].id].info.type++;
      }
      osid = objAnimatings[osid].next;
   }
   obj_load_art(FALSE);
   return(OK);
}

errtype do_level_entry_triggers()
{
   ObjSpecID osid;
   uchar special;

   osid = objTraps[0].id;
   while (osid != OBJ_SPEC_NULL)
   {
      if (ID2TRIP(objTraps[osid].id) == LEVEL_TRIG_TRIPLE)
      {
         if (comparator_check(objTraps[osid].comparator, objTraps[osid].id, &special))
         {
            grind_trap(objTraps[osid].trap_type,objTraps[osid].p1,objTraps[osid].p2,objTraps[osid].p3,objTraps[osid].p4,
               &objTraps[osid].destroy_count,objTraps[osid].id);
         }
      }
      osid = objTraps[osid].next;
   }

   do_special_reactor_hack();

   return(OK);
}

errtype do_shodan_triggers()
{
   ObjSpecID osid;
   uchar special;

   osid = objTraps[0].id;
   while (osid != OBJ_SPEC_NULL)
   {
      if (ID2TRIP(objTraps[osid].id) == SHODO_TRIG_TRIPLE)
      {
         if (comparator_check(objTraps[osid].comparator, objTraps[osid].id, &special))
         {
            grind_trap(objTraps[osid].trap_type,objTraps[osid].p1,objTraps[osid].p2,objTraps[osid].p3,objTraps[osid].p4,
               &objTraps[osid].destroy_count,objTraps[osid].id);
         }
      }
      osid = objTraps[osid].next;
   }
   return(OK);
}

errtype do_ecology_triggers()
{
   ObjSpecID osid,osid2;
   ObjClass cl;
   char counter = 0,quan;
   int trip;
   ObjSpec ospec;
   ObjID id;
   extern bool trigger_check;

   osid = objTraps[0].id;
   while (osid != OBJ_SPEC_NULL)
   {
      id = objTraps[osid].id;
      if (ID2TRIP(id) == ECOLOGY_TRIG_TRIPLE)
      {
         quan = objTraps[osid].comparator >> 24;
         cl = (ObjClass)((objTraps[osid].comparator & 0xFF0000) >> 16);
         trip = objTraps[osid].comparator & 0xFFFFFF;
         osid2 = (*(ObjSpec *)objSpecHeaders[cl].data).bits.id;
         counter = 0;
         while (osid2 != OBJ_SPEC_NULL)
         {
            ospec = *(ObjSpec *)(objSpecHeaders[cl].data + (osid2 * objSpecHeaders[cl].struct_size));
            if (ID2TRIP(ospec.bits.id) == trip)
               counter++;
            if (counter > quan)
               osid2 = OBJ_SPEC_NULL;
            else
               osid2 = ospec.next;
         }
         if (counter < quan)
         {
            trigger_check = FALSE;
            grind_trap(objTraps[osid].trap_type,objTraps[osid].p1,objTraps[osid].p2,objTraps[osid].p3,objTraps[osid].p4,
               &objTraps[osid].destroy_count,objTraps[osid].id);
            trigger_check = TRUE;
         }
      }
      osid = objTraps[osid].next;
   }
   return(OK);
}
