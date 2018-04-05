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
 * $Source: r:/prj/cit/src/RCS/physics.c $
 * $Revision: 1.259 $
 * $Author: xemu $
 * $Date: 1994/11/10 16:05:25 $
 *  
*/

#define __PHYSICS_SRC

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ai.h"
#include "combat.h"
#include "criterr.h"
#include "cyber.h"
#include "cybmem.h"
#include "cybstrng.h"
#include "damage.h"
#include "diffq.h" 		// for time limit
#include "drugs.h"
#include "effect.h"
#include "faketime.h"
#include "framer8.h"
#include "froslew.h"
#include "gamestrn.h"
#include "grenades.h"
#include "hud.h"
#include "ice.h"
#include "invent.h"
#include "loops.h"
#include "lvldata.h"
#include "map.h"
#include "mapflags.h"
#include "mfdext.h"
#include "musicai.h"
#include "objbit.h"
#include "objects.h"
#include "objsim.h"
#include "objprop.h"
#include "objuse.h"
#include "otrip.h"
#include "physics.h"
#include "physunit.h"
#include "player.h"
#include "render.h"
#include "sfxlist.h"
#include "tilename.h"
#include "tools.h"
#include "trigger.h"
#include "wares.h"
#include "weapons.h"


/*
#include <fastmat.h>
#include <faceleth.h>
#include <btfunc.h>
*/

#define EXPAND_FIX(x) fix_int(x), fix_frac(x)

#define sqr(fixval) (fix_mul(fixval,fixval))


// INTERNAL PROTOTYPES
// --------------------
bool safety_net_wont_you_back_me_up(ObjID oid);
void add_edms_delete(int ph);
void edms_delete_go(void);
void get_phys_state(int ph, State *new_state, ObjID id);
void physics_zero_all_controls(void);
errtype compare_locs(void);
void physics_set_relax(int axis, bool relax);
void relax_axis(int axis);
errtype collide_objects(ObjID collision, ObjID victim, int bad);
void terrain_object_collide(physics_handle src, ObjID target);
errtype run_cspace_collisions(ObjID obj, ObjID exclude, ObjID exclude2);
void state_to_objloc(State *s, ObjLoc *l);
bool get_phys_info(int ph, fix *list, int cnt);
fix ID2radius(ObjID id);


// STANDARD MODELS
// ---------------

#define STANDARD_HARDNESS  fix_make(15,0)
#define STANDARD_ROUGHNESS fix_make(5,0)
#define STANDARD_PEP    fix_make(5,0)
#define DEFAULT_SIZE    (FIX_UNIT/2)
#define STANDARD_HEIGHT fix_make(0,0xbd00)

Robot standard_robot = { STANDARD_MASS, DEFAULT_SIZE, STANDARD_HARDNESS,
                         STANDARD_PEP, STANDARD_GRAVITY, FALSE};

Pelvis standard_pelvis = { STANDARD_MASS, DEFAULT_SIZE, STANDARD_HARDNESS,
                         STANDARD_PEP, STANDARD_GRAVITY, STANDARD_HEIGHT, FALSE};

fix standard_corner[4] = {0,0,0,0};
Dirac_frame standard_dirac = { STANDARD_MASS, STANDARD_HARDNESS, STANDARD_ROUGHNESS,
                        STANDARD_GRAVITY,
#ifdef HMMMM
                        standard_corner, standard_corner, standard_corner, standard_corner, standard_corner,
                        standard_corner, standard_corner, standard_corner, standard_corner, standard_corner,
#endif
                        };   

extern ObjID physics_handle_id[];
extern int   physics_handle_max;

#define check_up(num) 

cams *motion_cam=NULL;     // what to move, default null is the default camera


// CONTROLS
// --------

byte player_controls[CONTROL_BANKS][DEGREES_OF_FREEDOM] =
 { { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 } };

extern errtype (*state_generators[])(ObjID id, int x, int y, ObjLocState *ret);
long old_ticks;

// Collision callback testing....
void cit_collision_callback(physics_handle C, physics_handle V, int bad, long DATA1, long DATA2, fix location[3]);
void cit_awol_callback(physics_handle caller);
void cit_sleeper_callback(physics_handle caller);
void cit_autodestruct_callback(physics_handle caller);

// mapping from physics controls to camera controls
int ctrl2cam[DEGREES_OF_FREEDOM] = { EYE_X, EYE_Y, EYE_Z, EYE_H, EYE_P, EYE_B };

#define SLEW_SCALE_N 16
#define SLEW_SCALE_D 100

#define MAX_EDMS_DELETE_OBJS  50

#ifdef EDMS_SAFETY_NET
short safety_fail_oid=-1;
uchar safety_fail_count=0;
#define SECRET_NET_OUT_P(x,y) (x>0x20)
#define TOGGLEABLE_SNET
#endif

bool safety_net_on = TRUE;
short curr_edms_del = 0;
short edms_delete_queue[MAX_EDMS_DELETE_OBJS];


bool safety_net_wont_you_back_me_up(ObjID oid)
{ 
   obj_move_to(oid, &objs[oid].loc, TRUE);
   if (safety_fail_oid==oid)
   {
      safety_fail_oid=-1;
      return FALSE;
   }
   if (oid==PLAYER_OBJ)
	   safety_fail_oid=oid;
   else
      if (safety_fail_oid==-1)
         safety_fail_oid=oid;
   safety_fail_count=3;
   return TRUE;
}

void add_edms_delete(int ph)
{
   int i=0;
   bool bad = FALSE;
   extern char *get_object_lookname(ObjID id,char use_string[], int sz);

   for (i=0; i < curr_edms_del; i++)
      if (edms_delete_queue[i] == ph)
         bad = TRUE;

   if (ph == -1)
      bad = TRUE;

   if (!bad)
   {
      edms_delete_queue[curr_edms_del] = ph;
      curr_edms_del++;
      objs[physics_handle_id[ph]].info.ph = -1;
      physics_handle_id[ph]=OBJ_NULL;
   }
}

void edms_delete_go()
{
   int i;
   for (i=0; i < curr_edms_del; i++)
   {
      if (edms_delete_queue[i] != -1)
         EDMS_kill_object(edms_delete_queue[i]);
      edms_delete_queue[i] = -1;
   }
   curr_edms_del = 0;
}

void get_phys_state(int ph, State *new_state, ObjID id)
{
   char use_mod = EDMS_ROBOT;
   if (id != OBJ_NULL)
   {
      use_mod = ObjProps[OPNUM(id)].physics_model;
#ifdef DIRAC_EDMS
      // This is hacked on account of the player having 2 physics models.
      // We may want to take an unused critter slot like the lifter bot to be the dirac-player
      // like sonic is our pelvis-player.
      if (id == PLAYER_OBJ)
         use_mod = (global_fullmap->cyber) ? EDMS_DIRAC : EDMS_PELVIS;
#endif
   }
   switch (use_mod)
   {
      case EDMS_PELVIS:
         EDMS_get_pelvic_viewpoint(ph,new_state);
         break;
      case EDMS_DIRAC:
         EDMS_get_Dirac_frame_viewpoint(ph,new_state);
         break;
      default:
         EDMS_get_state(ph, new_state);
         break;
   }
}

void physics_zero_all_controls()
{
   LG_memset(player_controls,0,sizeof(player_controls));
}

errtype physics_set_player_controls(int bank, byte xvel, byte yvel, byte zvel, byte xyrot, byte yzrot, byte xzrot)
{
   if (xvel != CONTROL_NO_CHANGE)  player_controls[bank][CONTROL_XVEL] = xvel;
   if (yvel != CONTROL_NO_CHANGE)  player_controls[bank][CONTROL_YVEL] = yvel;
   if (zvel != CONTROL_NO_CHANGE)  player_controls[bank][CONTROL_ZVEL] = zvel;
   if (xyrot != CONTROL_NO_CHANGE) player_controls[bank][CONTROL_XYROT] = xyrot;
   if (yzrot != CONTROL_NO_CHANGE) player_controls[bank][CONTROL_YZROT] = yzrot;
   if (xzrot != CONTROL_NO_CHANGE) player_controls[bank][CONTROL_XZROT] = xzrot;
   return OK;
}

errtype physics_set_one_control(int bank, int num, byte val)
{
   if (val != CONTROL_NO_CHANGE)
      player_controls[bank][num] = val;
   return OK;
}

errtype physics_get_one_control(int bank, int num, byte* val)
{
   *val = player_controls[bank][num];
   return OK;
}

int old_x = -1, old_y = -1, old_lev = -1;
char old_bits = 0;
extern uchar decon_count;
extern bool in_deconst;
extern bool in_peril;

#define TUNNEL_CONTROL_MAX          fix_make(0x8, 0)
#define MATCHBOX_SPEED              fix_make(0x3F, 0)


// The concept here is that if you want something to happen when the player switches
// square, put it here...

