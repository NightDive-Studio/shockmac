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
 * $Source: r:/prj/cit/src/RCS/objuse.c $
 * $Revision: 1.194 $
 * $Author: dc $
 * $Date: 1994/11/28 06:38:32 $
 */

#include <stdio.h>
#include <string.h>

#include "amap.h"
#include "audiolog.h"
#include "criterr.h"
#include "cyber.h"
#include "cybstrng.h"
#include "damage.h"
#include "diffq.h"
#include "doorparm.h"
#include "effect.h"
#include "faketime.h"
#include "fullscrn.h"
#include "gamestrn.h"
#include "grenades.h"
#include "gr2ss.h"
#include "hud.h"
#include "ice.h"
#include "input.h"
#include "invent.h"
#include "mainloop.h"
#include "mapflags.h"
#include "mfdint.h"
#include "mfdpanel.h"
#include "musicai.h"
#include "newmfd.h"
#include "objbit.h"
#include "objprop.h"
#include "objsim.h"
#include "objstuff.h"
#include "objuse.h"
#include "otrip.h"
#include "player.h"
#include "physics.h"
#include "render.h"
#include "saveload.h"
#include "schedule.h"
#include "sfxlist.h"
#include "target.h"
#include "tilename.h"
#include "tools.h"
#include "trigger.h"

#define MFD_FIXTURE_FLAG 0x8  // class flag for mfd fixtures

extern void mfd_setup_elevator(ushort levmask, ushort reachmask, ushort curlevel, uchar special);
extern void mfd_setup_keypad(char special);
extern void save_mfd_slot(int mfd_id);
extern void restore_mfd_slot(int mfd_id);
extern short qdata_get(short qdata);
errtype accesspanel_trigger(ObjID id);

// -----------
//  PROTOTYPES
// -----------
void zoom_mfd(int mfd);
int grab_and_zoom_mfd(int mfd_func, int mfd_slot);
errtype obj_access_fail_message(int stringref, char access_level, char offset);
bool really_really_locked(int qvar);
bool use_door(ObjID id, uchar in_inv, ObjID cursor_obj);
bool obj_too_smart(ObjID id);
void container_check(ObjID obj, char* count, ObjID* pidlist);
char container_extract(ObjID *pidlist, int d1, int d2);
void container_stuff(ObjID *pidlist, int numobjs, int* d1, int* d2);
bool is_container(ObjID id, int** d1, int** d2);
bool obj_fixture_zoom(ObjID id, bool in_inv, bool *messagep);
errtype obj_tractor_beam_func(ObjID id, bool on);
errtype gear_power_outage();
void unmulti_anim_callback(ObjID id, void *user_data);
errtype obj_screen_animate(ObjID id);
bool obj_keypad_crunch(int p, uchar digits[]);
errtype keypad_trigger(ObjID id, uchar digits[]);
bool try_use_epick(ObjID panel, ObjID cursor_obj);
ObjID door_in_square(ObjLoc* loc, bool usable);
void regenetron_door_hack();
errtype elevator_janitor_run();


// not in any way the right way to fix this bug, except
// that it is the fastest way.  Doug says go.  Ask TJS.
bool shameful_obselete_flag;

#define DOOR_TIME_UNIT 2 // how many time-setting units in a second


void zoom_mfd(int mfd)
{
   LGRect start = {{ -5, -5}, {5,5}};
   extern LGPoint use_cursor_pos;
   LGPoint ucp;

   extern void mfd_zoom_rect(LGRect*,int);
   extern Boolean DoubleSize;

   ucp = use_cursor_pos;
   if (!DoubleSize)
	   ss_point_convert(&ucp.x,&ucp.y,TRUE);
   RECT_MOVE(&start,ucp);
   mfd_zoom_rect(&start,mfd);
}

int grab_and_zoom_mfd(int mfd_func, int mfd_slot)
{
   int mfd = mfd_grab_func(mfd_func,mfd_slot);
   zoom_mfd(mfd);
   return mfd;
}


errtype obj_access_fail_message(int stringref, char access_level, char offset)
{
   char temp[40];
   strcpy(temp, get_temp_string(MKREF(RES_accessCards,access_level<<1)));
   strcat(temp, get_string(stringref + offset,NULL,0));
   message_info(temp);
   return(OK);
}

// dir true for door_closing
bool door_moving(ObjID door, bool dir)
{
   bool anim_data_from_id(ObjID id, bool* reverse, bool* cycle);

   bool moving, closing;

   moving=anim_data_from_id(door, &closing, NULL);
   return(moving && (!closing==!dir));
}

#define COMPAR_ESC_ACCESS 0xFF
extern bool comparator_check(int comparator, ObjID obj, uchar *special_code);

bool door_locked(ObjID obj)
{
   ObjSpecID spec = objs[obj].specID;
   if (!DOOR_CLOSED(obj))
      return FALSE;
   if (objDoors[spec].access_level != COMPAR_ESC_ACCESS &&
      objDoors[spec].access_level != 0)
   {
      int i;
      ulong try_combo = 1 << objDoors[spec].access_level;
      for (i=0; i < NUM_GENERAL_SLOTS; i++)
      {
         ObjID try_card = player_struct.inventory[i];
         if (ID2TRIP(try_card) == GENCARDS_TRIPLE)
         {
            if (objSmallstuffs[objs[try_card].specID].data1 & try_combo)
            {
               return FALSE;
            }
            else return TRUE;
         }
      }
      return TRUE;
   }
   if (objDoors[spec].locked == 0) return FALSE;
   if(objDoors[spec].access_level==COMPAR_ESC_ACCESS)
   {
      uchar special;
      int comp = objTraps[objs[objDoors[spec].locked].specID].comparator;
      // zorch the failure message out of the comparator before
      // checking.
      comp = comp&(~(0xFF<<24));
      return !comparator_check(comp,obj,&special);
   }

   return (QUESTBIT_GET(objDoors[spec].locked));
}

#ifdef DOOM_EMULATION_MODE
// questbits for doors that are "broken beyond repair" or such,
// and should not be openable even in doom emulation mode.
bool really_really_locked(int qvar)
{
   return(qvar==0x5E||qvar==0xE7);
}
#endif

// Okay, secretly we use our in_inv parameter not to really
// indicate that the door is in anybody's inventory, but as
// follows:
//   the 0x1 bit indicates not to use the door's other_half
//   the 0x2 bit indicates not to spew any relevant messages,
//      and should be used by anyone other than the player trying
//      to use the door.
//
bool use_door(ObjID id, uchar in_inv, ObjID)
{
   bool retval = FALSE;
   bool play_fx = FALSE;
   bool use_card = FALSE;
   int lqb;
   DoorSchedEvent new_event;
   ObjID try_card, other;
   char i;

   if ((objDoors[objs[id].specID].access_level != 0)
#ifdef DOOM_EMULATION_MODE
       && (QUESTVAR_GET(MISSION_DIFF_QVAR) >= 2))
#else
         )
#endif
   {
      int try_combo;
      bool rv;
      uchar special;
      int comp;

      if (objDoors[objs[id].specID].access_level > NUM_ACCESS_CODES) 
      {
         if(objDoors[objs[id].specID].access_level==COMPAR_ESC_ACCESS) 
         {
            comp = objTraps[objs[objDoors[objs[id].specID].locked].specID].comparator;
            rv = comparator_check(comp,id,&special);
            if(!rv) 
            {
               return((comp>>24)&&(special==0));
            }
            objDoors[objs[id].specID].access_level=0;
            objDoors[objs[id].specID].locked=0;
            goto access_ok;
         }
      }

      try_combo = 1 << objDoors[objs[id].specID].access_level;
      {
          bool some_card = FALSE;
          try_card = OBJ_NULL;
          for (i=0; i < NUM_GENERAL_SLOTS; i++)
          {
             try_card = player_struct.inventory[i];
             if (ID2TRIP(try_card) == GENCARDS_TRIPLE)
             {
                some_card = TRUE;
                if (objSmallstuffs[objs[try_card].specID].data1 & try_combo)
                {
                   use_card = TRUE;
                   break;
                }
             }
          }
          if (!use_card)
          {
            if (!(in_inv&0x2))
               obj_access_fail_message(REF_STR_DoorWrongAccess, objDoors[objs[id].specID].access_level, objDoors[objs[id].specID].stringnum);
            retval = TRUE;
            goto out;
          }
      }
   }
access_ok:
    if (DOOR_CLOSED(id)||door_moving(id,TRUE))
    {
       if (((lqb=objDoors[objs[id].specID].locked) != 0) && QUESTBIT_GET(objDoors[objs[id].specID].locked)
#ifdef DOOM_EMULATION_MODE
            && ((QUESTVAR_GET(MISSION_DIFF_QVAR) >= 1)||really_really_locked(lqb)))
#else
                  )
