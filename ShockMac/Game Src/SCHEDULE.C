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
 * $Source: r:/prj/cit/src/RCS/schedule.c $
 * $Revision: 1.60 $
 * $Author: xemu $
 * $Date: 1994/11/22 15:39:20 $
 *
 *
 */

#include "map.h"
#include "player.h"
#include "schedule.h"
#include "grenades.h"
#include "wares.h"
#include "objsim.h"
#include "objclass.h"
#include "otrip.h"
#include "faketime.h"
#include "trigger.h"
#include "objwpn.h"
#include "combat.h"
#include "objgame.h"
#include "objuse.h"
#include "gamesys.h"
#include "ai.h"
#include "frparams.h"
#include "damage.h"
#include "mainloop.h"
#include "colors.h"
#include "doorparm.h"
#include "mfdext.h"

#include "musicai.h"    // for the explosion
#include "sfxlist.h"
#include "effect.h"
#include "objprop.h"

#include "frflags.h"
#include "frprotox.h"


// this will need to be initialized.
height_semaphor h_sems[NUM_HEIGHT_SEMAPHORS];

// ---------
// INTERNALS
// ---------
int expand_tstamp(ushort s);
int compare_tstamps(ushort t1, ushort t2);
int compare_events(void* e1, void* e2);

bool register_h_event(uchar x, uchar y, bool floor, char* sem, char* key, bool no_sfx);
void unregister_h_event(char sem);

void null_event_handler(Schedule* s, SchedEvent* ev);
void trap_event_handler(Schedule* s, SchedEvent *ev);
void height_event_handler(Schedule* s, SchedEvent *ev);
void door_event_handler(Schedule* s,SchedEvent *ev);
void grenade_event_handler(Schedule* s,SchedEvent *ev);
void explosion_event_handler(Schedule* s,SchedEvent *ev);
void exposure_event_handler(Schedule* s,SchedEvent* ev);
void light_event_handler(Schedule* s,SchedEvent *ev);
void bark_event_handler(Schedule* s, SchedEvent *ev);
void email_event_handler(Schedule* s, SchedEvent* ev);

void reset_schedules(void);


// comparison function.  
// 8/27/94 THIS FIX FOR WRAPPING BUG MAKES THE COMPARE FUNCTION MUCH LESS GENERAL.
// I.e. it relies on the fact that timestamps are in terms of TICKS2TSTAMP(player_struct.game_time)
// in the future, this should be fixed by adding more bits to the timestamp so it doesn't wrap, and
// then you can have schedule that don't go on gametime. 


#define MAX_USHORT (0xFFFF)

int expand_tstamp(ushort s)
{
   int gametime = TICKS2TSTAMP(player_struct.game_time);
   int stamp = s;
   if (stamp   <= gametime - (MAX_USHORT/2))
      stamp += MAX_USHORT;
   else if (stamp >= gametime + (MAX_USHORT/2))
      stamp -= MAX_USHORT;
   return stamp;
}

int compare_tstamps(ushort t1, ushort t2)
{
   return expand_tstamp(t1) - expand_tstamp(t2);
}

int compare_events(void* e1, void* e2)
{
   return compare_tstamps(((SchedEvent *)e1)->timestamp, ((SchedEvent *)e2)->timestamp);
}



// -----------------
// SUPPORT FUNCTIONS
// -----------------

bool register_h_event(uchar x, uchar y, bool floor, char* sem, char* key, bool no_sfx)
{
   int i,fr;

   fr=-1;
   for(i=0;i<NUM_HEIGHT_SEMAPHORS;i++) {
      if (h_sems[i].inuse == 0)
         fr=i;
      else if(h_sems[i].x==x &&
         h_sems[i].y==y &&
         h_sems[i].floor==floor) {

         // conflict.  Take over the old semaphor.
         if(h_sems[i].key<MAX_HSEM_KEY)
            h_sems[i].key++;
         else
            h_sems[i].key=0;

         *key=h_sems[i].key;
         *sem=i;
         if (!no_sfx)
         {
            h_sems[i].inuse=2;
            if (play_digi_fx_loc(SFX_TERRAIN_ELEV_LOOP,-1,x << 8, y << 8) < 0)
               h_sems[i].inuse = 1;
         }
         else
            h_sems[i].inuse = 1;
         return(TRUE);
      }
   }
   // no conflict.  Allocate new semaphor.
   if(fr<0)
   {
      // no free semaphor.  Fail.
      return(FALSE);
   }
   else {
      h_sems[fr].x=x;
      h_sems[fr].y=y;
      h_sems[fr].floor=floor;
      h_sems[fr].key=0;
      *key=h_sems[fr].key;
      *sem=fr;
      if (!no_sfx)
      {
         h_sems[fr].inuse=2;
         if (play_digi_fx_loc(SFX_TERRAIN_ELEV_LOOP,-1,x << 8, y << 8) < 0)
            h_sems[fr].inuse=1;
      }
      else
         h_sems[fr].inuse=1;
      return(TRUE);
   }
}