errtype compare_locs(void)
{
   extern void expose_player(byte damage, ubyte type, ushort hlife);
   extern void check_hazard_regions(MapElem* );
   MapElem *newElem, *oldElem;
   extern int score_playing;

   if ((old_x != PLAYER_BIN_X) || (old_y != PLAYER_BIN_Y) || (old_lev != player_struct.level))
   {
      newElem = MAP_GET_XY(PLAYER_BIN_X, PLAYER_BIN_Y);
      oldElem = MAP_GET_XY(old_x, old_y);

      // Change music
      if (music_on) 
      {
         if (!global_fullmap->cyber)
         {
            in_deconst = me_bits_deconst(newElem);
            if (!in_deconst)
               decon_count=0;
            in_peril = me_bits_peril(newElem);
            if (old_bits != me_bits_music(newElem))
            {
               fade_into_location(PLAYER_BIN_X, PLAYER_BIN_Y);
               old_bits = me_bits_music(newElem);
            }
         }
      }

      // Abort physics if bad karma
      if (physics_running && time_passes && (me_tiletype(newElem) == TILE_SOLID))
      {
         physics_running = FALSE;
         critical_error(CRITERR_EXEC|2);
      }

      // Look for traps
      if (time_passes)
         check_entrance_triggers(old_x, old_y, PLAYER_BIN_X, PLAYER_BIN_Y);

      check_hazard_regions(newElem);

      old_x = PLAYER_BIN_X;   old_y = PLAYER_BIN_Y;   old_lev = player_struct.level;
   }

   return(OK);
}

bool control_relax[DEGREES_OF_FREEDOM];

void physics_set_relax(int axis, bool relax)
{
   control_relax[axis] = relax;
}

void relax_axis(int axis)
{
   switch(axis)
   {
   case CONTROL_XZROT:
      player_set_lean(0,player_struct.leany);
      break;
   case CONTROL_YZROT:
      player_set_eye(0);
      break;
   }
}


static ubyte crouch_controls[NUM_POSTURES] = {0, 6, 10};

#define CSPACE_COLLIDE_DIST   0xD0
#define CSPACE_FAR_COLLIDE_DIST  0x120

#define MAX_PITCH_RATE (FIXANG_PI/4)
#define MAX_LEAN_RATE  400


#define ITER_OBJSPECS(pmo,objspec) for(\
	pmo = (objspec[OBJ_SPEC_NULL]).id;\
	pmo != OBJ_SPEC_NULL;\
	pmo = objspec[pmo].next)

// Yow, we have GOT to be able to make this faster/better... -- Xemu
errtype run_cspace_collisions(ObjID obj, ObjID exclude, ObjID exclude2)
{
   ObjRefID oref = me_objref(MAP_GET_XY(OBJ_LOC_BIN_X(objs[obj].loc), OBJ_LOC_BIN_Y(objs[obj].loc)));
   short use_dist;
   while (oref != OBJ_REF_NULL)
   {
      ObjID oid = objRefs[oref].obj;
      ObjLoc l1 = objs[oid].loc;
      ObjLoc l2 = objs[obj].loc;
      switch(ID2TRIP(oid))
      {
         case CYBERTOG1_TRIPLE:
         case CYBERTOG2_TRIPLE:
         case CYBERTOG3_TRIPLE:
            use_dist = CSPACE_FAR_COLLIDE_DIST;
            break;
         default:
            use_dist = CSPACE_COLLIDE_DIST;
            break;
      }
      if ((oid != obj) && (oid != exclude) && (oid != exclude2) && (objs[oid].obclass != CLASS_PHYSICS))
      {
         // This really ought to take PX into account!
         if ((abs(l1.x - l2.x) < use_dist) &&
             (abs(l1.y - l2.y) < use_dist) &&
             ((abs(l1.z - l2.z) << SLOPE_SHIFT_D) < use_dist))
         {
            obj_cspace_collide(oid,obj);
         }
      }
      oref = objRefs[oref].next;
   }
   return(OK);
}

#define PLAYER_JIGGLE_THRESHOLD  0x3  // this is in objsys coords

#define MAX_SPRINT (fix_make(25,0))
#define MAX_JOG    (fix_make(8,0))
#define MAX_BOOSTER (fix_make(50,0))
#define SKATE_ALPHA_CUTOFF (fix_make(8,0))
#define MAX_BOOSTER_ALPHA (fix_make(40,0))

extern void physics_set_relax(int axis,bool relax);

// Takes a physics state and converts it into an Objloc
// externed in objsim.c
void state_to_objloc(State *s, ObjLoc *l)
{
   l->x = obj_coord_from_fix(s->X);
   l->y = obj_coord_from_fix(s->Y);
   l->z = obj_height_from_fix(s->Z);
   l->h = obj_angle_from_phys(s->alpha);
#ifdef WHY_DOESNT_THIS_WORK
   l->p = obj_angle_from_phys(s->beta);
   l->b = obj_angle_from_phys(s->gamma);
#endif
}


#define NO_DEFAULT_FORWARD_IN_CSPACE

#define CYB_VEL_DELTA 32
#define CYB_VEL_DELTA2 16   
ubyte old_head=0, old_pitch=0;
short last_deltap=0, last_deltah=0;
                                 
