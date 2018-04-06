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
 * $Source: r:/prj/cit/src/RCS/setup.c $
 * $Revision: 1.141 $
 * $Author: dc $
 * $Date: 1994/11/23 00:05:51 $
 */

#define __SETUP_SRC

#include <string.h>

#include "ShockDialogs.h"

//��#include <setploop.h>
#include "setup.h"
#include "colors.h"
#include "diffq.h"
#include "gamewrap.h"
#include "gr2ss.h"
#include "hud.h"
#include "init.h"
#include "miscqvar.h"
#include "player.h"
#include "version.h"

/*���
#include <mainloop.h>
#include <tools.h>
#include <input.h>
#include <screen.h>
#include <hkeyfunc.h>
#include <loops.h>
#include <keydefs.h>
#include <dpaths.h>
#include <cybmem.h>
#include <status.h>
#include <cutscene.h>
#include <wrapper.h>
#include <mlimbs.h>
#include <musicai.h>
#include <palfx.h>
#include <citmusic.h>
#include <verify.h>

// Resource stuff
#include <intro.h>
#include <gamescr.h>
#include <gamepal.h>
#include <gamestrn.h>
#include <cybstrng.h>

#include <faketime.h>
#include <scrntext.h>

#ifdef PLAYTEST
#include <mprintf.h>
#endif

#include <mlimbs.h>
extern errtype musicai_shutdown();
*/

#define CFG_NAME_VAR "name"

/* KLC - no longer needed
uiSlab setup_slab;
Region setup_root_region;
*/

bool play_intro_anim;
bool save_game_exists = FALSE;
bool startup_music;
int setup_mode;
int intro_num;
int diff_sum = 0;

extern char which_lang;

ubyte valid_save;

bool setup_bio_started = FALSE;

// ----------------------
// Internal Prototypes
// ----------------------
errtype journey_newgame_func();


#ifdef NOT_YET //���

#define ALT(x) ((x)|KB_FLAG_ALT)

#define KEYBOARD_FOCUS_COLOR  RED_BASE + 3
#define NORMAL_ENTRY_COLOR    RED_BASE + 7
#define CURRENT_DIFF_COLOR    RED_BASE + 3
#define SELECTED_COLOR        RED_BASE 
#define UNAVAILABLE_COLOR     RED_BASE + 11

#define JOURNEY_OPT_LEFT   79
#define JOURNEY_OPT_RIGHT  247

#define JOURNEY_OPT1_TOP  66
#define JOURNEY_OPT1_BOT  88 

#define JOURNEY_OPT2_TOP  97
#define JOURNEY_OPT2_BOT  119 

#define JOURNEY_OPT3_TOP  129
#define JOURNEY_OPT3_BOT  160 

#define JOURNEY_OPT4_TOP  160
#define JOURNEY_OPT4_BOT  182

#define DIFF_DONE_X1       119
#define DIFF_DONE_Y1       179
#define DIFF_DONE_X2       203
#define DIFF_DONE_Y2       198

#define DIFF_NAME_X  57
#define DIFF_NAME_Y  49
#define DIFF_NAME_X2  253
#define DIFF_NAME_Y2  65
#define DIFF_NAME_TEXT_X   124

// lets replace 40 #defines with 7, eh?
#define DIFF_X_BASE     28
#define DIFF_W_BASE     156
#define DIFF_W_ELEM     32
#define DIFF_H_ELEM     20

#define DIFF_OPT_TOP    99
#define DIFF_OPT_DELTA  53
#define DIFF_OPT_HEIGHT 24
#define DIFF_STRING_OFFSET_Y  10
#define DIFF_STRING_OFFSET_X  13


#define DIFF_TITLE1_X1    12
#define DIFF_TITLE1_Y1    70 
#define DIFF_TITLE1_X2    155
#define DIFF_TITLE1_Y2    93 
#define DIFF_TITLE1_OPT_TOP      94
#define DIFF_TITLE1_OPT_BOTTOM   118
#define DIFF_TITLE1_LEFT1        25
#define DIFF_TITLE1_RIGHT1       45
#define DIFF_TITLE1_LEFT2        59       // LOOK, a typo in the table... neat...
#define DIFF_TITLE1_RIGHT2       77
#define DIFF_TITLE1_LEFT3        89
#define DIFF_TITLE1_RIGHT3       109
#define DIFF_TITLE1_LEFT4        121
#define DIFF_TITLE1_RIGHT4       141

#define DIFF_SIZE_X    DIFF_TITLE1_RIGHT1 - DIFF_TITLE1_LEFT1
#define DIFF_SIZE_Y    DIFF_TITLE1_OPT_BOTTOM - DIFF_TITLE1_OPT_TOP

#define DIFF_TITLE2_X1    169
#define DIFF_TITLE2_Y1    70 
#define DIFF_TITLE2_X2    311
#define DIFF_TITLE2_Y2    93 
#define DIFF_TITLE2_OPT_TOP      94
#define DIFF_TITLE2_OPT_BOTTOM   118
#define DIFF_TITLE2_LEFT1        181
#define DIFF_TITLE2_RIGHT1       201
#define DIFF_TITLE2_LEFT2        213
#define DIFF_TITLE2_RIGHT2       233
#define DIFF_TITLE2_LEFT3        245
#define DIFF_TITLE2_RIGHT3       265
#define DIFF_TITLE2_LEFT4        277
#define DIFF_TITLE2_RIGHT4       297

