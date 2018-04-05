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
 * $Source: r:/prj/cit/src/RCS/mfdgames.c $
 * $Revision: 1.45 $
 * $Author: buzzard $
 * $Date: 1994/11/22 09:32:36 $
 *
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "faketime.h"
#include "player.h"
#include "newmfd.h"
#include "mfdint.h"
#include "mfddims.h"
#include "colors.h"
#include "rcolors.h"
#include "tools.h"
#include "mainloop.h"
#include "gameloop.h"
#include "wares.h"
#include "amap.h"
#include "objprop.h"
#include "objsim.h"
#include "miscqvar.h"
#include "cit2d.h"
#include "diffq.h"
#include "mfdgames.h"

#include "sfxlist.h"
#include "musicai.h"
#include "gamestrn.h"

#include "cybstrng.h"
#include "otrip.h"
#include "mfdart.h"
#include "gamescr.h"
#include "gr2ss.h"

#ifdef LOST_TREASURES_OF_MFD_GAMES
#include "minimax.h"
#include "limits.h"
#endif


// -------
// Globals
// -------

#ifdef LOST_TREASURES_OF_MFD_GAMES
extern void fstack_init(uchar* fs, uint siz);
#endif

#define PUZZLE_DIFFICULTY (QUESTVAR_GET(PUZZLE_DIFF_QVAR))

// special storage used by mfdgames and by wiring puzzles
// but note it's not managed correctly across save/restore etc.
//   Hey, this used to be 1024!
#define HIDEOUS_GAME_STORAGE            2048
unsigned char hideous_secret_game_storage[HIDEOUS_GAME_STORAGE + 4];

// stuff to check magic cookie in secret game stuff
#define COOKIE    (*((ulong *) hideous_secret_game_storage))

#define COOK_VAL(a,b,c,d)  ((((a)*256+(b))*256+(c))*256+(d))
#define GAME_COOK(d)       COOK_VAL('G','a','m',(d))

#define NUM_GAMES 8

LGRect GamesMenu;
static long score_time=0;
extern bool full_game_3d;

static int games_time_diff;

void Rect_gr_box(LGRect *r);
void Rect_gr_rect(LGRect *r);

#define STRING(x) get_temp_string(REF_STR_ ## x)

// note everything crammed into chars due to all mfds always active all have memory always architechture
// if they were ints, all would work, and be much happier to boot

// note that the starting fields of this must match the
// starting fields of Robot Invasion, since they share
// ball and player movement code
typedef struct {
   uchar game_mode;
   char  ball_pos_x, ball_pos_y;
   char  ball_dir_x, ball_dir_y;
   char  p_pos, p_spd;
   char  c_pos, c_spd;
   uchar c_score;
   uchar p_score;
   uchar last_point;
   uchar game_won;
} pong_state;

typedef struct
{
   uchar game_mode;
   ushort save_time;
   // current QIX endpoints
   char x[2],dx[2],y[2],dy[2];
   uchar color;
} menu_state;

typedef struct {
   uchar  game_mode;     // current game mode
   uchar  lane_cnt;      // current lane count
   uchar  diff;          // 0-0xf chance of a new car per lane
   char   player_lane;   // current player line
   char   player_move;   // which way we are going
   ushort lanes[8];      // two bits for each
   uchar  game_state;    // have we lost
   uint   frame_cnt;     // how many frames at this difficulty
//   uchar  last_bs;       // last button state
} road_state;

typedef struct
{
	// shared state with Ping
   uchar game_mode;
   char  ball_pos_x, ball_pos_y;
   char  ball_dir_x, ball_dir_y;
   char  p_pos, p_spd;

	// brick state
   ushort rows[3];	// BOTS_NUM_ROWS
   short hpos;
   char hspd;
   char vpos;
   char balls;
} bots_state;

#define PONG_X_DIR_MAX 7
		  
#define GAME_DATA_SIZE 32                      
#define GAME_DATA (&player_struct.mfd_access_puzzles[0])
#define GAME_DATA_2 (&player_struct.mfd_func_data[MFD_GAMES_FUNC][0])
#define GAME_MODE (*((uchar*)GAME_DATA))

#define GAME_MODE_MENU NUM_GAMES

//------------
// PROTOTYPES
//------------
void games_expose_pong(MFD *m,ubyte control);
void games_expose_null(MFD *m,ubyte control);
void games_expose_menu(MFD *m,ubyte control);
static void games_expose_mcom(MFD *m,ubyte control);
void games_expose_bots(MFD *m,ubyte control);
void games_expose_road(MFD *m,ubyte control);

void games_init_pong(void *game_state);
static void games_init_mcom(void *game_state);
void games_init_road(void *game_state);
void games_init_null(void *game_state);
void games_init_bots(void *game_state);

#ifdef LOST_TREASURES_OF_MFD_GAMES
void games_expose_15(MFD *m,ubyte control);
void games_init_15(void *game_state);
bool games_handle_15(MFD *m, uiEvent *e);

void games_expose_ttt(MFD *m,ubyte control);
void games_init_ttt(void* game_state);
bool games_handle_ttt(MFD *m, uiEvent *e);

void games_expose_wing(MFD *m,ubyte control);
void games_init_wing(void *game_state);
bool games_handle_wing(MFD *m, uiEvent *e);
#else
#define games_expose_15 games_expose_null
#define games_init_15 games_init_null
#define games_handle_15 games_handle_null

#define games_expose_ttt games_expose_null
#define games_init_ttt games_init_null
#define games_handle_ttt games_handle_null

#define games_expose_wing games_expose_null
#define games_init_wing games_init_null
#define games_handle_wing games_handle_null
#endif

bool games_handle_pong(MFD* m, uiEvent* e);
bool games_handle_road(MFD *m, uiEvent *e);
bool games_handle_menu(MFD* m, uiEvent* ev);
bool games_handle_null(MFD* m, uiEvent* ev);

void games_run_pong(pong_state *work_ps);
void games_run_road(road_state *work_ps);
void games_run_bots(bots_state *bs);

static void mcom_start_level(void);
int tictactoe_evaluator(void* pos);

void mfd_games_turnon(bool, bool );
void mfd_games_turnoff(bool, bool );

void (*game_expose_funcs[])(MFD *m,ubyte control)=
{
games_expose_pong,
games_expose_mcom,
games_expose_road,
games_expose_bots,
games_expose_15,
games_expose_ttt,
games_expose_null,
games_expose_wing,
games_expose_menu
};

void (*game_init_funcs[])(void* game_data)=
{
   games_init_pong,
   games_init_mcom,
   games_init_road,
   games_init_bots,
   games_init_15,
   games_init_ttt,
   games_init_null,
   games_init_wing,
   games_init_null
};

extern bool (*game_handler_funcs[])(MFD *m,uiEvent* ev);

#define NORMAL_DISPLAY  0
#define SCORE_DISPLAY   1
#define WIN_DISPLAY     2

#define WIN_PAUSE       (4*CIT_CYCLE)
#define SCORE_PAUSE     (2*CIT_CYCLE) 

#define MFD_VIEW_MID    (MFD_VIEW_WID/2)

// ===========================================================================
//                         * THE MFD GAMES CODE *        
// ===========================================================================
errtype mfd_games_init(MFD_Func*)
{
   GamesMenu.ul.x=1; GamesMenu.ul.y=1; GamesMenu.lr.x=6; GamesMenu.lr.y=6;
   player_struct.mfd_func_status[MFD_GAMES_FUNC]|=1<<4;

   return OK;
}

// ---------------------------------------------------------------------------
// mfd_games_handler()
bool mfd_games_handler(MFD *m, uiEvent *e)
{
   int cur_mode = GAME_MODE;
   bool retval = (*game_handler_funcs[cur_mode])(m,e);
   uiMouseEvent *mouse;
   LGRect r;

   // detect if you have games
   // if (player_struct.hardwarez[HARDWARE_AUTOMAP] == 0) return FALSE;
   
   mouse = (uiMouseEvent *) e;
   if (!(mouse->action & MOUSE_LDOWN)) return FALSE;       // ignore click releases

   // Did the user click in the "menu" clickbox?
   RECT_OFFSETTED_RECT(&GamesMenu, m->rect.ul, &r);
   if (RECT_TEST_PT(&r, e->pos))
   {
      GAME_MODE=GAME_MODE_MENU;
      mfd_notify_func(MFD_GAMES_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, FALSE);
      retval = TRUE;
   }
   return retval;
}


// ---------------------------------------------------------------------------
// mfd_games_expose()
//

void mfd_games_expose(MFD *m, ubyte control)
{
   int cur_mode;
   grs_font* fon;
   ulong dt=player_struct.deltat;
   extern int mfd_slot_primary(int slot);

   if (control & MFD_EXPOSE) {

      gr_push_canvas(pmfd_canvas);
      ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
      mfd_clear_rects();

      fon = (grs_font *)ResLock(RES_tinyTechFont);
      gr_set_font(fon);

      cur_mode=GAME_MODE;
#ifdef PLAYTEST
	 // so, like, this code is totally meaningless.
	 // I'm glad WATCOM bothers to warn about it.
	 // if anyone fixes it, probably should make
	 // cur_mode = GAME_MODE_MENU not 0 (ping)
      if ((cur_mode<0)&&(cur_mode>NUM_GAMES))
	cur_mode=0;
#endif
      if (COOKIE != GAME_COOK(GAME_MODE)) {
	// hey, our secret storage data became invalid...
	// umm, so, umm, what to do?  Hey, let's just pop
	// them back into menu mode for now
	cur_mode = GAME_MODE = GAME_MODE_MENU;
	COOKIE = GAME_COOK(cur_mode);
	// note we rely on the fact that if someone else
	// stomps GAME_MODE to GAME_MODE_MENU, this will
	// get caught by our cookie test, which will then
	// result in us being able to initialize the save_time field 
	((menu_state *) GAME_DATA)->save_time = 0;
      }
      if(m->id == mfd_slot_primary(MFD_INFO_SLOT))
	 games_time_diff+=dt;
      (*game_expose_funcs[cur_mode])(m,control);

      if(GAME_MODE!=GAME_MODE_MENU) {
	     gr_set_fcolor(MFD_BTTN_FLASH);
	     Rect_gr_rect(&GamesMenu);
	     gr_set_fcolor(WHITE);
	     Rect_gr_box(&GamesMenu);
      }

      ResUnlock(RES_tinyTechFont);

      gr_pop_canvas();
      mfd_update_rects(m);
   }

   return;
}

#define MENU_GAMELIST_X 5
#define MENU_GAMELIST_Y 15
#define MENU_GAMELIST_DY 7


#define MISC_SOFTWARE_GAMES (TRIP2TY(GAMES_TRIPLE) + NUM_ONESHOT_SOFTWARE)

#define TIME_TIL_SCREEN_SAVE    (60*30) // 30 fps, 60 frames

#define MAX_LINES       12
int ss_head=0;
typedef struct
{
  char x1,y1,x2,y2,c;  
} oldLines;
oldLines *old_lines = (oldLines *)(hideous_secret_game_storage + 4);

static void init_screen_save(menu_state *ms)
{
  int i;
  for (i=0; i < MAX_LINES; ++i)
    old_lines[i].x1 = old_lines[i].y1 = 
    old_lines[i].x2 = old_lines[i].y2 =
    old_lines[i].c = 0;

  ms->x[0] = MFD_VIEW_WID/2 - 4;
  ms->x[1] = MFD_VIEW_WID/2 + 4;
  ms->y[0] = ms->y[1] = MFD_VIEW_HGT/2;
  ms->dx[0] = 1;
  ms->dy[0] = 1;
  ms->dx[1] = 2;
  ms->dy[1] = -1;
  ms->color = 128;
}

#define draw_shadowed_text(s,x,y) draw_shadowed_string((s),(x),(y),full_game_3d)

void games_expose_menu(MFD *, ubyte )
{
   char buf[80];
   int i;
   ubyte cur_games=player_struct.softs.misc[MISC_SOFTWARE_GAMES];
   menu_state *ms = ((menu_state *) GAME_DATA);
   if (!full_game_3d)                   
//KLC - chg for new art   draw_res_bm(REF_IMG_bmBlankMFD, 0,0);
	 draw_hires_resource_bm(REF_IMG_bmBlankMFD, 0, 0);
   if (ms->save_time < TIME_TIL_SCREEN_SAVE) {
     ++ms->save_time;
     strcpy(buf,STRING(GamesMenu));
     wrap_text(buf,MFD_VIEW_WID-8);
     gr_set_fcolor(RED_8_BASE+4);
     draw_shadowed_text(buf,4,1);

     strcpy(buf,STRING(DontPlay));
     wrap_text(buf, MFD_VIEW_WID-5);
     draw_shadowed_text(buf,10,MFD_VIEW_HGT-14);

     gr_set_fcolor(GREEN_8_BASE+3);
     for (i=0; i<NUM_GAMES; i++)
       if (cur_games&(1<<i))
	 draw_shadowed_text(STRING(GameName0+i),MENU_GAMELIST_X+((i%2)*((MFD_VIEW_WID-10)/2)),
	   MENU_GAMELIST_Y+((i>>1)*MENU_GAMELIST_DY));
   } else {
     int i;
     if (ms->save_time == TIME_TIL_SCREEN_SAVE) {
       init_screen_save(ms);
       ++ms->save_time;
     }
     // run screen saver
     if ((rand() & 0xff) < 20) {
       for (i=0; i < 2; ++i) {
	 ms->dx[i] = (rand() % 6) - 3;
	 ms->dy[i] = (rand() % 6) - 3;
	 if (ms->dx[i] >= 0) ms->dx[i] += 2; else ms->dx[i] -= 1;
	 if (ms->dy[i] >= 0) ms->dy[i] += 2; else ms->dy[i] -= 1;
       }
     }
     if ((rand() & 0xff) < 80)
       ms->color = (rand() & 0x7f)+32;
     for (i=0; i < 2; ++i) {
       ms->x[i] += ms->dx[i];
       ms->y[i] += ms->dy[i];
       if (ms->x[i] < 0 || ms->x[i] >= MFD_VIEW_WID)
	 ms->x[i] += (ms->dx[i] = -ms->dx[i]);
       if (ms->y[i] < 0 || ms->y[i] >= MFD_VIEW_HGT)
	 ms->y[i] += (ms->dy[i] = -ms->dy[i]);
     }
     old_lines[ss_head].x1 = ms->x[0];
     old_lines[ss_head].y1 = ms->y[0];
     old_lines[ss_head].x2 = ms->x[1];
     old_lines[ss_head].y2 = ms->y[1];
     old_lines[ss_head].c = ms->color++;
     for(i=0; i < MAX_LINES; ++i) {
       gr_set_fcolor(old_lines[i].c);
       ss_int_line(old_lines[i].x1, old_lines[i].y1,
		   old_lines[i].x2, old_lines[i].y2);
     }
     if (++ss_head == MAX_LINES) ss_head = 0;
   }
   mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
}

bool games_handle_menu(MFD* m, uiEvent* ev)
{
   uiMouseEvent *mouse;
   int game;
   int cur_games = player_struct.softs.misc[MISC_SOFTWARE_GAMES];
   
   if (GAME_MODE == GAME_MODE_MENU)
     ((menu_state *) GAME_DATA)->save_time = 0;

   mouse = (uiMouseEvent *) ev;
   if (!(mouse->action & MOUSE_LDOWN)) return FALSE;       // ignore click releases

   if(ev->pos.y-m->rect.ul.y < MENU_GAMELIST_Y) return FALSE;
   game=(((ev->pos.y)-(m->rect.ul.y)-MENU_GAMELIST_Y)/MENU_GAMELIST_DY)*2;
   if((ev->pos.x)-(m->rect.ul.x) > MENU_GAMELIST_X+(MFD_VIEW_WID-10)/2)
      game++;

   if(game>NUM_GAMES || ((1 <<game) & cur_games) == 0) return FALSE;

   COOKIE = GAME_COOK(game);
   LG_memset(GAME_DATA,0,GAME_DATA_SIZE);
   GAME_MODE=game;
   game_init_funcs[GAME_MODE](GAME_DATA);

   string_message_info(REF_STR_GameDescrip0+game);

   mfd_notify_func(MFD_GAMES_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, TRUE);
   return TRUE;
}

void games_init_null(void*)
{
   return;
}

void games_expose_null(MFD *, ubyte )
{
   int  cur_games=0xff;

   if (!full_game_3d)
//KLC - chg for new art   draw_res_bm(REF_IMG_bmBlankMFD, 0,0);
	  draw_hires_resource_bm(REF_IMG_bmBlankMFD, 0, 0);
   ss_string(STRING(NotInstalled),10,20);
   ss_string(STRING(NotInstalled+1),15,35);
   mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
}

bool games_handle_null(MFD* , uiEvent* )
{
   return FALSE;
}