#endif
       {
          if (!(in_inv&0x2))
          {
            if (use_card)
               string_message_info(REF_STR_DoorCardGoodButLocked + objDoors[objs[id].specID].stringnum);
            else
               string_message_info(REF_STR_DoorLocked + objDoors[objs[id].specID].stringnum);
          }
            retval = TRUE;
       }
       else
       {
          int closetime;
    
          // This string is strangely here rather than above so as to
          // insure that we do no collide with any other messages
          if ((use_card) && (!(in_inv&0x2)))
          {
            char tempbuf[30], tb2[30];
            get_object_short_name(ID2TRIP(try_card),tempbuf,30);
            strcpy(tempbuf, get_temp_string(MKREF(RES_accessCards,(objDoors[objs[id].specID].access_level)<<1)));
            strcat(tempbuf, " ");
            strcat(tempbuf, get_string(REF_STR_DoorCardGood, tb2, 30));
            message_info(tempbuf);
            retval = TRUE;
          }
 
          add_obj_to_animlist(id, FALSE, FALSE,FALSE,32,NULL,NULL,0);    // play anim forwards
          play_fx = TRUE;

          // remove render-blocking bit, since well, we're open.
          objs[id].info.inst_flags &= ~(RENDER_BLOCK_FLAG);

          // If appropriate, have door automatically close again
          if (((closetime = objDoors[objs[id].specID].autoclose_time) > 0)
            && (closetime!=NEVER_AUTOCLOSE_COOKIE))
          {
            ushort new_code;
            new_event.timestamp = TICKS2TSTAMP(player_struct.game_time + (CIT_CYCLE*closetime)/DOOR_TIME_UNIT);
            new_event.type = DOOR_SCHED_EVENT;
            new_event.door_id = id;
            
            // Compute new secret code, poke it into event and door
            new_code = ((objs[id].info.inst_flags >> 6) + 1) & 0x3;
            new_event.secret_code = new_code;
            objs[id].info.inst_flags &= ~(0x3 << 6);  // clear out space for new code
            objs[id].info.inst_flags |= (new_code << 6); // set new code

            // schedule us!
            schedule_event(&global_fullmap->sched[MAP_SCHEDULE_GAMETIME],(SchedEvent *) &new_event);
          }
       }
    }
    else
    {
       char real_frame = objs[id].info.current_frame;
       add_obj_to_animlist(id, FALSE, TRUE,FALSE,32,NULL,NULL,0);     // play anim backwards
       play_fx = TRUE;
    }
    if (play_fx)
    {
       int sfx_id;
       switch(ID2TRIP(id))
       {
          case ACCESS_DOOR_TRIPLE:
          case REACTR_DOOR_TRIPLE:
          case BLAST_DOOR_TRIPLE:
          case STOR_DOOR_TRIPLE: 
              sfx_id = SFX_DOOR_METAL; break;
          case EXEC_DOOR_TRIPLE:
          case IRIS_TRIPLE:
              sfx_id = SFX_DOOR_IRIS; break;
          case DOUB_LEFTDOOR_TRIPLE:
          case DOUB_RITEDOOR_TRIPLE:
              sfx_id = SFX_DOOR_BULKHEAD; break;
          case LABFORCE_TRIPLE:
          case BROKLABFORCE_TRIPLE:
          case RESFORCE_TRIPLE:
          case BROKRESFORCE_TRIPLE:
              sfx_id = -1; break;
          case GENFORCE_TRIPLE:
          case CYBGENFORCE_TRIPLE:
              sfx_id = SFX_DOOR_GRATING; break;
          default: 
              sfx_id = SFX_DOOR_NORMAL;  break;
       }
       if (sfx_id != -1)
          play_digi_fx_obj(sfx_id, 1,id);
    }   
    if (((other=objDoors[objs[id].specID].other_half) != OBJ_NULL) && !(in_inv&0x1)) {
      bool otherdoor=objs[other].obclass==CLASS_DOOR;
      // use other half if we don't have the same closed-ness, in order to
      // cause us to have the same closed-ness.
      if(!otherdoor ||
         (door_moving(id,TRUE)!=(DOOR_REALLY_CLOSED(other)||door_moving(other,TRUE))))
       object_use(objDoors[objs[id].specID].other_half, otherdoor?(in_inv|0x1):FALSE, OBJ_NULL);
    }
 out:
   return retval;
}


// Maximum number of objects within another object
#define MAX_CONTAINER_CONTENTS  4

// If mission difficulty is low, returns TRUE on objects
// which might require literacy.
bool obj_too_smart(ObjID id)
{
   switch(QUESTVAR_GET(MISSION_DIFF_QVAR))
   {
      case 0:
         switch(ID2TRIP(id))
         {
            case PAPERS_TRIPLE:
            case TEXT1_TRIPLE:
            case EMAIL1_TRIPLE:
            case MAP1_TRIPLE:
            case VIDTEX_HARD_TRIPLE:
               return(TRUE);
               break;
         }
      case 1:
         switch(ID2TRIP(id))
         {
            case GENCARDS_TRIPLE            :
            case STDCARD_TRIPLE             :
            case SCICARD_TRIPLE             :
            case STORECARD_TRIPLE           :
            case ENGCARD_TRIPLE             :
            case MEDCARD_TRIPLE             :
            case MAINTCARD_TRIPLE           :
            case ADMINCARD_TRIPLE           :
            case SECCARD_TRIPLE             :
            case COMCARD_TRIPLE             :
            case GROUPCARD_TRIPLE           :
            case PERSCARD_TRIPLE            :
            case CYBERCARD_TRIPLE           :
               return(TRUE);
               break;
         }
         break;
      default:
         return(FALSE);
         break;
   }
   return(FALSE);
}

void container_check(ObjID obj, char* count, ObjID* pidlist)
{
   if (objs[obj].active && !obj_too_smart(obj)) {
      if(USE_MODE(obj)==PICKUP_USE_MODE) 
         pidlist[(*count)++] = obj; 
//      else 
//         Warning(("Non-pickup: trip %#x (%#x) in container!!\n",ID2TRIP(obj),obj));
   }
}

char container_extract(ObjID *pidlist, int d1, int d2)
{
   char retval = 0;
   container_check(d1 & 0xFFFF, &retval, pidlist);
   container_check(d1 >> 16, &retval, pidlist);
   container_check(d2 & 0xFFFF, &retval, pidlist);
   container_check(d2 >> 16, &retval, pidlist);
   return(retval);
}

void container_stuff(ObjID *pidlist, int numobjs, int* d1, int* d2)
{
   int i;
   for (i = numobjs; i < MAX_CONTAINER_CONTENTS; i++)
    pidlist[i] = OBJ_NULL;
   i = 0;
   *d1 = pidlist[i++]; 
   *d1 |= pidlist[i++] << 16;
   if (d2 == NULL) return;
   *d2 = pidlist[i++]; 
   *d2 |= pidlist[i++] << 16;
}

// Determines whether or not an object is a "container" and then 
// figures out what part of it's instance data is used for holding
// the objects.
// If we want to make more objects have these properties later, here is
// the place to add 'em.
bool is_container(ObjID id, int** d1, int** d2)
{
   ObjSpecID specid = objs[id].specID;
   bool retval = FALSE;
   if (objs[id].obclass == CLASS_CONTAINER)
   {
      // Containers
      *d1 = &objContainers[specid].contents1;
      *d2 = &objContainers[specid].contents2;
      retval = TRUE;
   }
   else
   {
      switch(ID2TRIP(id))
      {
         // Smallstuff
         case CORPSE1_TRIPLE:
         case CORPSE2_TRIPLE:
         case CORPSE3_TRIPLE:
         case CORPSE4_TRIPLE:
         case CORPSE5_TRIPLE:
         case CORPSE6_TRIPLE:
         case CORPSE7_TRIPLE:
         case CORPSE8_TRIPLE:
         case BRIEFCASE_TRIPLE:
       *d1 = &objSmallstuffs[specid].data1;
       *d2 = &objSmallstuffs[specid].data2;
       retval = TRUE;
            break;
         // Bigstuff
         case CABINET_TRIPLE:
            // Contents in data1, since cosmetic value and data2 taken for texturing
            // Maybe we can use cosmetic_value too, although thats mighty non-intuitive
       retval = TRUE;
       *d1 = &objBigstuffs[specid].data1;
       *d2 = NULL;
            break;
      }
   }
   return(retval);
}

