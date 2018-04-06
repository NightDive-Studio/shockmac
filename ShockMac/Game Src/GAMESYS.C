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
 * $Source: r:/prj/cit/src/RCS/gamesys.c $
 * $Revision: 1.128 $
 * $Author: xemu $
 * $Date: 1994/11/28 06:42:41 $
 *
 */

#include <stdlib.h>                      

#include "ai.h"
#include "combat.h"  		// for ADD_DESTROYED_OBJECT
#include "cyber.h"
#include "cybstrng.h"
#include "damage.h"
#include "diffq.h" 			// for time limit
#include "drugs.h"
#include "effect.h"
#include "faketime.h"
#include "frflags.h"
#include "frprotox.h"
#include "fullscrn.h"
#include "game_screen.h"
#include "gameloop.h"
#include "gameobj.h"
#include "gamerend.h"
#include "gamesys.h"
#include "gametime.h"
#include "hud.h"
#include "lvldata.h"
#include "mainloop.h"
#include "map.h"
#include "mfdint.h"
#include "miscqvar.h"
#include "musicai.h"
#include "newmfd.h"
#include "objbit.h"
#include "objcrit.h"
#include "objects.h"
#include "objgame.h"
#include "objprop.h"
#include "objsim.h"
#include "objstuff.h"
#include "objuse.h"
#include "otrip.h"
#include "palfx.h"
#include "pathfind.h"
#include "player.h"
#include "physics.h"
#include "rendfx.h"
#include "schedule.h"
#include "sfxlist.h"
#include "shodan.h"
#include "tools.h"
#include "tpolys.h"
#include "trigger.h"
#include "visible.h"
#include "wares.h"
#include "weapons.h"


// -------
// DEFINES
// -------
#define SECOND_UPDATE_FREQ APPROX_CIT_CYCLE_HZ
#define HEALTH_RESTORE_PRECISION (APPROX_CIT_CYCLE_SHFT)
#define HEALTH_RESTORE_SHF  (6)
#define HEALTH_RESTORE_UNIT (1 << HEALTH_RESTORE_SHF)       // unit time for hp/energy regen
#define HEALTH_RESTORE_MASK (HEALTH_RESTORE_UNIT - 1)
#define sqr(x) ((x)*(x))
#define MAX_FATIGUE 10000
#define FATIGUE_WARNING_LEVEL 7000


// -------
// GLOBALS
// -------
int run_fatigue_rate = 5;
extern short fr_solidfr_time;
extern short fr_sfx_time;
ulong fr_shake_time;
bool gamesys_on = TRUE;

// hud vars
short enviro_edrain_rate = 0;
short enviro_absorb_rate = 0;

LevelData level_gamedata;
Schedule game_seconds_schedule;

// prototypes
void game_sched_init(void);
void game_sched_free(void);
void unshodanizing_callback(ObjID id, void *user_data);
void check_nearby_objects(void);
void fatigue_player(void);
bool shodan_phase_in(uchar *bitmask, short x, short y, short w, short h, short num, bool dir);
void check_hazard_regions(MapElem* newElem);
bool panel_ref_sanity(ObjID obj);
void check_panel_ref(bool puntme);
int apply_rate(int var, int rate, int t0, int t1, int vmin, int vmax);
void do_stuff_every_second(void);
void expose_player_real(short damage, ubyte type, ushort tsecs);
void expose_player(byte damage, ubyte type, ushort tsecs);


void game_sched_init(void)
{
   schedule_init(&game_seconds_schedule,GAME_SCHEDULE_SIZE,FALSE);
}

void game_sched_free(void)
{
   extern errtype schedule_free(Schedule* s);
   schedule_free(&game_seconds_schedule);
}

short fatigue_accum_rate = 100;

#define OBJ_CHECK_TICKS    CIT_CYCLE
ulong obj_check_time = 0;

#define MACHINE_LAYER_BASE    25
#define OBJ_CHECK_RADIUS  7

void unshodanizing_callback(ObjID id, void *user_data)
{
   bool ud = (bool)user_data;
   if (ud)
   {
      objBigstuffs[objs[id].specID].data2 = SHODAN_STATIC_MAGIC_COOKIE | (TPOLY_TYPE_CUSTOM_MAT << TPOLY_INDEX_BITS);
      objBigstuffs[objs[id].specID].cosmetic_value = 0;
      objs[id].info.current_frame = 0;
   }
   else
      add_obj_to_animlist(id, FALSE, TRUE, FALSE, 0, 3, (void *)TRUE, ANIMCB_REMOVE);
}

#define STOCHASTIC_SHODAN_MASK   0xF
#define STOCHASTIC_MONSTER_MASK  0xF

#define STOCHASTIC_SPARKING_MASK    0xFF
#define STOCHASTIC_SPARKING_LEVEL   0x10
#define SPARKING_RADIUS          5

#define STOCHASTIC_FROG_MASK    0xF
#define STOCHASTIC_FROG_LEVEL   0x2
#define FIRST_GROVE_LEVEL       11

#define NEAR_NOISE_RADIUS  5
#define PERIL_RADIUS       4

#define MONSTER_THEME_PATH_LENGTH   12   