// -----------------
// mfd pong

#define PLY_PADDLE_YRAD 2
#define PLY_PADDLE_XRAD 6
#define CMP_PADDLE_YRAD 2
#define CMP_PADDLE_XRAD 6
#define PONG_BALL_YRAD  2
#define PONG_BALL_XRAD  3
#define PONG_BORDER     3
#define PONG_SWEET_SPOT 2

int get_new_x_dir(int paddle_loc, int ball_loc);
int generic_ball_and_paddle(void *game_state);

// -------------------
// generic paddle-hits-ball code

int get_new_x_dir(int paddle_loc,int ball_loc)
{
   int x_dir=((ball_loc-paddle_loc)/2);
   if (x_dir<0)
      if (x_dir>=-PONG_SWEET_SPOT) x_dir=0;
      else if (x_dir<-PONG_X_DIR_MAX) x_dir=-PONG_X_DIR_MAX;
      else x_dir++;
   else if (x_dir>0)
      if (x_dir<=PONG_SWEET_SPOT) x_dir=0;
      else if (x_dir>PONG_X_DIR_MAX) x_dir=PONG_X_DIR_MAX;
      else x_dir--;
   return x_dir;
}

// this function returns non-zero
// if the ball went off the bottom past the paddle
int generic_ball_and_paddle(void *game_state)
{
  pong_state *work_ps = (pong_state *) game_state;

   // now deal with moving the ball, assume it is currently a valid state
   work_ps->ball_pos_x+=work_ps->ball_dir_x;
   work_ps->ball_pos_y+=work_ps->ball_dir_y;
   work_ps->p_pos+=work_ps->p_spd;
   if (work_ps->ball_pos_x<PONG_BALL_XRAD)
   {
      work_ps->ball_pos_x=PONG_BALL_XRAD+(PONG_BALL_XRAD-work_ps->ball_pos_x);
      work_ps->ball_dir_x=-work_ps->ball_dir_x;
   }
   if (work_ps->ball_pos_x>MFD_VIEW_WID-1-PONG_BALL_XRAD)
   {
      work_ps->ball_pos_x=MFD_VIEW_WID-1-PONG_BALL_XRAD-(PONG_BALL_XRAD-(MFD_VIEW_WID-1-work_ps->ball_pos_x));
      work_ps->ball_dir_x=-work_ps->ball_dir_x;
   }
   if (work_ps->ball_dir_y>=0)
    // lets see whats up with the player
   {
     if (work_ps->ball_pos_y-PONG_BALL_YRAD>MFD_VIEW_HGT-PONG_BORDER)
	   {  // out the bottom
	     return 1;
	   }
     else if (work_ps->ball_pos_y>MFD_VIEW_HGT-PONG_BORDER-CMP_PADDLE_YRAD*2-PONG_BALL_YRAD)
	{  // was the player there to deflect it
	  if ((work_ps->ball_pos_x+PONG_BALL_XRAD>=work_ps->p_pos-PLY_PADDLE_XRAD)&&
	      (work_ps->ball_pos_x-PONG_BALL_XRAD<=work_ps->p_pos+PLY_PADDLE_XRAD))
	      {  // we got it, larry
		int nxdir=get_new_x_dir(work_ps->p_pos,work_ps->ball_pos_x);
		work_ps->ball_dir_y=-work_ps->ball_dir_y;       // hmm, does dirx work yet
		work_ps->ball_dir_x=(nxdir-work_ps->ball_dir_x)>>1;      // average of reflection and new angle??? is this good?
		  // minus looks good if it hits the edge of the paddle
		  // plus looks good if it hits the middle of the paddle
		  //   hmm... so need average of reflection and new, but
		  //   reflection has different meanings depending where it hit
		play_digi_fx(SFX_MFD_SUCCESS,1);
	      }
	   }
   }
   return 0;
}

void games_init_pong(void *game_state)
{
   pong_state *cur_ws=(pong_state *)game_state;
   cur_ws->ball_dir_x=cur_ws->ball_dir_y=cur_ws->c_spd=cur_ws->p_spd=cur_ws->p_score=cur_ws->c_score=cur_ws->game_won=0;
   cur_ws->c_pos=cur_ws->p_pos=MFD_VIEW_MID;      // move paddles to middle
   score_time=0;
   games_time_diff=0;
}

void games_run_pong(pong_state *work_ps)
{
   int c_des_spd=0;    // desired speed for the computer player
   if ((work_ps->ball_dir_x|work_ps->ball_dir_y)==0)
   {  // create new ball
      int serve_speed = (work_ps->c_score+work_ps->p_score)/8+1;
      work_ps->ball_pos_y=MFD_VIEW_HGT/2;
//      work_ps->ball_dir_x=(rand()%3)-1;       // -1 -> 1 for x
      work_ps->ball_dir_x=0;
      work_ps->ball_dir_y=((rand()%3)-1)*serve_speed;
      if (work_ps->ball_dir_y==0) work_ps->ball_dir_y=-serve_speed; // -2 -> 2, but not 0, for y
      if (work_ps->ball_dir_y < 0)
	 work_ps->ball_pos_x=work_ps->c_pos;
      else
	 work_ps->ball_pos_x=work_ps->p_pos;
      score_time=0;
   }

   // the brutally powerful AI, thats right, AI, think about it
   if (work_ps->ball_dir_y<0) // moving towards computer
   {
      if (work_ps->c_pos<work_ps->ball_pos_x-CMP_PADDLE_XRAD) // we are too far left
	 c_des_spd=(((work_ps->ball_pos_x-work_ps->c_pos)>>4)+1);
      else if (work_ps->c_pos>work_ps->ball_pos_x+CMP_PADDLE_XRAD)
	 c_des_spd=-(((work_ps->c_pos-work_ps->ball_pos_x)>>4)+1);
   }                              
   if (work_ps->c_spd>c_des_spd)
      work_ps->c_spd--;
   else if (work_ps->c_spd<c_des_spd)
      work_ps->c_spd++;

   if (generic_ball_and_paddle(work_ps)) {
       // the player missed.
       // what a loser
     play_digi_fx(SFX_MFD_BUZZ,1);
     work_ps->ball_dir_x=work_ps->ball_dir_y=0;
     work_ps->last_point=0;
     if (++work_ps->c_score==0x7) work_ps->game_won=1;
     score_time=player_struct.game_time;
     return;
   }

   work_ps->c_pos+=work_ps->c_spd;
   if (work_ps->ball_dir_y<0) // moving towards computer
   {
	   if (work_ps->ball_pos_y+PONG_BALL_YRAD<PONG_BORDER)
	   {  // out the top, point for the player
	 play_digi_fx(SFX_INVENT_WARE,1);
	 work_ps->ball_dir_x=work_ps->ball_dir_y=0;
	 work_ps->last_point=1;
	 if (++work_ps->p_score==0x7) work_ps->game_won=1;
	 score_time=player_struct.game_time;
      }
	   else if (work_ps->ball_pos_y<PONG_BORDER+CMP_PADDLE_YRAD*2+PONG_BALL_YRAD)
	   {  // was the computer there to deflect it
	      if ((work_ps->ball_pos_x+PONG_BALL_XRAD>=work_ps->c_pos-CMP_PADDLE_XRAD)&&
		  (work_ps->ball_pos_x-PONG_BALL_XRAD<=work_ps->c_pos+CMP_PADDLE_XRAD))
	      {  // we got it, larry
	    int nxdir=get_new_x_dir(work_ps->c_pos,work_ps->ball_pos_x);
		 work_ps->ball_dir_y=-work_ps->ball_dir_y;       // hmm, does dirx work yet
	    work_ps->ball_dir_x=(nxdir-work_ps->ball_dir_x)>>1;      // average of reflection and new angle??? is this good?
	    work_ps->ball_dir_x += rand()%3 -1;
      play_digi_fx(SFX_MFD_SUCCESS,1);
	      }
	   }
   }

}

#define PONG_CYCLE (CIT_CYCLE/30)
void games_expose_pong(MFD *m, ubyte tac)
{
   pong_state *cur_ps=(pong_state *)GAME_DATA;
   int game_use=NORMAL_DISPLAY;

   if (tac == 0 && score_time > 0)
      score_time = 0;
   // is not true
   if (cur_ps->game_won)
   {
      if (score_time+WIN_PAUSE>player_struct.game_time)
	 game_use=WIN_DISPLAY;
      else
//       { games_init_pong(cur_ps); return;}
       { GAME_MODE=GAME_MODE_MENU; return; }
   }
   else if (score_time+SCORE_PAUSE>player_struct.game_time) {
      game_use=SCORE_DISPLAY;
      games_time_diff=0;
   }
   else
   {
	   extern bool games_handle_pong(MFD*,uiEvent*);
	   uiEvent fake_event;

      for(; games_time_diff>=PONG_CYCLE; games_time_diff-=PONG_CYCLE) {
	      games_run_pong(cur_ps);
	      ui_mouse_get_xy(&fake_event.pos.x,&fake_event.pos.y);
	      games_handle_pong(m,&fake_event);
	      if (score_time > 0 || cur_ps->game_won)
		 break;
      }
   }
   if (!full_game_3d)
//KLC - chg for new art   draw_res_bm(REF_IMG_bmBlankMFD, 0,0);
	  draw_hires_resource_bm(REF_IMG_bmBlankMFD, 0, 0);

   gr_set_fcolor(ORANGE_YELLOW_BASE);
   ss_rect(cur_ps->c_pos-CMP_PADDLE_XRAD,PONG_BORDER,cur_ps->c_pos+CMP_PADDLE_XRAD,PONG_BORDER+CMP_PADDLE_YRAD*2);

   gr_set_fcolor(ORANGE_YELLOW_BASE);
   ss_rect(cur_ps->p_pos-PLY_PADDLE_XRAD,MFD_VIEW_HGT-PONG_BORDER-PLY_PADDLE_YRAD*2,cur_ps->p_pos+PLY_PADDLE_XRAD,MFD_VIEW_HGT-PONG_BORDER);

   gr_set_fcolor(GRAY_8_BASE);
   ss_rect(cur_ps->ball_pos_x-PONG_BALL_XRAD,cur_ps->ball_pos_y-PONG_BALL_YRAD,cur_ps->ball_pos_x+PONG_BALL_XRAD,cur_ps->ball_pos_y+PONG_BALL_YRAD);

   if (game_use!=NORMAL_DISPLAY)
   {
      char tmp[2]="V";
      if (cur_ps->last_point)
	 ss_string(STRING(YouHave),MFD_VIEW_MID-18,15);
      else
	 ss_string(STRING(ComputerHas),MFD_VIEW_MID-25,15);
      if (cur_ps->game_won)
	      ss_string(STRING(Won),MFD_VIEW_MID-8,22);
      else
	      ss_string(STRING(Scored),MFD_VIEW_MID-16,22);
      ss_string(STRING(You),MFD_VIEW_MID-25,35);
      tmp[0]='0'+cur_ps->p_score;
      ss_string(tmp,MFD_VIEW_MID+10,35);
      ss_string(STRING(Computer),MFD_VIEW_MID-25,45);
      tmp[0]='0'+cur_ps->c_score;
      ss_string(tmp,MFD_VIEW_MID+10,45);
   }
   mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);

   // autoreexpose
   mfd_notify_func(MFD_GAMES_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, FALSE);

   return;
}

#define RECTWID(r) ((r).lr.x-(r).ul.x)
#define RECTHGT(r) ((r).lr.y-(r).ul.y)

#define PONG_FUDGE 10

bool games_handle_pong(MFD* m, uiEvent* e)
{
   pong_state *cur_ps=(pong_state *)GAME_DATA;
   LGPoint pos = MakePoint(e->pos.x-m->rect.ul.x,
			 e->pos.y-m->rect.ul.y);
   ubyte spd = min(3,abs(pos.x - cur_ps->p_pos)/PLY_PADDLE_XRAD+1);
   if(pos.x+PONG_FUDGE<0 || pos.y+PONG_FUDGE<0 ||
      pos.x>=RECTWID(m->rect)+PONG_FUDGE || pos.y>=RECTHGT(m->rect)+PONG_FUDGE)
   {
      cur_ps->p_spd=0;
      return TRUE;
   }
   if (cur_ps->p_pos == pos.x)
      cur_ps->p_spd = 0;
   else if (cur_ps->p_pos > pos.x)
      cur_ps->p_spd = -spd;
   else
      cur_ps->p_spd = spd;
   return TRUE;
}

//----------------------------
// mfd road game....

#define NO_CAR      0
#define BAD_CAR     1
#define SMART_CAR   2
#define CAR_MASK    3

#define NORMAL_DISPLAY  0
#define SCORE_DISPLAY   1
#define WIN_DISPLAY     2

#define CAR_ROW     6
#define car_hit()   (cur_rs->lanes[cur_rs->player_lane]&(CAR_MASK<<(CAR_ROW*2)))

#define HIT_BAD     (BAD_CAR<<(CAR_ROW*2))
#define HIT_SMART   (SMART_CAR<<(CAR_ROW*2))

#define FRAMES_PER      100
#define FRAMES_RAMPDOWN  10

#define BASE_DIFF    5
#define DIFF_PER     4
#define DIFF_MUL     3
#define DIFF_MAX    13
#define LANE_MAX     8
#define GAME_WON     2
#define GAME_LOST    4

#define LANE_WID     8
#define LANE_HEIGHT  6
#define LANE_ROWS    8
#define ROAD_TOP     3
#define CAR_TOP      1
#define CAR_BOT      4
#define ROAD_BOTTOM  ROAD_TOP+(LANE_HEIGHT*LANE_ROWS)

#define row_top(y)  (ROAD_TOP+(LANE_HEIGHT*y))

#define BRIGHT_LED      (RED_8_BASE)
#define DIM_LED         (RED_8_BASE+5)
#define BRIGHT_PHOSPHOR (GREEN_8_BASE)
#define DIM_PHOSPHOR    (GREEN_8_BASE+5)
#define BRIGHT_CAR      (BLUE_8_BASE)
#define DIM_CAR         (BLUE_8_BASE+5)

#define CAR_IDX          3
static uchar c_cols[][2]={{BRIGHT_LED,DIM_LED},{BRIGHT_PHOSPHOR,DIM_PHOSPHOR},{45,61},{BRIGHT_CAR,DIM_CAR}};

void games_init_road(void *)
{
   road_state *cur_rs=(road_state *)GAME_DATA;
   games_time_diff=0;
   cur_rs->lane_cnt=3;    
   cur_rs->player_lane=1; 
   cur_rs->diff=BASE_DIFF;
   LG_memset(&cur_rs->player_move,0,sizeof(road_state)-4);     // clear rest of fields
}

void games_run_road(road_state *)
{
   road_state *cur_rs=(road_state *)GAME_DATA;
   int i;

   if (++cur_rs->frame_cnt==FRAMES_PER)    // end of level set
   {
      cur_rs->diff+=DIFF_MUL;
      if (cur_rs->diff<=DIFF_MAX)
	 cur_rs->frame_cnt=0;
   }
   for (i=0; i<cur_rs->lane_cnt; i++)     // update old cars
      cur_rs->lanes[i]<<=2;
   if (cur_rs->frame_cnt>FRAMES_PER)
   {
      if (cur_rs->frame_cnt>FRAMES_PER+FRAMES_RAMPDOWN)
      {
	 cur_rs->frame_cnt=0;
	 cur_rs->diff=BASE_DIFF;
	 if (++cur_rs->lane_cnt>LANE_MAX)
	 {
	    cur_rs->game_state=GAME_WON;
	    score_time=player_struct.game_time;
	 }
      }
      return;
   }
   for (i=0; i<cur_rs->lane_cnt; i++)     // new cars
      if ((rand()%0x3f)<cur_rs->diff)
	 if ((rand()%0x3f)==5)
	      cur_rs->lanes[i]|=SMART_CAR;
	 else
	      cur_rs->lanes[i]|=BAD_CAR;
   cur_rs->player_lane+=cur_rs->player_move;
   cur_rs->player_move= 0;
   if      (cur_rs->player_lane<0)                 cur_rs->player_lane=0; 
   else if (cur_rs->player_lane>=cur_rs->lane_cnt) cur_rs->player_lane=cur_rs->lane_cnt-1;

   if (car_hit()==HIT_SMART)
      LG_memset(cur_rs->lanes,0,8*sizeof(ushort));
   else if (car_hit())     // else if (car_hit()==HIT_BAD)
   {
      play_digi_fx(SFX_DESTROY_CRATE,1);
      cur_rs->game_state=GAME_LOST;
      score_time=player_struct.game_time;
   }
}

static void road_vline(int x, int yt, int yb, int c1, int c2)
{
   gr_set_fcolor(c1);
   ss_vline(x,yt,yb);
   gr_set_fcolor(c2);
   ss_vline(x-1,yt,yb);
   ss_vline(x+1,yt,yb);
}

