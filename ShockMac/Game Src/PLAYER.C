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
 * $Source: r:/prj/cit/src/RCS/player.c $
 * $Revision: 1.153 $
 * $Author: tjs $
 * $Date: 1994/11/21 21:40:54 $
 */

#define __PLAYER_SRC
#include <string.h>

#include "objprop.h"
#include "objwpn.h"
#include "player.h"
#include "mainloop.h"
#include "diffq.h" // for time limit
#include "newmfd.h"
#include "hud.h"
#include "wares.h"
#include "objsim.h"
#include "otrip.h"
#include "faketime.h"
#include "cybrnd.h"
#include "cyber.h"
#include "physics.h"
#include "palfx.h"
#include "emailbit.h"

/*
#include <config.h>
#include <gametime.h>
#include <hkeyfunc.h>
#include <olhext.h>
*/
#include "miscqvar.h"

#define CFG_FATIGUE_VAR "fatigue"

extern int nth_after_triple(int trip, ubyte n);

// couldnt we atob this and have a cfg file???
// then we could read it in, turn them on, free the memory, and not have to recompile
// for instance
// void setup_qbits(char *fn)
// { FILE *fp=fopen(fn,"rb"); if (fp!=NULL) { while(!feof(fp)) { fread(&cow,1,2,fp); qbit[cow]=1; } close(fp); } }

// yeah yeah, goddamn, gotta fix this.....someday....soon....i promise....really
short turnon_questbits[] = { 0x1,0x2,0x3,0x10,0x12,0x15,0x16,0x17,0x18,0x19,0x1a,
   0x20,0x21,0x24,0x25,
   0x4b,0x4c,0x4d,0x4e,0x4f,
   0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
   0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
   0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,
   0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
   0xe1,0xe3,0xe5,0xe7,0xe9,0xeb,0xed,0xef,
   0xf1,0xf3,0xf5,0xf7,0xf9,0xfb,0xfd,0xff,
   0x101,0x103,0x105,0x107,0x109,0x10b,0x10d,0x10f,
   0x111,0x113,0x115,0x117,0x119,0x11b,0x11d,0x11f,
   0x121,0x123,0x125,0x127,0x129,0x12b,
};

#define NUM_INIT_QV  3
ushort init_questvars[NUM_INIT_QV][2] = {
   {0x3, 2}, // engine state
   {0xC, 3}, // num groves
   {0x33, 256}, // joystick sensitivity
};

#define NUM_ON_QUESTBITS (sizeof(turnon_questbits)/sizeof(short))


// -------------------------------------------------------------
// init_player()
//
// -------------------------------------------------------------

extern void player_reset_eye(void);
#define REACTOR_COMBO_QVAR (0x1F)

errtype init_player(Player *pplr)
{
   int i,j;
   char tmp[sizeof(pplr->name)];
   long	tmpdiff;
   extern int _fr_global_detail;
   extern uchar which_lang;

   LG_memcpy(tmp,pplr->name,sizeof(tmp));				// Save these so they won't be cleared.
   tmpdiff = *(long *)pplr->difficulty;

   // Zero out whole structure
   LG_memset(pplr, 0, sizeof(Player));

   LG_memcpy(pplr->name,tmp,sizeof(pplr->name));	// Now restore them.
   BlockMoveData(&tmpdiff, pplr->difficulty, 4);
   
   // Set appropriate non-zero things.
   pplr->detail_level = _fr_global_detail;
   pplr->level = 1;
   for (i = 0; i < NUM_LEVELS; i++)
      pplr->initial_shodan_vals[i] = -1;
   pplr->rep = OBJ_NULL;
   pplr->curr_target = OBJ_SPEC_NULL;
   for (i = 0; i < NUM_GENERAL_SLOTS; i++)
      pplr->inventory[i] = OBJ_NULL;
   pplr->hit_points       = PLAYER_MAX_HP * 5 / 6;
   pplr->cspace_hp        = PLAYER_MAX_HP;
   pplr->cspace_time_base = BASE_CSPACE_TIME;
   pplr->hit_points_regen = 0;
   LG_memset(pplr->hit_points_lost,0,NUM_DAMAGE_TYPES*sizeof(pplr->hit_points_lost[0]));
   pplr->accuracy         = MAX_ACCURACY;
   pplr->energy           = MAX_ENERGY;
   pplr->shield_absorb_rate = 0;
   pplr->last_fire = 0;

   // Here is where we set the questbits for doors that start locked,
   // things that start frobbed or what not
   // To set your very own questbit, just add a line of the form:

   // QUESTBIT_ON(number);

   for (i=0; i < NUM_ON_QUESTBITS; i++)
      QUESTBIT_ON(turnon_questbits[i]);

   for (i=0; i < NUM_INIT_QV; i ++)
      QUESTVAR_SET(init_questvars[i][0], init_questvars[i][1]);

   while(QUESTVAR_GET(REACTOR_COMBO_QVAR)==QUESTVAR_GET(REACTOR_COMBO_QVAR+1)) {
      // randomize reactor combination. use effect_rnd 'cause why not.
      j = (RndRange(&effect_rnd,0,9)<<8)|(RndRange(&effect_rnd,0,9)<<4)
         | RndRange(&effect_rnd,0,9);
      QUESTVAR_SET(REACTOR_COMBO_QVAR,j);
      j = (RndRange(&effect_rnd,0,9)<<8)|(RndRange(&effect_rnd,0,9)<<4)
         | RndRange(&effect_rnd,0,9);
      QUESTVAR_SET(REACTOR_COMBO_QVAR+1,j);
   }
   QUESTVAR_SET(LANGUAGE_QVAR,which_lang);

   pplr->fatigue_regen = 0;
   pplr->fatigue_regen_base = 100;
   pplr->fatigue_regen_max  = 400;

   // Initialize MFD dynamic variables

   for(i=0;i<NUM_MFDS;i++)
      pplr->mfd_save_slot[i] = -1;

   for (i=0; i < NUM_MFDS; i++)
   {
      pplr->mfd_empty_funcs[i] = MFD_EMPTY;

      for (j = 0; j < MFD_NUM_VIRTUAL_SLOTS; j++)
         pplr->mfd_virtual_slots[i][j] = j;
   }
   for (i = 0; i < MFD_NUM_REAL_SLOTS; i++)
      pplr->mfd_all_slots[i] = i;

   for (i = 0; i < NUM_WEAPON_SLOTS; i++)
      pplr->weapons[i].type = EMPTY_WEAPON_SLOT;

   for (i = 0; i < NUM_GRENADEZ; i++)
      pplr->grenades_time_setting[i] = 70;

   pplr->hardwarez[CPTRIP(FULLSCR_HARD_TRIPLE)] = 1;
   pplr->email[26] = EMAIL_GOT;
   pplr->active_bio_tracks = 0xFF;
   pplr->actives[ACTIVE_EMAIL] = 0xFF;

//��� take out.  This gives me everything!
//for (i=0; i < NUM_HARDWAREZ; i++)
//	pplr->hardwarez[i] = 1;

//pplr->softs.misc[(TRIP2TY(GAMES_TRIPLE) + NUM_ONESHOT_SOFTWARE)] = 0xff;
/*
pplr->weapons[0].type = 4;
pplr->weapons[0].subtype = 1;
pplr->weapons[2].ammo = 25;
pplr->weapons[1].type = 4;
pplr->weapons[1].subtype = 2;
pplr->weapons[2].type = 0;
pplr->weapons[2].subtype = 4;
pplr->weapons[2].ammo = 100;
pplr->weapons[3].type = 1;
pplr->weapons[3].subtype = 0;
pplr->weapons[3].ammo = 150;
pplr->weapons[4].type = 2;
pplr->weapons[4].subtype = 0;
pplr->weapons[4].ammo = 50;
pplr->weapons[5].type = 4;
pplr->weapons[5].subtype = 0;
pplr->weapons[6].type = 3;
pplr->weapons[6].subtype = 0;
//���
*/

   // init physics stuff
   player_reset_eye();

   return OK;
}

