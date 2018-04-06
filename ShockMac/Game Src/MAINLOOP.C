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
 * $Source: r:/prj/cit/src/RCS/mainloop.c $
 * $Revision: 1.42 $
 * $Author: xemu $
 * $Date: 1994/11/09 02:09:05 $
 */

/*
 * Citadel main loops
 *
 * The idea here is that we have a separate loop for each game mode/setup
 * There is a 4/12 bit change flag which is 4 global and 12 local
 * The global loop checks OS and input
 * Then calls the local loop, which does it's internal inlined functions
 *  and then processes it's change flags
 * The global loop then gets control and does it's own change flags
 * If you want to switch modes/loops you call change_loop which both sets
 *  global change bit 3 as well setting some variables.  When the main loop
 *  reaches the bottom it triggers on change bit 3 and calls the switch code
 */

#define __MAINLOOP_SRC

#include <stdio.h>

#include "criterr.h"
#include "init.h"
#include "rendtool.h"
#include "game_screen.h"
#include "fullscrn.h"
#include "loops.h"

/*���
#include <input.h>
#include <setup.h>
#include <loopdbg.h>
#include <cutsloop.h>
#include <cutscene.h>

#include <wsample.h>
*/
// how is the game doing, anyway, set to true at end of time
bool cit_success=FALSE;

// are we "paused"
bool game_paused = FALSE;

void ShockMain(void);

extern void automap_loop(void);
extern void amap_start();
extern void amap_exit();

/*���
// Note that in the shipping version, the edit_loop stuff should never
// get called, but needs to be SOMETHING as a place holder
void (*citadel_loops[])(void)={game_loop,game_loop,game_loop,game_loop,setup_loop,game_loop,cutscene_loop,game_loop,automap_loop};
void (*enter_modes[])(void)={screen_start,fullscreen_start,screen_start,screen_start,setup_start,screen_start,cutscene_start,fullscreen_start,amap_start};
void (*exit_modes[])(void)={screen_exit,fullscreen_exit,screen_exit, screen_exit,setup_exit,screen_exit,cutscene_exit,fullscreen_exit,amap_exit};
*/
// ��� Next two lines are temporary
void (*enter_modes[])(void)={screen_start, fullscreen_start, NULL,NULL,NULL,NULL,NULL,NULL,amap_start};
void (*exit_modes[])(void)={screen_exit, fullscreen_exit, NULL, NULL,NULL,NULL,NULL,NULL,amap_exit};

void loopmode_switch(short *cmode)
{
#ifdef SVGA_SUPPORT
	extern bool wrapper_screenmode_hack;
#endif

	// Actually switch mode
	_last_mode = *cmode;
	(*exit_modes[_last_mode])();
	*cmode=_new_mode;
	_static_change = 0;
	if (*cmode>=0)
		(*enter_modes[*cmode])();
/*���
#ifdef SVGA_SUPPORT
	if (wrapper_screenmode_hack)
	{
	extern void screenmode_screen_init(void);
	extern void wrapper_start(void (*init)(void));
	wrapper_start(screenmode_screen_init);
	}
#endif*/
}

void loopmode_exit(short loopmode)
{
	if (exit_modes[loopmode])
		(*exit_modes[loopmode])();
}

/*���
void loopmode_enter(short loopmode)
{
   (*enter_modes[loopmode])();
}
*/
void ShockMain(void)
{
	init_all();
/*���
	while (_current_loop>=0)
	{
		if (!(_change_flag&(ML_CHG_BASE<<1)))
			loopLine(ML|1,input_chk());        			// go get the UI stuff going
		(*citadel_loops[_current_loop])();
		if (globalChanges)										// really, only loopmode_switch (the <<3 case) 
		{  																// will be in the game
			if (_change_flag&(ML_CHG_BASE<<0)) { loopLine(ML|0x10,loop_debug()); }
			if (_change_flag&(ML_CHG_BASE<<3)) { loopLine(ML|0x13,loopmode_switch(&_current_loop)); }
			chg_unset_flg(ML_CHG_BASE<<3);
		}
#ifdef ALWAYS_SHOW_FR
		fr_show_rate(-1);
#endif
		// OR in the static change flags...
		chg_set_flg(_static_change);
	}
*/
	cit_success=TRUE;
	// hit them atexit's
}

errtype static_change_copy()
{
   if (always_render)
      chg_set_sta(_current_3d_flag);
   else
      chg_unset_sta(_current_3d_flag);
   return(OK);
}