bool obj_fixture_zoom(ObjID id, bool in_inv, bool *messagep)
{
   bool retval = FALSE;
   bool zoom = (objs[id].info.inst_flags & CLASS_INST_FLAG);
   if (zoom && !in_inv)
   {
      int mfd = grab_and_zoom_mfd(MFD_FIXTURE_FUNC,MFD_INFO_SLOT);
      save_mfd_slot(mfd);
      mfd_notify_func(MFD_FIXTURE_FUNC,MFD_INFO_SLOT,TRUE,MFD_ACTIVE,TRUE);
      player_struct.panel_ref = id;
      mfd_change_slot(mfd,MFD_INFO_SLOT);
   }
   else
   {
      retval = trap_activate(id, messagep);
      if (retval)
      {
         if (objs[id].info.current_frame == 0)
            objs[id].info.current_frame = FRAME_NUM_3D(ObjProps[OPTRIP(ID2TRIP(id))].bitmap_3d);
         else
            objs[id].info.current_frame = 0;
      }
   }
   return(retval);
}

// GEAR, GEAR, GEAR!

#define BATTERY_ENERGY_BONUS     0x50
#define TRACBEAM_ENERGY_COST     0x20
#define TRACBEAM_DIST_MOD        0x20
extern ubyte pickup_distance_mod;

errtype obj_tractor_beam_func(ObjID id, bool on)
{
   if (on)
   {
      // Tractor beam turning on
      if (player_struct.energy_spend + TRACBEAM_ENERGY_COST > MAX_ENERGY ||
       player_struct.energy < TRACBEAM_ENERGY_COST)
    string_message_info(REF_STR_WareNoPower);
      else
      {
    string_message_info(REF_STR_TractorActivate);
    pickup_distance_mod += TRACBEAM_DIST_MOD;
    objs[id].info.inst_flags |= CLASS_INST_FLAG;
    set_player_energy_spend(player_struct.energy_spend + TRACBEAM_ENERGY_COST);
      }
   }
   else
   {
      // Tractor beam turning off
      set_player_energy_spend(player_struct.energy_spend - TRACBEAM_ENERGY_COST);
      objs[id].info.inst_flags &= ~CLASS_INST_FLAG;
      pickup_distance_mod -= TRACBEAM_DIST_MOD;
      string_message_info(REF_STR_TractorDeactivate);
   }
   return(OK);
}

errtype gear_power_outage()
{
   extern errtype obj_tractor_beam_func(ObjID id, bool on);

   ObjID obj;
   char i;
   for (i = 0; i < NUM_GENERAL_SLOTS; i++)
   {
      obj = player_struct.inventory[i];
      if ((obj != OBJ_NULL) && (objs[obj].active))
      {
    // Turn off any active gear 
    switch (ID2TRIP(obj))
    {
       case TRACBEAM_TRIPLE:
          if (objs[obj].info.inst_flags & CLASS_INST_FLAG)
              obj_tractor_beam_func(obj,FALSE);
          break;
    }
      }
   }
   return(OK);
}

// returns TRUE iff we tried to use an electronic pick on the panel.
//
bool try_use_epick(ObjID panel, ObjID cursor_obj)
{
   uchar sol;

   if(cursor_obj!=OBJ_NULL) {
      if(ID2TRIP(cursor_obj)==EPICK_TRIPLE) {
         if((sol=mfd_solve_accesspanel(panel))==EPICK_SOLVED) {
            obj_destroy(cursor_obj);
            pop_cursor_object();
         }
         else
            string_message_info(REF_STR_EPickFailure+sol-1);
      }
      return TRUE;
   }
   return FALSE;
}



#define PLASTIQUE_TIME  10
extern void remove_general_item(ObjID obj);
extern Boolean	gKeypadOverride;