#define ROAD_CYCLE (CIT_CYCLE/5)
#define uiMakeaDerMotionEventenHausen uiMakeMotionEvent
void games_expose_road(MFD *m, ubyte tac)
{
   road_state *cur_rs=(road_state *)GAME_DATA;
   int game_use=NORMAL_DISPLAY;

   if (tac == 0)
      score_time = 0;
   // is not true
   if (cur_rs->game_state)
   {
      if (score_time+WIN_PAUSE>player_struct.game_time)
	 game_use=WIN_DISPLAY;
      else
       { GAME_MODE=GAME_MODE_MENU; return; }
   }
   else
      for(; games_time_diff>=ROAD_CYCLE; games_time_diff-=ROAD_CYCLE)
      {
	 uiMouseEvent fake_event;
	 uiMakeaDerMotionEventenHausen(&fake_event);
	      games_run_road(cur_rs);
	 games_handle_road(m,(uiEvent *)&fake_event);
	      if (cur_rs->game_state) break;
      }

   if (!full_game_3d)
//KLC - chg for new art   draw_res_bm(REF_IMG_bmBlankMFD, 0,0);
	  draw_hires_resource_bm(REF_IMG_bmBlankMFD, 0, 0);

   {  // draw the game here, eh?
      int ledge, i, j;
      ledge=(MFD_VIEW_WID>>1)-((LANE_WID>>1)*cur_rs->lane_cnt);
      for (i=0; i<cur_rs->lane_cnt; i++, ledge+=LANE_WID)
      {
	 int msk=cur_rs->lanes[i];
	 road_vline(ledge,ROAD_TOP,ROAD_BOTTOM,c_cols[0][0],c_cols[0][1]);
	 for (j=0; j<LANE_ROWS; j++, msk>>=2)
	    if (msk&0x3)
		    road_vline(ledge+(LANE_WID>>1),row_top(j)+CAR_TOP,row_top(j)+CAR_BOT,c_cols[(msk&0x3)-1][0],c_cols[(msk&0x3)-1][1]);
	 if (i==cur_rs->player_lane)
	    road_vline(ledge+(LANE_WID>>1),row_top(CAR_ROW)+CAR_TOP,row_top(CAR_ROW)+CAR_BOT,c_cols[CAR_IDX][0],c_cols[CAR_IDX][1]);
      }
      road_vline(ledge,ROAD_TOP,ROAD_BOTTOM,c_cols[0][0],c_cols[0][1]);
   }

   // won lost state
   if (game_use!=NORMAL_DISPLAY)
   {
      gr_set_fcolor(GRAY_8_BASE);
      ss_string(STRING(YouHave),MFD_VIEW_MID-18,15);
      ss_string((cur_rs->game_state==GAME_WON ? STRING(Won) : STRING(Lost)),
		   MFD_VIEW_MID-8,22);
   }

   mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
   // autoreexpose
   mfd_notify_func(MFD_GAMES_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, FALSE);
}

bool games_handle_road(MFD *, uiEvent *e)
{
	uiMouseEvent *me=(uiMouseEvent *)e;
	
	if (me->type == UI_EVENT_MOUSE)
	{
		road_state *cur_rs=(road_state *)GAME_DATA;
		int bt=me->buttons;
		if (bt != 0)
			if (me->modifiers != 0)
				bt++;
	
//	if (bt!=cur_rs->last_bs)
//	{
//		cur_rs->last_bs=bt;
// KLC - no need to worry with left/right handed mouse for Mac.
//		if (QUESTVAR_GET(MOUSEHAND_QVAR))
//			bt = (((me->buttons)&2)?1:0)+(((me->buttons)&1)?2:0);
		if (bt==1)
			cur_rs->player_move=-1; 
		else if (bt==2)
			cur_rs->player_move= 1;
	}
	return TRUE;
}

//----------------------------
// mfd Robot Invasion ("bots")
//
// cross between breakout and space invaders

#define BOTS_NUM_ROWS           3
#define BOTS_NUM_COLUMNS        8
#define BOTS_MASK               ((1 << BOTS_NUM_COLUMNS) - 1)

#define BOT_WIDTH               8
#define BOT_HEIGHT              7
//#define BOT_TOP                 10

#define INVADER_TYPES 3

static int invader[INVADER_TYPES] =
{
  REF_IMG_RAsec1bot,
  REF_IMG_RAhopper,
  REF_IMG_RAservbot
};

static void bots_reset_ball(bots_state *bs)
{
  bs->ball_pos_y = MFD_VIEW_HGT - PONG_BORDER - 2*PONG_BALL_YRAD-2*PLY_PADDLE_YRAD;
  bs->ball_pos_x = bs->p_pos;
  bs->ball_dir_x = 0;
  bs->ball_dir_y = -1;
}

static void bots_reset_level(bots_state *bs)
{
  int i;
  for (i=0; i < BOTS_NUM_ROWS; ++i)
    bs->rows[i] = BOTS_MASK;
  bs->hpos = 0;
  bs->hspd = 1;
  bs->vpos = 10;
  bots_reset_ball(bs);
}

void games_init_bots(void *game_state)
{
  bots_state *bs = (bots_state *) game_state;

  bots_reset_level(bs);
  bs->p_pos = MFD_VIEW_WID/2;
  bs->p_spd = 0;
  bs->balls = 3;
}

#define HPOS            (bs->hpos >> 5)
#define BOT_TOP         (bs->vpos)

#define BOT_ON(x,y) ((x) >= 0 && (x) < BOTS_NUM_COLUMNS && (y) >= 0 && (y) < BOTS_NUM_ROWS && (bs->rows[y] & (1 << (x))))
#define CLEAR_BOT(x,y)  (bs->rows[y] &= ~(1 << (x)))

static int test_bot(bots_state *bs, int x, int y)
{
  if (BOT_ON(x,y)) {
    CLEAR_BOT(x,y);
    if (bs->hspd > 0) ++bs->hspd; else --bs->hspd;
    return 1;
  }
  return 0;
}

#define HPOS_HACK       3

void games_run_bots(bots_state *bs)
{
  int rev, guys, i;

  bs->hpos += bs->hspd;
  // test if we've scrolled a bot off the left
  // bot #x is at location hpos + x*BOT_WIDTH;
  // so if say bot #0 is scrolled off, it's because
  // hpos < 0; if bot #1, hpos < -BOT_WIDTH, etc.
  // so if y = hpos/-BOT_WIDTH, that's how many bots
  // we may have scrolled off.

  guys = bs->rows[0] | bs->rows[1] | bs->rows[2];

  if (!guys) {
    bots_reset_level(bs);
    return;
  }

  for (i=2; i >= 0; --i)
    if (bs->rows[i])
      break;
  if (BOT_TOP + BOT_HEIGHT*(i+1) > MFD_VIEW_HGT-PONG_BORDER-2*PLY_PADDLE_YRAD)
    goto loser;

#ifdef USE_BROKEN_CODE
  if (bs->hpos < HPOS_HACK) {
    rev = ((unsigned) (-HPOS-HPOS_HACK))/BOT_WIDTH;
    rev = (1 << rev) - 1;       // test bottommost bits
    if (guys & rev) 
      bs->hpos += 2*(bs->hspd = -bs->hspd);
  }
#else
  if (bs->hpos < 0) {
    for (i=0; i < BOTS_NUM_COLUMNS; ++i)
      if (guys & (1 << i)) break;
    if (i * BOT_WIDTH + HPOS - HPOS_HACK < 0) {
      bs->hpos += 2*(bs->hspd = -bs->hspd);
      ++bs->vpos;
    }
  }
#endif

  if (bs->hspd > 0) {
#ifdef USE_BROKEN_CODE
    // position of the rightmost bot is BOTS_NUM_COLUMNS * BOT_WIDTH + hpos,
    // which scrolls off if > MFD_VIEW_WID
    rev = (MFD_VIEW_WID - HPOS)/BOT_WIDTH - BOTS_NUM_COLUMNS;
    // rev is now the number of bots we'd've shifted off the right
    if (rev > 0) {
      if (rev > BOTS_NUM_COLUMNS)
	bs->hpos += 2*(bs->hspd = -bs->hspd);
      else {
	rev = ~((1 << (BOTS_NUM_COLUMNS - rev))-1);
	if (guys & rev)
	  bs->hpos += 2*(bs->hspd = -bs->hspd);
      }
    }
#else
    // so loop through and see whether any bots are offscreen
    for(i=BOTS_NUM_COLUMNS-1; i >= 0; --i)
      if (guys & (1 << i))
	break;
    // i is the rightmost bot
    if (HPOS + i*BOT_WIDTH + 3 > MFD_VIEW_WID) {
      bs->hpos += 2*(bs->hspd = -bs->hspd);
      ++bs->vpos;
    }
#endif
  }

  if (generic_ball_and_paddle(bs)) {
    if (--bs->balls)
      bots_reset_ball(bs);
    else {
     loser:
      GAME_MODE = GAME_MODE_MENU;
      return;
    }
  } else {
    // handle other bouncy conditions
    // switch from upward to downward
    int bot_left = (bs->ball_pos_x -PONG_BALL_XRAD - HPOS) / BOT_WIDTH;
    int bot_right = (bs->ball_pos_x-PONG_BALL_XRAD - HPOS) / BOT_WIDTH;
    int bot_top = (bs->ball_pos_y -BOT_TOP - PONG_BALL_YRAD) / BOT_HEIGHT;
    int bot_bot = (bs->ball_pos_y -BOT_TOP + PONG_BALL_YRAD) / BOT_HEIGHT;

    if (bs->ball_dir_x == 0) bs->ball_dir_x = 1;
    rev = 0;
    if (bs->ball_dir_y < 0) {
      if (bs->ball_pos_y+PONG_BALL_YRAD<PONG_BORDER)
	rev = 1;
      if (test_bot(bs, bot_left, bot_top) || test_bot(bs, bot_right, bot_top))
	     rev = 1;
    }
    if (bs->ball_dir_y > 0)
      if (test_bot(bs, bot_left, bot_bot) || test_bot(bs, bot_right, bot_bot))
	     rev = 1;

    if (rev)
      bs->ball_pos_y += (bs->ball_dir_y = -bs->ball_dir_y);

    rev = 0;
    if (bs->ball_dir_x < 0) 
      if (test_bot(bs, bot_left, bot_top) || test_bot(bs, bot_left, bot_bot))
	     rev = 1;
    if (bs->ball_dir_x > 0)
      if (test_bot(bs, bot_right, bot_top) || test_bot(bs, bot_right, bot_bot))
	     rev = 1;

    if (rev)
      bs->ball_pos_x += (bs->ball_dir_x = -bs->ball_dir_x);
  }
}

void games_expose_bots(MFD *m, uchar)
{
   bots_state *bs = (bots_state *) GAME_DATA;
   uiEvent fake_event;
   int i,j;
#ifdef SVGA_SUPPORT
   extern char convert_use_mode;
#endif
      
   for(; games_time_diff>=PONG_CYCLE; games_time_diff-=PONG_CYCLE) {
     games_run_bots(bs);
     ui_mouse_get_xy(&fake_event.pos.x,&fake_event.pos.y);
     games_handle_pong(m,&fake_event);
   }

   if (!full_game_3d)
//KLC - chg for new art   draw_res_bm(REF_IMG_bmBlankMFD, 0,0);
	  draw_hires_resource_bm(REF_IMG_bmBlankMFD, 0, 0);

   for (j=0; j < BOTS_NUM_ROWS; ++j) {
     gr_set_fcolor(WHITE);
     for (i=0; i < BOTS_NUM_COLUMNS; ++i) {
       if (bs->rows[j] & (1 << i)) {
#ifdef SVGA_SUPPORT
	 draw_res_bm_core(invader[j]+INVADER_TYPES*convert_use_mode,HPOS + BOT_WIDTH*i,BOT_HEIGHT*j+BOT_TOP - (j==1),FALSE);
#else
	 draw_res_bm(invader[j],HPOS + BOT_WIDTH*i,BOT_HEIGHT*j+BOT_TOP - (j==1));
#endif
       }
     }
   }

   gr_set_fcolor(ORANGE_YELLOW_BASE);
   ss_rect(bs->p_pos-PLY_PADDLE_XRAD,MFD_VIEW_HGT-PONG_BORDER-PLY_PADDLE_YRAD*2,bs->p_pos+PLY_PADDLE_XRAD,MFD_VIEW_HGT-PONG_BORDER);

   gr_set_fcolor(GRAY_8_BASE);
   ss_rect(bs->ball_pos_x-PONG_BALL_XRAD,bs->ball_pos_y-PONG_BALL_YRAD,bs->ball_pos_x+PONG_BALL_XRAD,bs->ball_pos_y+PONG_BALL_YRAD);

   mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
   // autoreexpose
   mfd_notify_func(MFD_GAMES_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, FALSE);

   return;
}
#undef HPOS


// 
//--------------------
// mfd missile command
//
// saved game state is merely the map of which guys are still alive,
// which round number this is, the number of shots remaining, the number
// of enemies remaining in the wave, and the state of the state machine.
// Thus, when you restore, the state machine state forces us into the
// right mode, and then you still have to fight the right enemies; but
// you can cheat a bit by saving right before you might've lost a guy.
// Not much we can do since save/restore events aren't visible to us.


typedef struct
{
  uchar game_mode, state;

  ushort enemies;
  ulong score;

  ushort level;        // 12

  uchar guys;       // 10           // guys & gun placements

  uchar lmissiles;
  uchar rmissiles;   // 10

  uchar quarter;
  uchar bob;
} mcom_state;

// this macro should return a long (well, a 32-bit int) _lvalue_
// (must be assignable) which is actually maintained even 
// when other games are played, i.e. it should be out of quest variables
// and hey, look, it is.

#define HISCORE_QVAR 0x2D

#define HISCORE         (*((ulong*)&player_struct.questvars[HISCORE_QVAR]))


#define MAX_EXPLOSIONS  30
#define MAX_ATTACKERS   24
#define MAX_MISSILES    16

#define EXPLODE_FRAMES  20
#define FIRE_FRAMES     12

// possible states for the state machine;
// we explicitly encode pauses into the numbering scheme
#define MCOM_WAIT_NEW_GAME      1
#define MCOM_WAIT_FOR_LEVEL     (MCOM_WAIT_NEW_GAME+1)
#define LEVEL_WAIT              30
#define MCOM_PLAY_GAME          (MCOM_WAIT_FOR_LEVEL + LEVEL_WAIT)
#define MCOM_REPORT_MISSILES    (MCOM_PLAY_GAME+1)
#define MISSILE_WAIT            16
#define MCOM_REPORT_guys      (MCOM_REPORT_MISSILES+MISSILE_WAIT)
#define guy_WAIT                16
#define MCOM_BONUS_guys       (MCOM_REPORT_guys+guy_WAIT)
#define BONUS_WAIT              8
#define MCOM_RELEVEL            (MCOM_BONUS_guys+BONUS_WAIT)


// scoring
#define SCORE_PER_MISSILE_KILLED        10
#define SCORE_PER_LEFTOVER_SHOT         1
#define SCORE_PER_GUY_ALIVE             100

#define DIEGO_SCORE                     5093

static long shodan_score[7] =
{
   1307496,
   1259431,
   1175035,
   1143910,
   1083477,
   1032148,
   1027869
};


#define EXPLODE         (hideous_secret_game_storage+4)

typedef struct
{
  uchar x;
  uchar y;
  uchar frame;
} expStruct;
expStruct *explode = (expStruct *)EXPLODE;

#define ATTACKER        (EXPLODE + sizeof(expStruct)*MAX_EXPLOSIONS)

typedef struct
{
  uchar sx;
  uchar sy;
  int x;       // 1 bit sign, 8 bit integer, 8 bit fraction
  int y;
  int dx;
  int dy;
} attackStruct;
attackStruct *attack = (attackStruct *)ATTACKER;

#define MISSILE         (ATTACKER + sizeof(attackStruct) * MAX_ATTACKERS)

typedef struct
{
  uchar sx;
  uchar sy;
  uchar ex;
  uchar ey;
  uchar frame;
} shotsStruct;
shotsStruct *shots = (shotsStruct *)MISSILE;

static int num_attackers, num_missiles, num_explosions;
#define GROUND_TOP      (4)
#define guy_TOP        (GROUND_TOP+3)

#define TRAIL_LENGTH    10

static unsigned char radius[] = { 1,2,3,4,5,5,6,6,7,7,7,
				  6,6,6,5,5,4,3,2,1 };

static void make_mcom_explode(int x, int y)
{
  if (num_explosions < MAX_EXPLOSIONS) {
    explode[num_explosions].x = x;
    explode[num_explosions].y = y;
    explode[num_explosions++].frame = 0;
  }
}