void unregister_h_event(char sem)
{
   if(sem<0 || sem>=NUM_HEIGHT_SEMAPHORS) return;
   h_sems[sem].inuse--;
}


// --------------
// EVENT HANDLERS
// --------------

void null_event_handler(Schedule*, SchedEvent*)
{
}

void trap_event_handler(Schedule*, SchedEvent *ev)
{
   ObjID    id1,id2;
   bool dummy;

   id1 = ((TrapSchedEvent *) ev)->target_id;
   id2 = ((TrapSchedEvent *) ev)->source_id;

   do_multi_stuff(id1);
   if (id2 != -1)
      trap_activate(id2, &dummy);
}

#define FLOOR_HEIGHT_DELTA    0x4
void height_event_handler(Schedule*, SchedEvent *ev)
{
   MapElem *pme;
   HeightSchedEvent hse = *(HeightSchedEvent *)ev;
   extern void rendedit_process_tilemap(FullMap* map,LGRect* r,bool newMap);
   short x,y;
   LGRect bounds;
   char ht, sign=(hse.steps_remaining>0)?1:-1;

   // has someone else claimed this square?
   if(h_sems[hse.semaphor].key!=hse.key)
   {
      return;
   }
   x=h_sems[hse.semaphor].x;
   y=h_sems[hse.semaphor].y;
   pme = MAP_GET_XY(x,y);
   // look at top bit of step_size to determine floor or ceiling
   if (hse.type == CEIL_SCHED_EVENT)
   {
      ht=me_height_ceil(pme);
      me_height_ceil_set(pme, ht+sign );
   }
   else
   {
      ObjRefID oref;
      ObjID id;
      ObjLoc newloc;
      ht=me_height_flr(pme);
      // Change height of objects, as well...
      oref = me_objref(pme);
      while (oref != OBJ_REF_NULL)
      {
         // If we are on the old height, move us to the new height
         id = objRefs[oref].obj;
         if (abs(obj_floor_height(id) - objs[id].loc.z) < FLOOR_HEIGHT_DELTA)
         {
            if (id == PLAYER_OBJ)
            {
               extern void slam_posture_meter_state(void);
               slam_posture_meter_state();
            }
            else if (ObjProps[OPNUM(id)].physics_model)
            {
               newloc = objs[id].loc;
               newloc.z = obj_floor_compute(id, ht+sign);
               obj_move_to(id, &newloc, TRUE);
            }
         }
         // iterate
         oref = objRefs[oref].next;
      }
      // Crank us to new height;
      me_height_flr_set(pme, ht+sign );
   }

   hse.steps_remaining-=sign;
   if (hse.steps_remaining != 0)
   {
      hse.timestamp = TICKS2TSTAMP(player_struct.game_time + (CIT_CYCLE * HEIGHT_STEP_TIME)/HEIGHT_TIME_UNIT);
      schedule_event(&(global_fullmap->sched[MAP_SCHEDULE_GAMETIME]), (SchedEvent *)&hse);
   }
   else {
      unregister_h_event(hse.semaphor);
   }


   {
      bounds.ul.x = bounds.lr.x = x;
      bounds.ul.y = bounds.lr.y = y;
      rendedit_process_tilemap(global_fullmap, &bounds, FALSE);
   }
}

#define ANTENNA_DESTROYED_QVAR   0x2
#define ANTENNAE_ALL_GONE_QBIT   0x99
#define PLOTWARE_QVAR            0x9
#define NUM_ANTENNAE_TO_DESTROY  4