#define DIFF_TITLE3_X1    12
#define DIFF_TITLE3_Y1    123 
#define DIFF_TITLE3_X2    155
#define DIFF_TITLE3_Y2    146 
#define DIFF_TITLE3_OPT_TOP      147
#define DIFF_TITLE3_OPT_BOTTOM   171
#define DIFF_TITLE3_LEFT1        25
#define DIFF_TITLE3_RIGHT1       45
#define DIFF_TITLE3_LEFT2        57
#define DIFF_TITLE3_RIGHT2       77
#define DIFF_TITLE3_LEFT3        89
#define DIFF_TITLE3_RIGHT3       109
#define DIFF_TITLE3_LEFT4        121
#define DIFF_TITLE3_RIGHT4       141

#define DIFF_TITLE4_X1    169
#define DIFF_TITLE4_Y1    123 
#define DIFF_TITLE4_X2    311
#define DIFF_TITLE4_Y2    146 
#define DIFF_TITLE4_OPT_TOP      147
#define DIFF_TITLE4_OPT_BOTTOM   171
#define DIFF_TITLE4_LEFT1        181
#define DIFF_TITLE4_RIGHT1       201
#define DIFF_TITLE4_LEFT2        213
#define DIFF_TITLE4_RIGHT2       233
#define DIFF_TITLE4_LEFT3        245
#define DIFF_TITLE4_RIGHT3       265
#define DIFF_TITLE4_LEFT4        277
#define DIFF_TITLE4_RIGHT4       297

#ifdef BIG_ARRAYS_OF_LINEAR_EQUATIONS_TO_CLARIFY_AND_YET_OBFUSCATE_THINGS
int diff_x[16] = {
   DIFF_TITLE1_LEFT1 + 2,
   DIFF_TITLE1_LEFT2 + 2,
   DIFF_TITLE1_LEFT3 + 2,
   DIFF_TITLE1_LEFT4 + 2,
   DIFF_TITLE2_LEFT1 + 2,
   DIFF_TITLE2_LEFT2 + 2,
   DIFF_TITLE2_LEFT3 + 2,
   DIFF_TITLE2_LEFT4 + 2,
   DIFF_TITLE3_LEFT1 + 2,
   DIFF_TITLE3_LEFT2 + 2,
   DIFF_TITLE3_LEFT3 + 2,
   DIFF_TITLE3_LEFT4 + 2,
   DIFF_TITLE4_LEFT1 + 2,
   DIFF_TITLE4_LEFT2 + 2,
   DIFF_TITLE4_LEFT3 + 2,
   DIFF_TITLE4_LEFT4 + 2
};

// couldnt at the least this have been of 4, and indexed by [idx>>2]
int diff_y[16] = {
   DIFF_TITLE1_OPT_TOP + 2,
   DIFF_TITLE1_OPT_TOP + 2,
   DIFF_TITLE1_OPT_TOP + 2,
   DIFF_TITLE1_OPT_TOP + 2,
   DIFF_TITLE2_OPT_TOP + 2,
   DIFF_TITLE2_OPT_TOP + 2,
   DIFF_TITLE2_OPT_TOP + 2,
   DIFF_TITLE2_OPT_TOP + 2,
   DIFF_TITLE3_OPT_TOP + 2,
   DIFF_TITLE3_OPT_TOP + 2,
   DIFF_TITLE3_OPT_TOP + 2,
   DIFF_TITLE3_OPT_TOP + 2,
   DIFF_TITLE4_OPT_TOP + 2,
   DIFF_TITLE4_OPT_TOP + 2,
   DIFF_TITLE4_OPT_TOP + 2,
   DIFF_TITLE4_OPT_TOP + 2
};
#endif

// why +2?
#define build_diff_x(char_num) ((DIFF_X_BASE+(DIFF_W_ELEM*(char_num&3))+(((char_num>>2)&1)*DIFF_W_BASE))+2)
#define build_diff_y(char_num) ((DIFF_OPT_TOP+((char_num>>3)*DIFF_OPT_DELTA))+2)

journey_y[8] = {
   JOURNEY_OPT1_TOP, JOURNEY_OPT1_BOT,
   JOURNEY_OPT2_TOP, JOURNEY_OPT2_BOT,
   JOURNEY_OPT3_TOP, JOURNEY_OPT3_BOT,
   JOURNEY_OPT4_TOP, JOURNEY_OPT4_BOT
};

errtype draw_difficulty_char(int char_num);
errtype draw_difficulty_description(int which_cat, int color);
errtype journey_continue_func(bool draw_stuff);

bool setup_sound_on=FALSE;

#define MAX_NAME_SIZE   sizeof(player_struct.name)
#define start_name (player_struct.name)

// -------------------------------------------------------------
// start_setup_sound()
//
char *stp_themes[]={"titloop.xmi","endloop.xmi"};

bool start_setup_sound(int which)
{
   if ((setup_sound_on)||(!music_on))
      return FALSE;

   if (citmusic_swap_to_xmi(stp_themes[which])!=-1)
   {
      if (which==0)
	      AIL_branch_index((SEQUENCE *)snd_get_sequence(cmus_theme_seq_id),0);
      setup_sound_on=TRUE;
   }
   return setup_sound_on;
}

// --------------------------------------------------------------------
// end_setup_sound()
//

void end_setup_sound(void)
{
   if (!setup_sound_on)
      return;
   citmusic_stop_simple_xmi();
   setup_sound_on = FALSE;
}

// --------------------------------------------------------------------
// compute_new_diff
//

errtype compute_new_diff()
{
   int i, new_sum =0;
   for (i=0; i < 4; i++)
      new_sum += player_struct.difficulty[i];
   diff_sum = new_sum;
   return(OK);
}

// --------------------------------------------------------------------
// difficult_draw
//

#define NUM_DIFF_CATEGORIES   4
char curr_diff = 0;
bool start_selected = FALSE;

