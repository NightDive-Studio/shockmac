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
 * $Source: r:/prj/cit/src/RCS/movekeys.c $
 * $Revision: 1.25 $
 * $Author: mahk $
 * $Date: 1994/11/22 22:53:07 $
 *
 */

#include <stdlib.h>

#include "input.h"
#include "player.h"
#include "physics.h"
#include "gamesys.h" 
#include "weapons.h"

// Here we go, it's gruesome keyboard polling code.  

// -------
// DEFINES
// -------

#define KBCOUNTRY_CFG "country" 


#define KEYBD_CONTROL_BANK 1
#define MOTION_RUN_KEY     'S'

#define CTRL KB_FLAG_CTRL
#define ALT  KB_FLAG_ALT
#define SHF  KB_FLAG_SHIFT


// Ok, here's a bunch of scan codes.
// KLC - these have all been changed for the Mac keyboards
#define _Q_			0x0C // q
#define _W_			0x0D // w
#define _E_			0x0E // e
#define _D_			0x02 // d
#define _A_			0x00 // a
#define _S_ 			0x01 // s
#define _Z_			0x06 // z
#define _X_			0x07 // x
#define _C_			0x08 // c
#define _UP_			0x7E // uparrow
#define _LEFT_		0x7B // leftarrow
#define _DOWN_	0x7D // downarrow
#define _RIGHT_	0x7C // rightarrow
#define _PGUP2_	0x5C // keypad pgup
#define _HOME2_	0x59 // keypad home
#define _END2_		0x53 // keypad End
#define _PGDN2_	0x55 // keypad pgdn
#define _PAD5_		0x57 // keypad 5
#define _UP2_		0x5B // second uparrow
#define _LEFT2_ 	0x56 // second leftarrow
#define _DOWN2_	0x54 // second downarrow
#define _RIGHT2_	0x58 // second rightarrow
#define _SPACE_	0x31 // spacebar
#define _ENTER2_	0x4C // keypad enter
#define _ENTER_	0x24 // enter
#define _R_			0x0F // r
#define _V_			0x09 // v
#define _J_			0x26 // j


// Ok, these are cooked keycodes for all the motion keys.  
// If you want to add a motion key for a motion function 
// that already exists, just add it to the appropriate #define.  
// Otherwise, create a new #define, and a new case in the parse_motion_key 
// switch statement.


#define RUN_CASES				case _UP_|SHF: \
                        						case _UP_|ALT: \
                           						case _UP2_|SHF: \
                           						case _UP2_|ALT: 
#define JOG_THRUST_CASES	case  _S_:   
#define RUN_THRUST_CASES	case  _S_|SHF:                           
#define JOG_CASES          		case _UP_: \
											case _UP2_:
#define BACK_CASES				case _X_: \
											case _X_|SHF: \
											case _DOWN_: \
											case _DOWN2_: \
 											case _DOWN_|ALT: \
 											case _DOWN2_|ALT: \
 											case _DOWN_|SHF: \
 											case _DOWN2_|SHF:
#define ROTL_CASES				case _LEFT_: \
											case _A_:  \
											case _LEFT2_: 
#define ROTL_FAST_CASES		case _A_|SHF: \
											case _LEFT_|SHF: \
											case _LEFT2_|SHF: 
#define ROTR_CASES				case _RIGHT_: \
											case _D_: \
											case _RIGHT2_:
#define ROTR_FAST_CASES		case _RIGHT_|SHF: \
											case _D_|SHF: \
											case _RIGHT2_|SHF:
#define SLIDL_CASES				case _Z_: \
											case _LEFT_|ALT: \
											case _END2_: \
											case _Z_|SHF: \
											case _LEFT2_|ALT:
#define SLIDR_CASES				case _C_: \
											case _RIGHT_|ALT: \
											case _RIGHT2_|ALT: \
											case _PGDN2_: \
											case _C_|SHF: 
#define LEANL_CASES				case _Q_: \
											case _LEFT_|CTRL: \
											case _LEFT2_|CTRL: \
											case _Q_|SHF:  
#define LEANR_CASES				case _E_: \
											case _RIGHT_|CTRL: \
											case _RIGHT2_|CTRL: \
											case _E_|SHF:  
#define LEANUP_CASES			case _W_: \
											case _W_|CTRL: \
											case _W_|ALT: \
											case _W_|SHF:
#define JUMP_CASES				case _J_: \
											case _J_|SHF:
//KLC									case _SPACE_|SHF: \
//KLC									case _SPACE_|ALT: \
//KLC									case _SPACE_|CTRL: \
//KLC									case _SPACE_:
#define EYEUP_CASES				case _R_: \
											case _R_|CTRL: \
											case _UP_|CTRL: \
											case _UP2_|CTRL: \
											case _R_|SHF:
#define EYEDOWN_CASES		case _V_: \
											case _V_|CTRL: \
											case _DOWN_|CTRL: \
											case _DOWN2_|CTRL: \
											case _V_|SHF:

#define FORW_L_CASES			case _HOME2_:
#define FORW_R_CASES			case _PGUP2_:


// -----------------
// CYBERSPACE CONTROLS
// -----------------
#define THRUST_CASES			case _S_: \
											case _S_|SHF: \
											case _PAD5_: \
//KLC									case _SPACE_: 
#define PITCH_UP_CASES		case _W_: \
											case _W_|SHF: \
											case _UP_:  \
											case _UP_|SHF: \
											case _UP_|CTRL: \
											case _UP_|ALT: \
 											case _UP2_:  \
 											case _UP2_|SHF: \
 											case _UP2_|CTRL: \
 											case _UP2_|ALT:
#define PITCH_DN_CASES		case _X_: \
											case _X_|SHF: \
											case _DOWN_: \
											case _DOWN_|SHF: \
											case _DOWN_|CTRL: \
											case _DOWN_|ALT: \
 											case _DOWN2_:  \
 											case _DOWN2_|SHF: \
 											case _DOWN2_|CTRL: \
 											case _DOWN2_|ALT:
#define BANK_L_CASES			case _A_: \
											case _A_|SHF: \
											case _LEFT2_: \
											case _LEFT2_|SHF: \
											case _LEFT2_|ALT: \
											case _LEFT2_|CTRL:
#define BANK_R_CASES			case _D_: \
											case _D_|SHF: \
											case _RIGHT2_: \
											case _RIGHT2_|SHF: \
											case _RIGHT2_|CTRL: \
 											case _RIGHT2_|ALT:
#define UP_L_CASES				case _HOME2_: \
											case _HOME2_|SHF: \
											case _HOME2_|CTRL: \
											case _HOME2_|ALT:
#define UP_R_CASES				case _PGUP2_:  \
											case _PGUP2_|SHF: \
											case _PGUP2_|CTRL: \
											case _PGUP2_|ALT:
#define DOWN_L_CASES			case _END2_:  \
											case _END2_|SHF: \
											case _END2_|CTRL: \
											case _END2_|ALT:
#define DOWN_R_CASES			case _PGDN2_: \
											case _PGDN2_|SHF: \
											case _PGDN2_|CTRL: \
											case _PGDN2_|ALT:
#define ROLL_L_CASES			case _Q_: \
											case _Q_|SHF: \
											case _Z_: \
											case _Z_|SHF:
#define ROLL_R_CASES			case _E_: \
											case _E_|SHF: \
											case _C_: \
											case _C_|SHF:


#define SLOW_TURN_RATE  (CONTROL_MAX_VAL/2)                   

#define LEAN_CONTROL_NUM CONTROL_XZROT
#define EYE_CONTROL_NUM  CONTROL_YZROT


// -------
// GLOBALS
// -------

extern uchar motion_key_scancodes[];  // KBC_NONE terminated scancodes

// ---------
// INFERNALS
// ---------
bool parse_motion_key(ushort keycode, short* cnum, short* cval);
bool parse_motion_key_cyber(ushort keycode, short* cnum, short* cval);
void init_motion_polling(void);
void setup_motion_polling(void);
void process_motion_keys(void);
bool motion_keycheck_handler(uiEvent* ev, LGRegion*, void*);


extern void physics_set_relax(int axis, bool relax);

static byte poll_controls[6];