static void make_mcom_shot(int x, int y, int sx)
{
  if (num_missiles < MAX_MISSILES) {
    shots[num_missiles].sx = sx;
    shots[num_missiles].sy = GROUND_TOP+9;
    shots[num_missiles].ex = x;
    shots[num_missiles].ey = y;
    shots[num_missiles++].frame = 0;
    play_digi_fx(SFX_GUN_RAILGUN,1);
  }
}


  // if you make this a #define, make sure you cast it to
  // an int, otherwise it's unsigned and hoses the dx calculation

  // compute current vertical speed based on level
static int v_speed(void) 
{
    return 24 + ((mcom_state *) GAME_DATA)->level*4;
}

static void make_attacker(int sx, int sy, int dx)
{
  if (num_attackers < MAX_ATTACKERS && ((mcom_state *) GAME_DATA)->enemies) {
    attack[num_attackers].sx = sx;
    attack[num_attackers].sy = sy;
    attack[num_attackers].x = sx*256;
    attack[num_attackers].y = sy*256;
    attack[num_attackers].dy = -v_speed();
    attack[num_attackers++].dx = (dx-sx)*v_speed()/(sy-guy_TOP);
    ((mcom_state *) GAME_DATA)->enemies--;
  }
}

static int guy_loc(int n)
{
  // 0 1 2 3 4 5 6 7 8 9
  // |     ^     ^     |
  // edge  silo  silo  edge
      
  // so first map guy number 0..5 into above numbering scheme
  int k = (n/2)*3 + (n&1) + 1;

  // then map 0 to 0 and 9 to MFD_VIEW_WID
  return MFD_VIEW_WID*k/9;
}

static void make_random_attacker(void)
{
  // attacker comes from anywhere on top, targetting any guy
  make_attacker(rand() % MFD_VIEW_WID, MFD_VIEW_HGT-1, MFD_VIEW_WID*((rand()%8)+1)/9);
}

#define SQRD(x) ((x)*(x))

#define BOAT_DEATH_NOISE SFX_SPARKING_CABLE
#ifdef DEMO
#define SWIMMER_DEATH_NOISE SFX_SPARKING_CABLE
#else
#define SWIMMER_DEATH_NOISE SFX_DEATH_9
#endif

static void advance_mcom_state(void)
{
  int i,j;
  bool silo=FALSE;
  mcom_state *ms = (mcom_state *) GAME_DATA;
  uchar old_quarter=ms->quarter;
  uchar old_bob=ms->bob;
  
    // this code used to be in the expose func, hopefully this doesn't
    // break it
  switch(ms->state) {
    case MCOM_WAIT_NEW_GAME:
    case MCOM_PLAY_GAME:
      break;
    default:
      switch(++ms->state) {
	case MCOM_PLAY_GAME: mcom_start_level(); break;
	case MCOM_RELEVEL: ms->state = MCOM_WAIT_FOR_LEVEL;
      }
  }

  // bob the swimmers
  ms->quarter=(player_struct.game_time/(CIT_CYCLE/4))&3;
  if(ms->quarter!=old_quarter) {
     // bob swimmers down
     ms->bob |= rand()&((1<<6)-1);
     // bob old swimmers up
     ms->bob &= ~old_bob;
  }

    // advance explosion animations
  for (i=0; i < num_explosions;)
    if (++(explode[i].frame) == EXPLODE_FRAMES)
      explode[i] = explode[--num_explosions];
    else
      ++i;

    // advance defense missiles steps... conveniently, all simply use
    // a frame counter.  if missile reaches end frame, blow it up
  for (i=0; i < num_missiles;)
    if (++(shots[i].frame) >= FIRE_FRAMES) {
      make_mcom_explode(shots[i].ex, shots[i].ey);
      shots[i] = shots[--num_missiles];
    } else
      ++i;

    // advance an attacking missile.  if the missile is within range
    // of an explosion, kaboom!  otherwise, if it's down at ground level,
    // kaboom, and blow something up too!
    // otherwise just advance it
  for (i=0; i < num_attackers; i) {
    // check if we've gotten blown up by an explosion
    for (j=0; j < num_explosions; ++j)
      if (SQRD((int) explode[j].x - (int) attack[i].x/256) + SQRD((int) explode[j].y - (int) attack[i].y/256) <= SQRD(radius[explode[j].frame]))
	break;
    if (j < num_explosions) {
      ms->score += SCORE_PER_MISSILE_KILLED;
     blowup:
      make_mcom_explode(attack[i].x/256, attack[i].y/256);
      attack[i] = attack[--num_attackers];
    } else {
      attack[i].x += attack[i].dx;
      attack[i].y += attack[i].dy;
      if (attack[i].y <= guy_TOP*256) {
	// blow up the guy that's here!
	j = (attack[i].x/256);
	j = (9*j + MFD_VIEW_WID/2)/MFD_VIEW_WID-1;
	if (j == 2) { ms->lmissiles = 0; silo=TRUE; }
	if (j == 5) { ms->rmissiles = 0; silo=TRUE; }
	if(ms->guys & (1<<j))
	  play_digi_fx(silo?BOAT_DEATH_NOISE:SWIMMER_DEATH_NOISE,1);
	else
	  play_digi_fx(SFX_SPARKING_CABLE,1);
	ms->guys &= ~(1 << j);
	goto blowup;
      } else
	++i;
    }
  }

}


#define ALL_guys              0xff      // binary 11111111
#define LGUN_FLAG               0x04      // binary 00000100
#define RGUN_FLAG               0x20      // binary 00100000
#define JUST_guys             (ALL_guys - LGUN_FLAG - RGUN_FLAG)

static void games_init_mcom(void *game_state)
{
  mcom_state *ms = (mcom_state *) game_state;

  ms->state = MCOM_WAIT_NEW_GAME;
  ms->guys = ALL_guys;
  num_attackers = num_missiles = num_explosions = 0;
  games_time_diff=0;
}

static void mcom_start_game(void)
{
  mcom_state *ms = (mcom_state *) GAME_DATA;
  ms->guys = ALL_guys;
  ms->state = MCOM_WAIT_FOR_LEVEL;
  ms->lmissiles = ms->rmissiles = 30;
  ms->level = 0;
  num_attackers = num_missiles = num_explosions = 0;
}

static void mcom_start_level(void)
{
  int i;
  mcom_state *ms = (mcom_state *) GAME_DATA;
  ++ms->level;
  ms->enemies = 4*(ms->level)/3 + 15;
  ms->guys |= (LGUN_FLAG | RGUN_FLAG);
  ms->lmissiles = ms->rmissiles = 30;
  for (i=0; i < (4 + (ms->level >> 3)); ++i)
    make_random_attacker();    
}

// hey, hey, we got some user input
static bool games_handle_mcom(MFD *m, uiEvent* e)
{
	mcom_state *ms = (mcom_state *)GAME_DATA;
	uiMouseEvent *mouse = (uiMouseEvent *) e;
	LGPoint pos = MakePoint(e->pos.x - m->rect.ul.x, e->pos.y - m->rect.ul.y);
	
	if (ms->state == MCOM_WAIT_NEW_GAME)
	{
		if (mouse->action & MOUSE_LDOWN)
			mcom_start_game();
	}
	else if (ms->state < MCOM_PLAY_GAME ||
			  (ms->state == MCOM_PLAY_GAME && (ms->enemies || num_attackers)))
	{
		int	left = (mouse->action & MOUSE_LDOWN) && (mouse->modifiers == 0);
		int	right = (mouse->action & MOUSE_LDOWN) && (mouse->modifiers != 0);
// KLC - no need to worry about left/right hand mouse for Mac version.
//		if (QUESTVAR_GET(MOUSEHAND_QVAR))
//		{
//			int temp = left;
//			left = right;
//			right = temp;
//		}
		if (left && ms->lmissiles)
			make_mcom_shot(pos.x, pos.y, MFD_VIEW_WID/3), --ms->lmissiles;
		if (right && ms->rmissiles)
			make_mcom_shot(pos.x, pos.y, MFD_VIEW_WID*2/3), --ms->rmissiles;
	}
	return TRUE;
}


// coordinates of the missiles in the boats
static signed char ox[10] = { -3,-1,1,3,-2,0,2,-1,1,0 };
static signed char oy[10] = { 1,1,1,1,3,3,3,5,5,7 };

static void draw_silo(int x, int num)
{
  int i;
    // bottom row of silo must have room for 4 missiles, so 9 pixels wide

  draw_res_bm(REF_IMG_Destroyer,x-4,GROUND_TOP-2);

    // now plot the missiles waiting to fire

  if (!num) return;
  gr_set_fcolor(ORANGE_8_BASE+1);
  num = num % 10;
  if (num == 0) num = 10;
  for (i=0; i < num; ++i)
      ss_rect(x+ox[i],GROUND_TOP+oy[i],x+ox[i]+1,GROUND_TOP+oy[i]+1);
}

// static unsigned char guy_disp[] = { 2,6,3,2,5 };

//#define gr_int_cline(x0,y0,c0,x1,y1,c1) \
//        gr_fix_cline(fix_make(x0,0),fix_make(y0,0),c0,\
//                     fix_make(x1,0),fix_make(y1,0),c1)
//           // convert 8-bit rgb values into grs_rgb
//#define make_rgb(r,g,b) (((b) << 24) | ((g) << 13) | ((r) << 2))

#define gr_int_cline(x0,y0,c0,x1,y1,c1) ss_int_line(x0,y0,x1,y1)

// update ten times per second.
#define MCOM_CYCLE (CIT_CYCLE/35)

static int hack[] = { 0, SCORE_PER_GUY_ALIVE, 
		      SCORE_PER_GUY_ALIVE, SCORE_PER_GUY_ALIVE*2 };
static void games_expose_mcom(MFD *, ubyte )
{
  int i,k;
  mcom_state *ms = (mcom_state *) GAME_DATA;

  if (!ms->state) { ms->state = MCOM_WAIT_NEW_GAME; ms->guys = ALL_guys; }


    // water
  gr_set_fcolor(BLUE_8_BASE+6);
  ss_rect(0,GROUND_TOP+1,MFD_VIEW_WID,MFD_VIEW_HGT);

    // sky
  gr_set_fcolor(AQUA_8_BASE+3);
  ss_rect(0,0,MFD_VIEW_WID,GROUND_TOP+1);

    // print current score behind floating guys
  {
    char buffer[16];
    sprintf(buffer,"%06d",ms->score);
    gr_set_fcolor(WHITE);
    ss_string(buffer, MFD_VIEW_WID-5*6+4, 0);
  }

  if (ms->guys & LGUN_FLAG)
    draw_silo(MFD_VIEW_WID/3, ms->lmissiles);
  if (ms->guys & RGUN_FLAG)
    draw_silo(MFD_VIEW_WID*2/3, ms->rmissiles);

    // floating guys
  for (i=0; i < 6; ++i)
    if ((1 << "\000\001\003\004\006\007"[i]) & ms->guys) {
      k = guy_loc(i);
      // now k is the center location of the guy, so now draw the guy
      draw_res_bm(REF_IMG_LittleGuy,k-2,GROUND_TOP-1+(((1<<i)&ms->bob)!=0));
    }

  // we've drawn all the background, now draw the foreground stuff
    
    // draw foreground information behind everything,
    // just because it looks cool in Llamatron
    // but note we draw it in front of non-moving stuff

  gr_set_fcolor(AQUA_8_BASE+3);
  if (ms->state == MCOM_WAIT_NEW_GAME) {
    int i;
    for (i=0; i < 8; ++i) {
      char buffer[16];
      if (HISCORE > DIEGO_SCORE) {
	strncpy(buffer, player_struct.name, 8); 
		// limited space in hiscore display, so strncpy
	strtoupper(buffer);
      }

      ss_string(i < 7 ? STRING(ShodanHiScore) : 
			HISCORE <= DIEGO_SCORE ? STRING(DiegoHiScore) : buffer,
		4, i*5 + 9);
	// Note that Shodan has scored 1 digit more than the authors of 
	// Eel Zapper were expecting, so other people have a leading blank
	// of course it's totally unrealistic unless that this would work
	// out right unless their score-painting code printed from the
	// right, but that's not unreasonable since conversion to decimal
	// starts from the right.
      sprintf(buffer, i < 7 ? "%07d" : "  %06d", i < 7 ? shodan_score[i] :
		HISCORE < DIEGO_SCORE ? DIEGO_SCORE : HISCORE);
      ss_string(buffer, MFD_VIEW_WID-30, i*5 + 9);
    }
    ss_string(STRING(ClickToPlay), MFD_VIEW_MID-25, MFD_VIEW_HGT-7);
  } else if (ms->state < MCOM_PLAY_GAME) {
    char buffer[16];
    sprintf(buffer, STRING(LevelNum), ms->level+1);
    ss_string(buffer, MFD_VIEW_MID-15, MFD_VIEW_HGT/2);
  } else if (ms->state == MCOM_PLAY_GAME) {
  } else {
	// ideally this will countup how many you got
    char buffer[32];
    int z;

    z = ms->state - MCOM_REPORT_MISSILES;
    if (z > MISSILE_WAIT) z = MISSILE_WAIT;
    ss_string(STRING(DepthChargeBonus), 2, MFD_VIEW_HGT/2-8);
    sprintf(buffer, "%d", (ms->lmissiles + ms->rmissiles) * SCORE_PER_LEFTOVER_SHOT * z / MISSILE_WAIT);
    ss_string(buffer, MFD_VIEW_MID-5, MFD_VIEW_HGT/2-3);
    z = ms->state - MCOM_REPORT_guys;
    if (z >= 0) {
      if (z > guy_WAIT) z = guy_WAIT; 
      ss_string(STRING(GuyBonus), 12, MFD_VIEW_HGT/2+8);
      sprintf(buffer, "%d", (hack[ms->guys & 3] + hack[(ms->guys >> 3) &3] + hack[(ms->guys >> 6) & 3])*z/guy_WAIT);
      ss_string(buffer, MFD_VIEW_MID-10, MFD_VIEW_HGT/2+13);
    }
  }

    // draw the incoming missiles

    // ugly hack so the following code is readable
#define A       attack[i]

  for (i=0; i < num_attackers; ++i) {
    gr_set_fcolor(GREEN_8_BASE+3);
    if (attack[i].y > (MFD_VIEW_HGT-TRAIL_LENGTH << 8))
      ss_fix_line(A.x << 8, A.y << 8, A.sx << 16, A.sy << 16);
    else
      ss_fix_line(A.x << 8, A.y << 8,
	  (A.x + (A.x - (A.sx << 8))*TRAIL_LENGTH*256/(A.y - (A.sy << 8))) << 8,
	  (A.y + (TRAIL_LENGTH<<8)) << 8);
#undef A

    gr_set_fcolor(GRAY_8_BASE);
    // an incredibly stupid way to plot a pixel!  fix me
    ss_hline(attack[i].x/256,attack[i].y/256,attack[i].x/256);
  }

    // draw the shots

  gr_set_fcolor(GREEN_BASE+2);
  for (i=0; i < num_missiles; ++i)
   draw_res_bm(REF_IMG_DepthCharge,
	       shots[i].sx+(shots[i].ex-shots[i].sx)*shots[i].frame/FIRE_FRAMES,
	       shots[i].sy+(shots[i].ey-shots[i].sy)*shots[i].frame/FIRE_FRAMES);

    // draw all the explosions

  for (i=0; i < num_explosions; ++i) {
    gr_set_fcolor(GRAY_8_BASE+8-radius[explode[i].frame]);
    ss_int_disk(explode[i].x, explode[i].y, radius[explode[i].frame]<<1);
  }



  // advance the state of all the objects

  for(; games_time_diff>=MCOM_CYCLE; games_time_diff-=MCOM_CYCLE) {
    advance_mcom_state();
    if (ms->state == MCOM_PLAY_GAME) {
      if ((rand() % 1000) < (ms->level+20))
	make_random_attacker();
      if (!(ms->guys & JUST_guys)) {
	ms->state = MCOM_WAIT_NEW_GAME;
	if (ms->score > HISCORE) HISCORE = ms->score;
      }
    }
  }

  // advance the game state if they're done
  if (ms->state == MCOM_PLAY_GAME && !ms->enemies && !num_explosions && !num_attackers && !num_missiles) {
    // bonuses
    ms->score += (ms->lmissiles + ms->rmissiles) * SCORE_PER_LEFTOVER_SHOT;
    ms->score += hack[ms->guys & 3] + hack[(ms->guys >> 3) &3] + hack[(ms->guys >> 6) & 3];
    ++ms->state;
  }
  
  mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
  // autoreexpose
  mfd_notify_func(MFD_GAMES_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, FALSE);
}