// We return whether or not we used the message line.
bool object_use(ObjID id, bool in_inv, ObjID cursor_obj)
{
   bool retval = FALSE, rv;
   ObjFixture *pfixt;
   ObjBigstuff *pbigs;
   char i;
   ObjSpecID osid;
   uchar special;
   extern errtype do_random_loot(ObjID corpse);
   extern char camera_map[NUM_HACK_CAMERAS];
   extern ObjID hack_cam_objs[NUM_HACK_CAMERAS];
   extern ubyte next_text_line;
   extern errtype inventory_draw_new_page(int num);
   extern void read_email(Id new_base, int num);
   int *d1,*d2;

   // First, the multi-class behavior objects
   if (is_container(id, &d1,&d2))
   {
      int mfd;
      extern bool gump_get_useful(void);
      extern void gump_clear(void);
      
      if(id==player_struct.panel_ref && object_on_cursor==NULL) {
         gump_get_useful();
         return TRUE;
      }
      mfd = grab_and_zoom_mfd(MFD_GUMP_FUNC,MFD_INFO_SLOT);
      do_random_loot(id);
      save_mfd_slot(mfd);
      mfd_notify_func(MFD_GUMP_FUNC,MFD_INFO_SLOT,TRUE,MFD_ACTIVE,TRUE);

      // oh, how we are filled with shame.  The gump_idlist doesn't
      // get filled until the expose func, one frame from now, so for
      // this one frame the idlist and the panel_ref will be out of
      // synch.  During this frame it is possible to gump_get_useful
      // from the wrong idlist for our current panel_ref!  A simple
      // but inelegant fix to the problem is to null the idlist so that
      // gump_pickups cannot work and we do not container_stuff the
      // altered idlist into the wrong container.  Get it?  Good.
      gump_clear();
  
      player_struct.panel_ref = id;
      mfd_change_slot(mfd,MFD_INFO_SLOT);
      retval = TRUE;
      return retval;
   }
   osid = objs[id].specID;

   if (global_fullmap->cyber)
   {
      bool did_something = FALSE;
      extern void long_bark(ObjID speaker_id, uchar mug_id, int string_id, ubyte color);
      switch(ID2TRIP(id))
      {
         case CYBERHEAL_TRIPLE:
            if (player_struct.cspace_hp != PLAYER_MAX_HP)
            {
               player_struct.cspace_hp = min(player_struct.cspace_hp + CYBERHEAL_QUANTITY + objSmallstuffs[objs[id].specID].data1, PLAYER_MAX_HP);
               hud_unset(HUD_CYBERDANGER);
               string_message_info(REF_STR_CspaceHeal);
               ADD_DESTROYED_OBJECT(id);
               chg_set_flg(VITALS_UPDATE);
            }
            else
               string_message_info(REF_STR_CspaceMaxHealth);
            did_something = TRUE;
            break;
         case CYBERMINE_TRIPLE:
            damage_object(PLAYER_OBJ, CYBERMINE_DAMAGE * (objSmallstuffs[objs[id].specID].data1 + 1), 1, 0);
            did_something = TRUE;
            break;
         case DATALET_TRIPLE:
            long_bark(OBJ_NULL, 0, REF_STR_DataletZero + objSmallstuffs[objs[id].specID].data1, 0x4c);
            did_something = TRUE;
            break;
         case CSPACE_EXIT_TRIPLE:
            player_struct.cspace_time_base = max(CSPACE_MIN_TIME, player_struct.cspace_time_base - CSPACE_EXIT_PENALTY);
            go_to_different_level(player_struct.realspace_level);
            did_something = TRUE;
            retval = obj_move_to(PLAYER_OBJ, &player_struct.realspace_loc, TRUE);
            break;
         case INFONODE_TRIPLE:
            long_bark(OBJ_NULL, 0, REF_STR_CspaceInfoBase + objSmallstuffs[objs[id].specID].data1, 0x4c);
            did_something = TRUE;
            break;
         case CYBERCARD_TRIPLE:
            if (QUESTVAR_GET(MISSION_DIFF_QVAR) > 1)
            {
               ObjLocState del_loc_state;
               // yank the object out of the map. 
               del_loc_state.obj = id;
               del_loc_state.loc = objs[id].loc;
               del_loc_state.loc.x = -1;
               ObjRefStateBinSetNull(del_loc_state.refs[0].bin);
               ObjUpdateLocs(&del_loc_state);
               inventory_add_object(id, FALSE);
            }
            break;
      }
      if (did_something)
         return(TRUE);
   }

   switch (objs[id].obclass)
   {
      extern bool comparator_check(int comparator, ObjID obj, uchar *special_code);
      case CLASS_TRAP:
         if (ID2TRIP(id) == MAPNOTE_TRIPLE)
         {
            char buf[80];

            lg_sprintf(buf,"\"%s\"",amap_note_string(id));
            message_info(buf);
            retval = TRUE;
         }
         break;
      case CLASS_DOOR:
         retval = use_door(id,in_inv,cursor_obj);
         break;

      case CLASS_FIXTURE:
         // Deal with some specifics....
         switch(ID2TRIP(id))
         {
            case ELEPANEL1_TRIPLE:
            case ELEPANEL2_TRIPLE:
            case ELEPANEL3_TRIPLE:
            {
               if (me_bits_music(MAP_GET_XY(PLAYER_BIN_X,PLAYER_BIN_Y)) != ELEVATOR_ZONE)
               {
                 string_message_info(REF_STR_UseTooFar); 
                 retval = TRUE;
               }
               else
               {
                  pfixt = &objFixtures[objs[id].specID];
#ifdef DOOM_EMULATION_MODE
                  if (QUESTVAR_GET(MISSION_DIFF_QVAR) == 0)
                  {
                     rv = TRUE;
                     special = FALSE;
                  }
                  else
#endif
                     rv = comparator_check(pfixt->comparator, id, &special);
                  if (rv || (special != 0))
                  {
                     int mfd = grab_and_zoom_mfd(MFD_ELEV_FUNC,MFD_INFO_SLOT);
                     // Set our reference...
                     save_mfd_slot(mfd);
          
                     // Call appropriate MFD function so that later, in turn, we get called
                     // First force the slot...         
                     mfd_setup_elevator(pfixt->p4 >> 16, pfixt->p4 & 0xFFFF, player_struct.level, special);
                     player_struct.panel_ref = id;
                     mfd_change_slot(mfd,MFD_INFO_SLOT);
                  }
               }
               retval = TRUE;
               break;
            }

            case KEYPAD1_TRIPLE:   
            case KEYPAD2_TRIPLE:
            {
               pfixt = &objFixtures[objs[id].specID];
               rv = comparator_check(pfixt->comparator, id, &special);
#ifdef DOOM_EMULATION_MODE
               if (rv && (QUESTVAR_GET(MISSION_DIFF_QVAR) <= 1))
               {
                  do_multi_stuff(qdata_get(pfixt->p1 >> 16));
                  do_multi_stuff(qdata_get(pfixt->p2 >> 16));
                  do_multi_stuff(qdata_get(pfixt->p3 >> 16));
                  play_digi_fx_obj(SFX_MFD_SUCCESS, 1, id);
               }
#endif
               else if (rv || (special != 0))
               {
                  int mfd = grab_and_zoom_mfd(MFD_KEYPAD_FUNC,MFD_INFO_SLOT);
                  // Set our reference...
                  save_mfd_slot(mfd);
          
                  // Call appropriate MFD function so that later, in turn, we get called
                  // First force the slot...         
                  objs[id].info.current_frame = 1;
                  gKeypadOverride = TRUE;
                  mfd_setup_keypad(special);
                  player_struct.panel_ref = id;
                  mfd_change_slot(mfd,MFD_INFO_SLOT);
               }
               retval =  TRUE;
               break;
            }

            case ACCPANEL1_TRIPLE:   
            case ACCPANEL2_TRIPLE:   
            case ACCPANEL3_TRIPLE:   
            case ACCPANEL4_TRIPLE:   
            case ACCPANEL5_TRIPLE:   
            case ACCPANEL6_TRIPLE:   
            {
               pfixt = &objFixtures[objs[id].specID];
               rv = comparator_check(pfixt->comparator, id, &special);
               if(rv)
               {
                  int accessmfd = NUM_MFDS;
                  
                  if(player_struct.panel_ref!=id) {
                     accessmfd = grab_and_zoom_mfd(mfd_type_accesspanel(id),MFD_INFO_SLOT);
                     save_mfd_slot(accessmfd);
          
                     // Call appropriate MFD function so that later, in turn, we get called
                     // First force the slot...         
                     objs[id].info.current_frame = 1;
                     mfd_setup_accesspanel(special,id);
                     player_struct.panel_ref = id;
                  }
                  else
                  {
                     int mfd_id = NUM_MFDS;
                     if (mfd_yield_func(mfd_type_accesspanel(id),&mfd_id))
                     {
                        zoom_mfd(mfd_id);
                     }
                  }

                  // electronic picks work even at difficulty 0, though
                  // you don't need them.  Only automatically solve the
                  // puzzle if a pick doesn't try to do so for you.
                  if(!try_use_epick(id,cursor_obj)) {
                     int info;

                     if(player_struct.difficulty[PUZZLE_DIFF_INDEX]==0)
                        mfd_solve_accesspanel(id);
                     else {

                        // give us another info MFD for help text.
                        // note that this all works without assuming
                        // that we have exactly 2 mfd's.  If you want
                        // to pitch that, you can compact this code
                        // a little bit just by setting mfd=0, info=1.

                        for(info=0;info<NUM_MFDS;info++)
                           if(info!=accessmfd) {
                              save_mfd_slot(info);
                              mfd_change_slot(info,MFD_INFO_SLOT);
                              break;
                           }
                     }
                  }
                  if (accessmfd < NUM_MFDS)
                     mfd_change_slot(accessmfd, MFD_INFO_SLOT);
               }
               retval = TRUE;
               break;
            }

            case ENRG_CHARGE_TRIPLE:
               pfixt = &objFixtures[objs[id].specID];
               rv = comparator_check(pfixt->comparator, id, &special);
               if (rv)
               {               
                  if (player_struct.game_time >= objFixtures[osid].p4)
                  {
                     // give the player some juice!
                     player_struct.energy = min(255, player_struct.energy + objFixtures[osid].p1);

                     // update the vitals window
                     chg_set_flg(VITALS_UPDATE);

                     // don't let us get energy until some time later
                     objFixtures[osid].p4 = (int)(player_struct.game_time + (CIT_CYCLE * objFixtures[osid].p2));

                     string_message_info(REF_STR_Recharge);
                     play_digi_fx_obj(SFX_ENERGY_RECHARGE, 1,id);

                     // Trigger any associated trap
                     do_multi_stuff(objFixtures[osid].p3);
                  }
                  else
                     string_message_info(REF_STR_NoRecharge);
               }
               retval = TRUE;
               break;

            case ANTENNA_PAN_TRIPLE:
               if (ID2TRIP(cursor_obj) == PLASTIQUE_TRIPLE)
               {
                  DoorSchedEvent new_event;

                  if (objs[id].info.current_frame == 1)
                  {
                     // Putting plastique on the panel.
                     string_message_info(REF_STR_PlastiqueOn);
                     obj_destroy(cursor_obj);
                     pop_cursor_object();

                     // Transmogrify der objectenhausen into a plastiqued panel
                     objs[id].info.type = 4;
                     objs[id].info.current_frame = 0;

                     // Set a timer to go boom
                     new_event.type = DOOR_SCHED_EVENT;
                     new_event.timestamp = TICKS2TSTAMP(player_struct.game_time + (CIT_CYCLE * PLASTIQUE_TIME));
                     new_event.door_id = id;
                     schedule_event(&(global_fullmap->sched[MAP_SCHEDULE_GAMETIME]), (SchedEvent *)&new_event);

                     // do any appropriate trap
                     do_multi_stuff(objFixtures[objs[id].specID].p2);
                  }
                  else
                     string_message_info(REF_STR_OpenPanelFirst);
               }
               else
               {
                  if (objs[id].info.current_frame)
                     objs[id].info.current_frame = 0;
                  else
                     objs[id].info.current_frame = 1;
               }
               retval =  TRUE;
               break;

            default:
               switch (objs[id].subclass)
               {
                  case FIXTURE_SUBCLASS_RECEPTACLE:
                  case FIXTURE_SUBCLASS_VENDING:
                     if ((cursor_obj != NULL) || (QUESTVAR_GET(MISSION_DIFF_QVAR) < 1))
                     {
                        if (ID2TRIP(id) == RETSCANNER_TRIPLE)
                        {
                           int head_count = 0;
                           if (ID2TRIP(cursor_obj) == HEAD_TRIPLE) 
                              head_count = objs[cursor_obj].info.current_frame + 1;
                           else if (ID2TRIP(cursor_obj) == HEAD2_TRIPLE)
                              head_count = objs[cursor_obj].info.current_frame + 11 + 1;
                           if (head_count > 0) 
                           {
                              if (((objFixtures[objs[id].specID].comparator & 0xFFFFFF) == head_count - 1) 
                                 || (QUESTVAR_GET(MISSION_DIFF_QVAR) < 1))
                              {
                                 objs[id].info.current_frame = 1;
                                 obj_fixture_zoom(id,in_inv, &retval);
                              }
                              else
                              {
                                 objs[id].info.current_frame = 2;
                                 string_message_info(REF_STR_WrongHead);
                              }
                           }
                        }
                        else if ((ID2TRIP(cursor_obj) == objFixtures[objs[id].specID].comparator & 0xFFFFFF) ||
                           (QUESTVAR_GET(MISSION_DIFF_QVAR) < 1))
                        {
                           obj_fixture_zoom(id,in_inv, &retval);
                           obj_destroy(cursor_obj);
                           pop_cursor_object();
                        }
                        else if (objFixtures[objs[id].specID].comparator >> 24)
                        {
                           string_message_info(REF_STR_TrapZeroMessage + (objFixtures[objs[id].specID].comparator >> 24));
#ifdef AUDIOLOGS
                           audiolog_bark_play(objFixtures[objs[id].specID].comparator >> 24);
#endif
                        }
                     }
                     else if (ID2TRIP(id) == RETSCANNER_TRIPLE)
                        string_message_info(REF_STR_WrongHead);
                     retval = TRUE;
                     break;
                  default:
                  {
                     bool access_okay = FALSE;
                     int try_combo;
                     if ((objFixtures[osid].access_level == 0) 
#ifdef DOOM_EMULATION_MODE
                        || (QUESTVAR_GET(MISSION_DIFF_QVAR) < 2)
#endif
                           )
                        access_okay = TRUE;
                     else
                     {
                        ObjID try_card;
                        bool had_card = FALSE;
                        try_combo = 1 << objFixtures[osid].access_level;
                        for (i=0; i < NUM_GENERAL_SLOTS; i++)
                        {
                           try_card = player_struct.inventory[i];
                           if (ID2TRIP(try_card) == GENCARDS_TRIPLE)
                           {
                              had_card = TRUE;
                              if (objSmallstuffs[objs[try_card].specID].data1 & try_combo)
                                 access_okay = TRUE;
                              else
                                 obj_access_fail_message(REF_STR_FixtureAccessBad, objFixtures[osid].access_level, 0);
                           }
                        }
                        if (!had_card)
                           obj_access_fail_message(REF_STR_FixtureAccessBad, objFixtures[osid].access_level, 0);
                     }
                     if (access_okay)
                     {
                        if (comparator_check(objFixtures[objs[id].specID].comparator, id, &special))
                        {
                           switch (ID2TRIP(id))
                           {
                              case BUTTON1_TRIPLE:
                              case BUTTON2_TRIPLE:
                                 play_digi_fx_obj(SFX_BUTTON,1,id); break;
                              case BIGRED_TRIPLE:
                                 play_digi_fx_obj(SFX_BIGBUTTON,1,id); break;
                              case BIGLEVER_TRIPLE:
                                 play_digi_fx_obj(SFX_BIGLEVER,1,id); break;
                              case LEVER1_TRIPLE:
                              case LEVER2_TRIPLE:
                                 play_digi_fx_obj(SFX_NORMAL_LEVER,1,id); break;
                              case SWITCH1_TRIPLE:
                              case SWITCH2_TRIPLE:
                                 play_digi_fx_obj(SFX_MECH_BUTTON,1,id); break;
                           }
                           obj_fixture_zoom(id,in_inv, &retval);
                           if (objs[id].subclass == FIXTURE_SUBCLASS_CYBER)
                           {
                              if (ID2TRIP(id) == CYBERTOG1_TRIPLE)
                              {
                                 objs[id].info.type = 1;
                              }
   #ifdef BROKEN_CYBERTOGS
                              else if (ID2TRIP(id) == CYBERTOG2_TRIPLE)
                                 objs[id].info.type = 0;
   #endif
                           }
                        }
                     }
                     break;
                  }
               }
               retval = TRUE;
               break;
         }
         break;
      case CLASS_CRITTER:
         {
          if (cursor_obj == OBJ_NULL)
            if (id != player_struct.curr_target)
               select_current_target(id,TRUE);
            else
               select_current_target(OBJ_NULL,TRUE);
         }
         retval =  TRUE;
         break;

      case CLASS_SMALLSTUFF:
         if (in_inv)
         {
            switch (ID2TRIP(id))
            {
               case AIDKIT_TRIPLE:
                  string_message_info(REF_STR_MedikitUse);
                  player_struct.hit_points = PLAYER_MAX_HP;
                  goto yankinv;
               case BATTERY2_TRIPLE:
                  player_struct.energy=255;
               case BATTERY_TRIPLE:
                  player_struct.energy = min(255, player_struct.energy + BATTERY_ENERGY_BONUS);
                  play_digi_fx(SFX_BATTERY_USE,1);
               yankinv:
                  remove_general_item(id);

                  // Make appropriate UI parts redraw
                  chg_set_flg(VITALS_UPDATE);
                  chg_set_flg(INVENTORY_UPDATE);
                  retval = TRUE;
                  break;
               case TRACBEAM_TRIPLE:
                  obj_tractor_beam_func(id,(objs[id].info.inst_flags & CLASS_INST_FLAG) == 0);
                  retval = TRUE;
                  break;
               default:                                                                     
   #ifdef SUPPORT_STUFF_OBJUSE
                  if (((ObjProps[OPNUM(id)].flags & CLASS_FLAGS) >> CLASS_FLAGS_SHF) == STUFF_OBJUSE_FLAG)
                        {
                           do_multi_stuff(objSmallstuffs[objs[id].specID].data1 & 0xFFFF);
                           do_multi_stuff(objSmallstuffs[objs[id].specID].data1 >> 16);
                           retval  = TRUE;
                        }
                  else
   #endif
                     break;
            }
            mfd_notify_func(NOTIFY_ANY_FUNC,MFD_ITEM_SLOT,FALSE,MFD_ACTIVE,FALSE);
         }
         else
         {
            switch (ID2TRIP(id))
            {
               case PAPERS_TRIPLE:
                  // Note secret perversion of email system
                  read_email(RES_paper0, objSmallstuffs[objs[id].specID].data1);
   #ifdef OLD_WAY
                  next_text_line = 0;
                  current_email = 0;
                  inventory_draw_new_page(EMAILTEXT_INV_PAGE);
                  if (ResInUse(RES_paper0 + objSmallstuffs[objs[id].specID].data1))
                     email_draw_text(RES_paper0 + objSmallstuffs[objs[id].specID].data1, FALSE);
                  else
                     email_draw_text(RES_paper0, FALSE);
   #endif
                  retval = TRUE;
                  break;
            }
         }
         break;

      case CLASS_BIGSTUFF:
         switch (ID2TRIP(id))
         {
            case SURG_MACH_TRIPLE:
               pbigs = &objBigstuffs[objs[id].specID];
               rv = comparator_check(pbigs->data1, id, &special);
               if (rv)
               {
                  if (pbigs->data2 & 0xFFFF)
                     player_struct.hit_points = min(PLAYER_MAX_HP, player_struct.hit_points + (pbigs->data2 & 0xFFFF));
                  else
                     player_struct.hit_points = PLAYER_MAX_HP;
                  chg_set_flg(VITALS_UPDATE);
                  string_message_info(REF_STR_SurgeryHeal);
                  trap_sfx_func(0,0,2,CIT_CYCLE << 1);
                  do_multi_stuff(pbigs->data2 >> 16);
                  play_digi_fx_obj(SFX_SURGERY_MACHINE,1,id);
               }
               retval =  TRUE;
               break;

            case CONTPAN_TRIPLE:
            case CONTPED_TRIPLE:
               if (objBigstuffs[osid].data1)
               {
                 break;
               }
            case TV_TRIPLE:
            case MONITOR2_TRIPLE:
            case SCREEN_TRIPLE:
            case BIGSCREEN_TRIPLE:
            case SUPERSCREEN_TRIPLE:
            {
               short v = objBigstuffs[osid].data2 & 0x7F;
               if ((v >= FIRST_CAMERA_TMAP) && (v < FIRST_CAMERA_TMAP + NUM_HACK_CAMERAS))
               {
                  if (camera_map[v-FIRST_CAMERA_TMAP] && hack_cam_objs[v - FIRST_CAMERA_TMAP])
                     hack_camera_takeover(v - FIRST_CAMERA_TMAP);
               }
               else
                  string_message_info(REF_STR_NormalScreen);
               retval = TRUE;
               break;
            }

            default:
               break;
         }

         if (((ObjProps[OPNUM(id)].flags & CLASS_FLAGS) >> CLASS_FLAGS_SHF) == STUFF_OBJUSE_FLAG)
         {
            do_multi_stuff(objBigstuffs[objs[id].specID].data1 & 0xFFFF);
            do_multi_stuff(objBigstuffs[objs[id].specID].data1 >> 16);
            if (objBigstuffs[objs[id].specID].data1)
               retval = TRUE;
         }
         break;
      default:
         break;
   }
   return(retval);
}