errtype physics_run(void)
{
   int i;
   extern int avail_memory(int debug_src);
   extern errtype TileMapUpdateCameras(struct _tilemap* );

   bool update = FALSE;
   int deltat = player_struct.deltat;
   fix plr_y, plr_z, time_diff;
   fix plr_alpha;
   State new_state;
   bool some_move = FALSE;
   extern int fire_kickback;
   extern bool hack_takeover;
   static long kickback_time=0;        // i bet this static will someday bite our butts, like save/rest mid kickback?
#ifdef EDMS_SAFETY_NET
   bool allow_move=TRUE;
#endif

   // Here we are computing the values of the player's controls
   // from the values of the original control banks.  The value
   // of each control is the average of its non-zero control 
   // values from each control bank, or zero if all are zero.  
   for (i=0; i<DEGREES_OF_FREEDOM; i++)
   {
      int b, n=0;
      short control = 0;
      for (b = 0; b < CONTROL_BANKS; b++)
         if (player_controls[b][i] != 0)
          { control += player_controls[b][i]; n++; }
      if (n > 0)
      {
         some_move = TRUE;    // should really do the n!=1 for 2 and 4 as shift too
         if (n > 1)
         {
            control /= n;
         }
      }
      else if (control_relax[i])
      {
         relax_axis(i);
      }
      player_struct.controls[i] = control;
   }
   update=some_move;    // well, set one of them only once
   if (physics_running && time_passes)
   {
      int i;
      ObjID oid;
      int damp; 
      fix plr_side;
      fix plr_lean;
      ObjSpecID   osid;
      {
         fix crouch = fix_make(0,player_struct.lean_filter_state);
         damp = 3*sqr(STANDARD_HEIGHT-crouch)/sqr(STANDARD_HEIGHT) + 1;
      }
      osid = objPhysicss[0].id;
      while (osid != OBJ_SPEC_NULL)
      {
         // clear the terrain flag

         if (objPhysicss[osid].p3.x>0)
            objPhysicss[osid].p3.x--;

         // clear the collision flag
         if (objPhysicss[osid].p3.y>0)
            objPhysicss[osid].p3.y--;

         osid = objPhysicss[osid].next;
      }

      if (motionware_mode == MOTION_SKATES && damp > 1) damp--;
      // Here' s where we do leaning.  
      if (player_struct.foot_planted)
      {
         plr_y = plr_alpha = plr_side = 0;
         physics_set_relax(CONTROL_XZROT,FALSE);
      }
      else
      {
         byte leanx = player_struct.leanx;
         byte ycntl = player_struct.controls[CONTROL_YVEL];
         short maxlean = (int)(SPRINT_CONTROL_THRESHOLD - abs(ycntl))*CONTROL_MAX_VAL/SPRINT_CONTROL_THRESHOLD;
         plr_side  = fix_make(player_struct.controls[CONTROL_XVEL],0) / 7 / damp;
         plr_alpha = fix_make(player_struct.controls[CONTROL_XYROT],0) / -5; // / damp;
         if (ycntl <= SPRINT_CONTROL_THRESHOLD)
            plr_y = ycntl*MAX_JOG/SPRINT_CONTROL_THRESHOLD;
         else
            plr_y = (ycntl-SPRINT_CONTROL_THRESHOLD)*(MAX_SPRINT-MAX_JOG)/(CONTROL_MAX_VAL - SPRINT_CONTROL_THRESHOLD) + MAX_JOG;
         plr_y /= damp;
         physics_set_relax(CONTROL_XZROT,abs((short)leanx) > maxlean);
      }
      if (player_struct.drug_status[DRUG_REFLEX] > 0 && !global_fullmap->cyber)
      {
         // Increase some controls to reflect smaller timestep
         plr_y = plr_y << 2;
         plr_alpha = plr_alpha << 2;
         plr_side = plr_side << 2;
      }
      switch (motionware_mode)
      {
         case MOTION_BOOST:
         {
            plr_y = MAX_BOOSTER;
            // whoop whoop hardcoded version number.
            if (player_struct.hardwarez[CPTRIP(MOTION_HARD_TRIPLE)] < 3)
            {
               if (plr_alpha < 0) plr_alpha = -MAX_BOOSTER_ALPHA;
               if (plr_alpha > 0) plr_alpha =  MAX_BOOSTER_ALPHA;
            }
            break;
         }
         case MOTION_SKATES:
            plr_y *= 2;
            if (plr_y > 3* MAX_SPRINT/2)
            {
               plr_y = 3*MAX_SPRINT/2;
            }
            else if (plr_y < -MAX_SPRINT)
            {
               plr_y = -MAX_SPRINT;
            }
            plr_side >>= 2;
            if (abs(plr_alpha) > SKATE_ALPHA_CUTOFF)
               if (plr_alpha > 0)
                  plr_alpha =  SKATE_ALPHA_CUTOFF + (plr_alpha - SKATE_ALPHA_CUTOFF)/2;
               else
                  plr_alpha =  -(SKATE_ALPHA_CUTOFF + (-plr_alpha - SKATE_ALPHA_CUTOFF)/2);

            break;
      }

      
      time_diff = fix_make(deltat, 0);      // do this here for constant length kickback hack
      if (time_diff>fix_make(CIT_CYCLE,0)/MIN_FRAME_RATE) time_diff=fix_make(CIT_CYCLE,0)/MIN_FRAME_RATE;

      if (player_struct.controls[CONTROL_XZROT] != 0)
      {
         short delta = player_struct.controls[CONTROL_XZROT]*MAX_LEAN_RATE/CONTROL_MAX_VAL;
         int leanx = player_struct.leanx;
         leanx = min(CONTROL_MAX_VAL,max(leanx+delta*deltat/CIT_CYCLE,-CONTROL_MAX_VAL));
         player_set_lean(leanx,player_struct.leany);
      }
      if (player_struct.controls[CONTROL_YZROT] != 0)
      {
         extern int player_get_eye_fixang(void);
         extern void player_set_eye_fixang(int);
         int delta = player_struct.controls[CONTROL_YZROT]*MAX_PITCH_RATE/CONTROL_MAX_VAL;
         int eye = player_get_eye_fixang();
         if (player_struct.drug_status[DRUG_REFLEX] > 0 && !global_fullmap->cyber)
            delta <<= 2;
         eye = min(FIXANG_PI/2,max(eye+delta*deltat/CIT_CYCLE,-FIXANG_PI/2));
         player_set_eye_fixang(eye);
      }
         
      plr_lean = fix_make((int)player_struct.leanx,0)/3;
      if (player_struct.controls[CONTROL_ZVEL] > 0)
      {
         extern void activate_jumpjets(fix* x,fix* y,fix* z);
         extern bool jumpjets_active;

         player_set_posture(POSTURE_STAND);
         plr_z = fix_make(player_struct.controls[CONTROL_ZVEL],0);
         activate_jumpjets(&plr_side,&plr_y,&plr_z);
         jumpjets_active = plr_z < 0;
      }
      else
      {
         extern bool jumpjets_active;
         plr_z     = fix_make(player_struct.controls[CONTROL_ZVEL],0);  /*  /3/damp; */
         jumpjets_active = FALSE;
      }

      if (global_fullmap->cyber) 
      {
         MapElem *pme = MAP_GET_XY(PLAYER_BIN_X, PLAYER_BIN_Y);
         if (me_light_flr(pme))
         {
            if (plr_y > TUNNEL_CONTROL_MAX)
               plr_y = TUNNEL_CONTROL_MAX;
         }
         // make controls non-linear.
         plr_side = fix_mul(plr_side,abs(plr_side)) /32;
         plr_alpha = fix_mul(plr_alpha,abs(plr_alpha))/32;

#ifdef NO_DEFAULT_FORWARD_IN_CSPACE
         if (QUESTVAR_GET(CYBER_DIFF_QVAR) > 1)
            plr_z += fix_make(QUESTVAR_GET(CYBER_DIFF_QVAR) * 5, 0);
#endif

         // Effects of turbo, multiply all movement axes by 4
         if (cspace_effect_times[CS_TURBO_EFF])
         {
            plr_y = plr_y << 1;
            plr_z = plr_z << 1;
            plr_side = plr_side << 1;
         }
#ifdef MATCHBOX_SUPPORT
         else if (cspace_effect_times[CS_MATCHBOX_EFF])
         {
            plr_z = MATCHBOX_SPEED;
         }
#endif
      }
      if (!global_fullmap->cyber && player_struct.drug_status[CPTRIP(GENIUS_DRUG_TRIPLE)] > 0)
      {
         // Reverse!
         plr_alpha = -plr_alpha;
         plr_side = - plr_side;
         plr_lean = - plr_lean;
      }
#ifdef DIRAC_EDMS
      if (global_fullmap->cyber)
      {
         EDMS_control_Dirac_frame(PLAYER_PHYSICS, plr_z, plr_alpha, plr_y,plr_side);
      }
      else
#endif
         EDMS_control_pelvis(PLAYER_PHYSICS,plr_y,plr_alpha,plr_side,plr_lean,plr_z,crouch_controls[player_struct.posture]);

#ifdef SOLITON_HACK_REFLEX
      if (player_struct.drug_status[DRUG_REFLEX] > 0 && !global_fullmap->cyber)
        EDMS_soliton_vector((time_diff / CIT_CYCLE) >> 2);
      else
#endif
        EDMS_soliton_vector(time_diff / CIT_CYCLE);

      edms_delete_go();

      if (--safety_fail_count==0) safety_fail_oid=-1;
      for (i=0; i<=physics_handle_max; i++)  // all ph
      {
         if ((oid=physics_handle_to_id(i))!=OBJ_NULL)  // valid ph, get oid
         {
            ObjLoc newloc;

            newloc = objs[oid].loc;
            EDMS_get_state(objs[oid].info.ph, &new_state);
            state_to_objloc(&new_state, &newloc);
            if ((oid == PLAYER_OBJ) && global_fullmap->cyber)
            {
               ubyte new_head,new_pitch;//, new_bank;
               short new_deltah, new_deltap;
               State cyber_state;
               extern bool new_cyber_orient;

               get_phys_state(objs[PLAYER_OBJ].info.ph, &cyber_state, PLAYER_OBJ);

               new_head = obj_angle_from_phys(cyber_state.alpha);
               new_pitch = obj_angle_from_phys(cyber_state.beta);

               new_deltah = abs(new_head-old_head);
               new_deltap = abs(new_pitch-old_pitch);

               if (((new_deltah < CYB_VEL_DELTA) && (new_deltap < CYB_VEL_DELTA)) || new_cyber_orient ||
                   ((abs(last_deltah-new_deltah)<CYB_VEL_DELTA2) && (abs(last_deltap-new_deltap)<CYB_VEL_DELTA2)))
               {
                  old_head = new_head;
                  old_pitch = new_pitch;
                  last_deltah = last_deltap = 0;

                  if (new_cyber_orient)
                     new_cyber_orient = FALSE;
               }
               else
               {
                  last_deltah = new_deltah;
                  last_deltap = new_deltap;
               }
            }
            // See if we should snap the player out of his
            // reverie, either hack camera or automap induced.
            if (oid == PLAYER_OBJ)
            {
               if (hack_takeover && 
                   (some_move || (abs(newloc.x - objs[oid].loc.x) + abs(newloc.y - objs[oid].loc.y) + abs(newloc.z - objs[oid].loc.z) 
                     > PLAYER_JIGGLE_THRESHOLD))) 
                  hack_camera_relinquish();
            }

#ifdef EDMS_SAFETY_NET
#ifdef TOGGLEABLE_SNET
            if (safety_net_on)
#endif
            {
               if (me_tiletype(MAP_GET_XY(OBJ_LOC_BIN_X(newloc), OBJ_LOC_BIN_Y(newloc))) == TILE_SOLID)
               {
                  safety_net_wont_you_back_me_up(oid);
                  allow_move=FALSE;
               }
               else if (new_state.Z < fix_from_map_height(me_height_flr(MAP_GET_XY(OBJ_LOC_BIN_X(newloc), OBJ_LOC_BIN_Y(newloc)))))
               {
	               if (safety_net_wont_you_back_me_up(oid))
                     allow_move=FALSE;
                  else
                  {
                     if (objs[oid].obclass==CLASS_CRITTER)
                     {
	                     newloc=objs[oid].loc;
	                     newloc.z+=3;
	                     safety_fail_oid=-1;
	                     allow_move=TRUE;
                     }
                     else
                     {
	                     add_edms_delete(objs[oid].info.ph);
                        allow_move=FALSE;    // just plain sit here and lose, eh?
                     }
                  }
               }
            }
            if (allow_move)
               obj_move_to(oid, &newloc, FALSE);
            else
               allow_move=TRUE;
#else
            obj_move_to(oid, &newloc, FALSE);
#endif
         }
      }
   }
   else if (some_move)
   {  // what is going on here... ah-ha, we objslew.. no wrong
      for (i = 0; i < DEGREES_OF_FREEDOM; i++)
         if (player_struct.controls[i] != 0)
            fr_camera_slewcam(motion_cam, ctrl2cam[i],player_struct.controls[i]*SLEW_SCALE_N/SLEW_SCALE_D);
   }
   old_ticks = *tmd_ticks;
   if (update || hack_takeover)
      chg_set_flg(_current_3d_flag);
   if (physics_running && global_fullmap->cyber)
   {
      ObjSpecID specid;
      // check for the player
      run_cspace_collisions(PLAYER_OBJ,OBJ_NULL,OBJ_NULL);

      // check for all slow projectiles
      ITER_OBJSPECS(specid, objPhysicss)
      {
         run_cspace_collisions(objPhysicss[specid].id, PLAYER_OBJ, objPhysicss[specid].owner);
      }
   }
   compare_locs();
   return(OK);
}