#ifdef LOST_TREASURES_OF_MFD_GAMES
//----------------------------
//----------------------------
// mfd 15-sliding-tile puzzle
//----------------------------
//----------------------------
#define MFD_PUZZLE_SIZE 4
#define MFD_PUZZLE_SQ (MFD_PUZZLE_SIZE*MFD_PUZZLE_SIZE)
#define PUZZ15_TILE_SIZE (p15_styles[((puzzle15_state *)GAME_DATA)->style].tsize)
#define PUZZ15_ULX ((MFD_VIEW_WID-(MFD_PUZZLE_SIZE*PUZZ15_TILE_SIZE))/2)
#define PUZZ15_ULY ((MFD_VIEW_HGT-(MFD_PUZZLE_SIZE*PUZZ15_TILE_SIZE))/2)
#define PUZZ15_CYCLE ((CIT_CYCLE/2)/PUZZ15_TILE_SIZE)
#define PUZZ15_INIT_SCRAM (4*PUZZLE_DIFFICULTY+5)
#define PUZZ15_WID (MFD_PUZZLE_SIZE*PUZZ15_TILE_SIZE)
#define PUZZ15_HGT (MFD_PUZZLE_SIZE*PUZZ15_TILE_SIZE)
#define PUZZ15_AFRAME_CYCLE (CIT_CYCLE)
#define NUM_PUZZ15_STYLES 5

typedef struct {
   uchar game_mode;
   uchar tilenum[MFD_PUZZLE_SQ];
   uchar current_frame;
   uchar anim_source;
   uchar anim_dir;
   uchar style;
   uchar scramble;
   uchar movedto;
   uchar animframe;
   bool  pause;
} puzzle15_state;

typedef struct {
   uchar bcolor;
   uchar fcolor;
   Ref back;
   bool numbers;
   uchar tsize;
   bool animating;
} puzz15_style;

static puzz15_style p15_styles[NUM_PUZZ15_STYLES] = {
   { 0x60, 0xB0 ,REF_IMG_EmailMugShotBase+11, FALSE, 13, FALSE },
   { 0x60, 0xB0 ,REF_IMG_EmailMugShotBase+23, FALSE, 13, FALSE },
   { 0xDE, 0x02 ,REF_IMG_TriopLogo15, FALSE, 8, FALSE },
   { 0xDE, 0x02 ,REF_IMG_DiegoAnim15, FALSE, 8, TRUE },
   { 0x4C, 0x01 , 0 , TRUE, 13, FALSE},
};

void games_init_15(void* game_state)
{
   puzzle15_state* state=(puzzle15_state*)game_state;
   int i;

   for(i=0;i<MFD_PUZZLE_SQ;i++)
      state->tilenum[i]=i+1;
   state->tilenum[MFD_PUZZLE_SQ-1]=0;
   state->anim_source=MFD_PUZZLE_SQ;             
   state->style=rand()%NUM_PUZZ15_STYLES;
   state->scramble=PUZZ15_INIT_SCRAM;
   games_time_diff=0;
}

static bool puzz15_won()
{
   puzzle15_state *st=(puzzle15_state *)GAME_DATA;
   int i;

   for(i=0;i<MFD_PUZZLE_SQ-1;i++) {
      if(st->tilenum[i]!=(i+1))
	 return(FALSE);
   }
   return(TRUE);
}

static void puzz15_xy(int ind, int* x, int* y)
{
   int r,c;
   r=ind/MFD_PUZZLE_SIZE;
   c=ind%MFD_PUZZLE_SIZE;
   *x=PUZZ15_ULX+(c*PUZZ15_TILE_SIZE);
   *y=PUZZ15_ULY+(r*PUZZ15_TILE_SIZE);
}

static bool puzz15_move(int x, int y)
{
   puzzle15_state *st=(puzzle15_state *)GAME_DATA;
   int dir=-1, ind;

   ind=x+y*MFD_PUZZLE_SIZE;

   if(x>0 && st->tilenum[ind-1]==0) dir=3;
   else if(y>0 && st->tilenum[ind-MFD_PUZZLE_SIZE]==0) dir=0;
   else if(x<MFD_PUZZLE_SIZE-1 && st->tilenum[ind+1]==0) dir=1;
   else if(y<MFD_PUZZLE_SIZE-1 && st->tilenum[ind+MFD_PUZZLE_SIZE]==0) dir=2;

   if(dir==-1) return FALSE;

   st->anim_source=y*MFD_PUZZLE_SIZE+x;
   st->current_frame=0;
   st->anim_dir=dir;

   return TRUE;
}

void games_expose_15(MFD *, ubyte control)
{
   int i,x,y,t,dx,dy,dt;
   short sw,sh;
   bool rex=FALSE;
   puzzle15_state* st=(puzzle15_state*)GAME_DATA;
   char buf[3];
   int cycle=PUZZ15_CYCLE, aframe;
   Ref back;
   bool full, solv, nums=p15_styles[st->style].numbers;

   full=(control&MFD_EXPOSE_FULL);

   if(st->scramble>0) {
      cycle/=3;
      if(st->anim_source==MFD_PUZZLE_SQ) {
	 do {
	    x=rand()%MFD_PUZZLE_SIZE;
	    y=rand()%MFD_PUZZLE_SIZE;
	    if((x+y*MFD_PUZZLE_SIZE)!=st->movedto)
	       rex=puzz15_move(x,y);
	 } while(!rex);
	 st->scramble--;
      }
   }

   if(p15_styles[st->style].animating) {
      rex=TRUE;
      aframe=(player_struct.game_time/PUZZ15_AFRAME_CYCLE)%4;
      if(aframe!=st->animframe) {
	 st->animframe=aframe;
	 full=TRUE;
      }
   }      

   back=p15_styles[st->style].back;
   if(back) back+=st->animframe;
   solv=st->pause && back;
   if(full)
   {
      if (!full_game_3d)
//KLC - chg for new art   draw_res_bm(REF_IMG_bmBlankMFD, 0,0);
	 	draw_hires_resource_bm(REF_IMG_bmBlankMFD, 0, 0);
      gr_set_fcolor(0xBF);
      ss_rect(PUZZ15_ULX-2,PUZZ15_ULY-2,PUZZ15_ULX+PUZZ15_WID+2,PUZZ15_ULY+PUZZ15_HGT+2);
      mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
   }
   for(i=0;i<MFD_PUZZLE_SQ;i++)
   {
      if(st->tilenum[i]==0 && !(solv))
      {
	 	puzz15_xy(i,&x,&y);
	 	gr_set_fcolor(0x1);
	 	ss_rect(x,y,x+PUZZ15_TILE_SIZE,y+PUZZ15_TILE_SIZE);
		mfd_add_rect(x,y,x+PUZZ15_TILE_SIZE,y+PUZZ15_TILE_SIZE);
      }
   }
   for(i=0;i<MFD_PUZZLE_SQ;i++) {
      // draw background
      puzz15_xy(i,&x,&y);
      dx=dy=0;
      t=st->tilenum[i];
      if(t!=0||(solv)) {
	 if(i==st->anim_source) {
	    switch(st->anim_dir) {
	       case 0: dy=st->current_frame; break;
	       case 1: dx=-st->current_frame; break;
	       case 2: dy=-st->current_frame; break;
	       case 3: dx=st->current_frame; break;
	    }
	    gr_set_fcolor(0x1);
	    ss_rect(x,y,x+PUZZ15_TILE_SIZE,y+PUZZ15_TILE_SIZE);
	    x-=dx;
	    y-=dy;
	 }
	 if(dx||dy||full) {
	    // draw background
	    gr_set_fcolor(0x1);
	    ss_rect(x+dx,y+dy,x+dx+PUZZ15_TILE_SIZE,y+dy+PUZZ15_TILE_SIZE);
	    mfd_add_rect(x+dx,y+dy,x+dx+PUZZ15_TILE_SIZE,y+dy+PUZZ15_TILE_SIZE);
	    gr_set_fcolor(p15_styles[st->style].bcolor+((i+(i/MFD_PUZZLE_SIZE))&1));
	    ss_rect(x,y,x+PUZZ15_TILE_SIZE,y+PUZZ15_TILE_SIZE);
	    // draw pretty bitmap
	    if(back) {
	       int bx,by,bw,bh;
	       ss_safe_set_cliprect(x,y,x+PUZZ15_TILE_SIZE,y+PUZZ15_TILE_SIZE);
	       puzz15_xy((t==0?MFD_PUZZLE_SQ:t)-1,&bx,&by);
	       bw=res_bm_width(back);
	       bh=res_bm_height(back);
	       draw_res_bm(back,PUZZ15_ULX+x-bx+(PUZZ15_WID-bw)/2,PUZZ15_ULY+y-by+(PUZZ15_HGT-bh)/2);
	       ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
	    }
	    // draw number
	    if(nums) {
	       gr_set_fcolor(p15_styles[st->style].fcolor);
	       numtostring(t, buf);
	       gr_string_size(buf,&sw,&sh);
	       sw--;  // assume blank pixel of kerning.
	       sh--;  // assume pixel descender.
	       ss_string(buf,x+(PUZZ15_TILE_SIZE-sw)/2,y+(PUZZ15_TILE_SIZE-sh)/2);
	    }
	    mfd_add_rect(x,y,x+PUZZ15_TILE_SIZE,y+PUZZ15_TILE_SIZE);
	 }
      }
   }

   rex=rex||(st->anim_source<MFD_PUZZLE_SQ);
   for(; games_time_diff>=cycle; games_time_diff-=cycle) {
      if(st->anim_source<MFD_PUZZLE_SQ) {
	 if(st->current_frame==PUZZ15_TILE_SIZE) {
	    st->current_frame=0;
	    dt=(st->anim_dir&1)?1:-MFD_PUZZLE_SIZE;
	    if(st->anim_dir>1) dt=-dt;
	    st->movedto=st->anim_source+dt;
	    st->tilenum[st->movedto]=st->tilenum[st->anim_source];
	    st->tilenum[st->anim_source]=0;
	    st->anim_source=MFD_PUZZLE_SQ;
	    mfd_notify_func(MFD_GAMES_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, TRUE);
	    if(puzz15_won()) {
	       st->pause=TRUE;
	       return;
	    }
	 }
	 else
	    st->current_frame++;
      }
   }

   // autoreexpose
   if(rex)
      mfd_notify_func(MFD_GAMES_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, FALSE);
}

bool games_handle_15(MFD *m, uiEvent *e)
{
   uiMouseEvent *me=(uiMouseEvent *)e;
   puzzle15_state *st=(puzzle15_state *)GAME_DATA;
   LGPoint pos = MakePoint(e->pos.x-m->rect.ul.x-PUZZ15_ULX,
			 e->pos.y-m->rect.ul.y-PUZZ15_ULY);

   if(st->scramble>0) return FALSE;

   if(!(me->action & MOUSE_LDOWN)) return FALSE;

   if(pos.x<0||pos.y<0) return TRUE;

   pos.x/=PUZZ15_TILE_SIZE;
   pos.y/=PUZZ15_TILE_SIZE;

   if(pos.x>=MFD_PUZZLE_SIZE || pos.y>=MFD_PUZZLE_SIZE)
      return TRUE;
   if(st->pause) {
      st->pause=FALSE;
      st->scramble=PUZZ15_INIT_SCRAM;
      mfd_notify_func(MFD_GAMES_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, FALSE);
      return TRUE;
   }
   if(st->anim_source<MFD_PUZZLE_SQ)
      return TRUE;

   if(puzz15_move(pos.x,pos.y))
      mfd_notify_func(MFD_GAMES_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, FALSE);

   return TRUE;
}

//----------------------------
//----------------------------
// mfd Tic-Tac-Toe
//----------------------------
//----------------------------

typedef struct {
   uchar owner[9];
} tictactoe;

typedef struct {
   uchar game_mode;
   tictactoe board;
   uchar whomoves;
   uchar whoplayer;
} ttt_state;

#define NOBODY 0
#define X 1
#define O 2
#define OTHERPLAYER(w) (X+O-(w))

#define TTT_SQ_WID 20
#define TTT_SQ_HGT 16

#define TTT_ULX ((MFD_VIEW_WID-(3*TTT_SQ_WID))/2)
// this leaves a little room at the top for text, which
// we secretly declare is 6 pixels
#define TTT_MESS_HGT 6
#define TTT_ULY (TTT_MESS_HGT+(MFD_VIEW_HGT-TTT_MESS_HGT-(3*TTT_SQ_HGT))/2)
#define TTT_PUZ_WID (3*TTT_SQ_WID)
#define TTT_PUZ_HGT (3*TTT_SQ_HGT)
#define TTT_LRX (TTT_ULX+TTT_PUZ_WID)
#define TTT_LRY (TTT_ULY+TTT_PUZ_HGT)

static void tictactoe_drawwin(ttt_state* st);
bool tictactoe_generator(void* pos, int index, bool minimizer_moves);

// ----------------------
// TIC-TAC-TOE:
//   static evaluator, move generator
// ----------------------

static int winnerval(uchar owner)
{
   if(owner==X) return INT_MAX;
   else if(owner==O) return INT_MIN;
   else return 0;
}

static bool tictactoe_over(tictactoe* st)
{
   int i,val;

   val=tictactoe_evaluator(st);
   if(val==winnerval(X)||val==winnerval(O)) return TRUE;

   for(i=0;i<9;i++) {
      if(st->owner[i]==NOBODY) return FALSE;
   }
   return TRUE;
}

static char corners_ttt[]={0,2,6,8};

void games_init_ttt(void* game_state)
{
   ttt_state* state=(ttt_state*)game_state;

   state->whomoves=X;
   state->whoplayer=(rand()&1)?X:O;

   fstack_init(hideous_secret_game_storage+sizeof(ttt_state),
      sizeof(hideous_secret_game_storage)-sizeof(ttt_state));
   if(state->whoplayer!=state->whomoves) {
      // fake straight to a corner move
      state->board.owner[corners_ttt[rand()&3]]=state->whomoves;
      state->whomoves=state->whoplayer;      
   }
}

static char initmove_ttt[]={0,1,4};

static int ttt_fullness(tictactoe* st)
{
   int i, ret=0;

   for(i=0;i<9;i++) {
      if(st->owner[i]!=NOBODY)
	 ret++;
   }
   return ret;
}

static char move_to_index(char move,tictactoe* st)
{
   int i;
   bool empty;

   empty=(ttt_fullness(st)==0);
   if(empty) return initmove_ttt[move];

   for(i=0;i<9 && move>=0;i++) {
      if(st->owner[i]==NOBODY) {
	 if(move==0)
	    return i;
	 move--;
      }
   }
   return -1;
}

void games_expose_ttt(MFD *, ubyte control)
{
   bool full;
   int val;
   static long timeformove=0, dt, timeout;
   char whichmove;
   ttt_state* st=(ttt_state*)GAME_DATA;
   int loops=0;

   full=(control&MFD_EXPOSE_FULL);

   if(full) {
      int x,y;
      bool over;
      uchar owner;
      Ref bm;

      if (!full_game_3d)
//KLC - chg for new art   draw_res_bm(REF_IMG_bmBlankMFD, 0,0);
	 	draw_hires_resource_bm(REF_IMG_bmBlankMFD, 0, 0);
      gr_set_fcolor(RED_8_BASE+4);
      over=tictactoe_over(&(st->board));
      if(!over) {
	 // note that we are shamelessly using "bm" to temporarily 
	 // house a string.  Sue me.
	 if(st->whomoves==st->whoplayer)
	    bm=REF_STR_YourMove;
	 else
	    bm=REF_STR_Thinking;
	 draw_shadowed_text(get_temp_string(bm),(MFD_VIEW_WID-gr_string_width(get_temp_string(bm)))/2,1);
      }
      // Hmm, this used to be gr_uvline.... maybe insufficent to just make it clipped.
      ss_vline(TTT_ULX+TTT_SQ_WID,TTT_ULY,TTT_ULY+TTT_PUZ_HGT);
      ss_vline(TTT_ULX+2*TTT_SQ_WID,TTT_ULY,TTT_ULY+TTT_PUZ_HGT);
      ss_int_line(TTT_ULX,TTT_ULY+TTT_SQ_HGT,TTT_ULX+TTT_PUZ_WID,TTT_ULY+TTT_SQ_HGT);
      ss_int_line(TTT_ULX,TTT_ULY+2*TTT_SQ_HGT,TTT_ULX+TTT_PUZ_WID,TTT_ULY+2*TTT_SQ_HGT);
      for(y=0;y<3;y++) {
	 for(x=0;x<3;x++) {
	    owner=st->board.owner[x+3*y];
	    if(owner==NOBODY)
	       bm=ID_NULL;
	    else if(owner==st->whoplayer)
	       bm=REF_IMG_ttt_Player;
	    else
	       bm=REF_IMG_ttt_Shodan;
	    if(bm!=ID_NULL)
	       draw_res_bm(bm,1+TTT_ULX+TTT_SQ_WID*x,1+TTT_ULY+TTT_SQ_HGT*y);
	 }
      }
      if(over) {
	 gr_set_fcolor(BLUE_8_BASE+2);
	 tictactoe_drawwin(st);
      }
      mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
   }
   if(st->whomoves==st->whoplayer) return;
   if(tictactoe_over(&(st->board))) return;

   // give us enough time to still achieve 15 fps
   dt=(CIT_CYCLE/15)-player_struct.deltat;
   // or give us at least 1/55 second per frame.
   if(dt<(CIT_CYCLE/55)) dt=CIT_CYCLE/55;

   timeformove+=dt;

   if(timeformove<=0)
      return;

   timeout=*tmd_ticks+timeformove;

   while(*tmd_ticks<timeout) {
      minimax_step();
      loops++;
      if(minimax_done())
	 timeout=*tmd_ticks;
   }
   timeformove=timeout-*tmd_ticks;

   if(minimax_done()) {
      minimax_get_result(&val,&whichmove);
      st->board.owner[move_to_index(whichmove,&(st->board))]=OTHERPLAYER(st->whoplayer);
      st->whomoves=st->whoplayer;
      mfd_notify_func(MFD_GAMES_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, TRUE);
   }
   else
      mfd_notify_func(MFD_GAMES_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, FALSE);
}