// Hey, this deals with motion keys
bool parse_motion_key(ushort keycode, short* cnum, short* cval)
{
   *cnum = -1;
   *cval = 0;
   switch (keycode)
   {
   RUN_THRUST_CASES
   RUN_CASES
  	   *cnum = CONTROL_YVEL;
      *cval = CONTROL_MAX_VAL;
	   break;
   JOG_THRUST_CASES
   JOG_CASES
   	 *cnum = CONTROL_YVEL;
	    *cval = CONTROL_MAX_VAL/2;
       break;

   ROTL_FAST_CASES        
         *cval = -(CONTROL_MAX_VAL-SLOW_TURN_RATE);
   turn_l:
   ROTL_CASES
	   *cnum = CONTROL_XYROT;
	   *cval += -SLOW_TURN_RATE;
	   break;

   ROTR_FAST_CASES        
         *cval = (CONTROL_MAX_VAL - SLOW_TURN_RATE);
   turn_r:
   ROTR_CASES
	   *cnum = CONTROL_XYROT;
	   *cval += SLOW_TURN_RATE;
	   break;

   BACK_CASES
	   *cnum = CONTROL_YVEL;
	   *cval = -CONTROL_MAX_VAL/2;
      break;
   SLIDL_CASES
	   *cnum = CONTROL_XVEL;
	   *cval = -CONTROL_MAX_VAL;
   	break;
   SLIDR_CASES
	   *cnum = CONTROL_XVEL;
	   *cval = CONTROL_MAX_VAL;
	   break;
   JUMP_CASES
	   *cnum = CONTROL_ZVEL;
	   *cval = MAX_JUMP_CONTROL;
	   break;
   LEANUP_CASES
      *cnum = LEAN_CONTROL_NUM;
      *cval = 0;
      physics_set_relax(*cnum,TRUE);
      break;
   LEANL_CASES
      *cval = -CONTROL_MAX_VAL;
      *cnum = LEAN_CONTROL_NUM;
      physics_set_relax(*cnum,TRUE);
      break;
   LEANR_CASES
	   *cval = CONTROL_MAX_VAL;
      *cnum = LEAN_CONTROL_NUM;
      physics_set_relax(*cnum,TRUE);
   	break;
   EYEUP_CASES
      *cnum = EYE_CONTROL_NUM;
      *cval = CONTROL_MAX_VAL;
//      physics_setq_relax(*cnum,TRUE);
      break;
   EYEDOWN_CASES
      *cnum = EYE_CONTROL_NUM;
      *cval = -CONTROL_MAX_VAL;
//      physics_set_relax(*cnum,TRUE);
      break;
   FORW_L_CASES
      *cnum = CONTROL_YVEL;
      *cval = CONTROL_MAX_VAL;
      if (abs(poll_controls[*cnum]) < abs(*cval))
         poll_controls[*cnum] = *cval;
      *cval = 0;
      goto turn_l;
   FORW_R_CASES
      *cnum = CONTROL_YVEL;
      *cval = CONTROL_MAX_VAL;
      if (abs(poll_controls[*cnum]) < abs(*cval))
         poll_controls[*cnum] = *cval;
      *cval = 0;
      goto turn_r;
   }
   return *cnum != -1;
}


bool parse_motion_key_cyber(ushort keycode, short* cnum, short* cval)
{
   keycode &= ~KB_FLAG_2ND;
   *cnum = -1;
   *cval = 0;
   switch (keycode)
   {
   THRUST_CASES
      *cnum =  CONTROL_ZVEL;
      *cval  = MAX_JUMP_CONTROL;
      break;

   PITCH_UP_CASES
  	   *cnum = CONTROL_YVEL;
      *cval = -CONTROL_MAX_VAL;
	   break;
   bank_l:
   BANK_L_CASES   
	   *cnum = CONTROL_XYROT;
	   *cval = -CONTROL_MAX_VAL;
      break;
   bank_r:
   BANK_R_CASES
      *cnum = CONTROL_XYROT;
      *cval = CONTROL_MAX_VAL;
	   break;
   UP_L_CASES
      *cnum = CONTROL_YVEL;
      *cval = -CONTROL_MAX_VAL;
      if (abs(poll_controls[*cnum]) < abs(*cval))
         poll_controls[*cnum] = *cval;
      goto bank_l;
   UP_R_CASES
      *cnum = CONTROL_YVEL;
      *cval = -CONTROL_MAX_VAL;
      if (abs(poll_controls[*cnum]) < abs(*cval))
         poll_controls[*cnum] = *cval;
      goto bank_r;
   DOWN_R_CASES
      *cnum = CONTROL_YVEL;
      *cval = CONTROL_MAX_VAL;
      if (abs(poll_controls[*cnum]) < abs(*cval))
         poll_controls[*cnum] = *cval;
      goto bank_r;
   DOWN_L_CASES
      *cnum = CONTROL_YVEL;
      *cval = CONTROL_MAX_VAL;
      if (abs(poll_controls[*cnum]) < abs(*cval))
         poll_controls[*cnum] = *cval;
      goto bank_l;
   PITCH_DN_CASES
	   *cnum = CONTROL_YVEL;
	   *cval = CONTROL_MAX_VAL;
      break;
   ROLL_L_CASES
      *cnum = CONTROL_XVEL;
      *cval = -CONTROL_MAX_VAL;
      break;
   ROLL_R_CASES
      *cnum = CONTROL_XVEL;
      *cval = CONTROL_MAX_VAL;
      break;
   }
   return *cnum != -1;
}