#ifdef NOT_YET //��� later, dude


#ifdef WACKY_OLD_TERR_FUNC

/// ---------------------------------------------------
/// HERE COMES THE TERRAIN FUNCTION 

/// 9/20 ML  I ain't tellin' you a secret... 
///          I ain't tellin' you goodBIYEYE...


/* ---------------------------------
   HAQ ALERT! HAQ ALERT!
   Ok, oftimes we're squaring small numbers.
   So we're going to use our own special
   8-24 intermediate fixpoint representation to
   store the squares
   -------------------------------- */

typedef fix  wacky; 

// Our own special 8-24 fixmul
wacky wacky_mul (fix a, fix b);
#pragma aux wacky_mul =\
   "imul    edx"     \
   "shr     eax,8"  \
   "shl     edx,24"  \
   "or      eax,edx" \
   parm [eax] [edx]  \
   modify [eax edx];

wacky wacky_div (wacky a, wacky b);
#pragma aux wacky_div =\
   "mov     edx,eax" \
   "sar     edx,8"  \
   "shl     eax,24"  \
   "idiv    ebx"     \
   parm [eax] [ebx]  \
   modify [eax edx];

typedef fix pt3d[3];

#define PTARGS(pt) fix_float((pt)[0]),fix_float((pt)[1]),fix_float((pt)[2])

#define dotprod(vec1,vec2,result) \
{\
   fix* v1 = (vec1);\
   fix* v2 = (vec2); \
   (result) = 0;\
   result += fix_mul(*(v1++),*(v2++));\
   result += fix_mul(*(v1++),*(v2++));\
   result += fix_mul(*(v1++),*(v2++));\
}



#define wsqr(fixval) (wacky_mul(fixval,fixval))
#define magsquared(vec,res)  \
{                            \
   fix* v1 = (vec);          \
   (res) = 0;                \
   (res) += wsqr(*(v1++));   \
   (res) += wsqr(*(v1++));   \
   (res) += wsqr(*(v1++));   \
}


bool vec_equal(fix* v1,fix *v2)    
{
   if (*(v1++) == *(v2++) && *(v1++) == *(v2++) && *(v1++) == *(v2++))
      return TRUE;
   else return FALSE;
}


#define wacky2fix(m2)  (m2 >> 8) 
#define fix2wacky(m)   (m << 8)
#define wacky_float(w) fix_float(wacky2fix(w))


#define NORMAL_X 0
#define NORMAL_Y 1
#define NORMAL_Z 2

int compute_normal_code(pt3d norm)
{
   if (abs(norm[0]) > abs(norm[1]))
   {
      if (abs(norm[0]) > abs(norm[2]))
         return NORMAL_X;
      else return NORMAL_Z;
   }
   else
   {
      if (abs(norm[1]) > abs(norm[2]))
         return NORMAL_Y;
      else return NORMAL_Z;
   }
}

#define PHYS_SPEW(x,y) 


#define crossprod(v1,v2,out)  \
{\
   fix* a1 = (v1);\
   fix* a2 = (v1);\
   fix* b1 = (v2);\
   fix* b2 = (v2);\
   fix* o = (out);\
   fix z = fix_mul(*(v1), (*++b2)) - fix_mul((*++a2),*(v2));    \
   *(o++) = fix_mul((*++a1),(*++b2)) - fix_mul((*++a2),(*++b1));\
   *(o++) = fix_mul((*++a1),*(v2)) - fix_mul(*(v1),(*++b1)); \
   *(o++) = z;  \
}                                                                    

fix project_onto_facelet(pt3d in, pt3d out, pt3d flet[NUM_POINTS])
{
   g3s_vector topt;
   fix dp;
   int i;
   g3s_vector norm = *(g3s_vector*)(flet[NORM_IDX]);
   g3_vec_sub(&topt,(g3s_vector*)in,(g3s_vector*)(flet[0]));
   dp = g3_vec_dotprod(&topt,&norm);
   g3_vec_scale(&norm,&norm,dp);
   // Subtract out normal component
   g3_vec_sub((g3s_vector*)out,(g3s_vector*)in,&norm);
   PHYS_SPEW(DSRC_PHYSICS_Terrain,("Projection onto facelet: %q %q %q --> %q %q %q\nproj =%q\n",PTARGS(in),PTARGS(out),fix_float(dp)));
   return dp;
}

int normcode_indices[3][2] = {{ 1,2},{0,2},{0,1}};

// Takes a point on the plane of a facelet, and computes 
// the distance from the projection to the facelet.  Returns 
// zero if the point projects onto the interior of the facelet.
fix facelet_distance_sq_4points(pt3d pt, pt3d flet[NUM_POINTS], uchar normcode)
{
   fix best_dsq = FIX_MAX;       // minimum distance squared
   int best_vert = -1;
   fix* best_edge;
   fix* cprod_edge;
   fix  cprod_msq;
   pt3d edgen;
   pt3d cprod;
   pt3d edgep;
   pt3d topt; // vertex to point;
   {
      pt3d *vert = flet;
      fix result;

      g3_vec_sub((g3s_vector*)topt,(g3s_vector*)pt,(g3s_vector*)vert);
      result = g3_vec_mag((g3s_vector*)topt);
      PHYS_SPEW(DSRC_PHYSICS_Terrain,("topt %q %q %q dsq %d best_dsq %d\n",PTARGS(topt),result,best_dsq));
      if (result < best_dsq)
      {
         best_dsq = result;
         best_vert = 0;
      }
      vert++;
      g3_vec_sub((g3s_vector*)topt,(g3s_vector*)pt,(g3s_vector*)vert);
      result = g3_vec_mag((g3s_vector*)topt);
      PHYS_SPEW(DSRC_PHYSICS_Terrain,("topt %q %q %q dsq %d best_dsq %d\n",PTARGS(topt),result,best_dsq));
      if (result < best_dsq)
      {
         best_dsq = result;
         best_vert = 1;
      }
      vert++;
      g3_vec_sub((g3s_vector*)topt,(g3s_vector*)pt,(g3s_vector*)vert);
      result = g3_vec_mag((g3s_vector*)topt);
      PHYS_SPEW(DSRC_PHYSICS_Terrain,("topt %q %q %q dsq %d best_dsq %d\n",PTARGS(topt),result,best_dsq));
      if (result < best_dsq)
      {
         best_dsq = result;
         best_vert = 2;
      }
      vert++;
      g3_vec_sub((g3s_vector*)topt,(g3s_vector*)pt,(g3s_vector*)vert);
      result = g3_vec_mag((g3s_vector*)topt);
      PHYS_SPEW(DSRC_PHYSICS_Terrain,("topt %q %q %q dsq %d best_dsq %d\n",PTARGS(topt),result,best_dsq));
      if (result < best_dsq)
      {
         best_dsq = result;
         best_vert = 3;
      }
   }
   PHYS_SPEW(DSRC_PHYSICS_Terrain,("Closest vertex %d\n",best_vert));
   {
      fix a1,a2,b1,b2,p1,p2,c1,c2;
      int i1,i2;
      fix d;
      int nx = (best_vert + 1) & 0x3;
      int pr = (best_vert + 3) & 0x3;
      fix* ep = edgep;
      fix* en = edgen;
      fix* tp = topt;
      fix* p  = pt;
      fix* vert = &flet[best_vert][0];
      fix* prev = &flet[pr][0];
      fix* next = &flet[nx][0];
      if (vec_equal(prev,vert))
      {
         PHYS_SPEW(DSRC_PHYSICS_Terrain,("Caught multiple on prev\n"));
         pr = (pr+3) & 0x3;
         prev = flet[pr];
      }
      if (vec_equal(next,vert))
      {
         PHYS_SPEW(DSRC_PHYSICS_Terrain,("Caught multiple on next\n"));
         nx = (nx+1)  & 0x3;
         next = flet[nx];
      }
      g3_vec_sub((g3s_vector*)ep,(g3s_vector*)prev,(g3s_vector*)vert);
      g3_vec_sub((g3s_vector*)en,(g3s_vector*)next,(g3s_vector*)vert);
      g3_vec_sub((g3s_vector*)tp,(g3s_vector*)p,(g3s_vector*)vert);
      PHYS_SPEW(DSRC_PHYSICS_Terrain,("vert: (%d) %q %q %q\n",best_vert,PTARGS(vert)));
      PHYS_SPEW(DSRC_PHYSICS_Terrain,("prev: (%d) %q %q %q\n",pr,PTARGS(prev)));
      PHYS_SPEW(DSRC_PHYSICS_Terrain,("next: (%d) %q %q %q\n",nx,PTARGS(next)));
      PHYS_SPEW(DSRC_PHYSICS_Terrain,("Normal code is %d\n",normcode));

      // now throw out one of the coordinates
      i1 = normcode_indices[normcode][0];
      i2 = normcode_indices[normcode][1];

      a1 = edgep[i1];
      a2 = edgep[i2];
      b1 = edgen[i1];
      b2 = edgen[i2];
      p1 = topt[i1];
      p2 = topt[i2];
      // now that we've picked the coordinate system, transform the point into the edge basis.
      c1 =  fix_mul(b2,p1) - fix_mul(b1,p2);
      c2 =  fix_mul(a1,p2) - fix_mul(a2,p1);
      d  =  fix_mul(a1,b2) - fix_mul(a2,b1);
      if (d < 0) c1 = -c1, c2 = -c2, d = -d;
      PHYS_SPEW(DSRC_PHYSICS_Terrain,("a: %q %q, b %q %q, c %q %q, p %q %q d %q\n",fix_float(a1),fix_float(a2),
         fix_float(b1),fix_float(b2),fix_float(c1),fix_float(c2),fix_float(p1),fix_float(p2),fix_float(d)));
      if (c1 < 0 || c2 < 0)
      {
         fix mag;
         // we're outside the quadrilateral
         PHYS_SPEW(DSRC_PHYSICS_Terrain,("Outside the quad\n"));
         if (c1 > 0)
         {
            p1 -= fix_mul(c1,a1);
            p2 -= fix_mul(c1,a2);
         }
         else if (c2 > 0)
         {
            p1 -= fix_mul(c2,b1);
            p2 -= fix_mul(c2,b2);
         }
         else
         {
            mag = g3_vec_mag((g3s_vector*)topt);
            PHYS_SPEW(DSRC_PHYSICS_Terrain,("topt %q %q %q mag %q\n",PTARGS(topt),fix_float(mag)));
            return mag;
         }
         topt[i1] = p1;
         topt[i2] = p2;

         mag = g3_vec_mag((g3s_vector*)topt);
         PHYS_SPEW(DSRC_PHYSICS_Terrain,("topt %q %q %q mag %q\n",PTARGS(topt),fix_float(mag)));
         return mag;
      }
      else return 0;

   }
   
}