void check_nearby_objects()
{
   short x, y;
   int dist,best_dist;
   ObjRefID oref;
   int trip;
   extern char mlimbs_machine;
   extern int mlimbs_monster;
   int new_monster;
   extern short compute_3drep(Obj *cobj, ObjID cobjid, int obj_type);
#ifdef USE_3DREP_FOR_SHODANIZING
   short rep;
#endif
   ObjID id;
   ObjSpecID osid;
   LGPoint dest_pt, source_pt;
   int pf_id;
   extern bool music_on;

   // Should probably distribute different kinds of checks so that there is not a big hit every OBJ_CHECK_TIME
   if (obj_check_time < player_struct.game_time)
   {
      mlimbs_machine = 0;
      mlimbs_peril = 0;
      best_dist = OBJ_CHECK_RADIUS + 1;
      source_pt.x = OBJ_LOC_BIN_X(objs[PLAYER_OBJ].loc);
      source_pt.y = OBJ_LOC_BIN_Y(objs[PLAYER_OBJ].loc);
      new_monster = -1;
      for (x = PLAYER_BIN_X - OBJ_CHECK_RADIUS; x < PLAYER_BIN_X + OBJ_CHECK_RADIUS; x++)
      {
         for (y = PLAYER_BIN_Y - OBJ_CHECK_RADIUS; y < PLAYER_BIN_Y + OBJ_CHECK_RADIUS; y++)
         {
            // Make sure we ain't lookin' off the edge of the map...
            if ((x < 0) || (x >= MAP_XSIZE) || (y < 0) || (y >= MAP_YSIZE))
               continue;

            // Hmm, I should probably modularize out the individual kinds of checks....
            oref = me_objref(MAP_GET_XY(x,y));
            while (oref != OBJ_REF_NULL)
            {
               id = objRefs[oref].obj;
               dist = long_fast_pyth_dist(x - PLAYER_BIN_X, y - PLAYER_BIN_Y);
               if (id != PLAYER_OBJ)
               {
                  osid = objs[id].specID;
                  trip = ID2TRIP(id);
                  switch(objs[id].obclass)
                  {
                     case CLASS_CRITTER:
                        if (music_on)
                        {
                           if ((new_monster == -1) && (dist < best_dist))
                           {
                              // Boy, I wonder how gruesomely slow this is going to be...
                              dest_pt.x = OBJ_LOC_BIN_X(objs[id].loc);
                              dest_pt.y = OBJ_LOC_BIN_Y(objs[id].loc);
                              pf_id = -1;
                              if (pf_id != -1)
                              {
                                 check_requests(TRUE);
                                 if ((paths[pf_id].num_steps != 0) && (paths[pf_id].num_steps <= MONSTER_THEME_PATH_LENGTH))
                                 {
                                    delete_path(pf_id);
                                    pf_id = -1;
                                 }
                              }
                              // go ahead if we couldn't allocate a path or the path was sufficiently short
                              if (pf_id == -1)
                              {
                                 switch (ID2TRIP(id))
                                 {
                                    case SERVBOT_TRIPLE:
                                    case REPAIRBOT_TRIPLE:
                                    case REPAIRBOT2_TRIPLE:
                                    case FLIER_TRIPLE:
                                    case AUTOBOMB_TRIPLE:
                                       new_monster = MONSTER_MUSIC_SMALL_ROBOT;
                                       break;
                                    default:
                                       new_monster = objs[id].subclass;
                                       break;
                                 }
                                 best_dist = dist;
                              }
                              else 
                              {
                                 delete_path(pf_id);
                                 pf_id = -1;
                              }
                           }
                           if ((dist < PERIL_RADIUS) && 
                              (objCritters[osid].mood != AI_MOOD_FRIENDLY) && (objCritters[osid].orders != AI_ORDERS_SLEEP))
                           {
                              mlimbs_peril = DEFAULT_PERIL_MAX;
                           }
                        }
                        if ((dist < NEAR_NOISE_RADIUS) &&
                           (CritterProps[CPTRIP(trip)].near_sound != 255) &&
                           (objCritters[osid].mood != AI_MOOD_ATTACKING) &&
                           (get_crit_posture(osid) != DEATH_CRITTER_POSTURE) &&
                           ((rand() & STOCHASTIC_MONSTER_MASK) == 1))
                              play_digi_fx_obj(CritterProps[CPTRIP(trip)].near_sound, 1, objCritters[osid].id);
                        break;
                  }
                  switch(trip)
                  {
                     case MUSIC_MARK_TRIPLE:
                        // Hm, should we have this use comparator_check
                        if (player_struct.level >= FIRST_GROVE_LEVEL)
                        {
                           if ((rand() & STOCHASTIC_FROG_MASK) < STOCHASTIC_FROG_LEVEL)
                              play_digi_fx_obj(SFX_GROVE_1,1,id);
                        }
                        else
                           mlimbs_machine = MACHINE_LAYER_BASE + objTraps[objs[id].specID].p1;
                        break;
                     case SPARK_CABLE_TRIPLE:
                        if ((dist < SPARKING_RADIUS) && ((rand() & STOCHASTIC_SPARKING_MASK) < STOCHASTIC_SPARKING_LEVEL))
                           play_digi_fx_obj(SFX_SPARKING_CABLE,1,id);
                        break;
                     case HORZ_KLAXON_TRIPLE:
                        {
                           bool digi_fx_playing(int fx_id, int *handle_ptr);
                           if (!digi_fx_playing(SFX_KLAXON,NULL))
                              play_digi_fx_obj(SFX_KLAXON,1,id);
                        }
                        break;
                     case TV_TRIPLE:
                     case MONITOR2_TRIPLE:
                     case SCREEN_TRIPLE:
                     case BIGSCREEN_TRIPLE:
                     case SUPERSCREEN_TRIPLE:
   #ifdef USE_3DREP_FOR_SHODANIZING
                        rep = compute_3drep(&(objs[id]), id, ObjProps[OPNUM(id)].render_type);
                        if ((rep & TPOLY_INDEX_MASK) == SHODAN_STATIC_MAGIC_COOKIE)
   #else
                        if (((objBigstuffs[objs[id].specID].data2 & TPOLY_INDEX_MASK) == SHODAN_STATIC_MAGIC_COOKIE) &&
                           (((objBigstuffs[objs[id].specID].data2 & TPOLY_TYPE_MASK) >> TPOLY_INDEX_BITS) == TPOLY_TYPE_CUSTOM_MAT))
   #endif
                        {
                           // Chance of shodanizing....
                           if ((rand() & STOCHASTIC_SHODAN_MASK) == 1)
                           {
                              objBigstuffs[objs[id].specID].data2 = FIRST_SHODAN_ANIM;
                              objBigstuffs[objs[id].specID].cosmetic_value = NUM_SHODAN_FRAMES;
                              objs[id].info.current_frame = 0;
                              // animate me:  cycle, but don't repeat   
                              add_obj_to_animlist(id, FALSE, FALSE, FALSE, 0, 3, (void *)0, ANIMCB_REMOVE);
                           }
                        }
                        break;
                  }
               }
               oref = objRefs[oref].next;
            }
         }
      }
      mlimbs_monster = new_monster;
      obj_check_time = player_struct.game_time + OBJ_CHECK_TICKS;
   }
}