// ---------
// EXTERNALS
// ---------

extern bool fire_slam;

void init_motion_polling(void)
{
   uiSetKeyboardPolling(motion_key_scancodes);
/* KLC no longer needed
   int i,cnt;
   cnt = 1;
   config_get_value(KBCOUNTRY_CFG,CONFIG_INT_TYPE,&i,&cnt);
   if (cnt > 0)
      kb_set_country(i);
*/
}

void setup_motion_polling(void)
{
   LG_memset(poll_controls,0,sizeof(poll_controls));
}

void process_motion_keys(void)
{
   physics_set_player_controls(KEYBD_CONTROL_BANK,poll_controls[0],poll_controls[1],poll_controls[2],poll_controls[3],poll_controls[4],poll_controls[5]);
}


extern Boolean	gKeypadOverride;

bool motion_keycheck_handler(uiEvent* ev, LGRegion*, void*)
{
	uiPollKeyEvent	*ke = (uiPollKeyEvent *)ev;
	
	// KLC - For Mac version, we'll cook our own, since we have the modifier information.
	ushort	cooked = ke->scancode | ke->mods;

	short cnum,cval;
	
	int	moveOK = TRUE;
	
/* KLC - this doesn't appear to help any
	// Check for firing weapon first (if it hasn't fired already).	
	if (ke->scancode == _SPACE_ || ke->scancode == _ENTER_ || ke->scancode == _ENTER2_)
	{
		if (!fire_slam)
		{
			extern LGRegion* _current_view;
			LGPoint pos;
			ui_mouse_get_xy(&pos.x,&pos.y);
	         fire_player_weapon(&pos, _current_view, !fire_slam);
	      	fire_slam = TRUE;
	    }
	}
	
	// Check for motion keys
	else
*/
	if (gKeypadOverride)														// if a keypad is showing
	{
		if (ke->scancode >= 0x52 && ke->scancode <= 0x5C)		// and a keypad number was entered,
			moveOK = FALSE;													// don't move.
	}
	
	if (moveOK)
	{
		if (global_fullmap->cyber && parse_motion_key_cyber(cooked,&cnum,&cval)
			 || parse_motion_key(cooked,&cnum,&cval))
		{
			if (abs(poll_controls[cnum]) < abs(cval))
				poll_controls[cnum] = cval;
		}
	}
	return TRUE;
}


// Here is the list of SCAN CODES for all the motion keys.  
// If you want to add a motion key, you MUST PUT ITS SCANCODE HERE
// or else the key will not get polled. 

uchar motion_key_scancodes[] =
   {
      _Q_     , // q
      _W_     , // w
      _E_     , // e
      _D_     , // d
      _A_     , // a
      _S_     , // s
      _Z_     , // z
      _X_     , // x
      _C_     , // c
      _UP_    , // uparrow
      _LEFT_  , // leftarrow
      _DOWN_  , // downarrow
      _RIGHT_ , // rightarrow
      _PGUP2_  , // keypad pgup
      _HOME2_  , // keypad home
      _END2_   , // End
      _PGDN2_ , // keypad pgdn
      _PAD5_  , // keypad 5
      _UP2_   , // second uparrow
      _LEFT2_ , // second leftarrow
      _DOWN2_ , // second downarrow
      _RIGHT2_, // second rightarrow
//KLC      _SPACE_ , // spacebar
//KLC      _ENTER2_, // keypad enter
//KLC      _ENTER_ , // enter
      _R_     , // r
      _V_     , // v
      _J_     , // j

      KBC_NONE, // end of list
   }; 
                                 