#define CATEGORY_STRING_BASE  REF_STR_diffCategories
#define DIFF_STRING_BASE      REF_STR_diffStrings
#define DIFF_NAME             REF_STR_diffName
#define DIFF_START            REF_STR_diffStart

short diff_titles_x[] = { DIFF_TITLE1_X1, DIFF_TITLE2_X1, DIFF_TITLE3_X1,DIFF_TITLE4_X1 };
short diff_titles_y[] = { DIFF_TITLE1_Y1, DIFF_TITLE2_Y1, DIFF_TITLE3_Y1,DIFF_TITLE4_Y1 };
char *valid_char_string = "0123456789:ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz ";

errtype difficulty_draw(bool full)
{
   errtype draw_username();
   int i;

   uiHideMouse(NULL);
   if (full)
   {
      draw_raw_res_bm_extract(REF_IMG_bmDifficultyScreen, 0, 0);
      if (which_lang)
         draw_raw_res_bm_extract(MKREF(RES_bmIntroGraphics4,which_lang-1),50,11);
   }
   setup_mode = SETUP_DIFFICULTY;
   for (i=0; i < NUM_DIFF_CATEGORIES; i++)
   {
      if (i == curr_diff && !start_selected)
         gr_set_fcolor(KEYBOARD_FOCUS_COLOR);
      else
         gr_set_fcolor(NORMAL_ENTRY_COLOR);
      res_draw_string(RES_citadelFont, CATEGORY_STRING_BASE + i, diff_titles_x[i] + 12, diff_titles_y[i] + 2);
   }
   if (start_selected)
      gr_set_fcolor(KEYBOARD_FOCUS_COLOR);
   else
      gr_set_fcolor(NORMAL_ENTRY_COLOR);
   res_draw_string(RES_citadelFont, DIFF_START, DIFF_DONE_X1 + 13, DIFF_DONE_Y1 + 2);
   if (full)
   {
      for (i=0; i<16; i++)
         draw_difficulty_char(i);
      for (i=0; i < 4; i++)
         draw_difficulty_description(i, NORMAL_ENTRY_COLOR);
      gr_set_fcolor(KEYBOARD_FOCUS_COLOR);
      res_draw_string(RES_citadelFont, DIFF_NAME, DIFF_NAME_X, DIFF_NAME_Y);
      draw_username(NORMAL_ENTRY_COLOR, start_name);
   }
   uiShowMouse(NULL);
   return(OK);
}

// -------------------------------------------------------------
// draw_username();
//
//

Rect name_rect={{DIFF_NAME_TEXT_X,DIFF_NAME_Y},{DIFF_NAME_X2,DIFF_NAME_Y2}};

errtype draw_username(int color, char *string)
{
   gr_set_fcolor(color);
   uiHideMouse(&name_rect);
   res_draw_text(RES_citadelFont, string, DIFF_NAME_TEXT_X, DIFF_NAME_Y);
   uiShowMouse(&name_rect);
   return(OK);
}

#define FLASH_TIME (CIT_CYCLE/4)

void flash_username(void)
{
   long flash_done;
   uiHideMouse(&name_rect);
   gr_set_fcolor(SELECTED_COLOR-4);
   res_draw_string(RES_citadelFont, DIFF_NAME, DIFF_NAME_X, DIFF_NAME_Y);
   flash_done=*tmd_ticks+FLASH_TIME;
   while (*tmd_ticks<flash_done);
   gr_set_fcolor(KEYBOARD_FOCUS_COLOR);
   res_draw_string(RES_citadelFont, DIFF_NAME, DIFF_NAME_X, DIFF_NAME_Y);
   uiShowMouse(&name_rect);   
}

// -------------------------------------------------------------
// draw_difficult_line()
//
//

errtype draw_difficulty_line(int which_line)
{
   int i;
   for (i=0; i < 4; i++)
      draw_difficulty_char((which_line * 4) + i);
   draw_difficulty_description(which_line, NORMAL_ENTRY_COLOR);
   return(OK);
}

// -------------------------------------------------------------
// draw_difficulty_description()
//

#define COMPUTE_DIFF_STRING_X(wcat) (DIFF_X_BASE + (DIFF_W_BASE * (wcat & 1)) - DIFF_STRING_OFFSET_X)
#define COMPUTE_DIFF_STRING_Y(wcat) (DIFF_OPT_TOP + ((wcat >> 1) * DIFF_OPT_DELTA) - DIFF_STRING_OFFSET_Y)

errtype draw_difficulty_description(int which_cat, int color)
{
   if (color != -1)
      gr_set_fcolor(color);
   res_draw_string(RES_smallTechFont, DIFF_STRING_BASE + (which_cat * 4) + player_struct.difficulty[which_cat],
      COMPUTE_DIFF_STRING_X(which_cat), COMPUTE_DIFF_STRING_Y(which_cat));
   return(OK);
}

// -------------------------------------------------------------
// draw_difficulty_char()
//
// char_num 1-16

// im going to rewrite this to use less memory, if no one minds....

errtype draw_difficulty_char(int char_num)
{
   char buff[]="X";
   uiHideMouse(NULL);
   if (player_struct.difficulty[char_num / 4] == char_num % 4)
      gr_set_fcolor(CURRENT_DIFF_COLOR);
   else
      gr_set_fcolor(NORMAL_ENTRY_COLOR);
   buff[0]=(char_num&3)+'0';
   res_draw_text(RES_citadelFont, buff, build_diff_x(char_num), build_diff_y(char_num));
   uiShowMouse(NULL);
   return(OK);
}

// -------------------------------------------------------------
// journey_draw()
//