#define mod3(x) (((x) > 2) ? (x) - 3 : (x))

fix facelet_distance_sq_3points(pt3d pt, pt3d flet[NUM_POINTS], uchar normcode)
{
   int i;
   fix best_dsq = FIX_MAX;       // minimum distance squared
   int best_vert = -1;
   fix* best_edge;
   fix* cprod_edge;
   fix  cprod_msq;
   pt3d edgen;
   pt3d cprod;
   pt3d edgep;
   pt3d topt; // vertex to point;
   for (i = 0; i < 3; i++)
   {
      int next = mod3(i+1);
      pt3d edge; // edge vector
      // build the edge & point vectors
      {
//         fix* e = edge;
//         fix* n = flet[next];
         fix* t = topt;
         fix* p = pt;
         fix* v = flet[i];
//         *(e++) = *(n++) - *(v);
         *(t++) = *(p++) - *(v++);
//         *(e++) = *(n++) - *(v);
         *(t++) = *(p++) - *(v++);
//         *(e++) = *(n++) - *(v);
         *(t++) = *(p++) - *(v++);
      }
      {
         fix result;
         dotprod(topt,topt,result);
         PHYS_SPEW(DSRC_PHYSICS_Terrain,("topt %q %q %q dsq %d best_dsq %d\n",PTARGS(topt),result,best_dsq));
         if (result < best_dsq)
         {
            best_dsq = result;
            best_vert = i;
         }   
      }
   }
   PHYS_SPEW(DSRC_PHYSICS_Terrain,("Closest vertex %d\n",best_vert));
    {
      fix a1,a2,b1,b2,p1,p2,c1,c2;
      fix d;
      int nx = (best_vert < 2) ? best_vert + 1 : 0;
      int pr = (best_vert > 0) ? best_vert - 1 : 2;
      fix* ep = edgep;
      fix* en = edgen;
      fix* tp = topt;
      fix* p  = pt;
      fix  dp;
      fix  cp,cn;
      fix  dn;
      fix msp;
      fix msn;
      fix* vert = &flet[best_vert][0];
      fix* prev = &flet[pr][0];
      fix* next = &flet[nx][0];
      PHYS_SPEW(DSRC_PHYSICS_Terrain,("vert: (%d) %q %q %q\n",best_vert,PTARGS(vert)));
      PHYS_SPEW(DSRC_PHYSICS_Terrain,("prev: (%d) %q %q %q\n",pr,PTARGS(prev)));
      PHYS_SPEW(DSRC_PHYSICS_Terrain,("next: (%d) %q %q %q\n",nx,PTARGS(next)));
      *(ep++) = *(prev++) - *(vert);
      *(en++) = *(next++) - *(vert);
      *(tp++) = *(p++)    - *(vert);
      vert++;
      *(ep++) = *(prev++) - *(vert);
      *(en++) = *(next++) - *(vert);
      *(tp++) = *(p++)    - *(vert);
      vert++;
      *(ep++) = *(prev++) - *(vert);
      *(en++) = *(next++) - *(vert);
      *(tp++) = *(p++)    - *(vert);
      vert++;
      PHYS_SPEW(DSRC_PHYSICS_Terrain,("Normal code is %d\n",normcode));

      // now throw out one of the coordinates
      switch(normcode)
      {
         case NORMAL_X:
            a1 = edgep[1];
            a2 = edgep[2];
            b1 = edgen[1];
            b2 = edgen[2];
            p1 = topt[1];
            p2 = topt[2];
            break;
         case NORMAL_Y:
            a1 = edgep[0];
            a2 = edgep[2];
            b1 = edgen[0];
            b2 = edgen[2];
            p1 = topt[0];
            p2 = topt[2];
            break;
         case NORMAL_Z:
            a1 = edgep[0];
            a2 = edgep[1];
            b1 = edgen[0];
            b2 = edgen[1];
            p1 = topt[0];
            p2 = topt[1];
            break;
      }
      // now that we've picked the coordinate system, transform the point into the edge basis.
      c1 = fix_mul(b2,p1) - fix_mul(b1,p2);
      c2 = -fix_mul(a2,p1) + fix_mul(a1,p2);
      d = fix_mul(a1,b2) - fix_mul(a2,b1);
      if (d < 0) c1 = -c1, c2 = -c2, d = -d;
      PHYS_SPEW(DSRC_PHYSICS_Terrain,("a: %q %q, b %q %q, c %q %q, p %q %q d %q\n",fix_float(a1),fix_float(a2),
         fix_float(b1),fix_float(b2),fix_float(c1),fix_float(c2),fix_float(p1),fix_float(p2),fix_float(d)));
      if (c1 < 0 || c2 < 0)
      {
         fix mag;
         // we're outside the quadrilateral
         PHYS_SPEW(DSRC_PHYSICS_Terrain,("Outside the quad\n"));
         if (c1 > 0)
         {
            p1 -= fix_mul(c1,a1);
            p2 -= fix_mul(c1,a2);
         }
         else if (c2 > 0)
         {
            p1 -= fix_mul(c2,b1);
            p2 -= fix_mul(c2,b2);
         }
         else
         {
            mag = g3_vec_mag((g3s_vector*)topt);
            PHYS_SPEW(DSRC_PHYSICS_Terrain,("topt %q %q %q mag %q\n",PTARGS(topt),fix_float(mag)));
            return mag;
         }
         switch (normcode)
         {
            case NORMAL_X:
               topt[1] =  p1;
               topt[2] =  p2;
               break;
            case NORMAL_Y:
               topt[0] =  p1;
               topt[2] =  p2;
               break;
            case NORMAL_Z:
               topt[0] =  p1;
               topt[1] =  p2;
               break;
         }

         mag = g3_vec_mag((g3s_vector*)topt);
         PHYS_SPEW(DSRC_PHYSICS_Terrain,("topt %q %q %q mag %q\n",PTARGS(topt),fix_float(mag)));
         return mag;
      }
      else return 0;

   }
   
}


// Given a facelet, translate it r units in the direction of its normal.
// (Mutates the facelet in place, leaves the normal intact)


