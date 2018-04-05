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
#ifndef __MAINLOOP_H
#define __MAINLOOP_H

#include "gameloop.h"
#include "frtypesx.h"

#define QUIT_LOOP    -1
#define GAME_LOOP     0
#define FULLSCREEN_LOOP 1
#define EDIT_LOOP     2
#define CYBER_LOOP    3
#define SETUP_LOOP    4
#define MWORK_LOOP    5
#define CUTSCENE_LOOP 6
#define SVGA_LOOP     7
#define AUTOMAP_LOOP  8

#define ML 0x1000
#define GL 0x1100
#define EL 0x1200
#define CL 0x1400
#define SL 0x1800
#define WL 0x2000
#define FL 0x2100
#define AL 0x2200

#define ML_CHG_MASK 0xF000             /* mask for main loop bits in change_flag */
#define ML_CHG_BASE 0x1000             /* mask for single main loop bit of change_flag */
#define LL_CHG_MASK 0x0FFF             /* mask for local loop bits of change_flag */
#define LL_CHG_BASE 0x0001             /* mask for single local loop bit out of change_flag */

#define chg_set_flg(x) (_change_flag|=x)
#define chg_get_flg(x) (_change_flag&x)
#define chg_unset_flg(x) (_change_flag &= ~(x))

#define chg_set_sta(x) (_static_change|=x)
#define chg_get_sta(x) (_static_change&x)
#define chg_unset_sta(x) (_static_change &= ~(x))

#define GL_CHG_1       (ML_CHG_BASE<<0)
#define GL_CHG_2       (ML_CHG_BASE<<1)
#define GL_CHG_3       (ML_CHG_BASE<<2)
#define GL_CHG_LOOP    (ML_CHG_BASE<<3)

void mainloop(int argc, char *argv[]);
void loopmode_switch(short *cmode);
errtype static_change_copy();
void loopmode_exit(short loopmode);

#ifdef __MAINLOOP_SRC
frc *_current_fr_context;
short _current_loop  = SETUP_LOOP;          /* which loop we currently are */
short _current_3d_flag = DEMOVIEW_UPDATE;
LGRegion *_current_view = NULL;
uint  _change_flag   = 0;          /* change flags for loop */
uint  _static_change = 0;          /* current static changes */
short _new_mode      = 0;          /* mode to change to, if any */
short _last_mode     = 0;          /* last mode, if you want to change back to it */
bool time_passes = TRUE;
bool saves_allowed = FALSE;
bool physics_running = TRUE;
bool ai_on = TRUE;
bool anim_on = TRUE;
bool player_invulnerable = FALSE;
bool player_immortal = FALSE;
bool always_render = FALSE;
bool pal_fx_on = TRUE;

#else     // NOT _MAINLOOP_SRC

extern short _current_loop;   	  /* which loop we currently are */
extern short _current_3d_flag;
extern frc *_current_fr_context;
#ifdef GADGET
extern Gadget *_current_root;
#endif
extern uint  _change_flag;         /* change flags for loop */
extern uint  _static_change;       /* current static changes */
extern short _new_mode;            /* mode to change to, if any */
extern short _last_mode;           /* last mode we were in, to switch back to */
extern bool player_invulnerable;
extern bool player_immortal;
extern bool physics_running;
extern bool ai_on;
extern bool anim_on;
extern bool always_render;
extern bool saves_allowed;
extern bool time_passes;
extern bool pal_fx_on;
extern LGRegion *_current_view;
#endif

#define loopLine(num,code_line) code_line

#define localChanges   (_change_flag&LL_CHG_MASK)
#define globalChanges  (_change_flag&ML_CHG_MASK)

#endif