#ifdef DEMO
char curr_setup_line = 1;
#else
char curr_setup_line = 0;
#endif
#define SETUP_STRING_BASE  REF_STR_journeyOpts
short setup_tops[] = { JOURNEY_OPT1_TOP, JOURNEY_OPT2_TOP, JOURNEY_OPT3_TOP, JOURNEY_OPT4_TOP };

#define NUM_SETUP_LINES 4

errtype journey_draw(char part)
{
   char i;

   uiHideMouse(NULL);

   if (setup_bio_started)
   {
      status_bio_end();
      setup_bio_started = FALSE;
   }

   // extract into buffer - AFTER we've stopped biorhythms (which used that buffer.....)
   if (part == 0)
      draw_raw_res_bm_extract(REF_IMG_bmJourneyOnwards, 0, 0);

   for (i=0; i < NUM_SETUP_LINES; i++)
   {
      if ((part == 0) || (part-1 == i))
      {
         int col;
         if (i == curr_setup_line)
            col=KEYBOARD_FOCUS_COLOR;
         else
            col=NORMAL_ENTRY_COLOR;
#ifdef DEMO
         if ((i == NUM_SETUP_LINES -1) || (i == 0))
#else
         if (i == NUM_SETUP_LINES -1)     // why is NUM_SETUP_LINES-1 necessarily continue?
#endif
         {
#ifndef DEMO
            if (!save_game_exists)
#endif
               col=UNAVAILABLE_COLOR;
         }
         gr_set_fcolor(col);
         res_draw_string(RES_citadelFont, SETUP_STRING_BASE + i, JOURNEY_OPT_LEFT + 15, setup_tops[i] + 4);
      }
   }
   uiShowMouse(NULL);
   setup_mode = SETUP_JOURNEY;
   return(OK);
}

// -------------------------------------------------------------
// journey_intro_func()
//

#pragma disable_message(202)
errtype journey_intro_func(bool draw_stuff)
{
#ifdef DEMO
   uiShowMouse(NULL);    // need to leave it hidden
   return(OK);
#else
   if (draw_stuff)
      res_draw_string(RES_citadelFont, SETUP_STRING_BASE, JOURNEY_OPT_LEFT + 15, JOURNEY_OPT1_TOP + 2);
   uiShowMouse(NULL);    // need to leave it hidden
   return(play_cutscene(START_CUTSCENE, TRUE));
#endif
}
#pragma enable_message(202)

#endif //NOT_YET���


// -------------------------------------------------------------
// journey_newgame_func
//

errtype journey_newgame_func()
{
   extern bool clear_player_data;

   clear_player_data = TRUE;

   object_data_load();

   player_struct.level = 0xFF;

   create_initial_game_func(0,0,0);
/*��� not yet
   change_mode_func(0,0,(void *)GAME_LOOP);
*/
   startup_music = TRUE;
   return(OK);
}


#ifdef NOT_YET  //���

// -------------------------------------------------------------
// journey_difficulty_func
//

errtype journey_difficulty_func(bool draw_stuff)
{
   if (draw_stuff)
      res_draw_string(RES_citadelFont, SETUP_STRING_BASE + 1, JOURNEY_OPT_LEFT + 15, JOURNEY_OPT2_TOP + 2);
   uiShowMouse(NULL);
   difficulty_draw(TRUE);
   compute_new_diff();
   status_bio_set(DIFF_BIO);
   status_bio_start();
   setup_bio_started = TRUE;
   return(OK);
}

#define CredResFnt   (RES_coloraliasedFont)
#define CredColor    (GREEN_BASE+4)
#define CredResource (RES_credits)

int credits_inp=0;
void *credits_txtscrn;

void journey_credits_func(bool draw_stuff)
{
   if (draw_stuff)
      res_draw_string(RES_citadelFont, SETUP_STRING_BASE + 2, JOURNEY_OPT_LEFT + 15, JOURNEY_OPT3_TOP + 2);
   setup_mode = SETUP_CREDITS;
   credits_txtscrn=scrntext_init(CredResFnt,CredColor,CredResource);
   if (draw_stuff)
   {
	   end_setup_sound();
      load_score_guts(7);
      grind_credits_music_ai();
      mlimbs_preload_requested_timbres();
   }
}

void journey_credits_done()
{
   extern char current_cutscene;
   if ((current_cutscene != WIN_CUTSCENE)&&(current_cutscene != SECRET_EXIT_TO_DOS_CUTSCENE))
   {
      musicai_shutdown();
	   start_setup_sound(0);
   }
   scrntext_free(credits_txtscrn);
   if (current_cutscene == WIN_CUTSCENE)
   {
      current_cutscene = SECRET_EXIT_TO_DOS_CUTSCENE;
      journey_credits_func(FALSE);
   }
   else if (current_cutscene == SECRET_EXIT_TO_DOS_CUTSCENE)
   {
      really_quit_key_func(0,0,0);
   }
   else
   {
      uiShowMouse(NULL);
      journey_draw(0);
   }
}

#define SG_SLOT_HT   17
#define SG_SLOT_WD   169
#define SG_SLOT_OFFSET_X   21
#define SG_SLOT_OFFSET_Y   6
#define SG_SLOT_X    79
#define SG_SLOT_Y    62

char curr_sg = 0;