void grow_facelet(fix r, fix flet[NUM_POINTS][3])
{
   pt3d *coor = flet;
   g3s_vector norm;
   fix* realnorm = flet[NORM_IDX];

   PHYS_SPEW(DSRC_PHYSICS_Terrain,("Growing by %q\n",fix_float(r)));
   // scale the normal vector
   g3_vec_scale(&norm,(g3s_vector*)realnorm,r);
   // now translate the coords three times
   g3_vec_add((g3s_vector*)*coor,(g3s_vector*)*coor,&norm);
   PHYS_SPEW(DSRC_PHYSICS_Terrain,("Point 0:  %q %q %q\n",PTARGS(*coor)));
   coor++;
   g3_vec_add((g3s_vector*)*coor,(g3s_vector*)*coor,&norm);
   PHYS_SPEW(DSRC_PHYSICS_Terrain,("Point 1:  %q %q %q\n",PTARGS(*coor)));
   coor++;
   g3_vec_add((g3s_vector*)*coor,(g3s_vector*)*coor,&norm);
   PHYS_SPEW(DSRC_PHYSICS_Terrain,("Point 2:  %q %q %q\n",PTARGS(*coor)));
   coor++;
   if (**coor != NO_POINT) // is there a fourth point?
   {
      g3_vec_add((g3s_vector*)*coor,(g3s_vector*)*coor,&norm);
      PHYS_SPEW(DSRC_PHYSICS_Terrain,("Point 3:  %q %q %q\n",PTARGS(*coor)));
   }
}
#else
typedef fix pt3d[3];
#endif


#pragma disable_message(202)
bool FF_terrain( fix X, fix Y, fix Z, bool fast, void* TFF )  { return(TRUE); }
bool FF_raycast (fix x, fix y, fix z, fix vec[3], fix range, fix where_hit[3], terrain_ff *tff) { return (TRUE);}
#pragma disable_message(202)

// ?????
void Terrain(fix fix_x, fix fix_y, fix fix_z, fix rad)
{
}

#ifdef OLD_TERR_FUNC
extern fix tfunc_rad, tfunc_pt[3];
extern fix tfunc_sum[3];
extern int tfunc_cnt[3];
extern fix tfunc_norms[3][3];                // floor, wall, ceil

void full_3d_facelet_action(fix (*fleto)[3], int which) // fix (*norm)[3], int *cnt, fix *sum)
{
   fix proj;
   pt3d projpt;
   fix dist;
   pt3d* flet = fleto;

   PHYS_SPEW(DSRC_PHYSICS_Terrain,("full 3d %d\n",which))
   grow_facelet(tfunc_rad,flet);
   proj = project_onto_facelet(tfunc_pt,projpt,flet);
   // proj must be between 0 and -rad
   if (proj > 0 || proj < -tfunc_rad) return;
   if (flet[3][0] == NO_POINT)
      dist = facelet_distance_sq_3points(projpt,flet,compute_normal_code(flet[NORM_IDX]));
   else
      dist = facelet_distance_sq_4points(projpt,flet,compute_normal_code(flet[NORM_IDX]));

//   PHYS_SPEW(DSRC_PHYSICS_Terrain,("Distance from facelet %d is %q, proj %q \n",i,fix_float(dist),fix_float(proj)));
   // if we're closer than our radius, 
   // scale and add to wall gradient.  
   if ( -proj <= tfunc_rad - dist)
   {
      tfunc_cnt[which] ++;
      g3_vec_add((g3s_vector*)tfunc_norms[which],(g3s_vector*)tfunc_norms[which],(g3s_vector*)flet[NORM_IDX]);
      tfunc_sum[which] -= proj;
      PHYS_SPEW(DSRC_PHYSICS_Terrain,("sum %q, which %d\n",fix_float(-proj),which));
   }
}
#endif

ubyte param_matters[MAP_TYPES] = { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, } ;
#endif  // NOT_YET���

errtype physics_init()
{
	EDMS_data init_data;

   // Start EDMS
	init_data.playfield_size = fix_make(MAP_XSIZE + 4,0);  // note assumption that X and Y sizes are same
	init_data.min_physics_handle = 0;
	init_data.collision_callback = cit_collision_callback;
	init_data.snooz_callback = cit_sleeper_callback;
	init_data.autodestruct_callback = cit_autodestruct_callback;
	init_data.awol_callback = cit_awol_callback;
	init_data.argblock_pointer = (void *)big_buffer;
	EDMS_startup(&init_data);

	// Create some defaults
	LG_memset(&standard_state,0,sizeof(State));	// _memset32l(&standard_state,0,12);

	return(OK);
}

#define NUM_WARMUP_ITERATIONS 10
#define EDMS_WARMUP_TIMESTEP  fix_make(0,0x2000)

/* KLC - doesn't do anything.
errtype physics_warmup()
{
//   int i;
//   for (i=0; i < NUM_WARMUP_ITERATIONS; i++)
//      EDMS_soliton_vector(EDMS_WARMUP_TIMESTEP);
   return(OK);
}
*/

errtype apply_gravity_to_one_object(ObjID oid, fix new_grav)
{
   errtype err = OK;
   if (CHECK_OBJ_PH(oid))
   {
      switch(ObjProps[OPNUM(oid)].physics_model)
      {
         case EDMS_ROBOT:
         {
            Robot parms;
            EDMS_get_robot_parameters(objs[oid].info.ph, &parms);
            parms.gravity = new_grav;
            EDMS_set_robot_parameters(objs[oid].info.ph, &parms);
            break;
         }
         case EDMS_PELVIS:
         {
            Pelvis parms;
            EDMS_get_pelvis_parameters(objs[oid].info.ph, &parms);
            parms.gravity = new_grav;
            EDMS_set_pelvis_parameters(objs[oid].info.ph, &parms);
            break;
         }
         case EDMS_DIRAC:
         {
            Dirac_frame parms;
            EDMS_get_Dirac_frame_parameters(objs[oid].info.ph, &parms);
            parms.gravity = new_grav;
            EDMS_set_Dirac_frame_parameters(objs[oid].info.ph, &parms);
            break;
         }
         default:
            err = ERR_RANGE;
            break;
      }
   }
   return err;
}


// ----------------------------------------------------------------
// Yucky coordinate transformation code.  

#define SIN(x) fix_sin(x)
#define COS(x) fix_cos(x)

// NOTE, the first index of the matrix is the ROW

// ----------------------------------------------
// AND NOW, THE THROWING CODE

#define THROW_YANG_MAX FIXANG_PI/6
#define THROW_XANG_MAX FIXANG_PI/4

fix ID2radius(ObjID id)
{
   int rad = ObjProps[OPNUM(id)].physics_xr;
   if (rad > 0) return fix_make(rad,0)/PHYSICS_RADIUS_UNIT;
   else return standard_robot.size;
}



#define THROW_DISPLACE_RANGE (FIX_UNIT/2)
#define THROW_ORDER2_SCALE (FIX_UNIT*2/3)
#define THROW_RAYCAST_MASS  fix_make(0,0x2000)
#define THROW_RAYCAST_SPEED fix_make(1,0)

bool player_throw_object(ObjID proj_id,  int x, int y, int lastx, int lasty, fix vel) 
{
   LGPoint pos = MakePoint(x,y);
   LGPoint lastpos = MakePoint(lastx,lasty);
   ObjLoc loc;
   Combat_Pt posvec;
   Combat_Pt vector;
   Combat_Pt vector2;
   Combat_Pt locvec;
   fix scale_mag;
   State new_state;
   fix radius = ID2radius(proj_id);

   find_fire_vector(&pos, &vector);
   g3_vec_scale((g3s_vector*)&posvec,(g3s_vector*)&vector,THROW_DISPLACE_RANGE+radius);
   posvec.z += radius; // put the bottom of the object at the cursor, not the center.
   g3_vec_normalize((g3s_vector*)&posvec);
   if (CHECK_OBJ_PH(PLAYER_OBJ))
   {
      if (global_fullmap->cyber)
         EDMS_get_Dirac_frame_viewpoint(objs[PLAYER_OBJ].info.ph, &new_state);
      else
         EDMS_get_pelvic_viewpoint(objs[PLAYER_OBJ].info.ph, &new_state);
   }

//   loc = objs[PLAYER_OBJ].loc;
   locvec.x = new_state.X;
   locvec.y = new_state.Y;
   locvec.z = new_state.Z;
   vector2 = locvec;

   // raycast out from the player to find a good place to put the object
   ray_cast_vector(PLAYER_OBJ,&locvec,posvec,THROW_RAYCAST_MASS,radius,THROW_RAYCAST_SPEED,THROW_DISPLACE_RANGE+radius);

   g3_vec_sub((g3s_vector*)&vector2,(g3s_vector*)&locvec,(g3s_vector*)&vector2);
   scale_mag = g3_vec_mag((g3s_vector*)&vector2);
   loc.p = loc.h = loc.b = 0;

   if (scale_mag < THROW_DISPLACE_RANGE+radius)
   {
      g3_vec_scale((g3s_vector*)&locvec,(g3s_vector*)&vector,max(0,scale_mag - radius/2));
      loc.x =  obj_coord_from_fix(new_state.X+locvec.x);  
      loc.y =  obj_coord_from_fix(new_state.Y+locvec.y);  
      loc.z = obj_height_from_fix(new_state.Z+locvec.z+radius);
      scale_mag = 0;
   }
   else
   {
      g3_vec_scale((g3s_vector*)&locvec,(g3s_vector*)&vector,max(0,THROW_DISPLACE_RANGE));
      loc.x =  obj_coord_from_fix(new_state.X+locvec.x);  
      loc.y =  obj_coord_from_fix(new_state.Y+locvec.y);  
      loc.z = obj_height_from_fix(new_state.Z+locvec.z+radius);
      scale_mag = FIX_UNIT;
   }
   if (scale_mag == 0 || vel == 0)
      string_message_info(REF_STR_DropMessage);

   find_fire_vector(&lastpos, &vector2);
   g3_vec_scale((g3s_vector*)&vector2,(g3s_vector*)&vector2,THROW_ORDER2_SCALE);
   g3_vec_sub((g3s_vector*)&vector,(g3s_vector*)&vector,(g3s_vector*)&vector2);
   g3_vec_normalize((g3s_vector*)&vector);
   g3_vec_scale((g3s_vector*)&vector,(g3s_vector*)&vector,fix_mul(vel,scale_mag));

   obj_move_to_vel(proj_id, &loc, TRUE, vector.x, vector.y, vector.z);
   // let's ignore the thrown object
   EDMS_ignore_collisions(objs[PLAYER_OBJ].info.ph, objs[proj_id].info.ph);
   if (objs[proj_id].obclass == CLASS_GRENADE)
   {
      extern void reactivate_mine(ObjID id);

      // let's get it to hit player after a little time
      if (objs[proj_id].subclass == GRENADE_SUBCLASS_DIRECT)
//      if (GrenadeProps[CPNUM(proj_id)].flags & GREN_MINE_TYPE)
         reactivate_mine(proj_id);
   }
   chg_set_flg(INVENTORY_UPDATE);
   return TRUE;
 }