int tictactoe_evaluator(void* pos)
{
   tictactoe* t = (tictactoe *)pos;
   uchar win;

   win=t->owner[0];
   if(win!=NOBODY) {
      if(t->owner[1]==win && t->owner[2]==win)
	 return winnerval(win);
      if(t->owner[3]==win && t->owner[6]==win)
	 return winnerval(win);
   }

   win=t->owner[8];
   if(win!=NOBODY) {
      if(t->owner[6]==win && t->owner[7]==win)
	 return winnerval(win);
      if(t->owner[2]==win && t->owner[5]==win)
	 return winnerval(win);
   }

   win=t->owner[4];
   if(win!=NOBODY) {
      if(t->owner[3]==win && t->owner[5]==win)
	 return winnerval(win);
      if(t->owner[1]==win && t->owner[7]==win)
	 return winnerval(win);
      if(t->owner[0]==win && t->owner[8]==win)
	 return winnerval(win);
      if(t->owner[6]==win && t->owner[2]==win)
	 return winnerval(win);
   }

   return winnerval(NOBODY);
}

// note that this procedure duplicates a lot of the work done by
// tictactoe_evaluator: we do not consolidate them because we don't
// want to slow down the evaluator.
void tictactoe_drawwin(ttt_state* st)
{
   uchar win, realwin;
   int i;
   LGPoint p1,p2;
   char buf[80];
   tictactoe* t=&(st->board);

   p1.x=-1;

   for(i=0;i<3;i++) {
      win=t->owner[i];
      if(t->owner[i+3]==win && t->owner[i+6]==win) {
	 realwin=win;
	 p1.x=TTT_ULX+(TTT_SQ_WID*i)+(TTT_SQ_WID/2); p1.y=TTT_ULY;
	 p2.x=p1.x; p2.y=TTT_LRY;
      }         
   }
   for(i=0;i<9;i+=3) {
      win=t->owner[i];
      if(t->owner[i+1]==win && t->owner[i+2]==win) {
	 realwin=win;
	 p1.x=TTT_ULX; p1.y=TTT_ULY+(TTT_SQ_HGT*i/3)+(TTT_SQ_HGT/2);
	 p2.x=TTT_LRX; p2.y=p1.y;
      }
   }
   win=t->owner[0];
   if(t->owner[4]==win && t->owner[8]==win) {
      realwin=win;
      p1.x=TTT_ULX; p1.y=TTT_ULY;
      p2.x=TTT_LRX; p2.y=TTT_LRY;
   }
   win=t->owner[6];
   if(t->owner[4]==win && t->owner[2]==win) {
      realwin=win;
      p1.x=TTT_ULX; p1.y=TTT_LRY;
      p2.x=TTT_LRX; p2.y=TTT_ULY;
   }
   if(p1.x>0) {
      ss_int_line(p1.x,p1.y,p2.x,p2.y);
      sprintf(buf,"%S%S",realwin==st->whoplayer?REF_STR_YouHave:REF_STR_ComputerHas,REF_STR_Won);
      draw_shadowed_text(buf,MFD_VIEW_WID-gr_string_width(buf)-1,1);
   }
}


bool tictactoe_generator(void* pos, int index, bool minimizer_moves)
{
   int i;
   tictactoe* t = (tictactoe *)pos;
   bool empty=TRUE;
   uchar mover=minimizer_moves?O:X;

   int realindex=index;

   if(tictactoe_evaluator(pos)!=winnerval(NOBODY)) return FALSE;  // already have a winner => no children

#define NO_SYMMETRIES
#ifdef NO_SYMMETRIES
   for(i=0;empty && i<9;i++) {
      if(t->owner[i]!=NOBODY)
	 empty=FALSE;
   }

   // don't bother with symmetries of starting moves
   if(empty) {
      switch(index) {
	 case 0: t->owner[0]=mover; return TRUE;
	 case 1: t->owner[1]=mover; return TRUE;
	 case 2: t->owner[4]=mover; return TRUE;
	 default: return FALSE;
      }
   }
#endif

   for(i=0;i<9;i++) {
      if(t->owner[i]==NOBODY) {
	 if(index==0) {
	    t->owner[i]=mover;
	    return TRUE;
	 }
	 index--;
      }
   }
   return FALSE;
}

bool games_handle_ttt(MFD *m, uiEvent *e)
{
   uiMouseEvent *me=(uiMouseEvent *)e;
   ttt_state *st=(ttt_state *)GAME_DATA;
   LGPoint pos = MakePoint(e->pos.x-m->rect.ul.x-TTT_ULX,
			 e->pos.y-m->rect.ul.y-TTT_ULY);

   if(!(me->action & MOUSE_LDOWN)) return FALSE;
   if(st->whomoves != st->whoplayer) return TRUE;
   if(tictactoe_over(&(st->board))) return TRUE;
   if(pos.x<0||pos.y<0) return TRUE;

   pos.x/=TTT_SQ_WID;
   pos.y/=TTT_SQ_HGT;

   if(pos.x>=TTT_PUZ_WID || pos.y>=TTT_PUZ_HGT)
      return TRUE;

   if(st->board.owner[pos.x+3*pos.y] != NOBODY)
      return TRUE;

   st->board.owner[pos.x+3*pos.y]=st->whoplayer;

   if(!tictactoe_over(&(st->board))) {
      st->whomoves=OTHERPLAYER(st->whoplayer);

      minimax_setup(&(st->board),sizeof(tictactoe),9,st->whomoves==O,
	 tictactoe_evaluator, tictactoe_generator, NULL);
   }

   mfd_notify_func(MFD_GAMES_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, TRUE);

   return TRUE;
}

//
//   MFD Wing Commander
//
// use the position of the mouse to steer
//   button 1 makes you fire; button 2 lets
//   you control thrust and rotate view
// always fire towards middle

// Object structure... this handles arbitrary object in 3d

// We'll always track all objects in arbitrary player-centric
// viewspace, a la Star Raiders

#define WING_QUEST_VAR          0x38

#define WING_SFX_GOODGUY_FIRE   SFX_GUN_SKORPION
#define WING_SFX_BADGUY_FIRE    SFX_GUN_STUNGUN

#define WING_SFX_HIT_PLAYER     SFX_METAL_SPANG        
#define WING_SFX_HIT_OTHER      SFX_GUN_PIPE_HIT_METAL

#define WING_SFX_EXPLODE        SFX_CPU_EXPLODE

#define WING_SFX_COMM           SFX_MFD_SUCCESS
#define WING_SFX_AUTOPILOT      SFX_SHIELD_UP
#define WING_SFX_THEYRE_ATTACKING_US_SIR        113

#define WING_HEAR_EXPLODE       512
#define WING_HEAR_HIT           256
#define WING_HEAR_FIRE          192



typedef struct {
    fix x,y,z;        // coordinates in 3space
    fix dx,dy,dz;     // velocity in 3space for objects
    int type;         // what typa object is it
    int damage;       // how much damage 'til it blows
} wing_obj;

typedef struct {
    fix x,y,z;
    int color;
} wing_star;
		      
#define MAX_WING_OBJECTS        ((HIDEOUS_GAME_STORAGE-512) / sizeof(wing_obj))
#define MAX_WING_STARS          (512 / sizeof(wing_star))

wing_obj *wing = (wing_obj *) (hideous_secret_game_storage + 4);
wing_star *wing_st = (wing_star *) (hideous_secret_game_storage + HIDEOUS_GAME_STORAGE - 512 +4);

// so I note that all of these variables should be in the
// player struct, that is in the mfd game state, and I'll
// fix that later.
static enum WingmanMode
{
  WINGMAN_FORMATION,
  WINGMAN_ATTACK
}; 

#if 0

static int WingmanMode wingman_mode = WINGMAN_FORMATION;
static int num_wing_objects, wing_frame_count;
static int wing_game_mode = WING_BRIEFING, wing_level;
static int wing_message, wing_message_timer;

#else

struct wing_data {
  uchar game_mode;
  uchar wd_wingman_mode;
  uchar wd_num_wing_objects;
  uchar wd_wing_frame_count;
  uchar wd_wing_game_mode;
  uchar wd_wing_level;
  uchar wd_wing_message;
  uchar wd_wing_message_timer;
};

#define WING_DATA       ((struct wing_data *) GAME_DATA)

#define wingman_mode    (WING_DATA->wd_wingman_mode)
#define num_wing_objects (WING_DATA->wd_num_wing_objects)
#define wing_frame_count (WING_DATA->wd_wing_frame_count)
#define wing_game_mode  (WING_DATA->wd_wing_game_mode)
#define wing_level      (WING_DATA->wd_wing_level)
#define wing_message    (WING_DATA->wd_wing_message)
#define wing_message_timer (WING_DATA->wd_wing_message_timer)

#endif


#ifdef PLAYTEST
static int wing_cheat = 0;
#endif

enum WingTypes
{
  WING_BLUE_HAIR,
  WING_SHOT,
  WING_BOOM,
  WING_WINGMAN,
  WING_BADGUY,
  WING_BADGUY2,
  WING_BADGUY3
};

#define WING_GOODGUY_MASK       ((1 << WING_BLUE_HAIR) | (1 << WING_WINGMAN))
#define WING_BADGUY_MASK        (0xff << WING_BADGUY)

enum WingGameMode
{
  WING_PLAY_GAME,
  WING_BRIEFING,
  WING_DEBRIEFING,
  WING_FLYBY,
  WING_YOUDIED
};


static int create_wing_object(int type, int dam, fix x, fix y, fix z)
{
  int i; 
  if (num_wing_objects < MAX_WING_OBJECTS) {
    i = num_wing_objects++;
    wing[i].type = type;
    wing[i].damage = dam;
    wing[i].x = x;
    wing[i].y = y;
    wing[i].z = z;
  } else
    i = -1;
  return i;
}

static void wing_delete_all_but(void)
{
  // delete everything other than wingman
  int i = 1;
  while (i < num_wing_objects)
    if (wing[i].type == WING_WINGMAN)
      ++i;
    else
      wing[i] = wing[--num_wing_objects];
}

// All the things that determine units, the scale of things,
// should go in this section for ease of manipulation

static fix wing_velocity[] = {
  fix_make(2,0),                // BLUE_HAIR
  fix_make(8,0),                // SHOT
  fix_make(0,0),                // BOOM
  fix_make(4,0),                // WINGMAN
  fix_make(3,0),                // BADGUY
  fix_make(2,0x8000),           // BADGUY2
  fix_make(5,0)                 // BADGUY3
};

static int wing_damage_amount[] = {
  20,
  1,
  20,   // countdown timer for explosion
  20,
  8,
  12,
  16
};


#define WING_FIRING_RANGE       fix_make(200,0)
#define WING_TRAIL              32      // 32 frames behind other person
#define FORMATION               fix_make(48,0)
#define WING_HIT_DISTANCE       fix_make(14,0)  // was 20

#define SHOT_TIME               30
#define SHOT_VALID_TIME         27
#define WING_ANIM_CYCLE         (CIT_CYCLE/4)
#define WING_CYCLE              (CIT_CYCLE/30)

#define WING_PHASES             4

#define WING_NUM_MISSIONS       13

#if 0

//
// Wing text stuff.
// We'll put 'em into cybstring when the whole thing is approved.
//
static char *wing_briefing[WING_NUM_MISSIONS+1] = {
// 1234567890123456789 1234567890123456789 1234567890123456789 12345678901234567890
  "A routine patrol for you, Captain Boopoototoka. Take Gypsy as wing.",
  "An X Industries base in this sector is under attack. You and Kludge help out.",
  "We're jumping outsector. Go with Kalimus and clear the jump point.",
  "You've got a solo patrol today. We don't expect anyone's around.",
  "You and Ghandi can go look for those supposed Tri-Lackys.",
  "Random's the only one who'll fly with you.  Do some reconnaissance.",
  "We're hitting the anvil factory. Clear out their guards with Newt.",
  "Without anvils, the TriLacky are in retreat.  Take Lucifer and clean up.",
  "You and St. Theresa should help kick the Cabal while they're down.",
  "I want Eyesnack, Miles, and Boopoototoka to take 'em out.",
  "We're in Tiger Sector. You and Harpo see what's up.",
  "We're pressing to the sector's heart.  Let's send everyone at once.",
  "Why don't you just finish off their navy by yourself, Bjorn?",
  "wiped out the TriLacky Cabal and won the war. Now we're all out of jobs."
};
static char *wing_debriefing[WING_NUM_MISSIONS] = {
  "Well done, Bjorn. I wonder what those pirates were doin' here.",
  "Well, we've stopped the pirates from sabotaging X. Good work.",
  "We've succeeded in jumping to Scary sector. Nothing to worry about here.",
  "No way are the Tri-Lacky here. Next time use your flight recorder!",
  "Ghandi didn't see anything. You forgot to take off your lens cap.", 
  "Tri-Lacky here! Who'd've believed it? With an anvil factory to boot.",
  "Excellent work. We're jumping to help up mop up Spilt Milk Sector.",
  "Good job, Bjorn. The TriLackys are running to their mothers.",
  "We've learned there's a TriLacky strike force hiding out here.",
  "I'm sure the Cabal is crying over Spilt Milk. Now we join the front line.", 
  "Well, I doubt they deciphered your coded transmissions, at least.",
  "Good show. Now we're approaching the Cabal's homeworld.",
  "Congratulations, Bjorn, err, Admiral Boopoototoka. You've single-handedly",
};
static char *wing_sighted[WING_NUM_MISSIONS] = {
  "They're attacking us, sir.",
  "I have enemies on my scanner.",
  "Easy pickings.",
  "",
  "Must we resort to violence?",
  "We got some Try-Unlucky here.",
  "Cabal sighted.",
  "Lost souls ahead.",
  "Visitors! And my place is a mess.",
  "We've got company.",
  "<Honk><honk><honk>!",
  "There sure is a lot of 'em.'",
  ""
};
static char *wing_dies[WING_NUM_MISSIONS] = {
  "Ahhhhhhhhhhhhhhh!",
  "System integrity failure.",
  "Nice knowing you.",
  "",
  "You're the traitor!", 
  "You good-for-nothing...",
  "Into the volcano!",
  "I am consigned to the flames.",
  "Mewmewmewmew...",
  "Where's the eject button?",
  "<Honk..>",
  "Ahhhhhhhhhhhhhhh!",
  ""
};
static char *wing_attack[WING_NUM_MISSIONS] = {
  "Engaging enemy, sir.",
  "Enemy destruction commencing.",
  "What a way to meet people.",
  "",
  "That's against my principles.",
  "You don't have to tell me twice.",
  "Eat my flames!",
  "Sinful.",
  "Sure, I'll be aggressive.",
  "Leave it to us.",
  "<Honk><honk>",
  "Into the hairball!",
  ""
};
static char *wing_form[WING_NUM_MISSIONS] = {
  "Forming on your wing.",
  "Undertaking requested maneuver.",
  "As you wish.",
  "",
  "Whatever.",
  "If you insist.",
  "There is no Cabal.",
  "By your command.",
  "Bless you.",
  "We're with you.",
  "<Honk>",
  "Be your'n, Bjorn.",
  ""
};

#endif

#define GHANDI_LEVEL(x)         ((x) >= 4*WING_PHASES && (x) < 5*WING_PHASES)

// Wing commander "levels"

#define W_BAD1  1
#define W_BAD2  8 
#define W_BAD3  64