void door_event_handler(Schedule*,SchedEvent *ev)
{
   ObjID    id;
   short old_dest;
   ushort code, curr_code;
   extern bool door_moving(ObjID id,bool dir);

   id = ((DoorSchedEvent *) ev)->door_id;
   code = ((DoorSchedEvent *) ev)->secret_code;
   if (objs[id].obclass == CLASS_DOOR)
   {
      // construct code out of top 2 bits of inst_flags
      curr_code = objs[id].info.inst_flags >> 6;

      // Make sure that we actually care about this autoclose event
      if (code == curr_code)
      {
         // note the secret dont-autoclose-me-even-if-I-already-
         // have-an-autoclose-scheduled cookie.
         if (!(DOOR_REALLY_CLOSED(id) || door_moving(id,TRUE)
               || objDoors[objs[id].specID].autoclose_time==NEVER_AUTOCLOSE_COOKIE))
            object_use(id, FALSE, OBJ_NULL);
      }
   }
   else if (ID2TRIP(id) == PLAS_ANTENNA_TRIPLE)
   {
      ObjID ground0, p3obj;
      ObjLoc blastLoc;
      ExplosionData* kaboom;
      extern short fr_sfx_time;

      // An earth-shattering kaboom.
      // turn the panel into a destroyed one
      objs[id].info.type += 1;

      // flash the screen
      fr_global_mod_flag(FR_SOLIDFR_SLDCLR, FR_SOLIDFR_MASK);
      fr_solidfr_color=GRENADE_COLOR;

      // shake yer bootie
      fr_global_mod_flag(FR_SFX_SHAKE, FR_SFX_MASK);
      fr_sfx_time = CIT_CYCLE << 1;  // 2 seconds of shake

      // qvar tricks
      old_dest = QUESTVAR_GET(ANTENNA_DESTROYED_QVAR);
      QUESTVAR_SET(ANTENNA_DESTROYED_QVAR,  old_dest + 1);
      if (old_dest + 1 >= NUM_ANTENNAE_TO_DESTROY)
      {
         QUESTVAR_SET(PLOTWARE_QVAR, QUESTVAR_GET(PLOTWARE_QVAR) + 1);
         QUESTBIT_ON(ANTENNAE_ALL_GONE_QBIT);
         do_multi_stuff(objFixtures[objs[id].specID].p1);
      }

      kaboom=&game_explosions[LARGE_GAME_EXPL];
      p3obj=ground0=objFixtures[objs[id].specID].p3;
      if(ground0==OBJ_NULL) ground0=id;
      ObjLocCopy(objs[ground0].loc,blastLoc);
      blastLoc.z=obj_height_from_fix(fix_from_obj_height(ground0)+4*RAYCAST_ATTACK_SIZE);
      if(p3obj) {
         ADD_DESTROYED_OBJECT(p3obj);
         destroy_destroyed_objects();
      }
      do_explosion(blastLoc, ground0, M_EXPL2, kaboom);
      fr_global_mod_flag(FR_SFX_SHAKE, FR_SFX_MASK);
      fr_sfx_time = CIT_CYCLE << 1;
      play_digi_fx_obj(SFX_EXPLOSION_1,1,id);
      mfd_notify_func(MFD_PLOTWARE_FUNC,MFD_ITEM_SLOT,FALSE,MFD_ACTIVE,TRUE);
   }
}

void grenade_event_handler(Schedule*,SchedEvent *ev)
{
   ObjID    id;
   ubyte    unique_id;

   id = ((GrenSchedEvent *) ev)->gren_id;

   if (is_obj_destroyed(id))
      return;

   // make sure we have a real grenade
   unique_id = objGrenades[objs[id].specID].unique_id;
   if ((((GrenSchedEvent *) ev)->unique_id == unique_id) && unique_id)
   {
      if (GrenadeProps[CPNUM(id)].flags & GREN_TIMING_TYPE)
         ADD_DESTROYED_OBJECT(id);
//         do_grenade_explosion(id, TRUE);
      else
         EDMS_obey_collisions(objs[id].info.ph);
   }
}

void explosion_event_handler(Schedule*, SchedEvent *)
{
}

void exposure_event_handler(Schedule* s,SchedEvent* ev)
{
   extern void expose_player(byte damage, ubyte type, ushort tsecs);
   SchedExposeData *xd = (SchedExposeData*)&ev->data;
   int count = xd->count;
   int damage = xd->damage;
   if (damage < 0)
      damage = (damage - 1)/2;
   else damage = (damage + 1)/2;
   expose_player(damage,xd->type,0);
//   count --;
   if (damage != xd->damage)
//      if (count > 0)
   {
      SchedEvent copy;
      xd->damage -= damage;
      xd->count = (ubyte)count;
      copy = *ev;
      copy.timestamp += xd->tsecs;
      schedule_event(s,&copy);
   }
}