/*���
#define CFG_FATIGUE_VAR "fatigue"
extern ubyte fatigue_threshold;
void reload_fatigue_parms()
{
   int i;
   int vec[5];
   i = 5;
//   player_struct.fatigue_regen = DEFAULT_FATIGUE_REGEN;
   if (config_get_value(CFG_FATIGUE_VAR,CONFIG_INT_TYPE,vec,&i))
   {
      switch (i)
      {
      case 5:
         fatigue_accum_rate                  = vec[4];
      case 4:
         player_struct.fatigue_regen_max     = vec[3];
      case 3:
         player_struct.fatigue_regen_base    = vec[2];
      case 2:
         fatigue_threshold                   = vec[1];
      case 1:
         run_fatigue_rate                    = vec[0];
      default:
         break;
      }
   }

}
*/

bool fatigue_warning;
#define fatigue_val(x) (((x) > SPRINT_CONTROL_THRESHOLD) ? ((int)(x) - SPRINT_CONTROL_THRESHOLD): 0)
#define FATIGUE_DENOM  (CONTROL_MAX_VAL - SPRINT_CONTROL_THRESHOLD)
bool gamesys_fatigue = TRUE;

#define SKATE_MOD 8

extern int EDMS_pelvis_is_climbing;


void fatigue_player(void)
{
   byte* c = player_struct.controls;
   int deltat,deltaf;
   extern bool jumpjets_active;
   if (gamesys_fatigue && !jumpjets_active && !EDMS_pelvis_is_climbing)
   {
      deltat = player_struct.deltat;
      deltaf = run_fatigue_rate*(fatigue_val(c[CONTROL_YVEL]) +
                     fatigue_val(2*c[CONTROL_ZVEL]) + 
   //              fatigue_val(c[CONTROL_XVEL])/64 +
   //              fatigue_val(c[CONTROL_XYROT])/256 +
   //              fatigue_val(c[CONTROL_XZROT])/256 +
   //              fatigue_val(c[CONTROL_YZROT])/256 +
               0);
      if (player_struct.posture != POSTURE_STAND)
         deltaf /= sqr(player_struct.posture+1);
      if (motionware_mode == MOTION_SKATES)
         deltaf /= SKATE_MOD;
      player_struct.fatigue += deltaf*deltat/FATIGUE_DENOM + player_struct.fatigue_spend*deltat;
      if (player_struct.fatigue > MAX_FATIGUE) player_struct.fatigue = MAX_FATIGUE;
      if (player_struct.drug_status[DRUG_STAMINUP] <= 0 && player_struct.fatigue > FATIGUE_WARNING_LEVEL) 
      {
         if (!fatigue_warning)
         {
            hud_set(HUD_FATIGUE);
            fatigue_warning = TRUE;
         }
      }
      else if (fatigue_warning)
      {
         hud_unset(HUD_FATIGUE);
         fatigue_warning = FALSE;
      }
   }
}



bool gamesys_render_fx = TRUE;
bool gamesys_restore_health = TRUE;
bool gamesys_slow_proj = TRUE;
bool gamesys_beam_wpns = TRUE;
bool gamesys_drugs = TRUE;

ulong next_contin_trig;

#define NUM_CONTIN_SECONDS    5
#define CONTIN_INTERVAL       CIT_CYCLE * NUM_CONTIN_SECONDS

short fr_surge_time = 0;
char surg_fx_frame = 0;
short surge_duration = 60;