errtype draw_sg_slot(int slot_num)
{
   char temp[64];
   short sz,x,y;

   uiHideMouse(NULL);
   if (curr_sg == slot_num)
      gr_set_fcolor(KEYBOARD_FOCUS_COLOR);
   else
      gr_set_fcolor(NORMAL_ENTRY_COLOR);
// if slot_num == -1 highlight the curr_sg slot with the SELECTED_COLOR color
   if (slot_num == -1)
   {
      gr_set_fcolor(SELECTED_COLOR);
      slot_num = curr_sg;
   }
   if(valid_save & (1<<slot_num)) {
      sz = strlen(comments[slot_num]);
      strcpy(temp,comments[slot_num]);
   }
   else {
      get_string(REF_STR_UnusedSave,temp,64);
   }
   gr_set_font((grs_font*)ResLock(RES_smallTechFont));
   gr_string_size(temp, &x, &y);
   while ((x > SG_SLOT_WD - SG_SLOT_OFFSET_X) && (sz > 0))
   {
      sz--;
      strcpy(temp, "");
      strncpy(temp,comments[slot_num],sz);
      temp[sz] = '\0';   
      gr_string_size(temp, &x, &y);
   }
   ResUnlock(RES_smallTechFont);
   // was RES_CitadelFont
   res_draw_text(RES_smallTechFont, temp, SG_SLOT_X + SG_SLOT_OFFSET_X, SG_SLOT_Y + SG_SLOT_OFFSET_Y + (slot_num * SG_SLOT_HT));
   uiShowMouse(NULL);
   return(OK);
}

errtype draw_savegame_names()
{
   int i;
   for (i=0; i < NUM_SAVE_SLOTS; i++)
      draw_sg_slot(i);
   return(OK);
}

#endif //NOT_YET���

extern void check_and_update_initial(void);

errtype load_that_thar_game(FSSpec *loadSpec)
{
   errtype retval;
//KLC - not in Mac version   if (valid_save & (1 << which_slot))
//KLC - not in Mac version   {
      extern bool clear_player_data;
      extern char curr_vol_lev;

//KLC - not in Mac version      draw_sg_slot(-1);             // highlight the current save game slot with SELECTED_COLOR
//KLC - have Mac version up at this point      begin_wait();
//��� is this needed?      check_and_update_initial();
      clear_player_data=TRUE;   // initializes the player struct in object_data_load
      object_data_load();
      player_create_initial();
      player_struct.level = 0xFF;      // make sure we load textures
//KLC - not in Mac version      Poke_SaveName(which_slot);
//KLC - not in Mac version      change_mode_func(0,0,(void *)GAME_LOOP);
      retval = load_game(loadSpec);
      if (retval != OK)
      {
//KLC - not in Mac version         strcpy(comments[which_slot], "<< INVALID GAME >>");
//KLC         end_wait();
//KLC - not in Mac version         uiHideMouse(NULL);
//KLC - not in Mac version         journey_continue_func(TRUE);
         return(retval);
      }
//KLC - don't do the following.
//      if (curr_vol_lev != 0)
      startup_music = TRUE;
//KLC      end_wait();
//KLC - not in Mac version   }
   return(OK);
}

#ifdef NOT_YET //���

// -------------------------------------------------------------
// journey_continue_func
//

#pragma disable_message(202)
errtype journey_continue_func(bool draw_stuff)
{
#ifndef DEMO
   if (save_game_exists)
   {
      draw_raw_res_bm_extract(REF_IMG_bmContinueScreen, 0, 0);
      setup_mode = SETUP_CONTINUE;
      draw_savegame_names();
   }
#endif

   uiShowMouse(NULL);
   return(OK);
}
#pragma enable_message(202)

#endif //NOT_YET���


#define SECRET_MISSION_DIFFICULTY_QB      0xB0
char diff_qvars[4] = { COMBAT_DIFF_QVAR, MISSION_DIFF_QVAR, PUZZLE_DIFF_QVAR, CYBER_DIFF_QVAR};

void go_and_start_the_game_already()
{
   char i;
   extern char curr_vol_lev;
   extern char curr_sfx_vol;
   extern bool fullscrn_vitals;
   extern bool fullscrn_icons;
   extern bool map_notes_on;
   extern uchar audiolog_setting;
#ifdef AUDIOLOGS
   extern char curr_alog_vol;
#endif
   extern bool mouseLefty;
   
/* KLC - no longer needed
#ifdef GAMEONLY
   if (strlen(start_name)==0)
    { flash_username(); return; }
#endif
   uiHideMouse(NULL);
   gr_set_fcolor(SELECTED_COLOR);
   res_draw_string(RES_citadelFont, DIFF_START, DIFF_DONE_X1 + 13, DIFF_DONE_Y1 + 2);
   uiShowMouse(NULL);
*/
//KLC - Mac cursor up at this point   begin_wait();

   journey_newgame_func();
#ifdef SVGA_SUPPORT
   QUESTVAR_SET(SCREENMODE_QVAR, convert_use_mode);
#endif
//��   QUESTVAR_SET(MUSIC_VOLUME_QVAR, (curr_vol_lev*curr_vol_lev)/100);
//��   QUESTVAR_SET(SFX_VOLUME_QVAR, (curr_sfx_vol*curr_sfx_vol)/100);
#ifdef AUDIOLOGS
//��   QUESTVAR_SET(ALOG_VOLUME_QVAR, (curr_alog_vol*curr_alog_vol)/100);
   QUESTVAR_SET(ALOG_OPT_QVAR, audiolog_setting);
#endif
   QUESTVAR_SET(FULLSCRN_ICON_QVAR, fullscrn_icons);
   QUESTVAR_SET(FULLSCRN_VITAL_QVAR, fullscrn_vitals);
//��   QUESTVAR_SET(AMAP_NOTES_QVAR, map_notes_on);
   QUESTVAR_SET(HUDCOLOR_QVAR, hud_color_bank);
//KLC - this is a global now   QUESTVAR_SET(GAMMACOR_QVAR, (short)((29*FIX_UNIT)/100));
//��   QUESTVAR_SET(MOUSEHAND_QVAR, mouseLefty);
   QUESTVAR_SET(DCLICK_QVAR, FIX_UNIT/3);
//��   {
//��      extern char hack_digi_channels;
//��      QUESTVAR_SET(DIGI_CHANNELS_QVAR, hack_digi_channels);
//��   }
   for (i=0; i < 4; i++)
      QUESTVAR_SET(diff_qvars[i], player_struct.difficulty[i]);
   if (QUESTVAR_GET(MISSION_DIFF_QVAR) == 3)
      hud_set(HUD_GAMETIME);
// KLC - following doesn't really apply in Mac version.
//   if (player_struct.difficulty[QUEST_DIFF_INDEX] == 1)
//      player_struct.terseness=TRUE;
   strncpy(player_struct.version, SYSTEM_SHOCK_VERSION,6);

/*  KLC - no longer needed
   if (setup_bio_started)
   {
      status_bio_end();
      setup_bio_started = FALSE;
   }
*/
//KLC   end_wait();
}