ObjID door_in_square(ObjLoc* loc, bool usable)
{
   ObjRefID oref;
   ObjID id;

   oref=me_objref(MAP_GET_XY(OBJ_LOC_BIN_X(*loc),OBJ_LOC_BIN_Y(*loc)));
   while(oref != OBJ_REF_NULL) {
      id=objRefs[oref].obj;

      if(objs[id].obclass==CLASS_DOOR) {
         if(!usable || USE_MODE(id)==USE_USE_MODE)
            return id;
      }
      
      oref=objRefs[oref].next;
   }
   return(OBJ_NULL);
}

void regenetron_door_hack()
{
   ObjID id;

   id=door_in_square(&(objs[PLAYER_OBJ].loc),TRUE);

   if(id && !door_moving(id,TRUE) && !door_moving(id,FALSE))
      objs[id].info.current_frame=0;
}

// Collects all the objects already in the elevator that you are going to
// and move them outside the door
#define MAX_JANITOR_OBJS   32
errtype elevator_janitor_run()
{
   short x0,x1,y0,y1,x,y;
   int i, j, obj_count = 0;
   ObjLoc dump_loc, newloc;
   ObjID objlist[MAX_JANITOR_OBJS],id;
   bool dupe;
   ObjRefID orefid;
   extern bool robot_antisocial;

   // clear out our movelist
   for (i=0; i < MAX_JANITOR_OBJS; i++)
      objlist[i] = OBJ_NULL;
   dump_loc.x = -1;

   // Compute bounding box of the elevator, see comments in compute_elev_objs
   x0 = PLAYER_BIN_X; y0 = PLAYER_BIN_Y;
   while (me_bits_music(MAP_GET_XY(x0,y0)) == ELEVATOR_ZONE)
      x0--;
   x0++;
   while (me_bits_music(MAP_GET_XY(x0,y0)) == ELEVATOR_ZONE)
      y0--;
   y0++;
   x1 = x0;  y1 = y0;
   while (me_bits_music(MAP_GET_XY(x1,y1)) == ELEVATOR_ZONE)
      x1++;
   x1--;
   while (me_bits_music(MAP_GET_XY(x1,y1)) == ELEVATOR_ZONE)
      y1++;
   y1--;
   
   // Collect all the objects
   for (x = x0; x <= x1; x++)
   {
      for (y = y0; y <= y1; y++)
      {
         orefid = me_objref(MAP_GET_XY(x,y));
         while (orefid != OBJ_NULL)
         {
            id = objRefs[orefid].obj;
            if ((objs[id].obclass == CLASS_DOOR) && (dump_loc.x == -1))
            {
               dump_loc.x = objs[id].loc.x & (~0xFF);
               dump_loc.y = objs[id].loc.y & (~0xFF);
               if ((objs[id].loc.y & 0xFF) <= 0x2)
                  dump_loc.y -= 0x100;
               else if ((objs[id].loc.y & 0xFF) >= 0xFC)
                  dump_loc.y += 0x100;
               else if ((objs[id].loc.x & 0xFF) <= 0x2)
                  dump_loc.x -= 0x100;
               else
                  dump_loc.x += 0x100;
            }
            else if ((id != OBJ_NULL) && (id != PLAYER_OBJ) && (ObjProps[OPNUM(id)].physics_model))
            {
               dupe = FALSE;
               for (j=0; j < obj_count; j++)
               {
                  if (objlist[j] == id)
                  {
                     dupe = TRUE;
                     break;
                  }
               }
               if (!dupe)
                  objlist[obj_count++] = id;
            }
            orefid = objRefs[orefid].next;
         }
      }
   }

   // Move all the stuff there
   robot_antisocial = TRUE;
   for (i=0; i < obj_count; i++)
   {
      newloc = objs[objlist[i]].loc;
      newloc.x = dump_loc.x + (rand() & 0xBF) + 0x20;
      newloc.y = dump_loc.y + (rand() & 0xBF) + 0x20;
      newloc.z = obj_floor_compute(objlist[i], me_height_flr(MAP_GET_XY(OBJ_LOC_BIN_X(dump_loc), OBJ_LOC_BIN_Y(dump_loc))));
      obj_move_to(objlist[i], &newloc, TRUE);
   }
   robot_antisocial = FALSE;

   return(OK);
}