#define NUM_SURG_FX_FRAMES 7
short surge_vals[NUM_SURG_FX_FRAMES] = { -1 << 8 , -5 << 8, 0 << 8, 2 << 8, 2 << 8, 1 << 8, 1 << 8 };

#define CONQUER_THRESHOLD     512
#define UNCONQUER_THRESHOLD   32
#define MAX_SHODAN_FAILURES   10
char thresh_fail = 0;
bool shodan_phase_in(uchar *bitmask, short x, short y, short w, short h, short num, bool dir)
{
   int i = 0,nx,ny,val,oval;
   while (i < num)
   {
      nx = rand() % w;
      ny = rand() % h;
      val = x + (y * FULL_VIEW_WIDTH);
      val += (nx + (ny * FULL_VIEW_WIDTH));
      oval = val;

      if (dir)
      {
         while (SHODAN_CONQUER_GET(bitmask,val) && (val < SHODAN_BITMASK_SIZE) && (val - oval <= CONQUER_THRESHOLD))
            val++;
         if (val < SHODAN_BITMASK_SIZE)
            SHODAN_CONQUER_SET(bitmask,val);
         if (val - oval > CONQUER_THRESHOLD)
         {
            i = num;
            thresh_fail++;
         }
      }
      else
      {
         while (!SHODAN_CONQUER_GET(bitmask,val) && (val < SHODAN_BITMASK_SIZE) && (val - oval <= UNCONQUER_THRESHOLD))
            val++;
         if (val < SHODAN_BITMASK_SIZE)
            SHODAN_CONQUER_UNSET(bitmask,val);
         if (val - oval > UNCONQUER_THRESHOLD)
            i = num;
      }
      i++;
   }
   if (thresh_fail > MAX_SHODAN_FAILURES)
   {
      return(TRUE);
   }
   return(FALSE);
}


#define NUM_SHODAN_REGIONS 4
short shodan_region_full_x[] = { 0, 0, FULL_VIEW_WIDTH * 7 / 8, 0 };
short shodan_region_full_y[] = { 0, 0, 0, FULL_VIEW_HEIGHT * 7 / 8 };
short shodan_region_full_width[] = { FULL_VIEW_WIDTH, FULL_VIEW_WIDTH / 8, FULL_VIEW_WIDTH / 8, FULL_VIEW_WIDTH};
short shodan_region_full_height[] = {FULL_VIEW_HEIGHT / 8, FULL_VIEW_HEIGHT, FULL_VIEW_HEIGHT, FULL_VIEW_HEIGHT / 8};

// stolen from trigger.c
#define GAME_OVER_HACK 0x6

#define KEY_CODE_ESC 0x1b

#define DETECT_AUDIOLOG_QVAR_CHANGE

errtype gamesys_run(void)
{
   ObjSpecID osi;
   bool dummy;
   extern void destroy_destroyed_objects(void);
   extern bool trap_activate(ObjID id, bool *use_message);
   extern void set_global_lighting(short new_val);
   extern uchar *shodan_bitmask;
   extern ulong page_amount;
#ifdef AUTOCORRECT_DIFF_TRASH
   int i;
#endif

#ifdef AUTOCORRECT_DIFF_TRASH
   for (i=0; i < 4; i++)
   {
      extern char diff_qvars[4];
      if (player_struct.difficulty[i] != QUESTVAR_GET(diff_qvars[i]))
         QUESTVAR_SET(diff_qvars[i],player_struct.difficulty[i]);
   }
#endif

//���   page_amount = 0;

   if (!gamesys_on)
      return(OK);

   if (gamesys_render_fx)
   {
      if (fr_solidfr_time > 0)
      {
         fr_solidfr_time -= player_struct.deltat;
         if (fr_solidfr_time <= 0)
            fr_global_mod_flag(0,FR_SOLIDFR_MASK);
      }

      if (fr_sfx_time > 0)
      {
         fr_sfx_time -= player_struct.deltat;
         if (fr_sfx_time <= 0)

         {
            fr_global_mod_flag(0,FR_SFX_MASK);
         }
      }
      if (fr_surge_time > 0)
      {
         fr_surge_time -= player_struct.deltat;
         if (fr_surge_time <= 0)
         {
            if (surg_fx_frame >= NUM_SURG_FX_FRAMES)
               fr_surge_time = 0;
            else
            {
               fr_surge_time = surge_duration;
               set_global_lighting(surge_vals[surg_fx_frame]);
            }
            surg_fx_frame++;
         }
      }
   }

   if (shodan_bitmask != NULL)
   {
      extern ulong time_until_shodan_avatar;
      if (player_struct.game_time > time_until_shodan_avatar)
      {
         char i;
         if (thresh_fail)
         {
            errtype trap_hack_func(int p1, int p2, int p3, int p4);
            extern void begin_shodan_conquer_fx(bool begin);
            begin_shodan_conquer_fx(FALSE);
            shodan_bitmask = NULL;
            trap_hack_func(GAME_OVER_HACK, 0, 0, 0);
            palfx_fade_down();
         }
         else
         {
            for (i=0; i < NUM_SHODAN_REGIONS; i++)
            {
               shodan_phase_in(shodan_bitmask, shodan_region_full_x[i], shodan_region_full_y[i], 
                  shodan_region_full_width[i], shodan_region_full_height[i],QUESTVAR_GET(CYBER_DIFF_QVAR) + 1,TRUE);
               shodan_phase_in(shodan_bitmask, 0, 0, FULL_VIEW_WIDTH, FULL_VIEW_HEIGHT, 
                  (3 * QUESTVAR_GET(CYBER_DIFF_QVAR)) + 1,TRUE);
            }
         }
         if (thresh_fail)
         {
            extern errtype mai_player_death();
            mai_player_death();
            time_until_shodan_avatar = player_struct.game_time + (CIT_CYCLE * 8);
         }
         else
            time_until_shodan_avatar = player_struct.game_time + SHODAN_INTERVAL;
      }
   }

   // update fatigue
   if (!global_fullmap->cyber)
   {

      fatigue_player();
      // update drug effects
      if (gamesys_drugs)
         drugs_update();

      // cool off all beam weapons
      if (gamesys_beam_wpns)
         cool_off_beam_weapons();
   }

   do_stuff_every_second();

   check_nearby_objects();

   // destroy old slow projectiles
   if (gamesys_slow_proj)
   {
      osi = objPhysicss[0].id;
      while (osi != OBJ_SPEC_NULL)
      {
         if (objPhysicss[osi].duration < player_struct.game_time)
         {
            ADD_DESTROYED_OBJECT(objPhysicss[osi].id);
         }
         osi = objPhysicss[osi].next;
      }
      destroy_destroyed_objects();
   }

   // Run continuous triggers
   if (player_struct.game_time > next_contin_trig)
   {
      osi = objTraps[0].id;
      while (osi != OBJ_SPEC_NULL)
      {
         if (ID2TRIP(objTraps[osi].id) == CONTIN_TRIG_TRIPLE)
            trap_activate(objTraps[osi].id, &dummy);
         osi = objTraps[osi].next;
      }
      next_contin_trig = player_struct.game_time + CONTIN_INTERVAL;
   }

   return(OK);
}