bool get_phys_info(int ph, fix *list, int cnt)
{
   ObjID id = physics_handle_id[ph];
   State new_state;
   if (ph==-1) return FALSE;
   get_phys_state(ph, &new_state,id);
   if (ObjProps[OPNUM(id)].physics_model == EDMS_ROBOT)
   {
         if (cnt>4)
            cnt=4;
   }

   switch (cnt-1)
   {
      default: if (cnt<=0) return FALSE;
      case 5:  list[5]=fixrad_to_fixang(new_state.gamma);
      case 4:  list[4]=fixrad_to_fixang(-new_state.beta);
      case 3:  list[3]=fixang_from_phys_angle(new_state.alpha);
      case 2:  list[2]=new_state.Z;
      case 1:  list[1]=new_state.Y;
      case 0:  list[0]=new_state.X;
	}
   return TRUE;
}


//-------------------------------------------
// POSTURE STUFF

errtype player_set_posture(ubyte new_posture)
{
   player_struct.posture = new_posture;
   return OK;
}


// --------------------------------------------
// LEANING

errtype player_set_lean(byte x, byte y)
{
   player_struct.leanx = x;
   player_struct.leany = y;
   return OK;
}


#define FIRST_ENRG_PROJ_TYPE 7

// -----------------------------------------------
// here is our wacky collision callback...

#define ENERGY_MINE_DRAIN  10

short PULSER_DAMAGE[10] = { 10, 15, 25, 40, 60, 85, 125, 170, 260, 500 };

#define NUM_DRILL_LEVELS      5
#define MAX_DRILL_DAMAGE      725
#define DRILL_DAMAGE(lev)     (MAX_DRILL_DAMAGE / (NUM_DRILL_LEVELS - (lev)))

ubyte ice_offense_values[] = { 1, 2, 3, 4 };
ubyte ice_penetration[] = { 5, 5, 10, 15};
ubyte ice_damage_modifiers[] = { 10, 20, 40, 80};


errtype collide_objects(ObjID collision, ObjID victim, int bad)
{
   bool destroy_me = TRUE;

   if (objs[collision].obclass == CLASS_GRENADE)
   {
      grenade_contact(collision, bad);
   }
   else if ((objs[collision].obclass == CLASS_PHYSICS) && (objs[collision].subclass == PHYSICS_SUBCLASS_SLOW))
   {
      ObjID owner = objPhysicss[objs[collision].specID].owner;
      int a = objPhysicss[objs[collision].specID].bullet_triple;
      int cp_num;
      ObjLoc   loc = objs[collision].loc;
      extern ObjID damage_sound_id;
      extern char damage_sound_fx;
      bool special_proj = FALSE;

      // set that we've already collided this time
      if (objPhysicss[objs[collision].specID].p3.y)
         return OK;
      else
         objPhysicss[objs[collision].specID].p3.y = 3;

      if (owner != PLAYER_OBJ)
      {
         cp_num = CPNUM(owner);
         switch (objs[owner].obclass)
         {
            case CLASS_CRITTER:
               attack_object(victim, CritterProps[cp_num].attacks[a].damage_type, CritterProps[cp_num].attacks[a].damage_modifier,
                  CritterProps[cp_num].attacks[a].offense_value, CritterProps[cp_num].attacks[a].penetration, 0, 100, NULL, 0, 0, NULL);
               break;
            default:
               if (global_fullmap->cyber && ICE_ICE_BABY(owner))
               {
                  attack_object(victim, CYBER_PROJECTILE_TYPE, ice_damage_modifiers[a],
                     ice_offense_values[a], ice_penetration[a], 0, 100, NULL, 0, 0, NULL);
               }
               break;
         }
      }
      else
      {
         extern void slow_proj_hit(ObjID id, ObjID victim);

         // Was the player firing a special projectile?
         switch (ID2TRIP(collision))
         {
            case DRILLSLOW_TRIPLE:
               if (ICE_ICE_BABY(victim))
               {
                  char soft_lvl = objPhysicss[objs[collision].specID].bullet_triple;
                  short dmg;

                  // Damage the ice -- higher level ICEs are tough, but we always
                  // do at least a little tiny bit of damage
                  dmg = max(1,DRILL_DAMAGE(soft_lvl-1) >> (ICE_LEVEL(victim)) * 2);

                  if (dmg < objs[victim].info.current_hp)
                  {
                     // If it is still alive, agitate it
                     objs[victim].info.current_hp -= dmg;
                     SET_ICE_AGIT(victim,min(ICE_AGIT(victim) + soft_lvl, MAX_AGIT));
                  }
                  else
                  {
                     objs[victim].info.current_hp = 0;
                     DEICE(victim);
                  }
               }
               special_proj = TRUE;
               break;
            case CYBERSLOW_TRIPLE:
               if (!(ICE_ICE_BABY(victim)))
               {
                  char soft_lvl = objPhysicss[objs[collision].specID].bullet_triple;
                  simple_damage_object(victim, PULSER_DAMAGE[soft_lvl-1], CYBER_PROJECTILE_TYPE, 0);
                  special_proj = TRUE;
               }
               break;
#ifdef MANY_CYBERSPACE_WEAPONS
            case DISCSLOW_TRIPLE:
               {
                  char soft_lvl = objPhysicss[objs[collision].specID].bullet_triple;
                  simple_damage_object(victim, disc_damage[soft_lvl-1], CYBER_PROJECTILE_TYPE, 0);
                  special_proj = TRUE;
               }
               break;
            case SPEWSLOW_TRIPLE:
               {
                  char soft_lvl = objPhysicss[objs[collision].specID].bullet_triple;
                  simple_damage_object(victim, cyberspew_damage[soft_lvl-1], CYBER_PROJECTILE_TYPE, 0);
                  special_proj = TRUE;
               }
#else
            case SPEWSLOW_TRIPLE:
            case DISCSLOW_TRIPLE:
               special_proj = TRUE;
               break;
#endif
               break;
         }

         if (!special_proj)
            slow_proj_hit(collision, victim);
      }

      // Why are we destroying "victim" if it's a slow projectile?!?!
      // cause the other one can't damage this one - because it'll be destroyed.
      // only one call allowed

      // this big if here - says that if both objects are physics objects
      // do the thing the flags say! (what to destroy)
      if ((objs[victim].obclass == CLASS_PHYSICS) && (objs[victim].subclass == PHYSICS_SUBCLASS_SLOW))
      {
         if (!(PhysicsProps[CPNUM(victim)].flags & PROJ_PRESERVE_PROJ_HIT))
            ADD_DESTROYED_OBJECT(victim);

         if (!(PhysicsProps[CPNUM(collision)].flags & PROJ_PRESERVE_PROJ_HIT))
            ADD_DESTROYED_OBJECT(collision);
      }
      else if (!(PhysicsProps[CPNUM(collision)].flags & PROJ_PRESERVE_HIT))
         ADD_DESTROYED_OBJECT(collision);

      if (damage_sound_fx != -1)
      {
         play_digi_fx_obj(damage_sound_fx,1,damage_sound_id);
      }
      damage_sound_fx = -1;
   }
   else if ((ID2TRIP(collision) == ENERGY_MINE_TRIPLE) && (victim == PLAYER_OBJ))
   {
      player_struct.energy = max(0, player_struct.energy - ENERGY_MINE_DRAIN);
      play_digi_fx(SFX_ENERGY_DRAIN,1);
      chg_set_flg(VITALS_UPDATE);
   }
   return(OK);
}

void terrain_object_collide(physics_handle src, ObjID target)
{
   ObjID hit_obj = physics_handle_to_id(src);
   if (is_obj_destroyed(hit_obj) || is_obj_destroyed(target))
      return;
   collide_objects(target, hit_obj, 0);
}

void cit_collision_callback(physics_handle C, physics_handle V, int bad, long, long, fix[3] )
{
   ObjID    collision;
   ObjID    victim;

   collision = physics_handle_to_id(C);
   if (is_obj_destroyed(collision))
      return;
   victim = physics_handle_to_id(V);
   if (is_obj_destroyed(victim))
      return;
   collide_objects(collision,victim,bad);
}

void cit_awol_callback(physics_handle caller)
{
   State s;

   if (caller != -1)
      get_phys_state(caller, &s, OBJ_NULL);
   else
   {
      return;
   }
   if ((fix_int(s.X) > global_fullmap->x_size) || (s.X < 0) ||
       (fix_int(s.Y) > global_fullmap->y_size) || (s.Y < 0))
   {
      if (caller != -1)
      {
         ADD_DESTROYED_OBJECT(physics_handle_to_id(caller));
      }
   }
}

void cit_sleeper_callback(physics_handle caller)
{
   ObjID id;
   id = physics_handle_to_id(caller);

   // Is this a kind of thing that wants to maintain it's physics-ness?
   // Live grenades should do this...
   if (ObjProps[OPNUM(id)].flags & EDMS_PRESERVE)
   {
      // Do put-to-sleep code here.
   }
   else
   {
      if (ID2TRIP(id) == L_MINE_TRIPLE)
      {
         if (objGrenades[objs[id].specID].flags & GREN_ACTIVE_FLAG)
         {
            objGrenades[objs[id].specID].flags |= GREN_MINE_STILL;
            EDMS_obey_collisions(caller);
            return; // let's not put it to sleep
         }
      }
      add_edms_delete(caller);
   }
}

void cit_autodestruct_callback(physics_handle)
{
}

bool robot_antisocial = FALSE;


// Build the model given a state and object ID, and assign appropriate
// data into the object and do appropriate bookkeeping
errtype assemble_physics_object(ObjID id, State *pnew_state)
{
	switch(ObjProps[OPNUM(id)].physics_model)
	{
		case EDMS_ROBOT:
		{
			Obj	*pObj = &objs[id];

			Robot new_robot;
			instantiate_robot(ID2TRIP(id),&new_robot);
			pObj->info.ph = EDMS_make_robot(&new_robot, pnew_state);
			if (CHECK_OBJ_PH(id))
			{
				if (robot_antisocial)
					EDMS_make_robot_antisocial(pObj->info.ph);

#ifdef SECRET_NON_COLLISION_BITS
				if ((global_fullmap->cyber) && (pObj->obclass == CLASS_PHYSICS) && (pObj->subclass == PHYSICS_SUBCLASS_SLOW))
					set_secret_non_collision_bit(pObj->info.ph);
#endif
				physics_handle_id[pObj->info.ph] = id;
			}
			break;
		}

		case EDMS_PELVIS:
		{
			Pelvis new_pelvis;
			instantiate_pelvis(ID2TRIP(id),&new_pelvis);
			objs[id].info.ph = EDMS_make_pelvis(&new_pelvis, pnew_state);
			physics_handle_id[objs[id].info.ph] = id;
			break;
		}
	   case EDMS_DIRAC:
	   {
	      Dirac_frame new_dirac;
	      instantiate_dirac(ID2TRIP(id),&new_dirac);
	      objs[id].info.ph = EDMS_make_Dirac_frame(&new_dirac, pnew_state);
	      physics_handle_id[objs[id].info.ph] = id;
	      break;
	   }
   }
   if ((CHECK_OBJ_PH(id)) && (objs[id].info.ph>physics_handle_max))
      physics_handle_max=objs[id].info.ph;
   return(OK);
}

// ======================================================================
//                        MODEL STUFF


// -------------------------------------------
// instantiate_robot() fills in the fields of a robot 
// structure from object properties specified by triple, 
// and level properties.

void instantiate_robot(int triple, Robot* new_robot)
{
   int newmass, newsize;
   short hard,pep;

   *new_robot = standard_robot;
   switch(level_gamedata.gravity)
   {
      case  LEVEL_GRAV_LOW:
         new_robot->gravity = fix_div(standard_robot.gravity, fix_make(3,0));
         break;
      case LEVEL_GRAV_ZERO:
         new_robot->gravity = 0;
         break;
      default:
         if (global_fullmap->cyber ||
             (((TRIP2CL(triple) == CLASS_CRITTER) && (CritterProps[CPTRIP(triple)].flags & AI_FLAG_FLYING)) ||
             (triple == ENERGY_MINE_TRIPLE)))
            new_robot->gravity = 0;
         break;
   }
   newmass = ObjProps[OPTRIP(triple)].mass;
   if (newmass > 0)
      new_robot->mass = fix_make(newmass,0)/(PHYS_MASS_UNIT*PHYS_MASS_C_NUM/PHYS_MASS_C_DEN);
   newsize = ObjProps[OPTRIP(triple)].physics_xr;
   if (newsize > 0)
      new_robot->size = fix_make(newsize,0) / PHYSICS_RADIUS_UNIT;
   hard = ObjProps[OPTRIP(triple)].hardness;
   if (hard > 0)
      new_robot->hardness = fix_make(hard,0) / PHYS_HARDNESS_UNIT;
   pep = ObjProps[OPTRIP(triple)].pep;
   if (pep > 0)
      new_robot->pep = fix_make(pep,0) / PHYS_PEP_UNIT;
//   new_robot->cyber_space = global_fullmap->cyber ? 2 : 0;
   if (new_robot->gravity)
      new_robot->cyber_space = 0;
   else
      new_robot->cyber_space = 1;
}


// -------------------------------------------
// instantiate_pelvis() fills in the fields of a pelvis
// structure from object properties specified by triple, 
// and level properties.

void instantiate_pelvis(int triple, Pelvis* new_pelvis)
{
   int newmass, newsize;
   short hard,pep;

   *new_pelvis = standard_pelvis;
   switch(level_gamedata.gravity)
   {
      case  LEVEL_GRAV_LOW:
         new_pelvis->gravity = fix_div(standard_pelvis.gravity, fix_make(3,0));
         break;
      case LEVEL_GRAV_ZERO:
         new_pelvis->gravity = 0;
         break;
      default:
         if (global_fullmap->cyber)
            new_pelvis->gravity = 0;
         break;
   }
   newmass = ObjProps[OPTRIP(triple)].mass;
   if (newmass > 0)
      new_pelvis->mass = fix_make(newmass,0)/(PHYS_MASS_UNIT*PHYS_MASS_C_NUM/PHYS_MASS_C_DEN);
   newsize = ObjProps[OPTRIP(triple)].physics_xr;
   if (newsize > 0)
      new_pelvis->size = fix_make(newsize,0) / PHYSICS_RADIUS_UNIT;
   hard = ObjProps[OPTRIP(triple)].hardness;
   if (hard > 0)
      new_pelvis->hardness = fix_make(hard,0) / PHYS_HARDNESS_UNIT;
   pep = ObjProps[OPTRIP(triple)].pep;
   if (pep > 0)
      new_pelvis->pep = fix_make(pep,0) / PHYS_PEP_UNIT;
   if (global_fullmap->cyber)
      new_pelvis->cyber_space = PELVIS_MODE_CYBER;
   else
      new_pelvis->cyber_space = (motionware_mode == MOTION_SKATES) ? PELVIS_MODE_SKATES : PELVIS_MODE_NORMAL;
}

// -------------------------------------------
// instantiate_dirac() fills in the fields of a Dirac_frame
// structure from object properties specified by triple, 
// and level properties.

void instantiate_dirac(int triple, Dirac_frame* new_dirac)
{
   int newmass;
   short hard,rough;

   *new_dirac = standard_dirac;
   switch(level_gamedata.gravity)
   {
      case  LEVEL_GRAV_LOW:
         new_dirac->gravity = fix_div(standard_dirac.gravity, fix_make(3,0));
         break;
      case LEVEL_GRAV_ZERO:
         Warning(("Zero gravity level!\n"));
         new_dirac->gravity = 0;
         break;
      default:
//         if (global_fullmap->cyber)
//            new_dirac->gravity = 0;
         break;
   }
   newmass = ObjProps[OPTRIP(triple)].mass;
   if (newmass > 0)
      new_dirac->mass = fix_make(newmass,0)/(PHYS_MASS_UNIT*PHYS_MASS_C_NUM/PHYS_MASS_C_DEN);
   hard = ObjProps[OPTRIP(triple)].hardness;
   if (hard > 0)
      new_dirac->hardness = fix_make(hard,0) / PHYS_HARDNESS_UNIT;
   rough = ObjProps[OPTRIP(triple)].pep;
   if (rough > 0)
      new_dirac->roughness = fix_make(rough,0) / PHYS_ROUGHNESS_UNIT;

   // Whut the heck do we do for the corners of a Dirac_frame?  Beyond my feeble ken, I fear.
   // DIRAC_FIX
}