#ifdef NOT_YET //���

// ------------------------------------
// journey functions

typedef errtype (*journey_func)(bool draw_stuff);

journey_func journey_funcs[4] =
{  journey_intro_func,
   journey_difficulty_func,
   journey_credits_func,
   journey_continue_func };

// if there are two different input events - only let's one
// call a journey_func
bool journey_lock = FALSE;

// -------------------------------------------------------------
// intro_mouse_handler()
//

bool intro_mouse_handler(uiEvent *ev, Region *r, void *user_data)
{
   uiMouseEvent *mev = (uiMouseEvent *)ev;
   int which_one = -1;
   int i = 0;
   int old_diff;
   bool diff_changed;
#ifndef NO_DUMMIES
   void *dummy;   dummy = user_data;   dummy = r;
#endif
   if (mev->action & MOUSE_LDOWN)
   {
      switch (setup_mode)
      {
         case SETUP_JOURNEY:
            if (!journey_lock)
            {
               if ((mev->pos.x > JOURNEY_OPT_LEFT) && (mev->pos.x < JOURNEY_OPT_RIGHT))
               {
                  while ((which_one == -1) && (i <= 6))
                  {
                     if ((mev->pos.y > journey_y[i]) && (mev->pos.y < journey_y[i+1]))
                        which_one=i>>1;
                     else
                        i += 2;
                  }
                  Spew(DSRC_USER_I_Screen, ("which_one = %d\n",which_one));
                  if (which_one != -1)
                  {
                     uiHideMouse(NULL);
                     gr_set_fcolor(SELECTED_COLOR);
                     journey_lock = TRUE;
                     journey_funcs[which_one](TRUE);
                     journey_lock = FALSE;
                  }
               }
            }
            break;
         case SETUP_CREDITS:
            credits_inp=-1; 
            break;
         case SETUP_CONTINUE:
            if ((mev->pos.x >= SG_SLOT_X) && (mev->pos.x <= SG_SLOT_X + SG_SLOT_WD) &&
                (mev->pos.y >= SG_SLOT_Y) && (mev->pos.y <= SG_SLOT_Y + (NUM_SAVE_SLOTS * SG_SLOT_HT)))
            {
               char which = (mev->pos.y - SG_SLOT_Y) / SG_SLOT_HT;
               char old_sg = curr_sg;
               curr_sg = which;
               draw_sg_slot(old_sg);
               load_that_thar_game(which);
            }
            break;
         case SETUP_DIFFICULTY:
            diff_changed = FALSE;
            // given that these are all rectangles, i bet you could just get mouse pos and divide, eh?
            for (i=0; i<16; i++)
            {
               if ((mev->pos.x > (build_diff_x(i) - 2)) && (mev->pos.x < (build_diff_x(i) - 2 + DIFF_SIZE_X)) &&
                  (mev->pos.y > (build_diff_y(i) - 2)) && (mev->pos.y < (build_diff_y(i) - 2 + DIFF_SIZE_Y)))
               {
                  old_diff = player_struct.difficulty[i/4];
                  draw_difficulty_description(i/4,0);
                  player_struct.difficulty[i/4] = i % 4;
                  Spew(DSRC_GAMESYS_Init, ("difficulty %d set to %d (verify = %d)\n",i/4,i%4,player_struct.difficulty[i/4]));
                  draw_difficulty_char(((i/4) * 4) + old_diff);
                  draw_difficulty_char(i);
                  draw_difficulty_description(i/4,NORMAL_ENTRY_COLOR);
                  diff_changed = TRUE;
               }
            }
            if (diff_changed)
               compute_new_diff();
            else if ((mev->pos.x > DIFF_DONE_X1) && (mev->pos.x < DIFF_DONE_X2) && (mev->pos.y > DIFF_DONE_Y1) && (mev->pos.y < DIFF_DONE_Y2))
               go_and_start_the_game_already();
            break;
      }
   }
   return(TRUE);
}

// -------------------------------------------------------------
// intro_key_handler
//

#pragma disable_message(202)