// ----------------------------------------
// check_hazard_regions()
//
// Checks to see if we're in a bio/radiation zone

void expose_player_real(short damage, ubyte type, ushort tsecs);


void check_hazard_regions(MapElem* newElem)
{
   fix hdiff = fix_from_obj_height(PLAYER_OBJ) - fix_from_map_height(me_height_flr(newElem));
   if (me_hazard_rad(newElem) > 0 && hdiff <= fix_make(level_gamedata.hazard.rad_h,0)/8)
   {
      short exp = (short)me_hazard_rad(newElem)*(short)level_gamedata.hazard.rad;
      exp -= (short)player_struct.hit_points_lost[RADIATION_TYPE-1];
      if (exp > 0) 
      {
         expose_player_real(exp,RADIATION_TYPE,0);
      }

      hud_set(HUD_RADIATION);
      if (rand()%0xFF < 0x80)
         play_digi_fx(SFX_RADIATION,1);
   }
   else hud_unset(HUD_RADIATION);

   if (!level_gamedata.hazard.zerogbio)
   {
      if (me_hazard_bio(newElem) > 0 && hdiff <= fix_make(level_gamedata.hazard.bio_h,0)/8)
      {
         short exp = me_hazard_bio(newElem)*level_gamedata.hazard.bio;
         exp -= player_struct.hit_points_lost[BIO_TYPE-1];
         if (exp > 0)
            expose_player_real(exp,BIO_TYPE,0);
         hud_set(HUD_BIOHAZARD);
      }
      else hud_unset(HUD_BIOHAZARD);
   }
   else
   {
      if (me_hazard_bio(newElem))
         hud_set(HUD_ZEROGRAV);
      else
         hud_unset(HUD_ZEROGRAV);
   }
   if ((player_struct.hud_modes & (HUD_RADIATION|HUD_BIOHAZARD|HUD_ENVIROUSE)) == 0)
   {
      enviro_edrain_rate = 0;
   }
}

#define Z_THRESHOLD FIX_UNIT

bool panel_ref_sanity(ObjID obj)
{
   int objtrip=OPNUM(obj), obj_type;
   obj_type=ObjProps[objtrip].render_type;
   if ((obj_type==FAUBJ_TPOLY)||(obj_type==FAUBJ_TEXBITMAP))
	   if(objs[obj].obclass==CLASS_FIXTURE)
      {
	      fixang obj_to_p, objh, delt;
	      fix dy,dx;
	
	      objh=fixang_from_phys_angle(phys_angle_from_obj(objs[obj].loc.h));
	      dy=fix_from_obj_coord(objs[PLAYER_OBJ].loc.y)-fix_from_obj_coord(objs[obj].loc.y);
	      dx=fix_from_obj_coord(objs[PLAYER_OBJ].loc.x)-fix_from_obj_coord(objs[obj].loc.x);
	
	      // x and y swapped here to transform coordinate system
	      obj_to_p=fix_atan2(dx,dy);
	      delt=(objh-FIXANG_PI/2)-obj_to_p;
	
//	      mprintf("Called with %d, locs %d %d and %d %d, got delt %x from %x and %x\n",
//	         obj,objs[PLAYER_OBJ].loc.x,objs[PLAYER_OBJ].loc.y,objs[obj].loc.x,objs[obj].loc.y,delt,objh,obj_to_p);
	
	      // hmmm?
	      if(delt>FIXANG_PI)
	         return FALSE;
	   }
   return TRUE;
}