#ifdef ELEVATOR_PACKRAT

#define MAX_ELEV_OBJS   16

// Goes through all the objects in the same elevator as the player, and fills objlist with them
errtype compute_elev_objs(ObjID *objlist)
{
   short i,j,x,y;
   short x0,x1,y0,y1;
   ObjRefID oref;
   ObjID id;
   MapElem *pme;
   bool dupe;

   for (i = 0; i < MAX_ELEV_OBJS; i++)
      objlist[i] = OBJ_NULL;
   i=0;

   // Wow, this is an wacky way of finding the bounding rectangle
   // of elevator music, but hey, it should work for any rectangle...
   x0 = PLAYER_BIN_X; y0 = PLAYER_BIN_Y;
   while (me_bits_music(MAP_GET_XY(x0,y0)) == ELEVATOR_ZONE)
      x0--;
   x0++;
   while (me_bits_music(MAP_GET_XY(x0,y0)) == ELEVATOR_ZONE)
      y0--;
   y0++;
   x1 = x0;  y1 = y0;
   while (me_bits_music(MAP_GET_XY(x1,y1)) == ELEVATOR_ZONE)
      x1++;
   x1--;
   while (me_bits_music(MAP_GET_XY(x1,y1)) == ELEVATOR_ZONE)
      y1++;
   y1--;

   // Go through all the elevator squares, collecting objects
   for (x=x0; x <= x1; x++)
   {
      for (y=y0; y <= y1; y++)
      {
         pme = MAP_GET_XY(x,y);
         oref = me_objref(pme);
         while (oref != OBJ_REF_NULL)
         {
            dupe = FALSE;
            id = objRefs[oref].obj;
            if ((id != OBJ_NULL) && (id != PLAYER_OBJ) && (ObjProps[OPNUM(id)].physics_model))
            {
               for (j=0; j < i; j++)
               {
                  if (objlist[j] == id)
                  {
                     dupe = TRUE;
                     break;
                  }
               }
               if (!dupe)
               {
                  ObjLoc newloc;
                  State st;
                  extern void state_to_objloc(State *s, ObjLoc *l);
                  int ph = objs[id].info.ph;
                  int *pd1, *pd2;

                  // Make sure it is okay to come with us.... first check physics then check
                  // for containerism, and grenadeliness.
                  if (ph != -1)
                  {
                     // if we are in physics, force us to the floor, etc. before going
                     EDMS_settle_object(ph);
                     EDMS_get_state(ph, &st);
                     state_to_objloc(&st, &newloc);
                     obj_move_to(id, &newloc, FALSE);
                     EDMS_kill_object(ph);
                     objs[id].info.ph = -1;
                  }
                  // This will cruelly strand the container's contents to the 
                  // eternal limbo of the unreferenced object.  
                  // Life is a grim place sometimes.
                  if (is_container(id, &pd1, &pd2))
                  {
                     *pd1 = 0;
                     *pd2 = 0;
                  }

                  // Boom go the grenades
                  if ((objs[id].obclass == CLASS_GRENADE) && (objGrenades[objs[id].specID].flags & GREN_ACTIVE_FLAG))
                     ADD_DESTROYED_OBJECT(id);
//                     do_grenade_explosion(id,TRUE);
                  else
                     objlist[i++] = id;
               }
            }
            oref = objRefs[oref].next;
         }
      }
   }
   return(OK);
}

#endif

//��� uncomment the next 2 lines for playable demo only!!!
//#define MAC_DEMO
//extern Boolean	gPlayingGame;
//���

// Eventually, this code will also go and do some nice level-changing
// animation.  For now, we just telemaport you.
// dest_level is the target level to be teleported to
// which_panel is an index into the list of "equivalent" panels that each panel keeps around.
// returns whether or not the elevator actually went anywhere
bool elevator_use(short dest_level, ubyte which_panel)
{
#ifdef MAC_DEMO
//   extern errtype trap_cutscene_func(int p1, int p2, int p3, int p4);
//   trap_cutscene_func(2,TRUE,0,0);
	uiHideMouse(NULL);
	ShowCursor();
 	Alert(1999, NULL);									// Show "thanks for playing demo" alert.
	gPlayingGame = FALSE;								// Hop out of the game loop.
#else
   errtype retval = TRUE;
   short xdiff, ydiff, zdiff;
   ObjLoc panel_loc, newloc;
   char old_zsh;
   int new_panel;
   ObjRefID oref;
   ObjID id;
   int nuframe;
#ifdef ELEVATOR_PACKRAT
   char i;
   ObjLoc temploc;
   ObjID tempid;
   ObjID elev_obj_list[MAX_ELEV_OBJS];
   ObjLoc elev_obj_diffs[MAX_ELEV_OBJS];
   extern void store_objects(char** buf, ObjID *obj_array, char obj_count);
   extern void restore_objects(char* buf, ObjID *obj_array, char obj_count);
   extern errtype obj_load_art(bool flush_all);
   extern bool robot_antisocial;
   char *buf;
#endif
   extern void check_panel_ref(bool puntme);

   if (dest_level == player_struct.level)
   {
      string_message_info(REF_STR_ElevatorSameFloor);
      return(FALSE);
   }
   else
      nuframe=(dest_level>player_struct.level);

   panel_loc = objs[player_struct.panel_ref].loc;

   oref = me_objref(MAP_GET_XY(OBJ_LOC_BIN_X(panel_loc),OBJ_LOC_BIN_Y(panel_loc)));

   while(oref != OBJ_REF_NULL) {
      ObjID id = objRefs[oref].obj;

      if((objs[id].obclass == CLASS_DOOR) && (!(DOOR_REALLY_CLOSED(id)))) {
         // there is an open elevator in the square, so don't let the elevator go
         string_message_info(REF_STR_ElevatorDoorOpen);
         return(FALSE);
      }
      oref = objRefs[oref].next;
   }

   string_message_info(REF_STR_ElevatorMove);

#ifdef ELEVATOR_PACKRAT
   // Fill list of elevator-contained objects
   compute_elev_objs(elev_obj_list);
   store_objects(&buf, elev_obj_list, MAX_ELEV_OBJS);
#endif

   // Compute our offset from the panel we actually frobbed with.
   xdiff = panel_loc.x - player_dos_obj->loc.x;
   ydiff = panel_loc.y - player_dos_obj->loc.y;
   zdiff = panel_loc.z - player_dos_obj->loc.z;
#ifdef ELEVATOR_PACKRAT
   for (i=0; i < MAX_ELEV_OBJS; i++)
   {
      if (elev_obj_list[i] != OBJ_NULL)
      {
         elev_obj_diffs[i].x = panel_loc.x - objs[elev_obj_list[i]].loc.x;
         elev_obj_diffs[i].y = panel_loc.y - objs[elev_obj_list[i]].loc.y;
         elev_obj_diffs[i].z = panel_loc.z - objs[elev_obj_list[i]].loc.z;
      }
   }
#endif
   old_zsh = MAP_ZSHF;

   if (full_game_3d)
   {
      render_run();
   }

   // Find what the equivalent panel on the new level is
   // we do this by deparsing the data stuffed into the trap data
   // of the panel.
   switch (which_panel)
   {
      case 0: case 1:
      new_panel = objFixtures[objs[player_struct.panel_ref].specID].p1;  break;
      case 2: case 3:
      new_panel = objFixtures[objs[player_struct.panel_ref].specID].p2;  break;
      case 4: case 5:
      new_panel = objFixtures[objs[player_struct.panel_ref].specID].p3;  break;
   }
   if ((which_panel % 2) == 0)
      new_panel = new_panel >> 16;
   else
      new_panel = new_panel & 0xFFFF;

   objs[player_struct.panel_ref].info.current_frame=nuframe;
   check_panel_ref(TRUE);
   
   // Teleport the player to that level, at the same relative distance
   // from the new panel as to the old.
   begin_wait();
   retval = trap_teleport_func(0xF000,0xF000,0xF000, dest_level);  // no change in x, y, or z
   
   if (retval == OK)
   {
      panel_loc = objs[new_panel].loc;
      objs[new_panel].info.current_frame=nuframe;
      newloc = player_dos_obj->loc;
      newloc.x = panel_loc.x - xdiff;
      newloc.y = panel_loc.y - ydiff;
#ifdef BROKEN_CODE
      newloc.z = panel_loc.z - zdiff;
      if (MAP_ZSHF > old_zsh)
         newloc.z = newloc.z << (MAP_ZSHF - old_zsh);
      else
         newloc.z = newloc.z << (old_zsh - MAP_ZSHF);
#endif
      if (MAP_ZSHF==old_zsh)
         newloc.z = panel_loc.z - zdiff;
      else if (MAP_ZSHF > old_zsh)
         newloc.z = panel_loc.z - (zdiff << (MAP_ZSHF - old_zsh));
      else 
         newloc.z = panel_loc.z - (zdiff >> (old_zsh - MAP_ZSHF));

      obj_move_to(PLAYER_OBJ, &newloc, TRUE);

      // Clear out old cruft in the new elevator squares
      elevator_janitor_run();

#ifdef ELEVATOR_PACKRAT
      // Reconsitute elevator-objects
      restore_objects(buf, elev_obj_list, MAX_ELEV_OBJS);

      // Move 'em to the right place
      robot_antisocial = TRUE;
      for (i=0; i < MAX_ELEV_OBJS; i++)
      {
         tempid = elev_obj_list[i];
         if (tempid != OBJ_NULL)
         {
            temploc = objs[tempid].loc;
            temploc.x = panel_loc.x - elev_obj_diffs[i].x;
            temploc.y = panel_loc.y - elev_obj_diffs[i].y;
            if (MAP_ZSHF == old_zsh)
               temploc.z = panel_loc.z - elev_obj_diffs[i].z;
            else if (MAP_ZSHF > old_zsh)
               temploc.z = panel_loc.z - (elev_obj_diffs[i].z << (MAP_ZSHF - old_zsh));
            else 
               temploc.z = panel_loc.z - (elev_obj_diffs[i].z >> (old_zsh - MAP_ZSHF));
            obj_move_to(tempid, &temploc, TRUE);
         }
      }
      robot_antisocial = FALSE;
      obj_load_art(FALSE);
#endif
      end_wait();

      stop_digi_fx();			// KLC - Moved this to before the door tries to open.

      // open the door, unless freight elevator
      id=door_in_square(&panel_loc,TRUE);
      if(DOOR_REALLY_CLOSED(id) &&
         !door_locked(id) && objDoors[objs[id].specID].other_half==0) {
         object_use(id,FALSE,OBJ_NULL);
      }            
   }
   else
      critical_error(CRITERR_FILE);
#endif
   return(TRUE);
}


errtype obj_door_lock(ObjID door_id, bool new_lock)
{
   if (new_lock)
      QUESTBIT_ON(objDoors[objs[door_id].specID].locked);
   else
      QUESTBIT_OFF(objDoors[objs[door_id].specID].locked);
   return(OK);
}


void multi_anim_callback(ObjID id, void *user_data);
bool in_anim_callback = FALSE;

void unmulti_anim_callback(ObjID id, void *user_data)
{
   int orig_parm  = (int) user_data;
   int *pp2, *pp1;

   if (in_anim_callback || !time_passes)
      return;
   in_anim_callback = TRUE;
   switch(objs[id].obclass)
   {
      case CLASS_BIGSTUFF:   
    pp1 = &objBigstuffs[objs[id].specID].data1;  
    pp2 = &objBigstuffs[objs[id].specID].data2;  
    break;
      case CLASS_SMALLSTUFF:   
    pp1 = &objSmallstuffs[objs[id].specID].data1;  
    pp2 = &objSmallstuffs[objs[id].specID].data2;  
    break;
   }
   objs[id].info.current_frame = 0;
   add_obj_to_animlist(id, TRUE, *pp1 & 0x2, *pp1 & 0x1,0, 5, NULL, ANIMCB_REPEAT|ANIMCB_CYCLE);
   *pp2 = orig_parm;
   in_anim_callback = FALSE;
}

void multi_anim_callback(ObjID id, void *)
{
   int *pp2, *pp1;
   bool do_swap = FALSE;

   if (in_anim_callback || !time_passes)
      return;
   in_anim_callback = TRUE;
   switch(objs[id].obclass)
   {
      case CLASS_BIGSTUFF:   
    pp1 = &objBigstuffs[objs[id].specID].data1;  
    pp2 = &objBigstuffs[objs[id].specID].data2;  
    break;
      case CLASS_SMALLSTUFF:   
    pp1 = &objSmallstuffs[objs[id].specID].data1;  
    pp2 = &objSmallstuffs[objs[id].specID].data2;  
    break;
   }

   if ((*pp1) >> 16)
   {
      // As we have other ways of determining when to switch, they can just go
      // into this case statement.
      switch ((*pp1) >> 28)
      {
    case 0:
       if (rand()%((*pp1 & 0xFFF0000) >> 16) == 1)
          do_swap = TRUE;
       break;
      }
      if (do_swap)
      {
    remove_obj_from_animlist(id);
    objs[id].info.current_frame = 0;
    add_obj_to_animlist(id, FALSE, FALSE, FALSE, 0,4, (void *)*pp2, ANIMCB_REMOVE);
    *pp2 = *pp2 >> 16;
      }
   }
   in_anim_callback = FALSE;
}


errtype obj_screen_animate(ObjID id)
{
   errtype retval = OK;
   remove_obj_from_animlist(id);
   switch(objs[id].obclass)
   {
      case CLASS_BIGSTUFF:
    if (objBigstuffs[objs[id].specID].data2 >> 16)
       retval = add_obj_to_animlist(id, TRUE, objBigstuffs[objs[id].specID].data1 & 0x2, objBigstuffs[objs[id].specID].data1 & 0x1,
          0,5, NULL, ANIMCB_REPEAT|ANIMCB_CYCLE);
    else
       retval = add_obj_to_animlist(id, TRUE, objBigstuffs[objs[id].specID].data1 & 0x2, objBigstuffs[objs[id].specID].data1 & 0x1,0,NULL, NULL, NULL);
    break;
      case CLASS_SMALLSTUFF:
    if (objSmallstuffs[objs[id].specID].data2 >> 16)
       retval = add_obj_to_animlist(id, TRUE, objSmallstuffs[objs[id].specID].data1 & 0x2, objSmallstuffs[objs[id].specID].data1 & 0x1,
          0,5, NULL, ANIMCB_REPEAT|ANIMCB_CYCLE);
    else
       retval = add_obj_to_animlist(id, TRUE, objBigstuffs[objs[id].specID].data1 & 0x2, objBigstuffs[objs[id].specID].data1 & 0x1,0,NULL, NULL, NULL);
    break;
   }
   return(OK);
}