uchar wing_level_data[] =
{
    // ? sector
  W_BAD1, W_BAD1, 0, W_BAD1,
  W_BAD1, 0, W_BAD1, 2*W_BAD1,
  W_BAD1*2, W_BAD1, W_BAD1, W_BAD1*4,

    // Scary Sector
  0, 0, W_BAD1*2, W_BAD1+W_BAD2,
  0, W_BAD1*2, 0, W_BAD1+W_BAD2*2,
  W_BAD2*2, 0, W_BAD1, W_BAD1*3,
  W_BAD1*2, 0, W_BAD1*3+W_BAD2, W_BAD1*2 + W_BAD2*2,

    // Spilt Milk Sector
  W_BAD1*2, 0, W_BAD1*2+W_BAD2*2, W_BAD1*2,
  0, W_BAD1*2+W_BAD2, W_BAD2*3, 0,
  W_BAD1+W_BAD2*2, W_BAD1*3, W_BAD1*4+W_BAD2*2, W_BAD1*3+W_BAD2*4+W_BAD3*3,

    // Tiger sector
  0,W_BAD2*4,0,W_BAD1*2+W_BAD3*2,
  0,0,0,W_BAD1*7+W_BAD2*5+W_BAD3*3,
  W_BAD1*4, W_BAD1*3+W_BAD2*2, W_BAD1*2+W_BAD2*3, W_BAD1*2+W_BAD2*2+W_BAD3*3
};

uchar wing_wingmen[WING_NUM_MISSIONS] =
{
  1,1,1,       // ? Sector
  0,1,1,1,     // Scary sector
  1,1,2,
  1,6,0
};

#define LEVEL_WINGMAN_COUNT()           wing_wingmen[wing_level/WING_PHASES]

enum WingMessage
{
  WING_SILENT=0,
  WING_SAYS_SIGHTED,
  WING_SAYS_DIE,
  WING_SAYS_ATTACK,
  WING_SAYS_FORM,
  WING_NO_WINGMAN
};

#define WING_MESSAGE_COUNT      30

static fix wing_distance(fix a, fix b, fix c)
{
  fix t;

  a = abs(a); b = abs(b); c = abs(c);
  t = a > b ? a + b/2 : b + a/2;
  t = c > t ? c + t/2 : t + c/2;

  return t;
}

static void wing_play_fx(int sound, int obj, int radius)
{
  if (sound == SFX_NONE) return;
  if (obj != 0) {
    if (fix_int(wing_distance(wing[obj].x, wing[obj].y, wing[obj].z)) > radius)
      return;
  }
  play_digi_fx(sound, 1);
}

static void wing_set_message(int mess)
{
  wing_message = mess;
  wing_message_timer = ((mess == WING_SILENT) ? 0 : WING_MESSAGE_COUNT);
  if (mess == WING_SAYS_SIGHTED && wing_level < WING_PHASES) {
    wing_play_fx(WING_SFX_THEYRE_ATTACKING_US_SIR, 0, 0);
  }
}

static int wing_find_wingman(int i)
{
  for (++i; i < num_wing_objects; ++i)
    if (wing[i].type == WING_WINGMAN)
      break;
  return i == num_wing_objects ? 0 : i;
}

static void wingman_order(void)
{
  int i = wing_find_wingman(0);
  if (!i)
    wing_set_message(WING_NO_WINGMAN);
  else if (wingman_mode == WINGMAN_FORMATION) {
    wingman_mode = WINGMAN_ATTACK;
    wing_set_message(WING_SAYS_ATTACK);
  } else {
    wingman_mode = WINGMAN_FORMATION;
    wing_set_message(WING_SAYS_FORM);
  }
}

static int wing_any_enemies(void)
{
  int i;
  for (i=1; i < num_wing_objects; ++i)
    if (wing[i].type >= WING_BADGUY || wing[i].type == WING_BOOM)
      return 1;
  return 0;
}

//
// Wing Commander AIs
//
// These blow ping out of the water.  You'll see.
//

// macros to determine when the AI should act
#define wing_let_ai_fire(w)      \
    (!(wing_frame_count & 7) && ((w)->type != WING_WINGMAN || !GHANDI_LEVEL(wing_level)))
#define wing_change_ai_facing(w) (!(wing_frame_count & 15) && (rand() & 8))

// Manhattan transfer
//   (once this used manhattan distance, now it uses "octagon" distance)
#define man_distance(w, a, b, c) wing_distance((w)->x-(a),(w)->y-(b),(w)->z-(c))
#define man_next_to(w,q,a,b,c)   man_distance(w, (q)->x+(a),(q)->y+(b),(q)->z+(c))
#define man_guy(w,q)             man_distance(w,(q)->x,(q)->y,(q)->z)

// More interesting movement
#define random_vel_adjust()     (fix_make(0, (rand() % 512 - 256) << 6))

static wing_obj *wing_find_nearest(wing_obj *w, int mask)
{
  int i;
  fix d,e;
  wing_obj *z = 0;

  d = 0x7fffffff;

  for (i=0; i < num_wing_objects; ++i) {
    if ((1 << wing[i].type) & mask) {
      if (wing[i].type == WING_WINGMAN && GHANDI_LEVEL(wing_level))
	continue;
      e = man_guy(w, &wing[i]) + (rand() % fix_make(4, 0));
      if (e < d) {
	z = &wing[i];
	d = e;
      }
    }
  }
  return z;
}
	
// routines for steering

// we call this with a _valid_ x,y,z velocity for w, that is one
// that's not too fast.  This routine then deals with rotation
// issues.
static void wing_try_for_velocity(wing_obj *w, fix x, fix y, fix z)
{
  // Basically, we only let one of x,y,z change signs at a time.
  // We prioritize z, then x, then y, to cause things to go left/right
  // more then up/down

  if ((z >= 0 && w->dz < 0) || (z <= 0 && w->dz > 0)) {
    if (abs(w->dz) > FIX_UNIT) z = 0;
    w->dz = z;
  } else if ((x >= 0 && w->dx < 0) || (x <= 0 && w->x > 0)) {
    if (abs(w->dx) > FIX_UNIT) x = 0;
    w->dx = x;
    w->dz = z;
    if ((y >= 0 && w->dy < 0) || (y <= 0 && w->dy > 0)) {
      w->dy = 0;
    } else { 
      if (abs(w->dy) > FIX_UNIT) y = 0;
      w->dy = y;
    }
  } else {
    if (abs(w->dy) > FIX_UNIT) y = 0;
    w->dx = x;
    w->dy = y;
    w->dz = z;
  }

  w->dx += random_vel_adjust();
  w->dy += random_vel_adjust();
  w->dz += random_vel_adjust();
}

// convert (dx,dy,dz) to be of length (m)
// Someone tell me why I made this fast and approximate
// (note approximate square root and use of shifts instead
// of divides and multiplies) when it's an MFD game?
static void wing_scale_velocity(fix *dx, fix *dy, fix *dz, fix m)
{
  fix x = *dx, y = *dy, z = *dz;
  fix v;
  
  // compute approximate velocity
  v = wing_distance(x,y,z);
  if (v == 0) {
    *dx = x; *dy = y; *dz = z;
    return;
  }

  // scale to guy's maximum velocity
  while (v < m/2) v *= 2, x *= 2, y *= 2, z *= 2;
  while (v >= m) v /= 2, x /= 2, y /= 2, z /= 2;

  if (v < m-m/4) v = v+v/2, x = x+x/2, y = y+y/2, z = z+z/2;

  *dx = x;
  *dy = y;
  *dz = z;
}

static void wing_try_to_goto(wing_obj *w, fix x, fix y, fix z)
{
  // compute effective direction
  x -= w->x;
  y -= w->y;
  z -= w->z;

  wing_scale_velocity(&x, &y, &z, wing_velocity[w->type]);
  wing_try_for_velocity(w, x,y,z);
}

static fix wing_vel, wing_acc;
static fixang wing_a, wing_b, wing_c; 


static void wing_fire_shot(wing_obj *w, int side)
{
  int i;
  fix x,y,z;
  // fire out the front of this ship at shot velocity

  if (w->type == WING_BLUE_HAIR)
    i = create_wing_object(WING_SHOT, SHOT_TIME, w->x+w->dx, w->y+wing_vel, w->z+w->dz);
  else
    i = create_wing_object(WING_SHOT, SHOT_TIME, w->x+w->dx, w->y+w->dy, w->z+w->dz);

  if (i == -1)
    return;

  if (w->type == WING_BLUE_HAIR) {
    x = z = 0;
    y = wing_velocity[WING_SHOT];
  } else {
    x = w->dx;
    y = w->dy;
    z = w->dz;
    wing_scale_velocity(&x, &y, &z, wing_velocity[WING_SHOT]);
    if (x == 0 && y == 0 && z == 0) {
      --num_wing_objects;
      return;
    } 
  }

  wing[i].x += (wing[i].dx = x) + y * side/2;
  wing[i].y += (wing[i].dy = y) + x * side/2;
  wing[i].z += (wing[i].dz = z) - FIX_UNIT*4;
}


static int wing_in_front_of(wing_obj *target, wing_obj *base)
{
  // if target is in front of base, then line from base to target
  // is in same direction as velocity of base

  int x,y,z;
  x = fix_int(target->x - base->x);
  y = fix_int(target->y - base->y);
  z = fix_int(target->z - base->z);

  x = x * fix_int(base->dx*64);
  y = y * fix_int(base->dy*64);
  z = z * fix_int(base->dz*64);

  return (x + y + z > 0);
}

static void wing_ai_fire(wing_obj *w, wing_obj *z)
{
  if (wing_in_front_of(z,w) && !(rand() % 4)) {
    wing_play_fx(w->type == WING_WINGMAN ? WING_SFX_GOODGUY_FIRE : WING_SFX_BADGUY_FIRE, w - wing, WING_HEAR_FIRE);
    wing_fire_shot(w, -1);
    wing_fire_shot(w,1);
  }
}

static void wing_do_ai(wing_obj *w)
{
  int mask;
  wing_obj *z;

  switch(w->type) {
    case WING_BLUE_HAIR:
      return;

    case WING_SHOT:
    case WING_BOOM:
      --w->damage;              // countdown until it is dead
      break;

    case WING_WINGMAN:
      switch(wingman_mode) {
	case WINGMAN_FORMATION:
	  // Formation: 
	  //   If near enough to player, turn to face same direction 
	  //   else turn towards player's destination

	  if (wing_change_ai_facing(w)) {
	    if (man_next_to(w, &wing[0], -FORMATION/2, 0, 0) < FORMATION*2) {
	      if (wing_vel > wing_velocity[w->type])
		wing_try_for_velocity(w, 0, wing_velocity[w->type], 0);
	      else
		wing_try_for_velocity(w, 0, wing_vel, 0);
	    } else {
	      // We should try to lead player somewhat, but
	      // it depends how close we are.  Hmm.
	      // We'll just go to where he was, and curve in.
	      wing_try_to_goto(w, -FORMATION/2, 0, 0);
	    }
	  }
	  if (wing_let_ai_fire(w)) {
	    z = wing_find_nearest(w, WING_BADGUY_MASK);
	    if (z && man_guy(w,z) < WING_FIRING_RANGE)
	      wing_ai_fire(w, z);
	  }
	  break;

	case WINGMAN_ATTACK:
	  mask = WING_BADGUY_MASK;        
	  goto attack_ai;
      }
      break;

    default:
      mask = WING_GOODGUY_MASK;
      // fallthrough

    attack_ai:
      z = wing_find_nearest(w, mask);
      if (z == 0 && w->type == WING_WINGMAN) {
	wingman_mode = WINGMAN_FORMATION;
	if (!(rand()%4))
	  wing_set_message(WING_SAYS_FORM);
      }
      if (z && wing_change_ai_facing(w)) {
	// if we're not in firing range, just move towards
	// otherwise if we're behind him turn to face him
	if (man_guy(w,z) < WING_FIRING_RANGE) 
	  //  seems better not to do this    || !wing_in_front_of(w,z))
	  wing_try_to_goto(w, z->x, z->y, z->z);
	else
	// otherwise try to get behind him
	  wing_try_to_goto(w, z->x - WING_TRAIL*(z->dx + z->dy/2), z->y - WING_TRAIL*(z->dy - z->dx/2), z->z - WING_TRAIL*z->dz);
      }
      if (z && man_guy(w,z) < WING_FIRING_RANGE && wing_let_ai_fire(w)) {
	wing_ai_fire(w,z);
      }
    // end of cases of object types
  }
}

static void wing_rotate_vector(fix *v, 
	fix sina, fix cosa, fix sinb, fix cosb, fix sinc, fix cosc)
{
   fix x,y,z;

   y = fix_mul(cosa, v[1]) + fix_mul(sina, v[2]);
   z = fix_mul(cosa, v[2]) - fix_mul(sina, v[1]);

   v[1] = fix_mul(cosb, y) - fix_mul(sinb, v[0]);
   x = fix_mul(cosb, v[0]) + fix_mul(sinb, y);

   v[2] = fix_mul(cosc, z) - fix_mul(sinc, x);
   v[0] = fix_mul(cosc, x) + fix_mul(sinc, z);
}


static void wing_move_world(fixang a, fixang b, fixang c, fix v)
{
  // move the world because player rotated by a & b
  // and moved forward by velocity v

  int i;
  fix sina,cosa,sinb,cosb,sinc,cosc;
  fix_sincos(a, &sina, &cosa);
  fix_sincos(b, &sinb, &cosb);
  fix_sincos(c, &sinc, &cosc);
  for (i = 1; i < num_wing_objects; ++i) {

    if (wing_game_mode == WING_PLAY_GAME)
      wing_do_ai(&wing[i]);

    wing_rotate_vector(&wing[i].x, sina, cosa, sinb, cosb, sinc, cosc);
    wing_rotate_vector(&wing[i].dx, sina, cosa, sinb, cosb, sinc, cosc);

    wing[i].x += wing[i].dx;
    wing[i].y += wing[i].dy - v;    // adjust for player's velocity
    wing[i].z += wing[i].dz;
  }
  for (i=0; i < MAX_WING_STARS; ++i)
    wing_rotate_vector(&wing_st[i].x, sina, cosa, sinb, cosb, sinc, cosc);
}

static void wing_handle_collisions(void)
{
  int i,j;
    // delete any objects which are dead
  i = 1; 
  while (i < num_wing_objects)
    if (wing[i].damage <= 0) {
      if (wing[i].type == WING_WINGMAN)
	wing_set_message(WING_SAYS_DIE);
      if (wing[i].type != WING_SHOT && wing[i].type != WING_BOOM) {
	// turn guys into explosions
	wing[i].type = WING_BOOM;
	wing[i].damage = wing_damage_amount[wing[i].type];
	++i;
	wing_play_fx(WING_SFX_EXPLODE, i, WING_HEAR_EXPLODE);
      } else
	wing[i] = wing[--num_wing_objects];
    } else
      ++i;

  for (j=1; j < num_wing_objects; ++j) {
    // check object j against all objects
    // we only check for collisions of shots against ships
    if (wing[j].type == WING_SHOT && wing[j].damage < SHOT_VALID_TIME) {
      for (i=0; i < num_wing_objects; ++i) {
	if (wing[i].type != WING_SHOT && wing[i].type != WING_BOOM) {
	  // are they in range of each other?
	  if (man_guy(&wing[i], &wing[j]) < WING_HIT_DISTANCE) {
	    // do damage to both; shots always die
	    wing[i].damage -= wing_damage_amount[wing[j].type];
	    wing[j].damage = 0;
	    wing_play_fx((i == 0) ? WING_SFX_HIT_PLAYER : WING_SFX_HIT_OTHER,
		    i, WING_HEAR_HIT);
	  }
	}
      }
    }
  }
}



static void wing_update_one_time_unit(void)
{
  for(; games_time_diff>=WING_CYCLE; games_time_diff-= WING_CYCLE) {
    wing_vel += wing_acc;
    if (wing_vel < 0) wing_vel = 0;
    if (wing_vel > wing_velocity[0]) wing_vel = wing_velocity[0];

    ++wing_frame_count;

    wing_move_world(wing_a, wing_b, wing_c, wing_vel);
    wing_handle_collisions();
  }
  if (wing[0].damage <= 0)
    wing_game_mode = WING_YOUDIED;
}

#define WING_FACE_FORWARD       0
#define WING_FACE_LEFT          1
#define WING_FACE_AWAY          2
#define WING_FACE_RIGHT         3
#define WING_FACE_UP            4
#define WING_FACE_DOWN          5

static int wing_get_facing(int i)
{
  int dx = abs(wing[i].dx);
  int dy = abs(wing[i].dy);
  int dz = abs(wing[i].dz);

  if (dx > dy)
    if (dx > dz)
      return wing[i].dx > 0 ? WING_FACE_RIGHT : WING_FACE_LEFT;
    else
      return wing[i].dz > 0 ? WING_FACE_UP : WING_FACE_DOWN;
  else
    if (dy > dz)
      return wing[i].dy > 0 ? WING_FACE_AWAY : WING_FACE_FORWARD;
    else
      return wing[i].dz > 0 ? WING_FACE_UP : WING_FACE_DOWN;
}