// -------------------------------------
// check_panel_ref 
//
// Checks to see if we've walked away from a panel
// in an mfd, and closes the mfd. 

void check_panel_ref(bool puntme)
{
   static short old_x,old_y;
   extern void restore_mfd_slot(int mfd_id);
   extern bool check_object_dist(ObjID obj1, ObjID obj2, fix crit);

   ObjID id = player_struct.panel_ref;

   if (id != OBJ_NULL && (id != PLAYER_OBJ || puntme))
   {
      extern ubyte mfd_get_func(ubyte mfd_id, ubyte s);
      extern bool mfd_distance_remove(ubyte slot_funca);
      bool punt = puntme;
      if(objs[id].active) {
         punt=punt || !check_object_dist(id,PLAYER_OBJ,MAX_USE_DIST);
         punt=punt || !panel_ref_sanity(id);
      }
      if (punt)
      {
         bool punt_mfd[NUM_MFDS], punting;
         int mfd_id;

         for(mfd_id=0;mfd_id<NUM_MFDS;mfd_id++) {
            punt_mfd[mfd_id]=mfd_distance_remove(mfd_get_func(mfd_id, MFD_INFO_SLOT));
            punting = punting || punt_mfd[mfd_id];
         }
         if (punting)
         {
               mfd_notify_func(MFD_EMPTY_FUNC, MFD_INFO_SLOT, TRUE, MFD_ACTIVE, TRUE);
         }
         for(mfd_id=0;mfd_id<NUM_MFDS;mfd_id++) {
            if(punt_mfd[mfd_id] && player_struct.mfd_current_slots[mfd_id]==MFD_INFO_SLOT) {
               restore_mfd_slot(mfd_id);
            }
         }
         player_struct.panel_ref = OBJ_NULL;         
      }
   }
   old_x = PLAYER_BIN_X; old_y = PLAYER_BIN_Y;
}


// =================================================
// do_stuff_every_second() 
// 
// deals with game system stuff that gets done every nerd second.
// (A nerd second is 256 clock ticks.  A nerd minute is 64 nerd seconds)


// 1 nerd minute = 64*256/(60*280) = 2*256/(15*35) = 512/525 minutes. 

// Rates at which exposure levels degrade by type, in units of 
// percent per nerd minute, compounded every nerd second.  

uchar exposure_degrade_rates[] =
{
   100, //   EXPLOSION_TYPE
   100, //   ENERGY_BEAM_TYPE    
   100, //   MAGNETIC_TYPE       
   100, //   RADIATION_TYPE      
   100, //
   100, //   ARMOR_PIERCING_TYPE 
   100, //   NEEDLE_TYPE         
   100, //   BIO_TYPE            
};


// Integrates the function f(t) = num >> denom_shf from t0 to t1, 
// dealing with roundoff in a graceful way so that 
// integrating over lots of small intervals roughly equals 
// the large interval.
#define find_delta(t0,t1,num,denom_shf) ((((t1)*(num)) >> (denom_shf)) - (((t0)*(num)) >> (denom_shf)))


// computes the change to var by the specified rate as accrued over the 
// time from t0 to t1, clamped by [vmin,vmax]. 
// returns the new value of var.

// t0 and t1 are times in nerd seconds, rate is in units per minute. 

int apply_rate(int var, int rate, int t0, int t1, int vmin, int vmax)
{
   int delta = find_delta(t0,t1,rate,HEALTH_RESTORE_SHF);
   int final = min(vmax,max(var + delta,vmin));
   return final;
}

#define ENERGY_VAR_RATE 50

ulong time_until_shodan_avatar = 0;
extern ObjID shodan_avatar_id;

#define MY_FORALLOBJSPECS(pmo,objspec) for(\
	pmo = (objspec[OBJ_SPEC_NULL]).id;\
	pmo != OBJ_SPEC_NULL;\
	pmo = objspec[pmo].next)

#define REACTOR_BOOM_QB       0x14 // stolen from triggger.c
#define BRIDGE_SEPARATED_QB   0x98

#define REALSPACE_HUDS    (HUD_RADPOISON|HUD_BIOPOISON|HUD_FATIGUE|HUD_BIOHAZARD|HUD_RADIATION|HUD_ZEROGRAV|HUD_ENVIROUSE)


extern Boolean	gPlayingGame;
extern Boolean	gDeadPlayerQuit;