bool intro_key_handler(uiEvent *ev, Region *r, void *user_data)
{
   uiCookedKeyEvent *kev = (uiCookedKeyEvent *)ev;
   int code = kev->code & ~(KB_FLAG_DOWN | KB_FLAG_2ND);
   char old_diff, old_setup_line = curr_setup_line, n=0;
   
   if (kev->code & KB_FLAG_DOWN) 
   {
      switch (setup_mode)
      {
         case SETUP_JOURNEY:
            switch (code)
            {
            case KEY_PAD_UP:   n=NUM_SETUP_LINES-2; // sneaky fallthrough action
            case KEY_PAD_DOWN:
               n++;
               curr_setup_line=(curr_setup_line+n)%NUM_SETUP_LINES;
#ifdef DEMO
               if (curr_setup_line==NUM_SETUP_LINES-1)     // why is NUM_SETUP_LINES-1 necessarily continue?
                  curr_setup_line = 2;
               if (curr_setup_line == 0)
                  curr_setup_line = 1;
#else
               if (curr_setup_line==NUM_SETUP_LINES-1)     // why is NUM_SETUP_LINES-1 necessarily continue?
                  if (!save_game_exists)
                     curr_setup_line=(curr_setup_line+n)%NUM_SETUP_LINES;
#endif
               journey_draw(old_setup_line+1);
               journey_draw(curr_setup_line+1);
               break;
            case KEY_ENTER:
               if (!journey_lock)
               {
                  uiHideMouse(NULL);
                  gr_set_fcolor(SELECTED_COLOR);
                  journey_lock = TRUE;
                  journey_funcs[curr_setup_line](TRUE);
                  journey_lock = FALSE;
               }
               break;
            }
            break;
         case SETUP_CREDITS:
            credits_inp=code;
            break;
         case SETUP_CONTINUE:
            switch(code)
            {
            case KEY_PAD_UP:   case KEY_PAD_LEFT:
               n=NUM_SAVE_SLOTS-2; 
            case KEY_PAD_DOWN: case KEY_PAD_RIGHT:
               n++;
               old_diff=curr_sg;
               curr_sg=(curr_sg+n)%NUM_SAVE_SLOTS;
               draw_sg_slot(old_diff);
               draw_sg_slot(curr_sg);
               break;
            case KEY_ENTER:
               load_that_thar_game(curr_sg);
               break;
            case KEY_ESC:
               journey_draw(0);
               break;
            }
            break;
         case SETUP_DIFFICULTY:
            switch(code)
            {
               case ALT('X'):         // Don't print the X when user ALT-X's out of the game
               case ALT('x'):break;
               case '-':          case KEY_PAD_LEFT:
                  n=NUM_DIFF_CATEGORIES-2;               // note sneaky -2 for fallthrough
               case '+':          case KEY_PAD_RIGHT:
                  n++;                                   // n now NDC-1 or 1
                  if (!start_selected)
                  {
                     old_diff = player_struct.difficulty[curr_diff];
                     draw_difficulty_description(curr_diff,0);
                     player_struct.difficulty[curr_diff] = (player_struct.difficulty[curr_diff] + n) % NUM_DIFF_CATEGORIES;
                     draw_difficulty_char(curr_diff * NUM_DIFF_CATEGORIES + player_struct.difficulty[curr_diff]);
                     draw_difficulty_char(curr_diff * NUM_DIFF_CATEGORIES + old_diff);
                     draw_difficulty_description(curr_diff,NORMAL_ENTRY_COLOR);
                     compute_new_diff();
                  }
                  break;
               case KEY_PAD_UP:   case (KEY_TAB | KB_FLAG_SHIFT):
                  n=NUM_DIFF_CATEGORIES-2;               // sneaky fallthrough
               case KEY_PAD_DOWN: case KEY_TAB:
                  n++;                                   // now -1 or 1
                  if (start_selected && n == 1)
                  {
                     start_selected = FALSE;
                     curr_diff = 0;
                  }
                  else if (start_selected && n == NUM_DIFF_CATEGORIES-1)
                  {
                     start_selected = FALSE;
                     curr_diff = NUM_DIFF_CATEGORIES-1;
                  }
                  else if ((curr_diff == NUM_DIFF_CATEGORIES-1 && n == 1) || (curr_diff == 0 && n == NUM_DIFF_CATEGORIES-1))
                     start_selected = TRUE;
                  else
                  {
                     start_selected = FALSE;
                     curr_diff = (curr_diff + n) % NUM_DIFF_CATEGORIES;
                  }
                  difficulty_draw(FALSE);
                  break;                   
               case KEY_ENTER:
//                  if (start_name[0] != '\0' && start_selected)
              // note go_and_start the game checks for null string and flashes, so it should get called
                  if (start_selected)
                     go_and_start_the_game_already();
                  break;
               case KEY_ESC:
                  journey_draw(0);
                  break;
               default:
                  draw_username(0,start_name);
                  n = strlen(start_name);
                  if ( (kb_isprint(kev->code) && (n < MAX_NAME_SIZE)) &&
                       ( ((kev->code&0xff)>=128)&&((kev->code&0xff)<=155) ||
                         ((kev->code&0xff)>=160)&&((kev->code&0xff)<=165) ||
                         (strchr(valid_char_string,(kev->code & 0xFF)) != NULL) ) )
                  {
                     start_name[n] = (kev->code & 0xFF);
                     start_name[n+1] = '\0';
                  }
                  if (((kev->code & 0xFF) == KEY_BS) && (n > 0))
                     start_name[n-1] = '\0';
                  if (!(gr_string_nwidth(start_name,n) < DIFF_NAME_X2 - DIFF_NAME_TEXT_X))
                     start_name[n]='\0';
                  draw_username(NORMAL_ENTRY_COLOR, start_name);
                  break;
            }
            break;
      }
   }
   return(main_kb_callback(ev,r,user_data));
}
#pragma enable_message(202)


// -------------------------------------------------------------
// setup_init()
//