static void scale_res_bm(int ref, int x, int y, int w, int h)
{
  FrameDesc *f;

  f = (FrameDesc *)RefLock(ref);
  if (!f) return;

  f->bm.bits = (uchar *)(f+1);
  ss_scale_bitmap(&f->bm, x, y, w, h);

  RefUnlock(ref);
}

#define BRIEF_X         2
#define BRIEF_Y         (MFD_VIEW_HGT-6)

#define WING_NUMCHARS   26

// the cleverer flow-text-up-from-the-bottom-
// so-that-it-always-fits routine

static void wing_print_message(char *s, int y)
{
  char *t,*u,c;
  int x;

  u = s + strlen(s) - 1;
  while (u - s + 1 > WING_NUMCHARS/2) {
    // u points to the last character which hasn't been plotted
    // find last whitespace WING_NUMCHARS or fewer characters
    if (u - WING_NUMCHARS < s) 
      t = s-1;
    else {
      t = u - WING_NUMCHARS;
      while (*t != ' ') {
	if (!*t) goto ouch;
	++t;
      }
    }
    // print that much of s
    c = u[1];
    u[1] = 0;
    // wait, first let's check if that's too long
    while (gr_string_width(t+1) > MFD_VIEW_WID - BRIEF_X-1) {
      ++t;
      while (*t != ' ') {
	if (!*t) goto ouch;
	++t;
      }
    }
    if (0) {
      ouch:
	if (u - WING_NUMCHARS < s) 
	  t = s-1;
	else 
	  t = u - WING_NUMCHARS;
    }
    ss_string(t+1,BRIEF_X,y);
    u[1] = c;
    u = t-1;
    y -= 5;
  }
  if (u >= s) {
    c = u[1];
    u[1] = 0;
    if (u-s < WING_NUMCHARS/2)
      x = (MFD_VIEW_WID - gr_string_width(s)) / 2 - BRIEF_X;
    else
      x = 0;
    ss_string(s,BRIEF_X + x,y);
    u[1] = c;
  }
}


#define WING_VIEW_ANGLE_SCALE_THING     fix_make(32,0)

static int wing_radius[] = {
  1,2,3,4,5,6,7,8,9,10,11,12,15,18,21,25,28,26,22,16,1000
};

static void wing_render_world(void)
{
  int i,sx,sy,k,j;
  fix sc,q;
  wing_obj temp;

  gr_set_fcolor(BLACK+1);
  ss_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);

  for (i=0; i < MAX_WING_STARS; ++i) {
    if (wing_st[i].y >= fix_make(0,0x1000)) {
      sc = fix_div(WING_VIEW_ANGLE_SCALE_THING, wing_st[i].y);
      sx = fix_int(fix_mul(sc, wing_st[i].x)) + MFD_VIEW_MID;
      sy = MFD_VIEW_HGT/2 - fix_int(fix_mul(sc, wing_st[i].z));
      ss_set_pixel(WHITE, sx, sy);
    }
  }

  // depth sort the objects with an insertion sort
  for (i=2; i < num_wing_objects; ++i) {
    temp = wing[i];
    q = temp.y;
    j = i-1;
    while (j > 0) {
      if (q < wing[j].y)
	break;
      wing[j+1] = wing[j];
      --j;
    }
    wing[j+1] = temp;
  }

  for (i=1; i < num_wing_objects; ++i) {
    if (wing[i].y >= fix_make(0,0x1000)) {
      sc = fix_div(WING_VIEW_ANGLE_SCALE_THING, wing[i].y);
      sx = fix_int(fix_mul(sc, wing[i].x)) + MFD_VIEW_MID;
      sy = MFD_VIEW_HGT/2 - fix_int(fix_mul(sc, wing[i].z));
      switch(wing[i].type) {
	case WING_WINGMAN:
	case WING_BLUE_HAIR:
	  k = fix_int(sc*16)+1;
	  scale_res_bm(REF_IMG_wing_ship + wing_get_facing(i),
		      sx-k/2, sy-k/2, k, k);
	  break;
	case WING_SHOT:
	  k = fix_int(sc*1) + 1;
	  if (k > 1)
	    scale_res_bm(REF_IMG_DepthCharge,
			sx, sy, fix_int(sc*1)+1, fix_int(sc*1)+1);
	  else
	    ss_set_pixel(ORANGE_8_BASE, sx, sy);
	  break;
	case WING_BOOM:
	  k = fix_int(sc*wing_radius[wing[i].damage])+1;
	  gr_set_fcolor(RED_8_BASE+2+rand()%4);
	  ss_int_disk(sx,sy, k);
	  break;

	default:
	  k = fix_int(sc*16) + 1;
	  scale_res_bm(REF_IMG_wing_bad1 + 6*(wing[i].type - WING_BADGUY) + wing_get_facing(i),
		      sx-k/2, sy-k/2, k, k);
      }
    }
  }

  for (i=1; i < num_wing_objects; ++i) 
    if (wing[i].type > WING_BOOM) {
      sx = fix_rint(wing[i].x / 128) + 10;
      sy = fix_rint(wing[i].y / 128) + 10;
      if (sx < 0) sx = 0; else if (sx > 20) sx = 20;
      if (sy < 0) sy = 0; else if (sy > 20) sy = 20;
      ss_set_pixel((wing[i].type == WING_WINGMAN ? GREEN_8_BASE : RED_8_BASE) + 2, sx, MFD_VIEW_HGT-1 - sy);
    }
  ss_set_pixel(GREEN_8_BASE+3, 10, MFD_VIEW_HGT-1-10);
    
  {
    char buffer[16];
    sprintf(buffer,"%03d",wing_vel*150/wing_velocity[0]);
    gr_set_fcolor(WHITE);
    ss_string(buffer, MFD_VIEW_WID-16, 0);
  }

  if (wing_message) {
    char *s;
    int z = wing_level/WING_PHASES;
    switch (wing_message) {
      case WING_SAYS_SIGHTED: s = STRING(WingSighted+z); break;
      case WING_SAYS_DIE: s = STRING(WingDies+z); break;
      case WING_SAYS_ATTACK: s = STRING(WingAttack+z); break;
      case WING_SAYS_FORM: s = STRING(WingForm+z); break;
      case WING_NO_WINGMAN: s = STRING(NoWing); break;
    }
    gr_set_fcolor(GREEN_8_BASE);
    wing_print_message(s, MFD_VIEW_HGT - 6);

    if (!--wing_message_timer)
      wing_message = 0;
  }
}

// this should actually take as a parameter
static void wing_play_cutscene(char *s)
{
  int anim;
  if (!full_game_3d)
//KLC - chg for new art   draw_res_bm(REF_IMG_bmBlankMFD, 0,0);
	  draw_hires_resource_bm(REF_IMG_bmBlankMFD, 0, 0);
  anim=(player_struct.game_time/WING_ANIM_CYCLE)%3;
  draw_res_bm(REF_IMG_GoofyNed + anim, 20, 2);

  // now display the text
  gr_set_fcolor(GRAY_8_BASE + 1);
  wing_print_message(s, BRIEF_Y);
}

static void wing_start_minor_level(void)
{
  int i,j, wingman=0, t, n;
  wing_game_mode = WING_PLAY_GAME;

  // reset player speed
  wing[0].dx = wing[0].dz = 0;
  wing[0].dy = wing_vel = wing_velocity[WING_BLUE_HAIR]/2;

  wing_a = wing_b = wing_c = 0;
  wing_acc = 0;

  // delete everything other than wingman
  wing_delete_all_but();

  // rotate the stars
  wing_move_world(0,0x8000,0,0);

  i = wing_find_wingman(0);
  n = 0;
  while (i) {
    wing[i].dx = wing[i].dz = 0;
    wing[i].dy = wing[0].dy;
    wing[i].y = wing[i].z = 0;
    wing[i].x = -FORMATION/3 + n++*FORMATION/2;
    wingman = 1;
    i = wing_find_wingman(i);
  }

  // create the enemy
  n = wing_level_data[wing_level];
  if (n && wingman) 
    wing_set_message(WING_SAYS_SIGHTED);
  else
    wing_set_message(WING_SILENT);

  t = WING_BADGUY;
  while (n) {
    for (i=0; i < (n&7); ++i) {
      j = create_wing_object(t, wing_damage_amount[t],
	      fix_make(rand()%512 - 256,0), 
	      fix_make(rand()%64 + 768,0), 
	      fix_make(rand()%512 - 256,0));
      if (j != -1) {
	wing[j].dx = (rand()%512 - 256) * 256;
	wing[j].dy = (rand()%512 - 256) * 256;
	wing[j].dz = (rand()%512 - 256) * 256;
      }
    }
    n >>= 3;
    ++t;
  }

  wingman_mode = WINGMAN_FORMATION;
  games_time_diff = 0;
}      

static void wing_start_major_level(void)
{
  int j;

  // reset all objects so player has new damage amount
  // and so there's always a wingman if there should be
  num_wing_objects = 0;
  // allocate player object
  j = create_wing_object(WING_BLUE_HAIR,wing_damage_amount[WING_BLUE_HAIR],0,0,0);
  wing_vel = wing_velocity[WING_BLUE_HAIR] / 2;
  wing_acc = 0;

  wing[j].dx = wing[j].dz = 0;
  wing[j].dy = wing_vel;        // so badguys can try to get behind you

  for (j=0; j < LEVEL_WINGMAN_COUNT(); ++j)
    create_wing_object(WING_WINGMAN, wing_damage_amount[WING_WINGMAN],
      -FORMATION/2 - FORMATION, 0, 0);

  wing_start_minor_level();
}

static void wing_start_flyby(void)
{
  int i;

  wing_play_fx(WING_SFX_AUTOPILOT, 0, 0);
  wing_delete_all_but();
  wing_move_world(0,0x8000,0,0);
  // create a dummy object for the player

  i = create_wing_object(WING_BLUE_HAIR, 1, 0,0,0);

  for (i=1; i < num_wing_objects; ++i) {
    wing[i].z = fix_make(6,0) * (i-1);
    wing[i].y = fix_make(140,0) - fix_make(8,0) * i;
    wing[i].x = fix_make(1,0) + fix_make(35,0) * i;
    wing[i].dx = wing[i].dz = 0;
    wing[i].dy = -fix_make(4,0);
  }

  wing_frame_count = 0;
  wing_game_mode = WING_FLYBY;
  games_time_diff = 0;
}

static void wing_handle_flyby(void)
{
  wing_a = wing_c = 0;
  wing_b = -0x1c0;
  wing_vel = 0;
  wing_update_one_time_unit();
  wing_render_world();

  if (wing_frame_count > 64) {
    wing_start_minor_level();
  }
}

static void wing_advance_to_next_level(void)
{
  if (wing_level % WING_PHASES == WING_PHASES-1) {
    wing_game_mode = WING_DEBRIEFING;
    QUESTVAR_SET(WING_QUEST_VAR, wing_level+1);
  } else {
    ++wing_level;
    wing_start_flyby();
  }
}

bool games_handle_wing(MFD *m, uiEvent *e)
{
   uiMouseEvent *me=(uiMouseEvent *)e;
   LGPoint pos = MakePoint(e->pos.x-m->rect.ul.x,
			 e->pos.y-m->rect.ul.y);
   fix x,y;

   if (wing_game_mode != WING_PLAY_GAME) {
     if (!(me->action & MOUSE_LDOWN))
       return FALSE;
     if (wing_game_mode == WING_DEBRIEFING) {
       wing_game_mode = WING_BRIEFING;
       ++wing_level;
       return TRUE;
     }
     if (wing_game_mode == WING_BRIEFING) {
       if (wing_level == WING_NUM_MISSIONS * WING_PHASES) {
	 QUESTVAR_SET(WING_QUEST_VAR, 0);
	 GAME_MODE = GAME_MODE_MENU;
	 return TRUE;
       }
       wing_start_major_level();
       return TRUE;
     }
     if (wing_game_mode == WING_YOUDIED) {
       wing_level = QUESTVAR_GET(WING_QUEST_VAR);
       wing_game_mode = WING_BRIEFING;
       return TRUE;
     }
     return FALSE;
   }

   x = (pos.x * FIX_UNIT*2) / MFD_VIEW_WID - FIX_UNIT;
   y = (pos.y * FIX_UNIT*2) / MFD_VIEW_HGT - FIX_UNIT;

   if (abs(x) < fix_make(0, 4096))
     x = 0;
   else
     x = x + (x > 0 ? -4096 : 4096);

   if (abs(y) < fix_make(0, 4096))
     y = 0;
   else
     y = y + (y > 0 ? -4096 : 4096);

#ifdef PLAYTEST
   if (wing_cheat && (me->action & MOUSE_RDOWN) && (me->buttons & 3)==3) {
     // right click while left button held 
     wing_delete_all_but();
     wing_level |= 3;
     return TRUE;
   }
#endif

   if(me->action & MOUSE_LDOWN) { 
     if ((me->buttons & 3) == 3) {
       // both buttons, assume it's an order
       wingman_order();
       wing_play_fx(WING_SFX_COMM, 0, 0);
       return TRUE;
     }
     if (!wing_any_enemies()) {
       wing_advance_to_next_level();
       return TRUE;
     }
     wing_play_fx(WING_SFX_GOODGUY_FIRE, 0, 0);
     wing_fire_shot(wing, -1);
     wing_fire_shot(wing, 1);
     return TRUE;
   }

   switch(me->buttons & 3) {
     case 0: 
     case 1: 
     case 3: // both buttons pushed.  Huh.
       wing_a = -y / 64;
       wing_b = -x / 64;
       wing_c = 0;
       wing_acc = 0;
       break;

     case 2: // do right mouse stuff
       wing_acc = -y / 4;
       wing_c = x / 64;
       wing_a = wing_b = 0;
   }

   return TRUE;
}

void games_init_wing(void *)
{
  int i;
  wing_level = QUESTVAR_GET(WING_QUEST_VAR);
  wing_game_mode = WING_BRIEFING;

  for (i=0; i < MAX_WING_STARS; ++i) {
    wing_st[i].x = fix_make(rand()%512-256,0);
    wing_st[i].y = fix_make(rand()%512-256,0);
    wing_st[i].z = fix_make(rand()%512-256,0);
  }
}

void games_expose_wing(MFD *, ubyte)
{
  switch(wing_game_mode) {
    case WING_PLAY_GAME:
      wing_update_one_time_unit();
      wing_render_world();
      break;

    case WING_BRIEFING:
      wing_play_cutscene(STRING(WingBriefing+wing_level/WING_PHASES));
      break;

    case WING_DEBRIEFING:
      wing_play_cutscene(STRING(WingDebriefing+wing_level/WING_PHASES));
      break;

    case WING_YOUDIED:
      wing_play_cutscene(STRING(WingYouDied));
      break;

    case WING_FLYBY:
      wing_handle_flyby();
      break;
  }

  mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
  // autoreexpose
  mfd_notify_func(MFD_GAMES_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, FALSE);
}



#endif // LOST_TREASURES

/*
// this is so lovely, it is a test function, joy
bool mfd_games_hack_func(short keycode, ulong context, void* data)
{
   mcom_state *cur_state=(mcom_state *)GAME_DATA;
   int mfd = mfd_grab_func(MFD_GAMES_FUNC,MFD_INFO_SLOT);

	// give the tester all of the frigging games
   player_struct.softs.misc[MISC_SOFTWARE_GAMES] = 0xff;
   GAME_MODE = GAME_MODE_MENU;
   mfd_notify_func(MFD_GAMES_FUNC,MFD_INFO_SLOT,TRUE,MFD_ACTIVE,TRUE);
   mfd_change_slot(mfd,MFD_INFO_SLOT);
#ifdef PLAYTEST
   wing_cheat = 1;
#endif
   return FALSE;
}
*/

void mfd_games_turnon(bool, bool real_start)
{
   if (real_start)
   {
      int mfd = mfd_grab_func(MFD_GAMES_FUNC,MFD_INFO_SLOT);
      GAME_MODE = GAME_MODE_MENU;
      mfd_notify_func(MFD_GAMES_FUNC,MFD_INFO_SLOT,TRUE,MFD_ACTIVE,TRUE);
	   mfd_change_slot(mfd,MFD_INFO_SLOT);
      player_struct.softs_status.misc[CPTRIP(GAMES_TRIPLE)] &= ~WARE_ON;
   }
}

void mfd_games_turnoff(bool, bool )
{

   // game shutdown code goes here. 
}

bool (*game_handler_funcs[])(MFD *m,uiEvent* ev) =
{
games_handle_pong,
games_handle_mcom,
games_handle_road,
games_handle_pong,      // just reuse the pong code for bots
games_handle_15,
games_handle_ttt,
games_handle_null,
games_handle_wing,
games_handle_menu
};