void do_stuff_every_second()
{
   long running_dt = player_struct.game_time - player_struct.last_second_update;
   extern int bio_energy_var;
   extern int bio_absorb;
   extern int rad_absorb;
   extern ulong player_death_time;
   int last = (player_struct.last_second_update >> HEALTH_RESTORE_PRECISION) & HEALTH_RESTORE_MASK;
   int next = (player_struct.game_time >> HEALTH_RESTORE_PRECISION) & HEALTH_RESTORE_MASK;

   if (running_dt > SECOND_UPDATE_FREQ)
   {
      if (global_fullmap->cyber)
      {
         char i;
         ObjSpecID osid;
         ObjID new_id, shrine_obj = OBJ_NULL;
         extern uchar *shodan_bitmask;

         for (i=0; i < NUM_CS_EFFECTS; i++)
            if ((cspace_effect_times[i] != 0) && (cspace_effect_times[i] <= player_struct.game_time))
            {
               cspace_effect_times[i] = 0;
               if (cspace_effect_turnoff[i] != NULL)
                  cspace_effect_turnoff[i](TRUE,TRUE);
            }

         if ((time_until_shodan_avatar != 0) && (player_struct.game_time > time_until_shodan_avatar) && 
               (shodan_avatar_id == OBJ_NULL) && (shodan_bitmask == NULL))
         {
            time_until_shodan_avatar = 0;
            MY_FORALLOBJSPECS(osid,objSmallstuffs)
            {
               if (ID2TRIP(objSmallstuffs[osid].id) == SHODO_SHRINE_TRIPLE)
               {
                  shrine_obj = objSmallstuffs[osid].id;
               }
            }
            if (shrine_obj != OBJ_NULL)
            {
               LGPoint sq;
               sq.x =  OBJ_LOC_BIN_X(objs[shrine_obj].loc);
               sq.y =  OBJ_LOC_BIN_Y(objs[shrine_obj].loc);
               new_id = object_place(CYBER_SHODAN_TRIPLE, sq);
               objCritters[objs[new_id].specID].mood = AI_MOOD_HOSTILE;
               shodan_avatar_id = new_id;
            }
         }
         hud_unset(REALSPACE_HUDS);
      }
      else
      {
         extern void update_email_ware(void);
         
         if ((QUESTBIT_GET(REACTOR_BOOM_QB)) && (!QUESTBIT_GET(BRIDGE_SEPARATED_QB)) && ((rand() & 0x3F) == 1))
         {
            play_digi_fx(SFX_RUMBLE,2);
            fr_global_mod_flag(FR_SFX_SHAKE, FR_SFX_MASK);
            fr_sfx_time = CIT_CYCLE * 5;  // 2 seconds of shake
         }
         if (next < last) next += HEALTH_RESTORE_UNIT;
         if (player_struct.energy_regen + player_struct.energy_spend != 0)
         {
            int num = player_struct.energy_regen
               - player_struct.energy_spend;
            if (num != 0)
            {
               int finale = apply_rate(player_struct.energy,num,last,next,0, MAX_ENERGY); 
               bio_energy_var -= ENERGY_VAR_RATE*(finale-player_struct.energy);
               player_struct.energy = (ubyte) finale;
               chg_set_flg(VITALS_UPDATE);
            }
         }
         if (player_struct.energy == 0)
         {
            extern void hardware_power_outage(void);
            extern errtype gear_power_outage(void);

            if (!player_struct.energy_out)
            {
               string_message_info(REF_STR_PowerRanOut);
               play_digi_fx(SFX_POWER_OUT,1);
               player_struct.energy_out = TRUE;
            }
            hardware_power_outage();
            gear_power_outage();
         }
         else
            player_struct.energy_out = FALSE;
         if (player_struct.hit_points < PLAYER_MAX_HP
            && player_struct.hit_points_regen != 0)
         {

            int num = player_struct.hit_points_regen;
            player_struct.hit_points =
               apply_rate(player_struct.hit_points,num,last,next,0,PLAYER_MAX_HP);
            mfd_notify_func(MFD_BIOWARE_FUNC,MFD_INFO_SLOT,FALSE,MFD_ACTIVE,FALSE);

         }
         if (player_struct.hit_points > 0)
         {
            int i;
            for (i = 0; i < NUM_DAMAGE_TYPES; i++)
               if (player_struct.hit_points_lost[i] > 0)
               {
                  int num = player_struct.hit_points_lost[i];
                  int degrade = exposure_degrade_rates[i];
                  int deltahp = player_struct.hit_points
                     - apply_rate(player_struct.hit_points,-num,last,next,0,PLAYER_MAX_HP);
                  degrade = min(100,find_delta(last,next,degrade,HEALTH_RESTORE_SHF));
                  damage_player((ubyte)deltahp,i+1,NO_SHIELD_ABSORBTION);
                  player_struct.hit_points_lost[i] = num*(100-degrade)/100;
               }
            if (player_struct.hit_points_lost[RADIATION_TYPE-1] > 0)
               hud_set(HUD_RADPOISON);
            else
               hud_unset(HUD_RADPOISON);
            if (player_struct.hit_points_lost[BIO_TYPE-1] > 0)
               hud_set(HUD_BIOPOISON);
            else
               hud_unset(HUD_BIOPOISON);
         }
         update_email_ware();
         
      }
      if (player_struct.fatigue > 0 && player_struct.controls[CONTROL_YVEL] <= SPRINT_CONTROL_THRESHOLD && !EDMS_pelvis_is_climbing)
      {
         int newf = player_struct.fatigue-player_struct.fatigue_regen;
         player_struct.fatigue_regen += fatigue_accum_rate;
         if (player_struct.fatigue_regen > player_struct.fatigue_regen_max)
            player_struct.fatigue_regen = player_struct.fatigue_regen_max;
         if (newf <= 0)
         {
            newf = 0;
         }
         if (newf != player_struct.fatigue)
         {
            mfd_notify_func(MFD_BIOWARE_FUNC,MFD_INFO_SLOT,FALSE,MFD_ACTIVE,FALSE);
            player_struct.fatigue = newf;
         }
      }
      else player_struct.fatigue_regen = player_struct.fatigue_regen_base;

	if(QUESTVAR_GET(MISSION_DIFF_QVAR)==3)
	{
		int remain=MISSION_3_TICKS-player_struct.game_time;
		
		if((remain/CIT_CYCLE)<=30)
			secret_render_fx=TIMELIMIT_REND_SFX;
		
		if(remain<CIT_CYCLE)
		{
			secret_render_fx=0;
//KLC            play_cutscene(ENDGAME_CUTSCENE,TRUE);
			gDeadPlayerQuit = TRUE;											// Pretend the player is dead.
			gPlayingGame = FALSE;											// Hop out of the game loop.
		}
	}

      player_struct.last_second_update = player_struct.game_time;

      // what is this code trying to do? ie. r_a -= (0-r_a)-r_a/4; looks like it can increase or decrease?, end <0, so on
      if (rad_absorb > 0) rad_absorb -= rand()%(rad_absorb)-rad_absorb/4;
      else rad_absorb = 0;
      if (bio_absorb > 0) bio_absorb -= rand()%(bio_absorb)-bio_absorb/4;
      else bio_absorb = 0;

      check_hazard_regions(MAP_GET_XY(PLAYER_BIN_X,PLAYER_BIN_Y));
      check_panel_ref(FALSE);
   }
   return;
}