#define MAX_KEYPAD_DIGITS  3

bool obj_keypad_crunch(int p, uchar digits[MAX_KEYPAD_DIGITS])
{
   bool retval = TRUE;
   int i;
   short combo = qdata_get(p & 0xFFFF);
   ObjID id = qdata_get(p >> 16);

   if (combo == 0)
      return(FALSE);
   for (i=0; i < MAX_KEYPAD_DIGITS; i++)
   {
      if (((combo >> (4* i)) & 0xF) != digits[MAX_KEYPAD_DIGITS -1 - i])
    retval = FALSE;
   }
   if (retval)
   {
      gKeypadOverride = FALSE;
      play_digi_fx_obj(SFX_MFD_SUCCESS, 1, id);
      do_multi_stuff(id);
   }
   return(retval);
}

errtype keypad_trigger(ObjID id, uchar digits[MAX_KEYPAD_DIGITS])
{
   ObjSpecID osid = objs[id].specID;
   char match = -1;
   if ((objs[id].obclass != CLASS_FIXTURE) || (!objs[id].active))
   {
      return(ERR_NOEFFECT);
   }
   if(obj_keypad_crunch(objFixtures[osid].p1,digits)) match=1;
   if(obj_keypad_crunch(objFixtures[osid].p2,digits)) match=1;
   if(obj_keypad_crunch(objFixtures[osid].p3,digits)) match=1;
   if (match == -1)
   {
      if (qdata_get(objFixtures[osid].p4 >> 16) == 0)
         string_message_info(REF_STR_KeypadBad);
      else
      {
         string_message_info(REF_STR_TrapZeroMessage + qdata_get(objFixtures[osid].p4 >> 16));
#ifdef AUDIOLOGS
         audiolog_bark_play(qdata_get(objFixtures[osid].p4 >> 16));
#endif
      }
      play_digi_fx_obj(SFX_MFD_BUZZ, 1,id);
      do_multi_stuff(qdata_get(objFixtures[osid].p4 & 0xFFFF));
   }
   else
   {
      if (match == CLASS_DOOR)
         string_message_info(REF_STR_KeypadGood);
   }
   return(OK);
}

// Access panels stick their door to be opened in P1
errtype accesspanel_trigger(ObjID id)
{
   TrapSchedEvent new_ev;
   ObjID trap;
   errtype err;

   trap=objFixtures[objs[id].specID].p1;

   new_ev.timestamp=TICKS2TSTAMP(player_struct.game_time)+1;
   new_ev.type=TRAP_SCHED_EVENT;
   new_ev.target_id=trap;
   new_ev.source_id=-1;
   err = schedule_event(&(global_fullmap->sched[MAP_SCHEDULE_GAMETIME]),(SchedEvent*)&new_ev);

   if(err)
   {
      // failed to schedule!  Forge ahead anyway.  As far as I know, the
      // worst thing that happens is your MFD state gets screwed up, and
      // not having a puzzle fire at all can lose you the game.

      do_multi_stuff(trap);
   }

   play_digi_fx_obj(SFX_PANEL_SUCCESS, 1,id);
   return(OK);
}


// Hmm, I wonder whether this could be a problem after loading a game...
#define CSPACE_OBJECT_DELAY_TIME    CIT_CYCLE

ObjID last_obj;
ulong last_obj_time;

// Collision with something while in cyberspace
errtype obj_cspace_collide(ObjID id, ObjID collider)
{
   extern errtype collide_objects(ObjID collision, ObjID victim, int bad);
   char str_buf[60],temp[20];
   int bigstuff_fake = 0, trip;
   bool select=FALSE;
   
   if (collider != PLAYER_OBJ)
   {
      // generate a fake physics-like collision callback
      collide_objects(collider,id,0);
      return(OK);
   }
   if (objs[id].obclass == CLASS_TRAP)
      return(OK);
   if ((last_obj == id) && (player_struct.game_time < last_obj_time))
      return(OK);
   last_obj = id;
   last_obj_time = player_struct.game_time + CSPACE_OBJECT_DELAY_TIME;
   if (ICE_ICE_BABY(id))
   {
      if (player_struct.hud_modes & HUD_FAKEID)
         hud_unset(HUD_FAKEID);
      else
      {
         string_message_info(REF_STR_IceEncrusted);
         return(OK);
      }
   }
#ifdef MATCHBOX_SUPPORT
   switch (ID2TRIP(id))
   {
      case ARROW_TRIPLE:
         cspace_effect_times[CS_MATCHBOX_EFF] = player_struct.game_time + cspace_effect_durations[CS_MATCHBOX_EFF];
         return(OK);
         break;
   }
#endif
   switch (objs[id].obclass)
   {
      case CLASS_FIXTURE:
      case CLASS_SMALLSTUFF:
         do_multi_stuff(id);
         break;
      case CLASS_BIGSTUFF:
         bigstuff_fake = MAKETRIP(CLASS_SOFTWARE, objBigstuffs[objs[id].specID].data1, objBigstuffs[objs[id].specID].data2);
      case CLASS_SOFTWARE:
      default:
         shameful_obselete_flag=FALSE;
         // if player has a valid currently selected combat soft, do not select
         // a new one.
         select=FALSE;
         if(bigstuff_fake!=0)
            trip=bigstuff_fake;
         else
            trip=ID2TRIP(id);
         if(TRIP2CL(trip)==CLASS_SOFTWARE && TRIP2SC(trip)==SOFTWARE_SUBCLASS_OFFENSE
            && !player_struct.softs.combat[player_struct.actives[ACTIVE_COMBAT_SOFT]]) {
            select=TRUE;
         }
         if ((bigstuff_fake != 0) || (USE_MODE(id) == PICKUP_USE_MODE && inventory_add_object(id, select)))
         {
            ObjLocState del_loc_state;
            int version=0;
            // yank the object out of the map. 
            del_loc_state.obj = id;
            del_loc_state.loc = objs[id].loc;
            del_loc_state.loc.x = -1;
            ObjRefStateBinSetNull(del_loc_state.refs[0].bin);
            ObjUpdateLocs(&del_loc_state);

            if (bigstuff_fake != 0)
            {
               inventory_add_object(id, select);
#ifdef SWITCH_BY_COLLIDE
               switch (objBigstuffs[objs[id].specID].data1)
               {
                     case SOFTWARE_SUBCLASS_OFFENSE: player_struct.actives[ACTIVE_COMBAT_SOFT] = objBigstuffs[objs[id].specID].data2; break;
                     case SOFTWARE_SUBCLASS_DEFENSE: player_struct.actives[ACTIVE_DEFENSE_SOFT] = objBigstuffs[objs[id].specID].data2; break;
               }
#endif
               get_object_short_name(bigstuff_fake, str_buf, 40);
               version=objBigstuffs[objs[id].specID].cosmetic_value;
            }
            else
            {
               // set it to be the "active" object under certain circumstances
               if (objs[id].obclass == CLASS_SOFTWARE)
               {
                  switch(objs[id].subclass)
                  {
#ifdef SWITCH_BY_COLLIDE
                     case SOFTWARE_SUBCLASS_OFFENSE: player_struct.actives[ACTIVE_COMBAT_SOFT] = objs[id].info.type; break;
                     case SOFTWARE_SUBCLASS_DEFENSE: player_struct.actives[ACTIVE_DEFENSE_SOFT] = objs[id].info.type; break;
#endif
                     case SOFTWARE_SUBCLASS_DATA:
                        string_message_info(REF_STR_CspaceData);
                        return(OK);
                  }
               }
               get_object_short_name(ID2TRIP(id), str_buf, 40);
               version=objSoftwares[objs[id].specID].version;
            }
            if(version && trip==GAMES_TRIPLE) {
               int game=0;
               while((version&1)==0) game++, version=version>>1;
               version=0;
               lg_sprintf(str_buf,"\"%S\" %S",REF_STR_GameName0+game,MKREF(RES_objshortnames,OPTRIP(GAMES_TRIPLE)));
            }
            if(version) {
               lg_sprintf(str_buf+strlen(str_buf)," %S%d",REF_STR_VersionPrefix,version);
            }
            strcat(str_buf, get_string(REF_STR_CspaceAcquire, temp,20));
            if(shameful_obselete_flag)
               string_message_info(REF_STR_AlreadyHaveOne);
            else
               message_info(str_buf);
         }
         break;
   }
   return(OK);
}