/*
// ---------------------------------------------------------
// player_dead()
//
// called if game is over and you don't run a cutscene!!!
//

#define FADE_DOWN_TIME   300
void player_dead(void)
{
   extern void mouse_unconstrain(void);

//   ulong  dead_time = *tmd_ticks;
//   extern void object_data_flush(void);

//   palfx_fade_down();
//   while (*tmd_ticks < (dead_time+FADE_DOWN_TIME)) ;
//   gr_clear(0);

#ifdef AUDIOLOGS
   {
      extern char secret_pending_hack;
      secret_pending_hack = 0;
   }
#endif

   mouse_unconstrain();

   change_mode_func(0,0,(void *)SETUP_LOOP);

//   palfx_fade_up(FALSE);
}

errtype player_tele_to(int x, int y)
{
   ObjLoc newloc;
   newloc = objs[PLAYER_OBJ].loc;
   newloc.x = (x << 8) + 0x80;
   newloc.y = (y << 8) + 0x80;
   return(OK);
}

*/

#define INITIAL_PLAYER_X   0x1E
#define INITIAL_PLAYER_Y   0x16

errtype player_create_initial()
{
   ObjLoc plr_loc;
   extern Pelvis standard_pelvis;

   plr_loc.x=(INITIAL_PLAYER_X<<8)+0x80;
   plr_loc.y=(INITIAL_PLAYER_Y<<8)+0x80;
   plr_loc.h = 192;
   plr_loc.z = obj_height_from_fix((standard_pelvis.height>>1)+fix_from_map_height(me_height_flr(MAP_GET_XY(plr_loc.x>>8,plr_loc.y>>8))));
   plr_loc.p = 0;
   plr_loc.b = 0;
   //of course z is about to be overwritten, but hey...
   player_struct.edms_state[0] = 0;
   obj_create_player(&plr_loc);
   return(OK);
}

#define CFG_PLAYER_VAR "eye" 
errtype player_startup(void)
{
   return OK;      
//      return player_create_initial();
}

errtype player_shutdown(void)
{
   return OK;
}

ubyte set_player_energy_spend(ubyte new_val)
{
   if (player_struct.energy_spend != new_val)
   {
      hud_set_time(HUD_ENERGYUSE, 3*CIT_CYCLE);
      player_struct.energy_spend = new_val;
   }
   return(player_struct.energy_spend);
}

//----------------------------------------------------------------
// KLC - Probably a goofy way to do this, but what the hey!  This tells me if the player struct
//            has the fullscreen ware on.
//----------------------------------------------------------------
Boolean IsFullscreenWareOn(void)
{
	ubyte		status = player_struct.hardwarez_status[CPTRIP(FULLSCR_HARD_TRIPLE)];
	
	return ((Boolean)(status & WARE_ON));
}