// ---------------------------------------------
// Expose_player exposes the player to damage of a specified type.  
// if tsecs is non-zero, it is the amount of time in which the damage will go away.  

// TSECS IS NO LONGER SUPPORTED, ALL EXPOSURE HAS A BUILT-IN DECAY RATE.

#define MAX_UBYTE 0xFF

int enviro_suit_absorb(int damage, int exposure, ubyte dtype);

void expose_player_real(short damage, ubyte type, ushort )
{
   int cval = player_struct.hit_points_lost[type-1];
   if (damage == 0) return;
   damage = max(-cval,min(damage,MAX_UBYTE-cval));
   if (damage > 0 && (type == BIO_TYPE || type == RADIATION_TYPE))
   {
      damage = enviro_suit_absorb(damage,cval,type);
   }
   cval += damage;
   player_struct.hit_points_lost[type-1] = (ubyte) cval;
#ifdef SCHEDULED_DECAY
   if (tsecs > 0)
   {
      SchedEvent  ev;
      SchedExposeData *xd = (SchedExposeData*)&ev.data;
      int count = 1;
      ev.timestamp = TICKS2TSTAMP(player_struct.game_time + tsecs*CIT_CYCLE);
      ev.type = EXPOSE_SCHED_EVENT;
      xd->damage = -(damage/count); // plus or minus exposure increment
      xd->type = type;
      xd->tsecs = tsecs;  
      xd->count = count;
      schedule_event(&game_seconds_schedule,&ev);
   }
#endif // SCHEDULED_DECAY
}


void expose_player(byte damage, ubyte type, ushort tsecs)
{
   expose_player_real(damage,type,tsecs);
}

//-------------------------------------------------------
// enviro_suit_absorb()
// 
// returns damage to player after enviro-suit

extern long long_sqrt(long num);

#define ENVIRO_ABSORB_DENOM 5 
#define ENVIRO_DRAIN_DENOM  1
#define ENVIRO_DRAIN_RATE   32


// biorhythm vars
int bio_absorb = 0;
int rad_absorb = 0;


int enviro_suit_absorb(int damage, int exposure, ubyte dtype)
{
   short drain;
   short absorb;
   short denom;
   short energy;
   short old_edrain_rate = enviro_edrain_rate;
   ubyte version = player_struct.hardwarez[CPTRIP(ENV_HARD_TRIPLE)];

   if (dtype == RADIATION_TYPE && version > 0) version--;
   if (version == 0) return damage;

   // Absorb all but 1/nth of damage
   denom = ENVIRO_ABSORB_DENOM+version;
   absorb = ((damage+exposure)*(denom-1))/denom;
   if (absorb == 0) return damage;

   // Compute drain for that absorption amount.
   denom = ENVIRO_DRAIN_DENOM+version+1;
   enviro_edrain_rate = exposure*60/(ENVIRO_DRAIN_RATE*denom);
   drain = (rand()%60+enviro_edrain_rate)/60; 

   // drain energy
   energy = drain_energy(drain);
   // did we have enough? 
   if (energy < drain)
   {
      // if not, recompute absorption.
      absorb = energy*denom/(denom-1);
   }
   enviro_absorb_rate = min(damage,absorb) >> 1;
   damage -= min(damage,absorb);
   switch(dtype)
   {
      case BIO_TYPE:
         bio_absorb = 8+long_sqrt((int)damage);
         break;
      case RADIATION_TYPE:
         rad_absorb = 8+long_sqrt((int)damage);
         break;
      default:
         break;
   }
   if (enviro_absorb_rate > 0)
   {
      if (old_edrain_rate == 0)
      {
         ulong time = 5 << APPROX_CIT_CYCLE_SHFT;
         hud_set_time(HUD_ENVIROUSE,time);
         hud_set_time(HUD_ENERGYUSE,time);
      }
   }
   else
      hud_unset(HUD_ENVIROUSE);
   return (byte)damage; 
}