extern bool muzzle_fire_light;
extern void lamp_turnon(bool visible, bool real);
extern void lamp_turnoff(bool visible, bool real);

void light_event_handler(Schedule*, SchedEvent *)
{
   muzzle_fire_light = FALSE;

   // is the player's lantern on - if not turn our faux lantern off, otherwise
   // just turn the lantern on - to reset its values
   if (!(player_struct.hardwarez_status[CPTRIP(LANTERN_HARD_TRIPLE)] & WARE_ON))
      lamp_turnoff(TRUE,FALSE);
   else
      lamp_turnon(TRUE,FALSE);
}

void bark_event_handler(Schedule*, SchedEvent *)
{
   ubyte mfd_id;
   void check_panel_ref(bool puntme);
   ubyte mfd_get_func(ubyte mfd_id,ubyte s);

   // timeout bark, if it's still there at all.
   for(mfd_id=0;mfd_id<NUM_MFDS;mfd_id++) {
      if (mfd_get_func(mfd_id,MFD_INFO_SLOT)==MFD_BARK_FUNC) {
         check_panel_ref(TRUE);
         break;
      }
   }
}

void email_event_handler(Schedule*, SchedEvent* ev)
{
   extern void add_email_datamunge(short mung, bool select);
   EmailSchedEvent *e = (EmailSchedEvent*)ev;
   add_email_datamunge(e->datamunge,TRUE);
}



// HERE IS THE ARRAY OF ALL EVENT HANDLERS 

static SchedHandler sched_handlers[] =
{
   null_event_handler,
   grenade_event_handler,
   explosion_event_handler,
   door_event_handler,
   trap_event_handler,
   exposure_event_handler,
   height_event_handler,
   height_event_handler,
   light_event_handler,
   bark_event_handler,
   email_event_handler,
};

#define NUM_EVENT_TYPES (sizeof(sched_handlers)/sizeof(SchedHandler))


// ---------
// EXTERNALS
// ---------

static ushort current_tstamp = 0;

errtype schedule_init(Schedule* s, int size, bool grow)
{
   return pqueue_init(&s->queue,size,sizeof(SchedEvent),compare_events,grow);
}

errtype schedule_free(Schedule* s)
{
   return pqueue_destroy(&s->queue);
}

errtype schedule_event(Schedule* s, SchedEvent *ev)
{
   errtype retval = OK;
   if (!time_passes)
      return ERR_NOEFFECT;
   if (current_tstamp > 0 && compare_tstamps(ev->timestamp,current_tstamp) < 0)
   {
      return ERR_NOEFFECT;
   }
   retval = pqueue_insert(&s->queue,ev);
   if (retval != OK)
   {
   }
   return retval;
}

errtype schedule_reset(Schedule* s)
{
   s->queue.fullness = 0;
   return OK;
}


void reset_schedules(void)
{
   int i;
   for (i = 0; i < NUM_MAP_SCHEDULES; i++)
      schedule_reset(&global_fullmap->sched[i]);
   schedule_reset(&game_seconds_schedule);
}

errtype schedule_run(Schedule* s, ushort time)
{
   SchedEvent ev;
   errtype err;
   current_tstamp = time;
   for(err = pqueue_least(&s->queue,&ev);
       err == OK && compare_tstamps(ev.timestamp,time) < 0;
       err = pqueue_least(&s->queue,&ev))
   {
      if (ev.type < NUM_EVENT_TYPES)
         sched_handlers[ev.type](s,&ev);
      pqueue_extract(&s->queue,&ev);
   }
   current_tstamp = 0;
   return OK;
}


void run_schedules(void)
{
   schedule_run(&global_fullmap->sched[MAP_SCHEDULE_GAMETIME],TICKS2TSTAMP(player_struct.game_time));
   schedule_run(&game_seconds_schedule,TICKS2TSTAMP(player_struct.game_time));
}

/*���
bool schedule_test_hotkey(short keycode, ulong context, void* data)
{
   SchedEvent e;
#ifndef NO_DUMMIES
   int dummy; dummy = keycode + context + (int)data;
#endif 
   e.timestamp = player_struct.game_time/CIT_CYCLE + 100;
   e.type = 0;
   schedule_event(&game_seconds_schedule,&e);
   return TRUE;
}
*/