errtype load_savegame_names()
{
   int i, filenum;
   char path[256];
   extern Datapath savegame_dpath;

   valid_save = 0;

   for (i=0; i<NUM_SAVE_SLOTS; i++)
   {
      Poke_SaveName(i);
      if (DatapathFind(&savegame_dpath, save_game_name, path))
      {
         filenum = ResOpenFile(path);
         if (ResInUse(OLD_SAVE_GAME_ID_BASE))
         {
#ifdef OLD_SG_FORMAT
            ResExtract(OLD_SAVE_GAME_ID_BASE, comments[i]);
            valid_save |= (1 << i);
#else
            strcpy(comments[i], "<< BAD VERSION >>");
#endif
         }
         else
         {
            if (ResInUse(SAVELOAD_VERIFICATION_ID))
            {
               int verify_cookie;
               ResExtract(SAVELOAD_VERIFICATION_ID, &verify_cookie);
               switch (verify_cookie)
               {
                  case OLD_VERIFY_COOKIE_VALID:
// Uncomment these lines to reject Shock Floppy save games
//                     sprintf(comments[i], "<< %s >>",get_temp_string(REF_STR_BadVersion + 1));
//                     break;
                  case VERIFY_COOKIE_VALID:
                     ResExtract(SAVE_GAME_ID_BASE, comments[i]);
                     valid_save |= (1 << i);
                     break;
                  default:  
                     sprintf(comments[i], "<< %s >>",get_temp_string(REF_STR_BadVersion));
                     break;
               }
            }
            else
               sprintf(comments[i], "<< %s >>",get_temp_string(REF_STR_BadVersion));
         }
         ResCloseFile(filenum);
      }
      else
         *(comments[i])='\0';
   }

   return(OK);
}

errtype setup_init(void)
{
#ifndef GAMEONLY
   int   data[1];
   int   cnt;
#endif

   generic_reg_init(TRUE,&setup_root_region,NULL,&setup_slab,intro_key_handler,intro_mouse_handler);

#ifndef GAMEONLY
   cnt = 1;
   if (config_get_value("intro", CONFIG_INT_TYPE, data, &cnt))
   {
      physics_running = TRUE;
      time_passes = TRUE;
      _current_loop = SETUP_LOOP;
   }
   if (!config_get_raw(CFG_NAME_VAR, start_name, 40))
      strcpy(start_name, get_temp_string(REF_STR_DefaultPlayName));
   load_savegame_names();
#endif

   return(OK);
}

// -------------------------------------------------------------
// setup_start()
//

static bool direct_into_cutscene = FALSE;
bool start_first_time = TRUE;

#define MAX_TRY_NUM   8
#define CFG_INIT_SVG "init_savegame"

void setup_start()
{
   extern errtype load_da_palette();
   int do_i_svg=-1, i_invuln=0;

   // Check to see whether or not to play the intro cut scene
#ifdef GAMEONLY
   load_savegame_names();
#endif

   startup_music = FALSE;
   save_game_exists = (valid_save != 0);

   if (setup_mode != SETUP_CREDITS)
   {
      if (!save_game_exists && start_first_time)
      {
         play_intro_anim = TRUE; 
         setup_mode = SETUP_ANIM; 
      }
      else
      {
         play_intro_anim = FALSE; 
         setup_mode = SETUP_JOURNEY;
      }
   }
   if (!start_first_time)
   {
//      start_setup_sound();
      closedown_game(TRUE);
   }
   start_first_time = FALSE;

#ifdef GADGET
   _current_root = NULL;     /* got rid of pointer type mismatch
                              * since one was a region and the other a gadget
                              * someone should probably go and figure it out
                              */
#endif
   _current_3d_flag = ANIM_UPDATE;
   _current_fr_context = NULL;
   _current_view = &setup_root_region;
   static_change_copy();
   message_info("");

   // clear the screen
   gr_clear(0);

   HotkeyContext = SETUP_CONTEXT;
   uiSetCurrentSlab(&setup_slab);

   // flush the keyboard and mouse - so we don't read old events
   kb_flush();
   mouse_flush();

   intro_num = ResOpenFile("intro.res");

   // slam in the right palette
   load_da_palette();

   // wacky initial savegame hackiness
	{
	   int i = 2;
	   int dvec[2];

      config_get_value(CFG_INIT_SVG,CONFIG_INT_TYPE,dvec,&i);
      if (i>0)
         do_i_svg=dvec[0];
      if (i>1)
         i_invuln=dvec[1];
   }

   if (do_i_svg!=-1)
   {
#ifdef PLAYTEST
      player_invulnerable = i_invuln;
#endif
      uiShowMouse(NULL);
      load_that_thar_game(do_i_svg);
   }
   else if (!play_intro_anim)
   {
      uiShowMouse(NULL);
      switch (setup_mode)
      {
         case SETUP_DIFFICULTY:
            difficulty_draw(TRUE);
            break;
         case SETUP_JOURNEY:
            journey_draw(0);
            break;
      }
      direct_into_cutscene = FALSE;
      if (setup_mode != SETUP_CREDITS)
         start_setup_sound(0);
      else
         start_setup_sound(1);
   }
   else
   {
      direct_into_cutscene = TRUE;
      play_cutscene(START_CUTSCENE, TRUE);
   }

}

//#define SAFETY_PUPS_NIECE
#ifdef SAFETY_PUPS_NIECE
uchar safety_pups_semaphore=0;
#endif

// -----------------------------------------------
// setup_exit()
//

void setup_exit()
{
   extern void end_intro_sound(void);

   ResCloseFile(intro_num);

#ifdef PALFX_FADES
   if (pal_fx_on) 
      palfx_fade_down();
   else {
      gr_set_fcolor(BLACK);
      gr_rect(0,0,320,200);
   }
#endif

   // make sure the sound is off before leaving
   end_setup_sound();

   if ((startup_music) && (music_on))
      start_music_func(0,0,0);

#ifdef SAFETY_PUPS_NIECE
   mlimbs_shutdown();
   safety_pups_semaphore=1;
#endif

   // must get rid of mouse - to maintain hidden mouse after loop
   if (!direct_into_cutscene)
      uiHideMouse(NULL);
   direct_into_cutscene = FALSE;
}

#endif //NOT_YET���
