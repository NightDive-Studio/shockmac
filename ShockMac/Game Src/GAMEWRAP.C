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
 * $Source: r:/prj/cit/src/RCS/gamewrap.c $
 * $Revision: 1.101 $
 * $Author: xemu $
 * $Date: 1994/11/26 03:36:36 $
 */

#define __GAMEWRAP_SRC

#include <string.h>

#include "Shock.h"
#include "ShockDialogs.h"

#include "amap.h"
#include "criterr.h"
#include "cybmem.h"
#include "cybstrng.h"
#include "dynmem.h"
#include "faketime.h"
#include "gamewrap.h"
#include "hkeyfunc.h"
#include "invent.h"
#include "invpages.h"
#include "mainloop.h"
#include "map.h"
#include "miscqvar.h"
#include "musicai.h"  // to tweak music upon startup
#include "newmfd.h"
#include "objects.h"
#include "objload.h"
#include "objsim.h"
#include "olhext.h"
#include "player.h"
#include "saveload.h"
#include "schedule.h"
#include "shodan.h"
#include "sideicon.h"
#include "status.h"
#include "tools.h"
/*
#include <physics.h>
#include <tilemap.h>
#include <damage.h>  // for destroyed_object stuff
#include <gamesys.h>
#include <frprotox.h>
#include <gamestrn.h>
#include <gr2ss.h>

#include <objprop.h>
#include <objbit.h>
#include <hud.h>

#include <wsample.h>
*/
#define SCHEDULE_BASE_ID 590


extern long old_ticks;
extern char saveload_string[30];
extern bool display_saveload_checkpoints;
extern ulong obj_check_time;
extern bool mlimbs_on;

//-------------------
//  INTERNAL PROTOTYPES
//-------------------
errtype load_game_schedules(void);
errtype interpret_qvars(void);


#define OldResIdFromLevel(level) (OLD_SAVE_GAME_ID_BASE+(level*2)+2)

errtype copy_file(FSSpec *srcFile, FSSpec *destFile, Boolean saveGameFile)
{
	OSErr	err;
	FInfo		fi;
	short		destRefNum, srcRefNum;
	long 		size, count;
	errtype	retv=OK;

	// If the output file is already there, delete it first.	
	err = FSpGetFInfo(destFile, &fi);
	if (err == noErr)
		FSpDelete(destFile);

	// Create and open the output resource file.
	FSpCreateResFile(destFile, 'Shok', (saveGameFile) ? 'Sgam' : '????', 0);
	if (ResError())
		return (ERR_FOPEN);
	
	if (FSpOpenRF(destFile, fsRdWrPerm, &destRefNum) != noErr)
		return(ERR_FOPEN);

	// Open the source resource file.
	if (FSpOpenRF(srcFile, fsRdPerm, &srcRefNum) != noErr)
	{
		FSClose(destRefNum);
		return(ERR_FOPEN);
	}
	SetFPos(srcRefNum, fsFromStart, 0);

	// Copy all the data from src to dest.
	size = BIG_BUFFER_SIZE / 2;
	while (size == BIG_BUFFER_SIZE / 2)
	{
		size = BIG_BUFFER_SIZE / 2;
		FSRead(srcRefNum, &size, big_buffer);
		count = size;
		FSWrite(destRefNum, &count, big_buffer);
		if (count != size)
		{
			retv = ERR_FWRITE;
			break;
		}
		AdvanceProgress();
	}
	FSClose(destRefNum);
	FSClose(srcRefNum);
	FlushVol(nil, destFile->vRefNum);
	return(retv);
}

void closedown_game(bool visible)
{
   extern void fr_closedown(void);
   extern void olh_closedown(void);
   extern void musicai_clear();
   extern void drug_closedown(bool visible);
   extern void hardware_closedown(bool visible);
   extern void clear_digi_fx();
   extern void reset_schedules(void);
   extern void hud_shutdown_lines(void);
   // clear any transient hud settings
   hud_shutdown_lines();
   drug_closedown(visible);
   hardware_closedown(visible);
   musicai_clear();
   clear_digi_fx();
   olh_closedown();
   fr_closedown();
   if (visible)
      reset_schedules();
}

void startup_game(bool visible)
{
   extern void drug_startup(bool visible);
   extern void hardware_startup(bool visible);
   drug_startup(visible);
   hardware_startup(visible);
   if (visible)
   {
      mfd_force_update();
      side_icon_expose_all();
      status_vitals_update(TRUE);
      inventory_page = 0;
      inv_last_page = INV_BLANK_PAGE;
   }
}

#ifdef NOT_YET 

void check_save_game_wackiness(void)
{
   // for now, the only thing we have heard of is a bridge in general inventory
   // so lets make sure geninv has only geninvable stuff
   int i;
   ObjID cur_test;
   for (i=0; i<NUM_GENERAL_SLOTS; i++)
   {
	   cur_test=player_struct.inventory[i];
#ifdef USELESS_OBJECT_CHECK
      if (cur_test!=OBJ_NULL)
      {
		   if ((ObjProps[OPNUM(cur_test)].flags & INVENTORY_GENERAL)==0)
            Warning(("You have obj %d a %d as the %d element of geninv, BADNESS\n",cur_test,OPNUM(cur_test),i));
//         else
//            Warning(("You have obj %d a %d as the %d element of geninv ok %x\n",cur_test,OPNUM(cur_test),i,ObjProps[OPNUM(cur_test)].flags));
      }
#endif 
   }
}

#endif //NOT_YET 

extern int flush_resource_cache();

errtype save_game(FSSpec *saveSpec)
{
	FSSpec		currSpec;
	int 			filenum;
	State 			player_state;
	errtype 		retval;
	int 			idx = SAVE_GAME_ID_BASE;

   //KLC - this does nothing now.		check_save_game_wackiness();
   //��� Why is this done???			closedown_game(FALSE);

   //KLC  do it the Mac way						i = flush_resource_cache();
	Size	dummy;
	MaxMem(&dummy);
	
	// Open the current game file to save some more resources into it.
	FSMakeFSSpec(gDataVref, gDataDirID, CURRENT_GAME_FNAME, &currSpec);
	filenum = ResEditFile(&currSpec, FALSE);
	if (filenum < 0)
	{
		DebugStr("\pCouldn't open Current Game\n");
		return ERR_FOPEN;
	}

/*KLC - no need for this on Mac, where we have sensible, descriptive file names
	// Save comment
	ResMake(idx, (void *)comment, strlen(comment)+1, RTYPE_APP, filenum, RDF_LZW);
	ResWrite(idx);
	ResUnmake(idx);	*/
	idx++;
	AdvanceProgress();

	// Save player struct (resource #4001)
	player_struct.version_num = PLAYER_VERSION_NUMBER;
	player_struct.realspace_loc = objs[player_struct.rep].loc;
	EDMS_get_state(objs[PLAYER_OBJ].info.ph, &player_state);
	LG_memcpy(player_struct.edms_state, &player_state, sizeof (fix) * 12);
//��� LZW later		ResMake(idx, (void *)&player_struct, sizeof(player_struct), RTYPE_APP, filenum, RDF_LZW);
	ResMake(idx, (void *)&player_struct, sizeof(player_struct), RTYPE_APP, filenum, 0);
	ResWrite(idx);
	ResUnmake(idx);
	idx++;
	AdvanceProgress();

	// Save game schedule (resource #590)
	idx = SCHEDULE_BASE_ID;
//��� LZW later		ResMake(idx, (void *)&game_seconds_schedule, sizeof(Schedule), RTYPE_APP, filenum, RDF_LZW);
	ResMake(idx, (void *)&game_seconds_schedule, sizeof(Schedule), RTYPE_APP, filenum, 0);
	ResWrite(idx);
	ResUnmake(idx);
	idx++;
	AdvanceProgress();
	
	// Save game schedule vec info (resource #591)
//��� LZW later		ResMake(idx, (void *)game_seconds_schedule.queue.vec, sizeof(SchedEvent)*GAME_SCHEDULE_SIZE, RTYPE_APP, filenum, RDF_LZW);
	ResMake(idx, (void *)game_seconds_schedule.queue.vec, sizeof(SchedEvent)*GAME_SCHEDULE_SIZE, RTYPE_APP, filenum, 0);
	ResWrite(idx);
	ResUnmake(idx);
	idx++;
	AdvanceProgress();
 	
 	ResCloseFile(filenum);
	AdvanceProgress();
	
	// Save current level
	retval = write_level_to_disk(ResIdFromLevel(player_struct.level), TRUE);
	if (retval)
	{
		DebugStr("\pReturn value from write_level_to_disk is non-zero!\n");//���
		critical_error(CRITERR_FILE|3);
	}

	// Copy current game out to save game slot
	if (copy_file(&currSpec, saveSpec, TRUE) != OK)
	{
		//��� Put up some alert here.
		DebugStr("\pNo good copy, dude!\n");
//		string_message_info(REF_STR_SaveGameFail);
	}
//KLC	else
//KLC		string_message_info(REF_STR_SaveGameSaved);
	old_ticks = *tmd_ticks;
	//��� do we have to do this?		startup_game(FALSE);
	return(OK);
}

errtype load_game_schedules(void)
{
   extern int compare_events(void*, void*);
   char* oldvec;
   int idx = SCHEDULE_BASE_ID;

   oldvec = game_seconds_schedule.queue.vec;
   ResExtract(idx++, (void *)&game_seconds_schedule);
   game_seconds_schedule.queue.vec = oldvec;
   game_seconds_schedule.queue.comp = compare_events;
   ResExtract(idx++, (void *)oldvec);
   return OK;
}

errtype interpret_qvars(void)
{
   extern void recompute_music_level(ushort var);
   extern void recompute_digifx_level(ushort var);
#ifdef AUDIOLOGS
   extern void recompute_audiolog_level(ushort var);
#endif
#ifdef SVGA_SUPPORT
   extern short mode_id;
#endif
   extern void digichan_dealfunc(short val);
   extern void dclick_dealfunc(ushort var);
   extern void joysens_dealfunc(ushort var);
   extern void language_change(uchar lang);
   extern errtype load_da_palette();
   extern bool fullscrn_vitals;
   extern bool fullscrn_icons;
   extern bool map_notes_on;
   extern uchar audiolog_setting;

//KLC - don't do this here - it's a global now.   load_da_palette();
/*��� later
   gamma_dealfunc(QUESTVAR_GET(GAMMACOR_QVAR));

   dclick_dealfunc(QUESTVAR_GET(DCLICK_QVAR));
   joysens_dealfunc(QUESTVAR_GET(JOYSENS_QVAR));
   recompute_music_level(QUESTVAR_GET(MUSIC_VOLUME_QVAR));
   recompute_digifx_level(QUESTVAR_GET(SFX_VOLUME_QVAR));
#ifdef AUDIOLOGS
   recompute_audiolog_level(QUESTVAR_GET(ALOG_VOLUME_QVAR));
   audiolog_setting = QUESTVAR_GET(ALOG_OPT_QVAR);
#endif
   fullscrn_vitals = QUESTVAR_GET(FULLSCRN_VITAL_QVAR);
   fullscrn_icons = QUESTVAR_GET(FULLSCRN_ICON_QVAR);
   map_notes_on = QUESTVAR_GET(AMAP_NOTES_QVAR);
   hud_color_bank = QUESTVAR_GET(HUDCOLOR_QVAR);

   digichan_dealfunc(QUESTVAR_GET(DIGI_CHANNELS_QVAR));

   mouse_set_lefty(QUESTVAR_GET(MOUSEHAND_QVAR));

   language_change(QUESTVAR_GET(LANGUAGE_QVAR));
*/
/*KLC - can't ever change screenmode in Mac version
   mode_id = QUESTVAR_GET(SCREENMODE_QVAR);
   if (mode_id != convert_use_mode)
      chg_set_flg(GL_CHG_LOOP);
*/
   return(OK);
}

//char saveArray[16];	//����������temp

errtype load_game(FSSpec *loadSpec)
{
   int 			filenum;
   ObjID 		old_plr;
   bool 		bad_save = FALSE;
   char 		orig_lvl;
   FSSpec	currSpec;
   extern errtype change_detail_level(byte new_level);
   extern void player_set_eye_fixang(int ang);
   extern uint dynmem_mask;

   closedown_game(TRUE);
//KLC - don't do this here   stop_music();

// KLC - user will not be able to open current game file in Mac version, so skip this check.
//   rv = DatapathFind(&savegame_dpath, CURRENT_GAME_FNAME, dpath_fn);   
//   if (strcmp(fname, CURRENT_GAME_FNAME))
   {
      errtype	retval;

	 FSMakeFSSpec(gDataVref, gDataDirID, CURRENT_GAME_FNAME, &currSpec);
      
      // Copy game to load to the current file game.
      retval = copy_file(loadSpec, &currSpec, FALSE);
      if (retval != OK)
      {
		//��� bring up an alert here??
         string_message_info(REF_STR_LoadGameFail);
         return(retval);
      }
   }

   // Load in player and current level
   filenum = ResOpenFile(&currSpec);
   old_plr = player_struct.rep;
   orig_lvl = player_struct.level;
   ResExtract(SAVE_GAME_ID_BASE + 1, (void *)&player_struct);

   obj_check_time = 0;	// KLC - added because it needs to be reset for Mac version.

//KLC - this is a global pref now.    change_detail_level(player_struct.detail_level);
   player_struct.rep = old_plr;
   player_set_eye_fixang(player_struct.eye_pos);
   if (!bad_save)
      obj_move_to(PLAYER_OBJ, &(player_struct.realspace_loc), FALSE);
   if (load_game_schedules() != OK)
      bad_save = TRUE;
   ResCloseFile(filenum);
   if (orig_lvl == player_struct.level)
   {
//      Warning(("HEY, trying to be clever about loading the game! %d vs %d\n",orig_lvl,player_struct.level));
      dynmem_mask = DYNMEM_PARTIAL;
   }
   load_level_from_file(player_struct.level);
   obj_load_art(FALSE);							//���KLC - added here (removed from load_level_data)
//KLC   string_message_info(REF_STR_LoadGameLoaded);
   dynmem_mask = DYNMEM_ALL;
   chg_set_flg(_current_3d_flag);
   old_ticks = *tmd_ticks;
   interpret_qvars();
   startup_game(FALSE);

//KLC - do following instead     recompute_music_level(QUESTVAR_GET(MUSIC_VOLUME_QVAR));
	if (music_on)
	{
		mlimbs_on = TRUE;
		mlimbs_AI_init();
		mai_intro();																		//KLC - added here
		load_score_for_location(PLAYER_BIN_X, PLAYER_BIN_Y);		//KLC - added here
	}

//�������� temp
//BlockMove(0, saveArray, 16);

   return(OK);
}


errtype load_level_from_file(int level_num)
{
	errtype	retval;
	FSSpec	fSpec;

	FSMakeFSSpec(gDataVref, gDataDirID, CURRENT_GAME_FNAME, &fSpec);

	retval = load_current_map(ResIdFromLevel(level_num), &fSpec);
	if (retval == OK)
	{
		player_struct.level = level_num;
		
		compute_shodometer_value(FALSE);

		// if this is the first time the level is loaded, compute the inital shodan security level
		if (player_struct.initial_shodan_vals[player_struct.level] == -1)
			player_struct.initial_shodan_vals[player_struct.level] = QUESTVAR_GET(SHODAN_QV);
	}
	return(retval);
}


#ifdef NOT_YET //���

void check_and_update_initial(void)
{
   extern Datapath savegame_dpath;
   char archive_fname[128];
   char dpath_fn[50];
   char* tmp;
	extern char real_archive_fn[20];
   if (!DatapathFind(&savegame_dpath, CURRENT_GAME_FNAME, archive_fname))
   {
      tmp=getenv("CITHOME");
      if (tmp) { strcpy(dpath_fn,tmp); strcat(dpath_fn,"\\"); } else dpath_fn[0]='\0';
      strcat(dpath_fn, "data\\");
      strcat(dpath_fn, CURRENT_GAME_FNAME);

      if (!DatapathFind(&DataDirPath, real_archive_fn, archive_fname))
         critical_error(CRITERR_RES|0x10);
      if (copy_file(archive_fname, dpath_fn) != OK)
         critical_error(CRITERR_FILE|0x7);
   }   

}

#endif //NOT_YET ���


bool create_initial_game_func(short , ulong , void* )
{
	FSSpec	archiveSpec, currSpec;
	OSErr	err;
	
	int i;
	extern int actual_score;
	byte plrdiff[4];
	char tmpname[sizeof(player_struct.name)];
	short plr_obj;
	extern errtype do_level_entry_triggers();

	free_dynamic_memory(DYNMEM_ALL);
	
	// Copy archive into local current game file.

	// First, make sure the archive file is actually there.
	err = FSMakeFSSpec(gCDDataVref, gCDDataDirID, ARCHIVE_FNAME, &archiveSpec);
	if (err == fnfErr)
	{
		load_dynamic_memory(DYNMEM_ALL);
		return(TRUE);
	}
	
	// Next, copy the archive file to an untitled game.
	FSMakeFSSpec(gDataVref, gDataDirID, CURRENT_GAME_FNAME, &currSpec);

	if (copy_file(&archiveSpec, &currSpec, FALSE) != OK)
		critical_error(CRITERR_FILE|7);

/* KLC - I don't think you actually have to load the player in for a new game, since "init_player"
                zeroes it out anyway.
                
   // Load in player and current level
   filenum = ResOpenFile(&fSpec);
   if (filenum < 0)
   {
      string_message_info(REF_STR_GameInitFail);
      return(TRUE);
   }
*/
   plr_obj = PLAYER_OBJ;
   for (i=0; i<4; i++)
      plrdiff[i] = player_struct.difficulty[i];
   LG_memcpy(tmpname,player_struct.name,sizeof(tmpname));

	//KLC - don't need this anymore.  ResExtract(SAVE_GAME_ID_BASE + 1, (void *)&player_struct);

   init_player(&player_struct);
   obj_check_time = 0;							// KLC - added here cause it needs to be reset in Mac version

   player_struct.rep = OBJ_NULL;

   load_level_from_file(player_struct.level);
   obj_load_art(FALSE);							//���KLC - added here (removed from load_level_data)
   amap_reset();
   player_create_initial();

   LG_memcpy(player_struct.name,tmpname,sizeof(player_struct.name));
   for (i=0; i<4; i++)
      player_struct.difficulty[i] = plrdiff[i];

   // KLC - not needed any longer ResCloseFile(filenum);

   // Reset MFDs to be consistent with starting setup  
   init_newmfd();

   // No time elapsed, really, honest
   old_ticks = *tmd_ticks;

   // Setup some start-game stuff
   // Music
	current_score = actual_score = last_score = PERIL_SCORE;	// KLC - these aren't actually
	mlimbs_peril = 1000;														// going to do anything.

	if (music_on)
	{
		mlimbs_on = TRUE;
		mlimbs_AI_init();
		mai_intro();																		//KLC - added here
		load_score_for_location(PLAYER_BIN_X, PLAYER_BIN_Y);		//KLC - added here
	}

   load_dynamic_memory(DYNMEM_ALL);

   // Do entry-level triggers for starting level
   // Hmm, do we actually want to call this any time we restore
   // a saved game or whatever?  No, probably not....hmmm.....
   do_level_entry_triggers();
   
   // KLC - if not already on, turn on-line help on.
   if (!olh_active)
      toggle_olh_func(0, 0, NULL);
   
   // turn on help overlay. 
   olh_overlay_on = TRUE;

   // Plot timers

   return(FALSE);
}

errtype write_level_to_disk(int idnum, bool flush_mem)
{
	FSSpec	currSpec;

   // Eventually, this ought to cleverly determine whether or not to pack
   // the save game resource, but for now we will always do so...

	FSMakeFSSpec(gDataVref, gDataDirID, CURRENT_GAME_FNAME, &currSpec);
	
	return(save_current_map(&currSpec, idnum, flush_mem,TRUE));
}
