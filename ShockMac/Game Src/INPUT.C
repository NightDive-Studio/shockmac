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
 * $Source: r:/prj/cit/src/RCS/input.c $
 * $Revision: 1.293 $
 * $Author: jaemz $
 * $Date: 1994/11/23 00:16:31 $
 */

// lets get this somewhere else so we can get it in the manual or something, and not get warnings..
#ifdef SPACEBALL_SUPPORT
static char sbcopy[] = "Spaceball Interface Copyright 1994 Spaceball Technologies Inc.";
#endif


#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "Shock.h"
#include "ShockBitmap.h"
#include "InitMac.h"
#include "Prefs.h"
#include "DialogHelpers.h"
#include "ShockHelp.h"

#if __profile__
#include <Profiler.h>
#endif

#define __INPUT_SRC

#include "input.h"

#include "ai.h"
#include "aiflags.h"
#include "citres.h"
#include "colors.h"
#include "criterr.h"
#include "cybstrng.h"
#include "doorparm.h"
#include "drugs.h"
#include "emailbit.h"
#include "faketime.h"
#include "frflags.h" 		// until we do the right thing re: static
#include "FrUtils.h"
#include "fullscrn.h"
#include "gamesys.h"
#include "gamescr.h"
#include "gamestrn.h"
#include "gr2ss.h"
#include "grenades.h"
#include "hkeyfunc.h"
#include "MacTune.h"
#include "mainloop.h"
#include "musicai.h"
#include "newmfd.h"
#include "objbit.h"
#include "objects.h"
#include "objload.h"
#include "objsim.h"
#include "objprop.h"
#include "objuse.h"
#include "olhext.h"
#include "otrip.h"
#include "physics.h"
#include "player.h"
#include "screen.h"
#include "status.h"
#include "svgacurs.h"
#include "tools.h"
#include "weapons.h"

#ifdef NOT_YET //KLC - for VR headsets

#include <conio.h>
#include <config.h>
#include <inp6d.h>
#ifdef STEREO_SUPPORT
#include <i6dvideo.h>
#include <mfddims.h>
#endif

#include <paintbit.h>
#include <wrapper.h>
#include <editscrn.h>
#include <frcamera.h>
#include <frprotox.h>
#ifdef SVGA_SUPPORT
#include <frtypes.h>
#endif


#endif //NOT_YET

#define CHECK_FOR_A_PACKET

#ifdef SVGA_SUPPORT
extern frc *svga_render_context;
#endif

// -------
// DEFINES
// -------

extern Boolean	DoubleSize;

#define CFG_DCLICK_RATE_VAR "dclick_time" 
#define CFG_OOMPHOMETER     "throw_oomph"
#define CFG_INP6D_GO        "inp6d"

#define DOWN(x) ((x)|KB_FLAG_DOWN)
#define CONTROL(x) (DOWN(x)|KB_FLAG_CTRL)
#define ALT(x) (DOWN(x)|KB_FLAG_ALT)

ubyte use_distance_mod = 0;
ubyte pickup_distance_mod = 0;
ubyte fatigue_threshold = 5;

#define MAX_FATIGUE 10000 // this is stolen from gamesys.c
#define FATIGUE_COEFF CIT_CYCLE
#define FATIGUE_THRESHOLD ((player_struct.drug_status[CPTRIP(STAMINA_DRUG_TRIPLE)] ==  0) ? (fatigue_threshold*CIT_CYCLE) : MAX_FATIGUE)
#define PLAYER_FATIGUE ((player_struct.fatigue > FATIGUE_THRESHOLD) ? (player_struct.fatigue - FATIGUE_THRESHOLD)/FATIGUE_COEFF : 0)

#define MOTION_FOOTPLANT_SCANCODE 0x2A


#define AIM_SCREEN_MARGIN 5
#define sqr(x) ((x)*(x))

extern LGRect target_screen_rect;
extern int fr_get_at_raw(frc *fr, int x, int y, bool again, bool transp);

extern void mouse_unconstrain(void);

char *get_object_lookname(ObjID id,char use_string[], int sz);

extern uiSlab fullscreen_slab;
extern uiSlab main_slab;

static ushort mouse_constrain_bits = 0;

#define FIREKEY_CONSTRAIN_BIT 1
#define LBUTTON_CONSTRAIN_BIT MOUSE_LDOWN
#define RBUTTON_CONSTRAIN_BIT MOUSE_RDOWN
#define LOCK_CONSTRAIN_BIT    0x8000

typedef struct _3d_mouse_stuff
{
   bool ldown;
   bool rdown; 
   int  lastsect;
   LGPoint lastleft;
   LGPoint lastright;
   frc *fr;
} view3d_data;


// -------
// GLOBALS
// -------


Ref motion_cursor_ids[] =                   {
						     REF_IMG_bmUpLeftCursor,
						     REF_IMG_bmUpCursor,
						     REF_IMG_bmUpRightCursor,
						     0,
						     REF_IMG_bmLeftCursor,
						     REF_IMG_bmDownCursor,
						     REF_IMG_bmRightCursor,
						     0,
						     REF_IMG_bmCircLeftCursor,
						     REF_IMG_bmTargetCursor,
						     REF_IMG_bmCircRightCursor,
						     0,
						     REF_IMG_bmUpLeftCursor,
						     REF_IMG_bmSprintCursor,
						     REF_IMG_bmUpRightCursor,
						   } ;

#define NUM_MOTION_CURSORS 15
#define NUM_CYBER_CURSORS 9
#define CYBER_CURSOR_BASE REF_IMG_bmCyberUpLeftCursor

LGCursor motion_cursors[NUM_MOTION_CURSORS];
grs_bitmap motion_cursor_bitmaps[NUM_MOTION_CURSORS];

static uchar posture_keys[NUM_POSTURES] = { 't', 'g', 'b' } ; 

int input_cursor_mode = INPUT_NORMAL_CURSOR;
int throw_oomph = 5;

bool inp6d_headset=FALSE;
bool inp6d_stereo=FALSE;
bool inp6d_doom=FALSE;
bool inp6d_stereo_active=FALSE;
int inp6d_stereo_div=fix_make(3,0x4000);        // 3.25 inches apart
fix inpJoystickSens=FIX_UNIT;

// checking for game paused
extern bool game_paused;

#ifdef SVGA_SUPPORT
extern bool change_svga_mode(short keycode, ulong context, void* data);
#endif
extern bool toggle_bool_func(short keycode, ulong context, void* thebool);
LGPoint use_cursor_pos;

#ifdef RCACHE_TEST
extern bool res_cache_usage_func(short keycode, ulong context, void* data);
#endif
//extern bool texture_annihilate_func(short keycode, ulong context, void* data);

// 6d wackiness
bool inp6d_exists=FALSE;
void inp6d_chk(void);

#if defined(VFX1_SUPPORT)||defined(CTM_SUPPORT)
#include <i6dvideo.h>

static int  tracker_initial_pos[3]={0,0,0};
bool recenter_headset(short keycode, ulong context, void* data);
#endif

// globals for doubling headset angular values
bool  inp6d_hdouble = FALSE;
bool  inp6d_pdouble = FALSE;
bool  inp6d_bdouble = FALSE;

// and joysticks, heck, why be efficient
bool joystick_mouse_emul = FALSE;
uchar joystick_count=0;
bool recenter_joystick(short keycode, ulong context, void* data);

bool change_gamma(short keycode, ulong context, void* data);

// -------------
//  PROTOTYPES
// -------------
void handle_keyboard_fatigue(void);
void poll_mouse(void);
bool posture_hotkey_func(short keycode, ulong context, void* data);
bool eye_hotkey_func(short keycode, ulong context, int data);

void reload_motion_cursors(bool cyber);
void free_cursor_bitmaps();
void alloc_cursor_bitmaps(void);

int view3d_mouse_input(LGPoint pos, LGRegion* reg, bool move ,int* lastsect);
void view3d_dclick(LGPoint pos, frc* fr);
void look_at_object(ObjID id);
bool view3d_mouse_handler(uiMouseEvent* ev, LGRegion* r, view3d_data* data);
void view3d_rightbutton_handler(uiMouseEvent* ev, LGRegion* r, view3d_data* data);
bool view3d_key_handler(uiCookedKeyEvent* ev, LGRegion* r, void* data);
void use_object_in_3d(ObjID obj);

bool MacQuitFunc(short keycode, ulong context, void* data);
bool MacResFunc(short keycode, ulong context, void* data);
bool MacSkiplinesFunc(short keycode, ulong context, void* data);
bool MacDetailFunc(short keycode, ulong context, void* data);
bool MacHelpFunc(short keycode, ulong context, void* data);


// -------------
// INPUT POLLING
// -------------

void handle_keyboard_fatigue(void)
{
   byte cval;
   physics_get_one_control(KEYBD_CONTROL_BANK,CONTROL_YVEL,&cval);
   if (cval > 0)
   {
      int f = max(CONTROL_MAX_VAL - PLAYER_FATIGUE,SPRINT_CONTROL_THRESHOLD);
      if (cval > f)
         physics_set_one_control(KEYBD_CONTROL_BANK,CONTROL_YVEL,f);
   }
   physics_get_one_control(KEYBD_CONTROL_BANK,CONTROL_ZVEL,&cval);
   if (cval > 0)
   {
      int f = max(MAX_JUMP_CONTROL - PLAYER_FATIGUE,MAX_JUMP_CONTROL/2);
      if (cval > f)
         physics_set_one_control(KEYBD_CONTROL_BANK,CONTROL_ZVEL,f);
   }
}

#ifdef NOT_YET //���

//#define CONSTRAIN_TO_FAUXREND

#pragma disable_message(202)
void view3d_constrain_mouse(LGRegion* view, int mouse_bit)
{
   mouse_constrain_bits |= mouse_bit;
   if (mouse_constrain_bits != 0)
   {
#ifdef CONSTRAIN_TO_FAUXREND
      {
         fauxrend_context *cc = (fauxrend_context *)_current_fr_context;
         mouse_constrain_xy(cc->xtop,cc->ytop,cc->xtop+cc->xwid-1,cc->ytop+cc->ywid-1);
         Warning(("cc->ytop = %d!  view->abs_y = %d!\n",cc->ytop,view->abs_y));
      }
#else
      ui_mouse_constrain_xy(view->abs_x,view->abs_y,view->abs_x+RectWidth(view->r)-1,view->abs_y+RectHeight(view->r)-1);       
#endif
   }
}
#pragma enable_message(202)

void view3d_unconstrain_mouse(int mouse_bit)
{
   mouse_constrain_bits &= ~mouse_bit;
   if (mouse_constrain_bits == 0)
   {
      mouse_unconstrain();
   }
}

#ifdef PLAYTEST
bool inp6d_player=TRUE, inp6d_motion=TRUE, inp6d_conform=TRUE;
#endif

#endif //NOT_YET


// Sends a motion event to the 3d view.

bool view3d_got_event = FALSE;

void poll_mouse(void)
{
   if (_current_view != NULL)
   {
      uiMouseEvent ev;
      uiMakeMotionEvent(&ev);
      ev.type = UI_EVENT_USER_DEFINED;
      mouse_constrain_bits |= LOCK_CONSTRAIN_BIT;
      uiDispatchEventToRegion((uiEvent*)&ev,_current_view);
      mouse_constrain_bits &= ~LOCK_CONSTRAIN_BIT;
   }
}


bool checking_mouse_button_emulation = FALSE;
bool mouse_button_emulated = FALSE;

bool citadel_check_input(void);
// HATE HATE HATE HATE

bool citadel_check_input(void)
{
   if (uiCheckInput())
      return(TRUE);

   if (checking_mouse_button_emulation)
      mouse_button_emulated = FALSE;

/*KLC - no headsets in Mac version

   if (inp6d_exists) 
#ifdef PLAYTEST
      if (inp6d_motion)
#endif
	   {
	      switch (i6d_device)
	      {
#ifdef SPACEBALL_SUPPORT
	      case I6D_SBALL:    sball_chk(); break;
#endif
#ifdef CTM_SUPPORT
         case I6D_ALLPRO:
         case I6D_CTM:      ctm_chk(); break;
#endif
	      case I6D_CYBERMAN: cyberman_chk(); break;
#ifdef VFX1_SUPPORT
	      case I6D_VFX1:     vfx1_chk(); break;
#endif
	      case I6D_SWIFT:    swift_chk(); break;
	      default:           inp6d_chk(); break;
		   }
      }
   if (joystick_count) 
      joystick_chk();
*/

   // if we're suppose to emulate a mouse button - let's do it!
   if (mouse_button_emulated)
      return(TRUE);
   return(FALSE);
}


void input_chk(void)
{
   extern void setup_motion_polling(void);
   extern void process_motion_keys(void);

   setup_motion_polling();
   view3d_got_event =FALSE;
   uiPoll();
   if (!view3d_got_event)
	   poll_mouse();
// KLC - not needed on MAC   kb_flush_bios();
// KLC - not needed on MAC   mouse_set_velocity(0,0);

/* KLC - comment out for now
   if (inp6d_exists) 
#ifdef PLAYTEST
      if (inp6d_motion)
#endif
	   {
	      switch (i6d_device)
	      {
#ifdef SPACEBALL_SUPPORT
	      case I6D_SBALL:    sball_chk(); break;
#endif
#ifdef CTM_SUPPORT
         case I6D_ALLPRO:   
         case I6D_CTM:      ctm_chk(); break;
#endif
	      case I6D_CYBERMAN: cyberman_chk(); break;
#ifdef VFX1_SUPPORT
	      case I6D_VFX1:     vfx1_chk(); break;
#endif
	      case I6D_SWIFT:    swift_chk(); break;
	      default:           inp6d_chk(); break;
		   }
      }

   if (joystick_count) 
      joystick_chk();
*/
   process_motion_keys();
   handle_keyboard_fatigue();
}


#ifdef NOT_YET //KLC - stuff for VR headsets

#define Y_CEN         0       
#define MAX_VAL       (1<<15)
#define TRA_TOL       (MAX_VAL>>8)
#define ROT_TOL       (MAX_VAL>>5)
#define Tra_Scale(x)  (((x)*CONTROL_MAX_VAL)/(MAX_VAL-TRA_TOL))
#define Rot_Scale(x)  (((x)*CONTROL_MAX_VAL)/(MAX_VAL-ROT_TOL))

#ifdef SPACEBALL_SUPPORT
// i really really really want to rewrite all of this cruft

#define abs( x ) ( (x) < 0 ? -(x) : (x) )
#define isqr( x ) (((x) * (x)) >> 16)
#define icube( x ) ((isqr( x ) * x ) >> 16 )
#define sign( x ) ( (x) < 0 ? -1 : 1 )

int sb_rot_sens = 21;
int sb_tran_sens = 13;
int sb_pitch_sens = 35;
int sb_float_sens = 1;
int sb_jump_thresh = 40;
int sb_jump_sens = 30;
int sb_pitch_thresh = 5;
int sb_crouch_thresh = 60;
int sb_prone_thresh = 90;

// The number of pitch angles (looking up and down ) allowed
#define NUM_PITCH 10
int sb_num_pitch = NUM_PITCH;
int sb_pitch_div = 100 / NUM_PITCH;
int sb_pitch_angles = FALSE;

bool sb_major_axis = FALSE;

void FilterSpaceballDataMajorAxis( long *vals )
{
   int i, ind;
   long max;

   max = abs( vals[0] );
   ind = 0;

   for ( i = 1; i < 6; i++ ) {

      if ( abs( vals[ i ] ) < max ) {
         vals[i] = 0;
         continue;
      }

      max = abs( vals[i] );
      vals[ ind ] = 0;
      ind = i;

   }
}

void FilterSpaceballDataSimple( long *tx, long *ty, long *tz, long *rx, long *ry, long *rz )
{
   *rx /= sb_pitch_sens;
   *ry /= sb_rot_sens;
   *rz /= sb_rot_sens;
   *tx /= sb_tran_sens;
   *ty /= sb_tran_sens;
   *tz /= sb_jump_sens;
}

void FilterSpaceballDataSquare( long *tx, long *ty, long *tz, long *rx, long *ry, long *rz )
{

   *rx = sign( *rx ) * isqr( *rx ) / sb_pitch_sens;
   *ry = icube( *ry ) / sb_rot_sens;
   *rz = sign( *rz ) * isqr( *rz ) / sb_rot_sens;
   *tx = sign( *tx ) * isqr( *tx ) / sb_tran_sens;
   *ty = sign( *ty ) * isqr( *ty ) / sb_tran_sens;
   *tz = sign( *tz ) * isqr( *tz ) / sb_jump_sens;

}

void FilterSpaceballDataFloating( long *tx, long *ty, long *tz, long *rx, long *ry, long *rz )
{
   long max;

   max = abs( *tx );

   if ( abs( *ty ) > max )
      max = abs( *ty );

   if ( abs( *tz ) > max )
      max = abs( *tz );

   if ( abs( *rx ) > max )
      max = abs( *rx );

   if ( abs( *ry ) > max )
      max = abs( *ry );

   if ( abs( *rz ) > max )
      max = abs( *rz );

   *tx = (( *tx * ( 0x8000 - ( max - abs(*tx) ) * sb_float_sens )) >> 16 )
       / sb_tran_sens;

   *ty = (( *ty * ( 0x8000 - ( max - abs(*ty) ) * sb_float_sens )) >> 16 )
       / sb_tran_sens;

   *tz = (( *tz * ( 0x8000 - ( max - abs(*tz) ) * sb_float_sens )) >> 16 )
       / sb_jump_sens;

   *rx = (( *rx * ( 0x8000 - ( max - abs(*rx) ) * sb_float_sens )) >> 16 )
       / sb_pitch_sens;

   *ry = (( *ry * ( 0x8000 - ( max - abs(*ry) ) * sb_float_sens )) >> 16 )
       / sb_rot_sens;

   *rz = (( *rz * ( 0x8000 - ( max - abs(*rz) ) * sb_float_sens )) >> 16 )
       / sb_rot_sens;


}

// *** WARNING *** WARNING *** WARNING *** WARNING *** WARNING *** WARNING *** WARNING *** WARNING *** WARNING *** WARNING ***
// These #defines are also used in panels.c - if your change them here be sure to change them there!

#define SB_FILTER_ORIG 0      // original SB input by Looking glass
#define SB_FILTER_SIMPLE 1    // Using a simple sensitivity value to map values
#define SB_FILTER_SQUARE 2    // Square the SB data to give emphasis to larger values and de-emphasize low values
#define SB_FILTER_FLOATING 3  // Floating sensitivity depending upon input from SB

void FilterSpaceballDataOrig( long *tx, long *ty, long *tz, long *rx, long *ry, long *rz )
{

   if ( abs( *tx ) > TRA_TOL ) {

      if ( *tx > TRA_TOL ) 
    *tx = Tra_Scale( *tx - TRA_TOL ); 
      else 
    *tx = Tra_Scale( *tx + TRA_TOL ); 

   }

   else 
      *tx = 0;

   if ( abs( *ty ) > TRA_TOL ) {

      if ( *ty > TRA_TOL ) 
    *ty = Tra_Scale( *ty - TRA_TOL ); 
      else 
    *ty = Tra_Scale( *ty + TRA_TOL ); 

   }

   else 
      *ty = 0;

   if ( abs( *tz ) > TRA_TOL ) {

      if ( *tz > TRA_TOL ) 
    *tz = Tra_Scale( *tz - TRA_TOL ); 
      else 
    *tz = Tra_Scale( *tz + TRA_TOL );

   } 
   else 
      *tz = 0;

   if ( *rx > ROT_TOL ) 
      *rx = Rot_Scale( *rx - ROT_TOL ); 
   else if ( *rx < -ROT_TOL ) 
      *rx = Rot_Scale( *rx + ROT_TOL ); 
   else 
      *rx = 0;

   if ( *ry > ROT_TOL ) 
      *ry = Rot_Scale( *ry - ROT_TOL ); 
   else if ( *ry < -ROT_TOL ) 
      *ry = Rot_Scale( *ry + ROT_TOL ); 
   else 
      *ry = 0;

   if ( *rz > ROT_TOL ) 
      *rz = Rot_Scale( *rz - ROT_TOL ); 
   else if ( *rz < -ROT_TOL ) 
      *rz = Rot_Scale( *rz + ROT_TOL ); 
   else 
      *rz = 0;

}

static void (*SbFilterFunc)( long *, long *, long *, long *, long *, long * ) = FilterSpaceballDataSquare;

void SetSbFilterFunc( int val )
{

   switch( val ) {

   case SB_FILTER_ORIG:
      SbFilterFunc = FilterSpaceballDataOrig;
      break;

   case SB_FILTER_SIMPLE:
      SbFilterFunc = FilterSpaceballDataSimple;
      break;

   case SB_FILTER_SQUARE:
      SbFilterFunc = FilterSpaceballDataSquare;
      break;

   case SB_FILTER_FLOATING:
      SbFilterFunc = FilterSpaceballDataFloating;
      break;;

   }
}

#define JUMP_MULTIPLE 20

sball_jump_filter( int jump_val )
{
   static int old_val = 0;
   static int count = 0;


// mprintf( "Jump value %d ", jump_val );

   // If the jump value is really a duck value then bag outta here
   if ( jump_val < 0 )
      return jump_val;

/* if ( count > 10 ) {

      count = old_val = 0;
      return jump_val;

   }
*/

   // Make the jump value be a multiple of JUMP_MULTIPLE
   jump_val = ( jump_val / JUMP_MULTIPLE ) * JUMP_MULTIPLE;

// mprintf( "%d\n", jump_val );

   // If the jump value is in the same bin as the old duck value then
   // we assume the is the jump value we want and return the value
   if ( old_val == jump_val ) {

//    mprintf( "Stable value %d\n", jump_val );
      count = old_val = 0;
      return jump_val;

   }

   // If the new value is in a bin that is less than the old value then
   // (the person is letting up on the spaceball) then use the old value
   // as the jump value
   if ( jump_val < old_val ) {
      int ret;

//    mprintf( "Less value %d %d\n", old_val, jump_val );
      ret = old_val;
      count = old_val = 0;
      return ret;

   }

   old_val = jump_val;
   ++count;

   return 0;

}

#define PITCH_MAX_DELTA 15

int sb_pitch_constant = FALSE;

#define SAMPLE_SIZE 20

int FilterPitch( long *pitch )
{
   long i;
   static long ring_buf[ SAMPLE_SIZE ];
   static long ring_index = 0;

#if 0
   static long max_val = 0;
#endif
   long accum;

   if ( abs( *pitch ) < sb_pitch_thresh )
      *pitch = 0;

   else {

      if ( *pitch > 0 )
         *pitch -= sb_pitch_thresh;
      else  
         *pitch += sb_pitch_thresh;


   }

#if 0
   if ( sb_pitch_constant ) {

      if ( max_val > 0 && *pitch > 0 ) {

         if ( *pitch < max_val )
            *pitch = max_val;
         else
            max_val = *pitch;

         return FALSE;
      }


      if ( max_val < 0 && *pitch < 0 ) {

         if ( *pitch > max_val )
            *pitch = max_val;
         else
            max_val = *pitch;


         return FALSE;
      }

      max_val += *pitch;
      *pitch = max_val;

//    mprintf( "%d %d\n", max_val, *pitch );

      return FALSE;

   }
#endif

   ring_buf[ ring_index ] = *pitch;

   ++ring_index;
   if ( ring_index == SAMPLE_SIZE )
      ring_index = 0;

   accum = 0;
   for ( i = 0; i < SAMPLE_SIZE; i++ )
      accum += ring_buf[i];

   *pitch = accum / SAMPLE_SIZE;


   if ( *pitch )
      return TRUE;

   return FALSE;

}

void sball_chk(void)
{
   i6s_event *inp6d_in;
   float foo;
   long vals[6];

   static int count = 0;
   static int down_doo_be_doo = FALSE;
   static int did_a_jump = FALSE;
   static int doing_pitch = FALSE;

   // we'll want to put in code here to check for mouse_button_emulation
//   if (checking_mouse_button_emulation && <SOME CODE TO CHECK FOR BUTTON PRESS>)
//      mouse_button_emulated = TRUE;

   if (game_paused)
      return;

   if ( did_a_jump )
      physics_set_player_controls( INP6D_CONTROL_BANK, 0, 0, 0, 0, 0, 0 );

   inp6d_in=i6_poll();

   if (inp6d_in==NULL) {
	   if ( doing_pitch ) {
	      inp6d_in->x = 0;
	      inp6d_in->y = 0;
	      inp6d_in->z = 0;
	      inp6d_in->rx = 0;
	      inp6d_in->ry = 0;
	      inp6d_in->rz = 0;
	   }
	   else return;
   }

   vals[0] =  (float)inp6d_in->x * 0.9;
   vals[1] =  -inp6d_in->z;
   vals[2] =  inp6d_in->y;

   foo = -(float)inp6d_in->ry * 1.8;

   if ( foo > 32000 )
      vals[3] = 32000;
      else if ( foo < -32000 )
      vals[3] = -32000;
   else
      vals[3] = foo;
 
   vals[4] =  inp6d_in->rx;
   vals[5] = -(float)inp6d_in->rz * 0.9;

   if ( sb_major_axis )
      FilterSpaceballDataMajorAxis( vals );

   (*SbFilterFunc)( &vals[0], &vals[1], &vals[2], &vals[4], &vals[3], &vals[5] );
 

#ifdef PLAYTEST
   if (!inp6d_player)
   {
/*    mprintf("Parsed %d %d %d %d %d %d to %d %d %d %d %d %d\n",
      inp6d_in->x,  inp6d_in->y,  inp6d_in->z,  inp6d_in->rx,     
      inp6d_in->ry,     inp6d_in->rz,  vals[0], vals[1], vals[2], 
      vals[3], vals[4], vals[5] );
*/
      fr_camera_slewcam( NULL, EYE_X, vals[0] / 5 );
      fr_camera_slewcam( NULL, EYE_Y, vals[1] / 5 );
      fr_camera_slewcam( NULL, EYE_Z, vals[2] / 5 );
      fr_camera_slewcam( NULL, EYE_H, vals[3] / 3 );
      fr_camera_slewcam( NULL, EYE_P, vals[4] / 3 );
      fr_camera_slewcam( NULL, EYE_B, vals[5] / 3 );
   }
   else
#endif
   {

   if ( abs( vals[2] ) > sb_jump_thresh ) {

      if ( vals[2] > 0 )
         vals[2] -= sb_jump_thresh;
      else
         vals[2] += sb_jump_thresh;


   }
   else
      vals[2] = 0;


   doing_pitch = FilterPitch( &vals[4] );

   if ( abs( vals[0] ) > CONTROL_MAX_VAL )
      vals[0] = sign( vals[0] ) * CONTROL_MAX_VAL;
   if ( abs( vals[1] ) > CONTROL_MAX_VAL )
      vals[1] = sign( vals[1] ) * CONTROL_MAX_VAL;
   if ( abs( vals[2] ) > CONTROL_MAX_VAL )
      vals[2] = sign( vals[2] ) * CONTROL_MAX_VAL;
   if ( abs( vals[3] ) > CONTROL_MAX_VAL )
      vals[3] = sign( vals[3] ) * CONTROL_MAX_VAL;
   if ( abs( vals[4] ) > CONTROL_MAX_VAL )
      vals[4] = sign( vals[4] ) * CONTROL_MAX_VAL;
   if ( abs( vals[5] ) > CONTROL_MAX_VAL )
      vals[5] = sign( vals[5] ) * CONTROL_MAX_VAL;

   vals[2] = sball_jump_filter( vals[2] );
   if ( vals[2] > 0 )
      did_a_jump = TRUE;


/*    mprintf("%d Parsed %d %d %d %d %d %d to %d %d %d %d %d %d\n", count,
      inp6d_in->x,  inp6d_in->y,  inp6d_in->z,  inp6d_in->rx,     
      inp6d_in->ry,     inp6d_in->rz,  vals[0], vals[1], vals[2], 
      vals[3], vals[4], vals[5] );
*/
   ++count;

   
   if ( sb_pitch_angles )
      vals[4] = ( vals[4] / sb_pitch_div ) * sb_pitch_div;
   

   if ( vals[2] >= 0 ) {

      if ( down_doo_be_doo ) {
         player_set_posture( POSTURE_STAND );
         down_doo_be_doo = FALSE;
      }


      physics_set_player_controls( INP6D_CONTROL_BANK, vals[0], 
         vals[1], vals[2], vals[3], 0,0 );


   }

   else {

      physics_set_player_controls( INP6D_CONTROL_BANK, vals[0], 
         vals[1], 0, vals[3], 0,0 );

      if ( abs( vals[2] ) < sb_crouch_thresh ) {
//       mprintf( "Stand\n" );
         player_set_posture( POSTURE_STAND );
         down_doo_be_doo = FALSE;
      }

      else if ( abs( vals[2] ) < sb_prone_thresh * 2 ) {
//       mprintf( "Stoop!\n" );
         player_set_posture( POSTURE_STOOP );
         down_doo_be_doo = TRUE;
      }

      else {
//       mprintf( "Prone\n" );
         player_set_posture( POSTURE_PRONE );
         down_doo_be_doo = TRUE;
      }

   }

   player_set_lean(vals[5],0);
   player_set_eye( vals[4] );


   }
}
#endif


#endif //NOT_YET


bool main_kb_callback(uiEvent *h, LGRegion *r, void *udata)
{

   LGRegion *dummy2;
   void *dummy3;
   dummy2 = r;
   dummy3 = udata;
   
#ifdef INPUT_CHAINING
   kb_flush_bios();
#endif // INPUT_CHAINING

   if (h->type == UI_EVENT_KBD_COOKED) 
      return hotkey_dispatch(h->subtype) == OK;
   return FALSE;
}


bool posture_hotkey_func(short keycode, ulong context, void* data)
{
#ifndef NO_DUMMIES
   ulong dummy; dummy = context + keycode;
#endif 
   return player_set_posture((ubyte)data) == OK;
}


bool eye_hotkey_func(short, ulong , int data)
{
   extern void player_set_eye(byte);
   extern byte player_get_eye(void);
   byte eyectl = player_get_eye();
   int r=1+(player_struct.drug_status[DRUG_REFLEX] > 0 && !global_fullmap->cyber);

   if (data == 0)
   {
      player_set_eye(0);
      return TRUE;
   }
   for(;r>0;r--) {
      if (data < 0)
      {
         if (eyectl > 0)
            eyectl = 0;
         else eyectl = (eyectl - CONTROL_MAX_VAL)/3;
      }
      else
      {
         if (eyectl < 0)
            eyectl = 0;
         else eyectl = (eyectl + CONTROL_MAX_VAL)/3;
      }
   }
   player_set_eye(eyectl);
   return TRUE;

}


#define EYE_POLLING
#ifndef EYE_POLLING
static ushort eye_up_keys[] =
   {
      KEY_UP|KB_FLAG_SHIFT,
      KEY_PAD_UP|KB_FLAG_SHIFT,
      'r',
      'R',
   };

#define NUM_EYE_UP_KEYS (sizeof(eye_up_keys)/sizeof(ushort))
                                 
static ushort eye_dn_keys[] =
   {
      KEY_DOWN|KB_FLAG_SHIFT,
      KEY_PAD_DOWN|KB_FLAG_SHIFT,
      'v',
      'V',
   };

#define NUM_EYE_DN_KEYS (sizeof(eye_dn_keys)/sizeof(ushort))
#endif // !EYE_POLLING

static ushort eye_lvl_keys[] =
   {
      'f',
      'F',
   };

#define NUM_EYE_LVL_KEYS (sizeof(eye_lvl_keys)/sizeof(ushort))
// -------------------------------------
// INITIALIZATION
extern bool reload_weapon_hotkey(short keycode, ulong context, void* data);
extern errtype simple_load_res_bitmap_cursor(LGCursor* c, grs_bitmap* bmp, Ref rid);
extern bool unpause_game_func(short keycode, ulong context, void* data);
extern bool saveload_hotkey_func(short keycode, ulong context, void* data);
#ifdef AUDIOLOGS
extern bool audiolog_cancel_func(short keycode, ulong context, void* data);
#endif
extern bool keyhelp_hotkey_func(short keycode, ulong context, void* data);
extern bool demo_quit_func(short keycode, ulong context, void* data);
extern bool hud_color_bank_cycle(short keycode, ulong context, void* data);
extern void init_side_icon_hotkeys(void);
extern void init_invent_hotkeys(void);
extern bool toggle_view_func(short keycode, ulong context, void* data);
bool toggle_profile(short keycode, ulong context, void* data);
#ifdef PLAYTEST
extern bool automap_seen(short keycode, ulong context, void* data);
extern bool maim_player(short keycode, ulong context, void* data);
extern bool salt_the_player(short keycode, ulong context, void* data);
extern bool give_player_hotkey(short keycode, ulong context, void *data);
extern bool change_clipper(short keycode, ulong context, void *data);
#endif
#ifndef PLAYTEST
extern bool version_spew_func(short keycode, ulong context, void* data);
extern bool location_spew_func(short keycode, ulong context, void* data);
#endif

#define ckpoint_input(val) Spew(DSRC_TESTING_Test0,("ii %s @%d\n",val,*tmd_ticks));

#define CYB_CURS_ID(i)  (CYBER_CURSOR_BASE+(i))
#define REAL_CURS_ID(i) (motion_cursor_ids[i])

void reload_motion_cursors(bool cyber)
{
   int i;
   extern short cursor_color_offset;

//KLC   uiHideMouse(NULL);
   if (!cyber)
   {
      for (i = 0; i < NUM_MOTION_CURSORS; i++)
      {
    	   grs_bitmap* bm = &motion_cursor_bitmaps[i];
         if (REAL_CURS_ID(i) != 0)
         {
      	   load_hires_bitmap_cursor(&motion_cursors[i],bm,REAL_CURS_ID(i),FALSE);
         }
      }

      // slam the cursor color back to it's childhood colors
      cursor_color_offset = RED_BASE+4;
   }
   else 
      for (i = 0; i < NUM_CYBER_CURSORS; i++)
      {
         grs_bitmap* bm = &motion_cursor_bitmaps[i];
         load_hires_bitmap_cursor(&motion_cursors[i],bm,CYB_CURS_ID(i),FALSE);
      }
//KLC   uiShowMouse(NULL);
}

void free_cursor_bitmaps()
{
   int i=0;
   for (; i < max(NUM_MOTION_CURSORS,NUM_CYBER_CURSORS); i++)
   {
      grs_bitmap* bm = &motion_cursor_bitmaps[i];
      if (bm->bits!=NULL)
	      DisposePtr((Ptr)bm->bits);
   }
}

void alloc_cursor_bitmaps(void)
{
   int i;
   short w,h;
   for (i = 0; i < min(NUM_MOTION_CURSORS,NUM_CYBER_CURSORS); i++)
   {
      int cybsz;
      int realsz = 0;
      grs_bitmap* bm = &motion_cursor_bitmaps[i];
      w = res_bm_width(CYB_CURS_ID(i));
      h = res_bm_height(CYB_CURS_ID(i));
//      ss_point_convert(&w,&h,FALSE);
      cybsz = w * h;
      
      if (REAL_CURS_ID(i) != 0)
      {
         w = res_bm_width(REAL_CURS_ID(i));
         h = res_bm_height(REAL_CURS_ID(i));
//         ss_point_convert(&w,&h,FALSE);
         realsz = w * h;
      }

      bm->bits = (uchar *)NewPtr(max(cybsz,realsz));
   }
   for (; i < max(NUM_MOTION_CURSORS,NUM_CYBER_CURSORS); i++)
   {
      grs_bitmap* bm = &motion_cursor_bitmaps[i];
      int sz =  0;
      if (REAL_CURS_ID(i) == 0)
      {
         bm->bits=NULL;    // lets check this
         continue;
      }
      else
      {
         w = res_bm_width(REAL_CURS_ID(i));
         h = res_bm_height(REAL_CURS_ID(i));
//         ss_point_convert(&w,&h,FALSE);
         sz = w * h;
      }
      bm->bits = (uchar *)NewPtr(sz);
   }
}


#include "frtypes.h"
extern Boolean	gPlayingGame;
extern Boolean	DoubleSize;
extern Boolean	SkipLines;
extern void change_svga_screen_mode(void);
Boolean	gShowFrameCounter = FALSE;
Boolean	gShowMusicGlobals = FALSE;

bool MacQuitFunc(short , ulong , void*)
{
	if (*tmd_ticks > (gGameSavedTime + (5 * CIT_CYCLE)))		// If the current game needs saving...
	{
		short					btn;
		Boolean				savedOK;
		ModalFilterUPP	stdFilterProcPtr;
		
		uiHideMouse(NULL);								// Setup the environment for doing Mac stuff.
		ShowCursor();
		CopyBits(&gMainWindow->portBits, &gMainOffScreen.bits->portBits, &gActiveArea, &gOffActiveArea, srcCopy, 0L);

		ShowMenuBar();
		stdFilterProcPtr = NewModalFilterProc(ShockAlertFilterProc);
 		btn = Alert((global_fullmap->cyber) ? 1010 :1009, stdFilterProcPtr);		// Want to save it first?
		DisposeRoutineDescriptor(stdFilterProcPtr);
		HideMenuBar();
 		
		SetPort(gMainWindow);							// Update area behind the alert
		BeginUpdate(gMainWindow);
  		CopyBits(&gMainOffScreen.bits->portBits, &gMainWindow->portBits, &gOffActiveArea, &gActiveArea, srcCopy, 0L);
		EndUpdate(gMainWindow);

		if (global_fullmap->cyber)						// In cyberspace, all you can do is end the game
		{															// or just keep playing.
			if (btn == 1)
				gPlayingGame = FALSE;
		}
		else														// If in normal space, save the game first.
		{
	 		switch(btn)
	 		{
	 			case 1:											// Yeah, save it
					if (gIsNewGame)
					{
						ShowMenuBar();
					 	savedOK = DoSaveGameAs();
						HideMenuBar();
					}
					else
						savedOK = DoSaveGame();
					if (!savedOK)
						break;
	 			case 2:											// No, don't save it
					gPlayingGame = FALSE;
	 				break;
			}
		}
		
		HideCursor();								// go back to Shock.
		uiShowMouse(NULL);
	}
	else
		gPlayingGame = FALSE;
	return TRUE;
}

bool MacResFunc(short , ulong , void *)
{
 	DoubleSize = !DoubleSize;
	change_svga_screen_mode();
	
	if (DoubleSize)
		message_info("Low res.");
	else
	{
		message_info("High res.");
		SkipLines = FALSE;
	}
	gShockPrefs.doResolution = (DoubleSize) ? 1 : 0;	// KLC - Yeah, got to update this one too
	gShockPrefs.doUseQD = SkipLines;						// KLC - and this one
	SavePrefs(kPrefsResID);									// KLC - and save the prefs out to disk.
	
	return TRUE;
}

bool MacSkiplinesFunc(short , ulong , void *)
{
	if (!DoubleSize)							// Skip lines only applies in double-size mode.
	{
		message_info("Skip lines works only in low-res mode.");
		return FALSE;
	}
	SkipLines = !SkipLines;
	gShockPrefs.doUseQD = SkipLines;
	SavePrefs(kPrefsResID);
	return TRUE;
}

bool MacDetailFunc(short , ulong , void *)
{
	char	msg[32];
	char	detailStr[8];
	fauxrend_context *_frc = (fauxrend_context *) svga_render_context;
	
	if (_frc->detail == 4)								// Adjust for that global detail nonsense.
		_frc->detail = _fr_global_detail;

	_frc->detail++; 									// Cycle through the detail levels.
	if (_frc->detail>=4)
		_frc->detail = 0;
	_fr_global_detail = _frc->detail;				// Update the global guy.

	gShockPrefs.doDetail = _frc->detail;			// Update and save our prefs.
	SavePrefs(kPrefsResID);

	switch(_frc->detail)								// Show a nice, informative message.
	{
		case 0:
			strcpy(detailStr, "Min");
			break;
		case 1:
			strcpy(detailStr, "Low");
			break;
		case 2:
			strcpy(detailStr, "High");
			break;
		case 3:
			strcpy(detailStr, "Max");
	}
	sprintf(msg, "Detail level: %s",detailStr);
 	message_info(msg);
	return TRUE;
}

/*
// Temporary function.  Remove for final build

bool temp_FrameCounter_func(short , ulong , void *)
{
	gShowFrameCounter = !gShowFrameCounter;
	
	if (gShowFrameCounter)
		message_info("Frame counter on.");
	else
		message_info("Frame counter off.");
}

// end temp functions
*/

bool MacHelpFunc(short , ulong , void*)
{
	if (music_on)									// Setup the environment for doing Mac stuff.
		MacTuneKillCurrentTheme();
	uiHideMouse(NULL);
	status_bio_end();
	CopyBits(&gMainWindow->portBits, &gMainOffScreen.bits->portBits, &gActiveArea, &gOffActiveArea, srcCopy, 0L);
	ShowMenuBar();
	ShowCursor();

	ShowShockHelp();
 		
	SetPort(gMainWindow);							// Update area behind the alert
	BeginUpdate(gMainWindow);
	CopyBits(&gMainOffScreen.bits->portBits, &gMainWindow->portBits, &gOffActiveArea, &gActiveArea, srcCopy, 0L);
	EndUpdate(gMainWindow);

	HideCursor();										// go back to Shock.
	HideMenuBar();
	uiShowMouse(NULL);
	status_bio_start();
	if (music_on)
		MacTuneStartCurrentTheme();

	return TRUE;
}


void init_input(void)
{
	extern void init_motion_polling();
	int i = 0;
//KLC	int kbdt, joy_type;
	int dvec[2];
	
	// init keyboard
//KLC	for (i = 0; i < 0x80; i++)
//KLC	kb_clear_state(i,KBA_REPEAT);
	hotkey_init(NUM_HOTKEYS);

/* KLC
   kbdt=dt_keyboard();
   if (kbdt!=kb_get_country())
   {
	   kb_set_country(kbdt);
      Warning(("Setting kb country to %d\n",kbdt));
   }
*/
   init_motion_polling();

   // init mouse
//KLC   mouse_set_timestamp_register((ulong*)tmd_ticks);
   dvec[0] = 10; //KLC 30;		// default double click deleay;
   dvec[1] = 45; // 175;	// default double click time
   i = 2;
//KLC   config_get_value(CFG_DCLICK_RATE_VAR,CONFIG_INT_TYPE,dvec,&i);
   uiDoubleClickDelay = dvec[0];
   uiDoubleClickTime = dvec[1];
   uiDoubleClicksOn[MOUSE_LBUTTON] = TRUE; // turn on left double clicks
   uiAltDoubleClick = TRUE;
   i = 1;
   uiSetMouseMotionPolling(TRUE);
   
   // Load cursors
   
   alloc_cursor_bitmaps();
   reload_motion_cursors(FALSE);


// GAME HOTKEYS

// MFDs
   keyboard_init_mfd();

// Game wrapper hotkeys
   // these are in all versions, playtest and not
   hotkey_add(CONTROL('f'), DEMO_CONTEXT, change_mode_func,  (void *)FULLSCREEN_LOOP);
#ifdef AUDIOLOGS
   hotkey_add(CONTROL('.'), DEMO_CONTEXT, audiolog_cancel_func, NULL);
#endif
   hotkey_add(CONTROL('s'), DEMO_CONTEXT, save_hotkey_func, NULL);
   hotkey_add(CONTROL('S'), DEMO_CONTEXT, save_hotkey_func, NULL);
/*KLC - not in Mac version
   hotkey_add(CONTROL('l'), DEMO_CONTEXT, saveload_hotkey_func, (void *)TRUE);
   hotkey_add(CONTROL('L'), DEMO_CONTEXT, saveload_hotkey_func, (void *)TRUE);

   hotkey_add('?', DEMO_CONTEXT, keyhelp_hotkey_func, NULL);

   hotkey_add('/',DEMO_CONTEXT,toggle_bool_func,&joystick_mouse_emul);
*/
   hotkey_add(ALT(KEY_BS), DEMO_CONTEXT, reload_weapon_hotkey, (void*)0);
   hotkey_add(CONTROL(KEY_BS), DEMO_CONTEXT, reload_weapon_hotkey, (void*)1);
   hotkey_add(ALT('\''), DEMO_CONTEXT, arm_grenade_hotkey, (void*)0);
   hotkey_add(CONTROL('\''), DEMO_CONTEXT, select_grenade_hotkey, (void*)0);
   hotkey_add(ALT(';'), DEMO_CONTEXT, use_drug_hotkey, (void*)0);
   hotkey_add(CONTROL(';'), DEMO_CONTEXT, select_drug_hotkey, (void*)0);


//#ifndef PLAYTEST
//   hotkey_add(DOWN(KEY_PRNTSCRN), EVERY_CONTEXT, gifdump_func, NULL);

   hotkey_add(DOWN(KEY_BS), DEMO_CONTEXT,clear_fullscreen_func, NULL);
   hotkey_add(ALT('h'), DEMO_CONTEXT, hud_color_bank_cycle, NULL);
   hotkey_add(ALT('H'), DEMO_CONTEXT, hud_color_bank_cycle, NULL);
   hotkey_add(CONTROL('m'), DEMO_CONTEXT, toggle_music_func, NULL);
   hotkey_add(CONTROL('M'), DEMO_CONTEXT, toggle_music_func, NULL);
//   hotkey_add(DOWN(KEY_SPACE),DEMO_CONTEXT,unpause_game_func,(void *)TRUE);   
   hotkey_add(DOWN('p'),DEMO_CONTEXT,pause_game_func,(void *)TRUE);
   hotkey_add(DOWN(KEY_ESC),DEMO_CONTEXT,pause_game_func,(void *)TRUE);
   for (i = 0; i < NUM_POSTURES; i++)
   {
      hotkey_add(DOWN(posture_keys[i]),DEMO_CONTEXT,posture_hotkey_func,(void*)i);
      hotkey_add(DOWN(toupper(posture_keys[i])),DEMO_CONTEXT,posture_hotkey_func,(void*)i);
   }
   hotkey_add(CONTROL('q'), DEMO_CONTEXT, MacQuitFunc, NULL);
   hotkey_add(CONTROL('1'), DEMO_CONTEXT, MacResFunc, NULL);
   hotkey_add(CONTROL('2'), DEMO_CONTEXT, MacSkiplinesFunc, NULL);
   hotkey_add(CONTROL('3'), DEMO_CONTEXT, MacDetailFunc, NULL);
   hotkey_add(CONTROL('/'), DEMO_CONTEXT, MacHelpFunc, NULL);
   hotkey_add(CONTROL('?'), DEMO_CONTEXT, MacHelpFunc, NULL);

/*���
   hotkey_add(ALT('x'),DEMO_CONTEXT,demo_quit_func,NULL);
   hotkey_add(ALT('x'),SETUP_CONTEXT,really_quit_key_func,NULL);
   hotkey_add(ALT('v'),DEMO_CONTEXT,toggle_view_func,NULL);
   hotkey_add(ALT('V'),DEMO_CONTEXT,toggle_view_func,NULL);
   hotkey_add(ALT(KEY_F7),DEMO_CONTEXT,version_spew_func,NULL);
*/
//   hotkey_add(CONTROL('8'),DEMO_CONTEXT,location_spew_func,NULL);  //testing
//   hotkey_add(CONTROL('0'),DEMO_CONTEXT,temp_FrameCounter_func, NULL); //testing

   hotkey_add(CONTROL('d'), DEMO_CONTEXT, change_mode_func,  (void *) GAME_LOOP);
   hotkey_add(CONTROL('D'), DEMO_CONTEXT, change_mode_func,  (void *) FULLSCREEN_LOOP);
   hotkey_add(CONTROL('a'), DEMO_CONTEXT, change_mode_func,  (void *) AUTOMAP_LOOP);
   hotkey_add(CONTROL('A'), DEMO_CONTEXT, change_mode_func,  (void *) AUTOMAP_LOOP);
/*���
#else
   hotkey_add(DOWN(KEY_SPACE),DEMO_CONTEXT,unpause_game_func,(void *)TRUE);   
   hotkey_add(CONTROL('a'), DEMO_CONTEXT, change_mode_func,  (void *) AUTOMAP_LOOP);
   hotkey_add(CONTROL('A'), DEMO_CONTEXT, change_mode_func,  (void *) AUTOMAP_LOOP);

   hotkey_add_help(DOWN(KEY_BS), DEMO_CONTEXT,clear_fullscreen_func, NULL,
      "Clears all overlays from the fullscreen view.");
   hotkey_add_help(DOWN(KEY_PAUSE),DEMO_CONTEXT,pause_game_func,(void *)TRUE, "pause the game, gee.");
   hotkey_add_help(DOWN('p'),DEMO_CONTEXT,pause_game_func,(void *)TRUE, "pause the game, gee.");
   hotkey_add_help(DOWN(KEY_ESC),DEMO_CONTEXT,wrapper_options_func,(void *)TRUE,
      "Opens up the options menu on the main game screen.");
   hotkey_add_help(CONTROL('h'), DEMO_CONTEXT, hud_color_bank_cycle, NULL, "cycle hud colors");
   hotkey_add_help(CONTROL('H'), DEMO_CONTEXT, hud_color_bank_cycle, NULL, "cycle hud colors");
   hotkey_add_help(CONTROL(ALT('~')),EVERY_CONTEXT,toggle_profile,(void *)TRUE, "toggle profile");

   for (i = 0; i < NUM_POSTURES; i++)
   {
      hotkey_add_help(DOWN(posture_keys[i]),DEMO_CONTEXT,posture_hotkey_func,(void*)i,"change posture");
      hotkey_add_help(DOWN(toupper(posture_keys[i])),DEMO_CONTEXT,posture_hotkey_func,(void*)i,"change posture");
   }
   hotkey_add_help(ALT('x'),EDIT_CONTEXT|SETUP_CONTEXT,quit_key_func,NULL,"quit, but ask for confirm.");
   hotkey_add_help(ALT('x'),DEMO_CONTEXT,demo_quit_func,NULL,"quit, but ask for confirm on options panel.");
   hotkey_add_help(ALT('X'),EVERY_CONTEXT,really_quit_key_func,NULL, "quit, no questions asked.");
   hotkey_add_help(ALT('v'),EVERY_CONTEXT,toggle_view_func,NULL, "toggles between game mode and fullscreen mode.");
   hotkey_add_help(ALT('V'),EVERY_CONTEXT,toggle_view_func,NULL, "toggles between game mode and fullscreen mode.");
   hotkey_add(ALT('d'),EVERY_CONTEXT,mono_config_func,NULL);

   // these are some random hotkeys - debugging
   hotkey_add(CONTROL('q'),EVERY_CONTEXT,maim_player,"maim player");
   hotkey_add(CONTROL('z'),EVERY_CONTEXT,change_clipper,"maim player");
   hotkey_add(ALT(KEY_F2),EVERY_CONTEXT,salt_the_player, "Salt the Player");
   hotkey_add(ALT(KEY_F3),EVERY_CONTEXT,give_player_hotkey, "Give Player Loot");

   // Meta-slewing
   hotkey_add(DOWN('r'), EDIT_CONTEXT, stupid_slew_func, (void *)13);
   hotkey_add(DOWN('>'), DEMO_CONTEXT|EDIT_CONTEXT, stupid_slew_func, (void *)14);
   hotkey_add(DOWN('<'), DEMO_CONTEXT|EDIT_CONTEXT, stupid_slew_func, (void *)15);

   // 3d zoomin
   hotkey_add(CONTROL('['), DEMO_CONTEXT|EDIT_CONTEXT, zoom_3d_func, (void *)TRUE);
   hotkey_add(CONTROL(']'), DEMO_CONTEXT|EDIT_CONTEXT, zoom_3d_func, (void *)FALSE);

   ckpoint_input("hotkeys in");

//   hotkey_add(DOWN('['),EDIT_CONTEXT,zoom_func,(void *)ZOOM_IN);
//   hotkey_add(DOWN(']'),EDIT_CONTEXT,zoom_func,(void *)ZOOM_OUT);
//   hotkey_add(CONTROL('d'),EDIT_CONTEXT,to_demo_mode_func,NULL);
   
#endif
   hotkey_add(KEY_F11,DEMO_CONTEXT|EDIT_CONTEXT,change_gamma,(void *) 1);
   hotkey_add(KEY_F12,DEMO_CONTEXT|EDIT_CONTEXT,change_gamma,(void *)-1);
*/
   hotkey_add(CONTROL('h'),DEMO_CONTEXT,toggle_olh_func,NULL);
   hotkey_add(CONTROL('H'),DEMO_CONTEXT,toggle_olh_func,NULL);
   hotkey_add(ALT('o'),DEMO_CONTEXT,olh_overlay_func,&olh_overlay_on);
   hotkey_add(ALT('O'),DEMO_CONTEXT,olh_overlay_func,&olh_overlay_on);
/*
   // take these ifdefs out if memory bashing on shippable
//   hotkey_add(ALT(CONTROL(KEY_F4)),EVERY_CONTEXT,texture_annihilate_func,NULL);
#ifdef RCACHE_TEST
   hotkey_add(ALT(KEY_F4),EVERY_CONTEXT,res_cache_usage_func,(void *)TRUE);
   hotkey_add(CONTROL(KEY_F4),EVERY_CONTEXT,res_cache_usage_func,(void *)FALSE);
#endif
   init_side_icon_hotkeys();
*/
   init_invent_hotkeys();

   for (i = 0; i < NUM_EYE_LVL_KEYS; i++)
   {
      hotkey_add(DOWN(eye_lvl_keys[i]),DEMO_CONTEXT,(hotkey_callback)eye_hotkey_func,(void*)0);
   }

/* KLC - stuff for VR headsets
   if (config_get_raw(CFG_INP6D_GO,NULL,0))
   {
	   ckpoint_input("inp6d start");

      // hack for these config variables, not sure where else to put them
      inp6d_hdouble = config_get_raw("inp6d_hdouble",NULL,0);
      inp6d_pdouble = config_get_raw("inp6d_pdouble",NULL,0);
      inp6d_bdouble = config_get_raw("inp6d_bdouble",NULL,0);

#if defined(VFX1_SUPPORT)||defined(CTM_SUPPORT)||defined(SPACEBALL_SUPPORT)
	   inp6d_exists=(i6_probe()==0 && i6_startup()==0);
      if ((config_get_raw("inp6d_force",NULL,0)))
        inp6d_exists=(i6_force(I6D_VFX1) == 0);
#else
	   inp6d_exists=(i6_probe_small()==0 && i6_startup()==0);
#endif
#if defined(VFX1_SUPPORT)||defined(CTM_SUPPORT)
      if ((i6d_device==I6D_VFX1)||(i6d_device==I6D_CTM)||(i6d_device==I6D_ALLPRO))
      {
         extern bool fullscrn_vitals, fullscrn_icons;
         i6s_event *inp6d_geth;
         do {
            inp6d_geth=i6_poll();
         } while (inp6d_geth==NULL);

         {
            hotkey_add(ALT('g'),DEMO_CONTEXT|EDIT_CONTEXT,recenter_headset,NULL);
            inp6d_headset=TRUE;
	         tracker_initial_pos[0]= inp6d_geth->ry;
	         tracker_initial_pos[1]= inp6d_geth->rx;
	         tracker_initial_pos[2]=-inp6d_geth->rz;
	         fullscrn_vitals=fullscrn_icons=FALSE;
            if (i6_video(I6VID_STARTUP,NULL))               
               Warning(("Headset video startup failed\n"));
   	      if ((config_get_raw("inp6d_stereo",NULL,0))&&(i6d_device!=I6D_ALLPRO))
            {
               int cnt=1, rval[1];
	            if (i6_video(I6VID_STR_START,NULL))
	               Warning(("Headset stereo startup failed\n"));
               else
                { inp6d_stereo=TRUE; inp6d_stereo_active=FALSE; }
               config_get_value("inp6d_stereo",CONFIG_INT_TYPE,rval,&cnt);
               if (cnt>0) inp6d_stereo_div=rval[0];
            }
    	      if (config_get_raw("inp6d_doom",NULL,0))
               inp6d_doom=TRUE;
         }
      }     // end of is it a tracker....
#endif
	   ckpoint_input("inp6d end");
   } else inp6d_exists=FALSE;
   {
      int cnt=1, rval[1];
      config_get_value("joystick",CONFIG_INT_TYPE,rval,&cnt);
      if (cnt>0)
      {
         extern ushort wrap_joy_type;
         extern ushort high_joy_flags;
         joy_type=rval[0];
         wrap_joy_type  = joy_type & ~JOY_NO_NGP;
         high_joy_flags = joy_type & JOY_NO_NGP;
      }
   }
   if (joystick_count=joy_init(joy_type))
   {
#ifdef PLAYTEST
      mprintf("Got %d joystick pots\n",joystick_count);
#endif
      hotkey_add(ALT('j'),DEMO_CONTEXT|EDIT_CONTEXT,recenter_joystick,NULL);
   }
 */
}

void shutdown_input(void)
{
   hotkey_shutdown();
   kb_flush_bios();

//   kb_clear_state(0x1d, 3);
//   kb_clear_state(0x9d, 3);
//   kb_clear_state(0x38, 3);
//   kb_clear_state(0xb8, 3);
}


// ------------------------
// 3D VIEW/MOTION INTERFACE
// ------------------------

// -------
// DEFINES
// -------

#define VIEW_LSIDE      0 
#define VIEW_HCENTER    1
#define VIEW_RSIDE      2

#define VIEW_TOP        0
#define VIEW_BOTTOM     4
#define VIEW_VCENTER    8 
#define VIEW_WAYTOP     12

#define CYBER_VIEW_TOP     0
#define CYBER_VIEW_CENTER  3
#define CYBER_VIEW_BOTTOM  6


#define CENTER_WD_N 1 
#define CENTER_WD_D 8
#define CYBER_CENTER_WD_D 6
#define CENTER_HT_N 1
#define CENTER_HT_D 8    
#define CYBER_CENTER_HT_D 6


// -------
// GLOBALS
// -------

short object_on_cursor = 0;
LGCursor object_cursor;

// ------------------------------------------------------------------------------
// view3d_rightbutton_handler deals with firing/throwing objects in 3d.  

bool mouse_jump_ui = TRUE;
bool fire_slam = FALSE;
bool left_down_jump = FALSE;

void reset_input_system(void)
{
   if (fire_slam)
   {
	  if (full_game_3d)
      	uiPopSlabCursor(&fullscreen_slab);
      else
      	uiPopSlabCursor(&main_slab);
      fire_slam = FALSE;
   }
   mouse_unconstrain();
}

#define DROP_REGION_Y(reg) ((reg)->abs_y + 7*RectHeight((reg)->r)/8)
bool weapon_button_up = TRUE;


// ---------
// INTERNALS
// ---------

// -------------------------------------------------------------------------------------------
// view3d_mouse_input sets/unsets physics controls based on mouse position in 3d 

// return whether any control was applied
int view3d_mouse_input(LGPoint pos, LGRegion* reg,bool move,int* lastsect)
{  // do we really recompute these every frame?? couldnt we have a context or something... something, a call to reinit, something
   static int dougs_goofy_hack=FALSE;

	int cnum = 0;
   byte xvel = 0;
   byte yvel = 0;
   byte xyrot = 0;
   bool thrust = FALSE;
   bool cyber = global_fullmap->cyber && time_passes;

   short   cx, cy, cw, ch, x, y;

   if (DoubleSize)
   {
      pos.x *= 2;
      pos.y *= 2;
   }
   
   if (!cyber)
   {
      cx = reg->abs_x + RectWidth(reg->r)/2;
      cy = reg->abs_y + 2*RectHeight(reg->r)/3;
      cw = RectWidth(reg->r)*CENTER_WD_N/CENTER_WD_D;
      ch = RectHeight(reg->r)*CENTER_HT_N/CENTER_HT_D;
   }
   else
   {
      cx = reg->abs_x + RectWidth(reg->r)/2;
      cy = reg->abs_y + RectHeight(reg->r)/2;
      cw = RectWidth(reg->r)*CENTER_WD_N/CYBER_CENTER_WD_D;
      ch = RectHeight(reg->r)*CENTER_HT_N/CYBER_CENTER_HT_D;
   }
#ifdef SVGA_SUPPORT
   ss_point_convert(&(cx),&(cy),FALSE);
   ss_point_convert(&(cw),&(ch),FALSE);
#endif
	x  = pos.x - cx;
	y  = pos.y - cy;

   // ok, the idea here is to make sure single left click doesnt move, or at least tells you whats up...
   if ((dougs_goofy_hack==FALSE)&&move)
    { dougs_goofy_hack=TRUE; move=FALSE; }
   else if (!move)
      dougs_goofy_hack=FALSE;

	if (x < -cw)
   {
      cnum = VIEW_LSIDE;
      if (move)
         xyrot = (x + cw)*100/(cx - cw - reg->abs_x);
   }
	else if (x > cw)
   {
      cnum = VIEW_RSIDE;
      if (move)
         xyrot =  (x - cw)*100/(cx - cw - reg->abs_x);
   }
	else
      cnum = VIEW_HCENTER;

   if (cyber)
   {
      if (y < -ch)
      {
         if (move)
            yvel = -(-ch - y)*CONTROL_MAX_VAL/(cy - ch - reg->abs_y);
         cnum += CYBER_VIEW_TOP;
      }
      else if (y > ch)
      {
         cnum += CYBER_VIEW_BOTTOM;
         if (move)
         {
#ifdef CYBER_ROLL_REGION
	         if (xyrot == 0)
#endif // CYBER_ROLL_REGION
	            yvel = -(ch - y)*CONTROL_MAX_VAL/(cy - ch - reg->abs_y);
#ifdef CYBER_ROLL_REGION
	         else
	         {
	            xvel = xyrot;
	            xyrot = 0;
	         }
#endif // CYBER_ROLL_REGION
         }
      }
      else
      {
         if ((thrust=((cnum== VIEW_HCENTER) && move)) == TRUE)
            physics_set_one_control(MOUSE_CONTROL_BANK,CONTROL_ZVEL,MAX_JUMP_CONTROL);

         cnum += CYBER_VIEW_CENTER;
      }
   }
   else
   {
	   if (y < -ch)
      {
         short ycntl = (-ch - y)*CONTROL_MAX_VAL/(cy - ch - reg->abs_y);
         if (move)
         {
	         int f = PLAYER_FATIGUE;
	         if (ycntl + f > CONTROL_MAX_VAL)
	         {  // compute new mouse cursor position
	            int newy;
	            f = max(CONTROL_MAX_VAL-f,SPRINT_CONTROL_THRESHOLD);
	            newy = f*(ch+reg->abs_y-cy)/CONTROL_MAX_VAL - ch +cy;
	            ycntl = (ycntl + f)/2;
	            // put the cursor between here and there
	            if (newy > pos.y)
	               mouse_put_xy(pos.x,newy);
	         }
	         yvel = ycntl;
         }

         if (ycntl > SPRINT_CONTROL_THRESHOLD)
	         cnum |= VIEW_WAYTOP;
         else
            cnum |= VIEW_TOP;

      }
	   else if (y > ch)
      {
         cnum |= VIEW_BOTTOM;
         if (move)
         {
	         if (xyrot == 0)
	            yvel = (ch - y)*CONTROL_MAX_VAL/(cy - ch - reg->abs_y);
	         else
	         {
	            xvel = xyrot;
	            xyrot = 0;
	         }
         }
      }
	   else
         cnum |= VIEW_VCENTER;
   }

   if (*lastsect != cnum)
   {
      extern LGRegion* fullview_region;
      LGCursor*  c = &motion_cursors[cnum];
//      Warning(("hey, cursor num = %d!\n",cnum));

      if (reg == fullview_region)
         uiSetGlobalDefaultCursor(c);
      else
         uiSetRegionDefaultCursor(reg,c);
      *lastsect = cnum;
   }

   if (!thrust)
      physics_set_player_controls(MOUSE_CONTROL_BANK,xvel,yvel,CONTROL_NO_CHANGE,xyrot,CONTROL_NO_CHANGE,CONTROL_NO_CHANGE);

   if (dougs_goofy_hack)
	   return xvel|yvel|xyrot;
   return 0;
}


// Not a directly-installed mouse handler, called from view3d_mouse_handler
void view3d_rightbutton_handler(uiMouseEvent* ev, LGRegion* r, view3d_data* data)
{
   extern LGCursor fire_cursor;
   extern bool hack_takeover;
   LGPoint aimpos = ev->pos;

	if (DoubleSize)																// If double sizing, convert the y to 640x480, then
		aimpos.y = SCONV_Y(aimpos.y) >> 1;							// half it.  The x stays as is.
	else
		ss_point_convert(&(aimpos.x),&(aimpos.y),FALSE);

   // Don't do nuthin if we're in a hack camera
   if (hack_takeover)
      return;

   if (ev->action & MOUSE_RUP)
   {
      if (!data->rdown)
         data->lastright = aimpos;
      else
         data->rdown = FALSE;
      left_down_jump = FALSE;
      weapon_button_up = TRUE;
      if (fire_slam)
      {
		if (full_game_3d)
            uiPopSlabCursor(&fullscreen_slab);
         else
            uiPopSlabCursor(&main_slab);
         fire_slam = FALSE;
      }
   }

   if (ev->action & MOUSE_RDOWN)
   {
      data->rdown = TRUE;
      data->lastright = aimpos;
      left_down_jump = data->ldown && !global_fullmap->cyber;
//���      view3d_constrain_mouse(r,RBUTTON_CONSTRAIN_BIT);
   }

/*���
   if (mouse_jump_ui && data->ldown && !global_fullmap->cyber)
   {
	   if (ev->action & MOUSE_RDOWN)
      {
	      physics_set_one_control(MOUSE_CONTROL_BANK,CONTROL_ZVEL, MAX_JUMP_CONTROL);
         return;
      }
   }
*/
   switch (input_cursor_mode)
   {
   case INPUT_NORMAL_CURSOR:
      if (!global_fullmap->cyber && (player_struct.fire_rate == 0) && !(ev->action & MOUSE_RDOWN)) break;
      if (left_down_jump) break;
      if (data->rdown)
      {
	      if (fire_player_weapon(&aimpos,r,weapon_button_up) && (ev->action & MOUSE_RDOWN) && !fire_slam)
         {
            if (full_game_3d)
               uiPushSlabCursor(&fullscreen_slab, &fire_cursor);
            else
               uiPushSlabCursor(&main_slab, &fire_cursor);
            fire_slam = TRUE;
         }
         weapon_button_up = FALSE;
      }
      break;
   case INPUT_OBJECT_CURSOR:
      if (ev->action & MOUSE_RUP)
      {
         fix vel = throw_oomph*FIX_UNIT;
         short dropy = DROP_REGION_Y(r);
         short y = aimpos.y;
//         if (convert_use_mode != 0)
		if (DoubleSize)														// If double sizing, convert the y to 640x480, then
			dropy = SCONV_Y(dropy) >> 1;								// half it.  The x stays as is.
		else
			dropy = SCONV_Y(dropy);
         if  (y >= dropy && data->lastright.y >= dropy)
         {
            vel = 0;
         }
	      if (player_throw_object(object_on_cursor,aimpos.x,y,data->lastright.x,data->lastright.y,vel))
         {
			pop_cursor_object();
			uiShowMouse(NULL);		//KLC - added to make sure new cursor shows.
         }
	      data->rdown = FALSE;
      }
      break;
   }
}


// ----------------------------------------------------------------
// use_object_in_3d deals with double-clicking on an object in the 3d

bool check_object_dist(ObjID obj1, ObjID obj2, fix crit)
{
   bool retval = FALSE;
   extern fix ID2radius(ObjID);
   fix critrad = ID2radius(obj2);
   fix dx = fix_from_obj_coord(objs[obj1].loc.x) - fix_from_obj_coord(objs[obj2].loc.x);
   fix dy = fix_from_obj_coord(objs[obj1].loc.y) - fix_from_obj_coord(objs[obj2].loc.y);
   fix dz = fix_from_obj_height(obj1) - fix_from_obj_height(obj2);
   if (-dz > critrad/2 && -dz < critrad + FIX_UNIT/4)
   {
      crit *= 2;
   }
   retval = fix_fast_pyth_dist(dx, dy) < crit;
   if (retval)
   {
      retval = -(critrad*2 + crit/2) < dz && dz < crit/2 + critrad*2;
   }
   return retval;
}

#define TELE_ROD_DIST 16  // 16 feet


void use_object_in_3d(ObjID obj)
{
   bool success = FALSE;
   ObjID telerod = OBJ_NULL;
   bool showname = FALSE;
   extern bool object_use(ObjID id, bool in_inv, ObjID cursor_obj);
   extern ObjID physics_handle_id[MAX_OBJ];
   int mode = USE_MODE(obj);
   char buf[80];
   Ref usemode=ID_NULL;
   extern short loved_textures[];
	extern char* get_texture_name(int,char*,int);
   extern char* get_texture_use_string(int,char*,int);

   if (global_fullmap->cyber)
   {
      if (ID2TRIP(obj) != INFONODE_TRIPLE)
      {
         switch(USE_MODE(obj)) {
            case USE_USE_MODE:
               usemode=REF_STR_PhraseUse;
               break;
            case PICKUP_USE_MODE:
               usemode=REF_STR_PhrasePickUp;
               break;
         }
         // exceptions
         switch(objs[obj].obclass) {
            case CLASS_BIGSTUFF:
               usemode=REF_STR_PhrasePickUp;
               break;
            case CLASS_CRITTER:
               usemode=ID_NULL;
               break;
         }
         if(usemode!=ID_NULL) {
            lg_sprintf(buf,get_temp_string(REF_STR_CyberspaceUse),get_temp_string(usemode));
            message_info(buf);
         }
         return;
      }
   }

   if (input_cursor_mode == INPUT_OBJECT_CURSOR)
   {
      mode = USE_USE_MODE;
      if (ID2TRIP(object_on_cursor) == ROD_TRIPLE)
      {
         telerod = object_on_cursor;
         object_on_cursor = OBJ_NULL;
         use_distance_mod += TELE_ROD_DIST;
      }
   }

   switch(ID2TRIP(obj)) {

      case TMAP_TRIPLE:
         get_texture_use_string(loved_textures[objBigstuffs[objs[obj].specID].data2],buf,80);
         message_info(buf);
         return;
      case BRIDGE_TRIPLE:
      {
         int dat=((objBigstuffs[objs[obj].specID].data1)>>16)&0xFF;
         if(dat&0x80) {
            get_texture_name(loved_textures[dat&(~0x80)],buf,80);
            message_info(buf);
            return;
         }
         break;
      }
   }

   switch(mode)
   {
   case PICKUP_USE_MODE:
      {
	      ObjLocState del_loc_state;
         void grenade_contact(ObjID id, int severity);

	      if (!check_object_dist(obj,PLAYER_OBJ,MAX_PICKUP_DIST))
	      {
	         string_message_info(REF_STR_PickupTooFar);
	         showname = FALSE;
	         break;
	      }
	      // yank the object out of the map. 
	      del_loc_state.obj = obj;
	      del_loc_state.loc = objs[obj].loc;
	      del_loc_state.loc.x = -1;
	      ObjRefStateBinSetNull(del_loc_state.refs[0].bin);
	      ObjUpdateLocs(&del_loc_state);
	      if (objs[obj].info.ph != -1)
	      {
	         EDMS_kill_object(objs[obj].info.ph);
            physics_handle_id[objs[obj].info.ph]=OBJ_NULL;
	         objs[obj].info.ph = -1;
	      }
	      // Put it on the cursor
//         showname = TRUE;
	      push_cursor_object(obj);

         if(objs[obj].obclass==CLASS_GRENADE)
            grenade_contact(obj,INT_MAX);

         success = TRUE;
      }
      break;
   case USE_USE_MODE:
      showname = FALSE;
      if (objs[obj].obclass != CLASS_CRITTER && ID2TRIP(obj)!=MAPNOTE_TRIPLE &&
         !check_object_dist(obj,PLAYER_OBJ,MAX_USE_DIST))
      {
   	   string_message_info(REF_STR_UseTooFar);
	      break;
      }
      if (!object_use(obj,FALSE,object_on_cursor))
      {
         if (objs[obj].obclass != CLASS_DOOR)
            goto cantuse;
         else
            showname = TRUE;
      }
      if (telerod != OBJ_NULL)
      {
         object_on_cursor = telerod;
         use_distance_mod -= TELE_ROD_DIST;
      }
      success = TRUE;

      break;
   cantuse:
   default:
      {
         char use_str[80],buf2[50];
         lg_sprintf(use_str,get_temp_string(REF_STR_CantUse),get_object_lookname(obj,buf2,50));
         message_info(use_str);
      }
      break;
   }
   if (success && !global_fullmap->cyber)
   {
      objs[obj].info.inst_flags |= OLH_INST_FLAG;
   }
   if (showname)
      look_at_object(obj);

}


//-------------------------------------------------------------------------
// look_at_object prints a descriptive string of the object in the message line

// these are just cribbed here from email.c...
#define EMAIL_BASE_ID   RES_email0
#define TITLE_IDX       1
#define SENDER_IDX      2

char *get_object_lookname(ObjID id,char use_string[], int sz)
{
   int ref = -1;
   int l;
   int usetrip = ID2TRIP(id);
   extern short loved_textures[];
	extern char* get_texture_name(int,char*,int);

   strcpy(use_string, "");

   switch(objs[id].obclass)
   {
   case CLASS_FIXTURE:
   case CLASS_DOOR:
      if (objs[id].info.make_info != 0)
         ref = REF_STR_Name0 + objs[id].info.make_info;
      break;
   case CLASS_GRENADE:
      if(objGrenades[objs[id].specID].flags & GREN_ACTIVE_FLAG) {
         get_string(REF_STR_WordLiveGrenade,use_string,sz);
         l=strlen(use_string);
         if(l+1<sz) use_string[l]=' ';
      }
      break;
   case CLASS_SOFTWARE:
      if (objs[id].subclass == SOFTWARE_SUBCLASS_DATA)
      {
         short cont = objSoftwares[objs[id].specID].data_munge;
         short num = cont & 0xFF;
         if(global_fullmap->cyber) {
            ref=REF_STR_DataObj;
            break;
         }
         switch(cont >> 8)
         {
            case LOG_VER: num += NUM_EMAIL_PROPER; break;
            case DATA_VER: num += (NUM_EMAIL - NUM_DATA); break;
         }
         ref = MKREF(EMAIL_BASE_ID + num, TITLE_IDX);
      }
      break;
   case CLASS_BIGSTUFF:
      if (global_fullmap->cyber)
      {
         usetrip = MAKETRIP(CLASS_SOFTWARE, objBigstuffs[objs[id].specID].data1, objBigstuffs[objs[id].specID].data2);
      }
      else
      {
         switch (ID2TRIP(id))
         {
            case ICON_TRIPLE:
               ref = REF_STR_IconName0 + objs[id].info.current_frame;
               break;
            case TMAP_TRIPLE:
               get_texture_name(loved_textures[objBigstuffs[objs[id].specID].data2],use_string,sz);
               return(use_string);
            case BRIDGE_TRIPLE:
            {
               int dat=((objBigstuffs[objs[id].specID].data1)>>16)&0xFF;
               if(dat&0x80) {
                  get_texture_name(loved_textures[dat&(~0x80)],use_string,sz);
                  return(use_string);
               }
               break;
            }
         }
         if(ref<0) {
            if (objs[id].info.make_info != 0)
               ref = REF_STR_Name0 + objs[id].info.make_info;
         }
      }
      break;
   case CLASS_SMALLSTUFF:
   {
      switch (ID2TRIP(id))
      {
         case PERSCARD_TRIPLE:
         {
            char buf[50];
            int acc, len;
            acc = objSmallstuffs[objs[id].specID].data1;
#define PERSONAL_BITS_SHIFT 24
            // get rid of all but personal access bits
            get_object_long_name(ID2TRIP(id),use_string,sz);
            acc = acc>>PERSONAL_BITS_SHIFT;
            ref = PERSONAL_BITS_SHIFT;
            if(acc==0) return(use_string);
            for(;(acc&1)==0;acc=acc>>1) ref++;
            ref = MKREF(RES_accessCards,(ref<<1)+1);
            get_string(ref,buf,sizeof(buf));
            len=strlen(buf);
            while(!isspace(buf[len]) && len>0) len--;
            if(isspace(buf[len]))
            {
               strcat(use_string,"-");
               strcat(use_string,buf+len+1);
            }
            return(use_string);
         }
         case HEAD_TRIPLE:
         case HEAD2_TRIPLE:
            if (objs[id].info.make_info != 0)
               ref = REF_STR_Name0 + objs[id].info.make_info;
            break;
      }
   }
   break;
   case CLASS_HARDWARE:
      {
         get_object_long_name(ID2TRIP(id),use_string, sz);
         strcat(use_string," v");
//KLC         itoa(objHardwares[objs[id].specID].version,use_string+strlen(use_string),10);
         numtostring(objHardwares[objs[id].specID].version, use_string+strlen(use_string));
         return(use_string);
      }
   case CLASS_CRITTER:
      {
         char temp[128];
         Ref mod_refid = -1;
         if (objCritters[objs[id].specID].orders == AI_ORDERS_SLEEP)
            mod_refid = REF_STR_Sleeping;
         else if (objCritters[objs[id].specID].flags & AI_FLAG_TRANQ)
            mod_refid = REF_STR_Drugged;
         else if (objCritters[objs[id].specID].flags & AI_FLAG_CONFUSED)
            mod_refid = REF_STR_Stunned;
         if (mod_refid != -1)
         {
            get_string(mod_refid, temp, 128);
            lg_sprintf(use_string, temp, get_object_long_name(usetrip,NULL,0));
            return(use_string);
         }
      }
      break;
   }
   // If we haven't set ref or ref is garbage, use the long name.
   if ((ref == -1) || !(RefIndexValid((RefTable*)ResGet(REFID(ref)),REFINDEX(ref))))
   {
     strcat(use_string, get_object_long_name(usetrip,NULL,0));
   }
   else
      get_string(ref,use_string,sz);
   return(use_string);
}

void look_at_object(ObjID id)
{
   char buf[50];
   get_object_lookname(id,buf,sizeof(buf));
   message_info(buf);
}


// ------------------------------------------------------------------------
// view3d_dclick dispatches double clicks based on cursor mode

// Not a directly-installed mouse handler, called from view3d_mouse_handler
void view3d_dclick(LGPoint pos, frc* )
{
   extern short loved_textures[];
   extern char* get_texture_use_string(int,char*,int);
   extern bool hack_takeover;
   short obj_trans, obj;
   frc *use_frc;

   extern int _fr_glob_flags;

   if (hack_takeover) return;
   switch (input_cursor_mode)
   {
      case INPUT_NORMAL_CURSOR:
      case INPUT_OBJECT_CURSOR:
         use_frc = svga_render_context;
         obj = fr_get_at_raw(use_frc,pos.x,pos.y,FALSE,FALSE);
         if ((obj > 0) && (objs[obj].obclass == CLASS_DOOR))
         {
            obj_trans = fr_get_at_raw(use_frc,pos.x,pos.y,FALSE,TRUE);
            if (obj != obj_trans) 
            {
               if (DOOR_REALLY_CLOSED(obj))
               {
                  string_message_info(REF_STR_PickupTooFar);
                  return;
               }
               else
                  obj = obj_trans;
            }
         }
         else if (obj>0)
         {
            obj=fr_get_at_raw(use_frc,pos.x,pos.y,FALSE,TRUE);
         }
         if ((short)obj < 0)
         {
            // Don't display texture look strings in cspace....eventually we should do some cool hack
            // for looking through walls, some sort of cspace fr_get_at or something
            if (global_fullmap->cyber)
               string_message_info(REF_STR_CybWallUse);
            else
               message_info(get_texture_use_string(loved_textures[~obj],NULL,0));
         }
         else if ((short)obj > 0)
         {
            use_cursor_pos = pos;
            use_object_in_3d(obj);
         }
         else {
            if(!global_fullmap->cyber) {
               if(!(_fr_glob_flags & FR_SOLIDFR_STATIC))
                  string_message_info(REF_STR_InkyUse);
            }
         }
   }
}               


// -------------------------------------------------------------------------------
// view3d_mouse_handler is the actual installed mouse handler, dispatching to the above functions
bool view3d_mouse_handler(uiMouseEvent* ev, LGRegion* r, view3d_data* data)
{
   static bool got_focus = FALSE;
   bool retval = TRUE;
   LGPoint pt;
   LGPoint evp = ev->pos;
   extern int _fr_glob_flags;

   pt = evp;
   
#ifdef STEREO_SUPPORT
   if (convert_use_mode == 5)
   {
      extern bool inventory_mouse_handler(uiEvent* ev, LGRegion* r, void* data);
      extern bool mfd_view_callback(uiEvent *e, LGRegion *r, void *udata);
      switch (i6d_device)
      {
         case I6D_CTM:
            if (full_visible & FULL_INVENT_MASK)
               return(inventory_mouse_handler((uiEvent *)ev,r,data));
            if ((full_visible & FULL_L_MFD_MASK) && (evp.x < ((MFD_VIEW_LFTX + MFD_VIEW_WID) << 1)))
               return(mfd_view_callback((uiEvent *)ev,r,(void *)0));
            if ((full_visible & FULL_R_MFD_MASK) && (evp.x > ((MFD_VIEW_RGTX + MFD_VIEW_WID) >> 1)))
               return(mfd_view_callback((uiEvent *)ev,r,(void *)1));
            break;
         case I6D_VFX1:
            if (full_visible & FULL_INVENT_MASK)
               return(inventory_mouse_handler((uiEvent *)ev,r,data));
            if ((full_visible & FULL_L_MFD_MASK) && (evp.x < ((MFD_VIEW_LFTX + MFD_VIEW_WID) << 1)))
               return(mfd_view_callback((uiEvent *)ev,r,(void *)0));
            if ((full_visible & FULL_R_MFD_MASK) && (evp.x > ((MFD_VIEW_RGTX + MFD_VIEW_WID) >> 1)))
               return(mfd_view_callback((uiEvent *)ev,r,(void *)1));
            break;
      }
   }
#endif

#ifdef STEREO_SUPPORT
   if (convert_use_mode != 5)
#endif
      if (DoubleSize)														// If double sizing, convert the y to 640x480, then
      	evp.y = SCONV_Y(evp.y) >> 1;								// half it.  The x stays as is.
      else
      	ss_point_convert(&(evp.x),&(evp.y),FALSE);

   view3d_got_event = TRUE;
   pt.x += r->r->ul.x - r->abs_x;
   pt.y += r->r->ul.y - r->abs_y;

   if (!RECT_TEST_PT(r->r,pt))
   {
      data->ldown = FALSE;
      physics_set_player_controls(MOUSE_CONTROL_BANK,0,0,CONTROL_NO_CHANGE,0,CONTROL_NO_CHANGE,CONTROL_NO_CHANGE);
      return(FALSE);
   }
   if (ev->action & MOUSE_LDOWN)
   {
      data->ldown = TRUE;
      data->lastleft = evp;
      if (full_game_3d && !got_focus)
      {
         if (uiGrabFocus(r,UI_EVENT_MOUSE|UI_EVENT_MOUSE_MOVE) == OK)
            got_focus = TRUE;
      }
      chg_set_flg(_current_3d_flag);
//���      view3d_constrain_mouse(r,LBUTTON_CONSTRAIN_BIT);
   }
   if (ev->action & MOUSE_LUP || !(ev->buttons & (1 << MOUSE_LBUTTON)))
   {
      data->ldown = FALSE;           
      if (full_game_3d && got_focus)
      {
         if (uiReleaseFocus(r,UI_EVENT_MOUSE|UI_EVENT_MOUSE_MOVE) == OK)
            got_focus = FALSE;
      }
//���      view3d_unconstrain_mouse(LBUTTON_CONSTRAIN_BIT);
   }
   if (ev->action & MOUSE_LUP &&
      abs(evp.y - data->lastleft.y) < uiDoubleClickTolerance &&
      abs(evp.x - data->lastleft.x) < uiDoubleClickTolerance)
   {
      ObjID		id;
      frc 			*use_frc;
      short 		rabsx,rabsy;
      
      use_frc = svga_render_context;
      rabsx=r->abs_x;
      rabsy=r->abs_y;
      if (!DoubleSize)
         ss_point_convert(&rabsx,&rabsy,FALSE);
      
      id = fr_get_at(use_frc,evp.x-rabsx,evp.y-rabsy,TRUE);
      if ((short)id > 0)
      {
	    look_at_object(id);
      }
      else if ((short)id < 0)
      {
    	   extern short loved_textures[];
	      extern char* get_texture_name(int,char*,int);
         int tnum = loved_textures[~id];
         if (global_fullmap->cyber)
            string_message_info(REF_STR_CybWall);
         else
   	      message_info(get_texture_name(tnum,NULL,0));
      }
      else
      {
         if(!global_fullmap->cyber) {
            if(!(_fr_glob_flags & FR_SOLIDFR_STATIC))
               string_message_info(REF_STR_InkyBlack);
         }
      }
      data->lastleft.x = -255;
   }
   if ((ev->action & (MOUSE_RDOWN|MOUSE_RUP)) || (ev->buttons & (1 << MOUSE_RBUTTON)))
     view3d_rightbutton_handler(ev,r,data);

/* KLC - done in another place now.
   else
   {
      view3d_unconstrain_mouse(RBUTTON_CONSTRAIN_BIT);
      if (fire_slam)
      {
		if (full_game_3d)
         		uiPopSlabCursor(&fullscreen_slab);
         else
         		uiPopSlabCursor(&main_slab);
         fire_slam = FALSE;
      } 
   }

*/
   if ((ev->buttons & (1 << MOUSE_RBUTTON)) == 0 
      || ((ev->buttons & (1 << MOUSE_RBUTTON)) == 0 && global_fullmap->cyber))
      physics_set_one_control(MOUSE_CONTROL_BANK,CONTROL_ZVEL,0);

   if (ev->action & UI_MOUSE_LDOUBLE)
   {
      //Spew(DSRC_USER_I_Motion,("use this, bay-bee!\n"));
      view3d_dclick(evp,data->fr);
      data->lastleft = MakePoint(-100,-100);
   }

   // Do mouse motion.
   if (view3d_mouse_input(evp,r,data->ldown,&data->lastsect)!=0)
      data->lastleft = MakePoint(-1,-1);     // if the player is moving, not a down

   return(retval);
}

typedef struct _view3d_kdata
{
   int maxctrl;  // max control as affected by fatigue
} view3d_kdata;


#define FIRE_KEY KEY_SPACE  //KLC for PC was KEY_ENTER

bool view3d_key_handler(uiCookedKeyEvent* ev, LGRegion* r, void* )
{
	bool retval = FALSE;
//KLC   static bool fire_key_down = FALSE;
	LGPoint evp;
	
	if (ev->code == DOWN(FIRE_KEY))									// If fire key was pressed
	{
		if (weapon_button_up)												// and we haven't fired already
		{
      		evp = ev->pos;
			ss_point_convert(&(evp.x),&(evp.y),FALSE);
      		fire_player_weapon(&evp, r, !fire_slam);
      		fire_slam = TRUE;
      		weapon_button_up = FALSE;
		}
/*
      evp = ev->pos;
#ifdef SVGA_SUPPORT
      ss_point_convert(&(evp.x),&(evp.y),FALSE);
#endif
//���temp      fire_player_weapon(&evp,r,!fire_key_down);
      fire_player_weapon(&evp, r, !fire_slam);
//���      view3d_constrain_mouse(_current_view,FIREKEY_CONSTRAIN_BIT);
//KLC      retval = fire_key_down = TRUE;
      fire_slam = TRUE;
   }
   else if (ev->code == FIRE_KEY)
   {
//���      view3d_unconstrain_mouse(FIREKEY_CONSTRAIN_BIT);
      retval = TRUE;
//KLC      fire_key_down = FALSE;
      fire_slam = FALSE;
*/
   }
   return retval;
}

#ifdef NOT_YET  //KLC - for VR headsets

#define CORE6D_MAX_VAL       (1<<15)
#define CORE6D_TRA_TOL       (MAX_VAL>>8)
#define CORE6D_ROT_TOL       (MAX_VAL>>5)
#define Tra_Scale(x)         (((x)*CONTROL_MAX_VAL)/(MAX_VAL-TRA_TOL))
#define Rot_Scale(x)         (((x)*CONTROL_MAX_VAL)/(MAX_VAL-ROT_TOL))

// total hack lame inp6d function for now....
void inp6d_chk(void)
{
//   static inp6d_raw_event last_swift;
   i6s_event *inp6d_in;
   int xp, yp, zv, h, p, b;

   // we'll want to put in code here to check for mouse_button_emulation
//   if (checking_mouse_button_emulation && <SOME CODE TO CHECK FOR BUTTON PRESS>)
//      mouse_button_emulated = TRUE;

   if (game_paused) return;
   inp6d_in=i6_poll();
   if (inp6d_in==NULL) return;
   xp=inp6d_in->x;
   if (abs(xp)>TRA_TOL) if (xp>TRA_TOL) xp=Tra_Scale(xp-TRA_TOL); else xp=Tra_Scale(xp+TRA_TOL); else xp=0;
   yp=-inp6d_in->z;
   if (abs(yp)>TRA_TOL) if (yp>TRA_TOL) yp=Tra_Scale(yp-TRA_TOL); else yp=Tra_Scale(yp+TRA_TOL); else yp=0;
   zv=inp6d_in->y;
   if (abs(zv)>TRA_TOL) if (zv>TRA_TOL) zv=Tra_Scale(zv-TRA_TOL); else zv=Tra_Scale(zv+TRA_TOL); else zv=0;

   h=-inp6d_in->ry; if (h>ROT_TOL) h=Rot_Scale(h-ROT_TOL); else if (h<-ROT_TOL) h=Rot_Scale(h+ROT_TOL); else h=0;
   p= inp6d_in->rx; if (p>ROT_TOL) p=Rot_Scale(p-ROT_TOL); else if (p<-ROT_TOL) p=Rot_Scale(p+ROT_TOL); else p=0;
   b=-inp6d_in->rz; if (b>ROT_TOL) b=Rot_Scale(b-ROT_TOL); else if (b<-ROT_TOL) b=Rot_Scale(b+ROT_TOL); else b=0;

#ifdef PLAYTEST
   if (inp6d_dbg)
	   mprintf("Parsed %04x %04x %04x %04x %04x %04x to %d %d %d %d %d %d\n",
	      inp6d_in->x,  inp6d_in->y,  inp6d_in->z,  inp6d_in->rx,     inp6d_in->ry,     inp6d_in->rz,
	      xp,yp,zv,h,p,b);
#endif

#ifdef PLAYTEST
   if (!inp6d_player)
   {
	   fr_camera_slewcam(NULL,EYE_X,xp/5);
	   fr_camera_slewcam(NULL,EYE_Y,yp/5);
	   fr_camera_slewcam(NULL,EYE_Z,zv/5);
	   fr_camera_slewcam(NULL,EYE_H,h/3);
	   fr_camera_slewcam(NULL,EYE_P,p/3);
	   fr_camera_slewcam(NULL,EYE_B,b/3);
	}
	else
#endif
   {
	   physics_set_player_controls(INP6D_CONTROL_BANK, xp, yp, zv, h, p, b);
	   fr_camera_slewcam(NULL,EYE_P,p/3);     // hack horribly for now... yea!
   	fr_camera_slewcam(NULL,EYE_B,b/3);
   }
}

#define ANG_P 0
#define ANG_B 1
#define ANG_H 2
#if defined(VFX1_SUPPORT)||defined(CTM_SUPPORT)
short l_angs[3];

short *set_abs_head(i6s_event *e)
{
   if (!inp6d_doom)
   {
	   l_angs[ANG_B]=( e->ry)-tracker_initial_pos[0];
	   l_angs[ANG_P]=( e->rx)-tracker_initial_pos[1];
   }
   else
      l_angs[ANG_B]=l_angs[ANG_P]=0;
   l_angs[ANG_H]=(-e->rz)-tracker_initial_pos[2];
   if (inp6d_hdouble) l_angs[ANG_H] += l_angs[ANG_H];
   if (inp6d_pdouble) l_angs[ANG_P] += l_angs[ANG_P];
   if (inp6d_bdouble) l_angs[ANG_B] += l_angs[ANG_B];
   return l_angs;
}


#define BREAK_REGION (CONTROL_MAX_VAL/4)
#define XTRA_REGION  (CONTROL_MAX_VAL-BREAK_REGION)

// add a double LGRegion fix...
short deparse_angle_region(short angle, short min, short mid, short max)
{
   int sgn;
   if (abs(angle)<min) return 0;
   sgn=angle>0?1:-1;
   angle=abs(angle);
   if (angle<mid)
	   return sgn*((angle-min)*BREAK_REGION/(mid-min));
   if (angle<max)
	   return sgn*(BREAK_REGION+(angle-mid)*XTRA_REGION/(max-mid));
   return sgn*CONTROL_MAX_VAL;
}

void slam_head(short *angs)
{
#ifdef PLAYTEST
   static int last_head_h;
#endif
//   if (global_fullmap->cyber)
//   {     // secret head joystick dented with the promise of power
//      short h,p,b;
//      h=deparse_angle_region(angs[ANG_H],0x0400,0x1400,0x2000);
//      p=deparse_angle_region(angs[ANG_P],0x0400,0x1400,0x2000);
//      b=deparse_angle_region(angs[ANG_B],0x0300,0x0B00,0x1400);
//      physics_set_player_controls(INP6D_CONTROL_BANK, b, -p, 0, h, 0, 0);
//   }
//   else
   {
	   fr_camera_setone(NULL,EYE_P,angs[ANG_P]);
	   fr_camera_setone(NULL,EYE_B,angs[ANG_B]);
#ifdef PLAYTEST
	   if (!inp6d_link)
#endif
	      fr_camera_setone(NULL,EYE_HEADH,angs[ANG_H]);
#ifdef PLAYTEST
   	else
	   {
	      int h_diff=(angs[ANG_H]-last_head_h)/cam_slew_scale[EYE_H];    //   HAQ
	      fr_camera_slewone(NULL,EYE_H,h_diff);
	      last_head_h=angs[ANG_H];
	   }
#endif
   }
}
#endif

#define USE_UPPER_BOUND   (CIT_CYCLE/4)
#define RELOAD_TIME       (CIT_CYCLE*2)

bool reload_current_weapon(void);
bool inp_reloaded = FALSE;
bool inp_sidestep = FALSE;
int use_but_time = 0;
int weap_time = 0;
void inp_weapon_button(bool pull)
{
   int w = player_struct.actives[ACTIVE_WEAPON];                // check if we need to reload
   bool reloaded = FALSE;

   if (object_on_cursor)
   {
      LGPoint pos = MakePoint(_current_view->abs_x + RectWidth(_current_view->r)/2,
                            _current_view->abs_y + RectHeight(_current_view->r)/2);

      ui_mouse_put_xy(pos.x,pos.y);
#ifdef SVGA_SUPPORT
      ss_point_convert(&(pos.x),&(pos.y),FALSE);
#endif
	   if(player_throw_object(object_on_cursor,pos.x,pos.y,pos.x,pos.y,throw_oomph*FIX_UNIT))
         pop_cursor_object();
      return;
   }

   // reload if conditions are right
   if ((player_struct.weapons[w].type != GUN_SUBCLASS_BEAM) &&
         (player_struct.weapons[w].type != GUN_SUBCLASS_HANDTOHAND) &&
         (player_struct.weapons[w].type != GUN_SUBCLASS_BEAMPROJ) &&
         (player_struct.weapons[w].ammo == 0))
   {  // reload

      if (weap_time==0) {
         weap_time=*tmd_ticks;
         reloaded=FALSE;
      }
      else if (!reloaded)
         if (*tmd_ticks>weap_time+RELOAD_TIME) {
            reload_current_weapon();
            reloaded=TRUE;
         }
   }
   else if (weap_time==0)
   {
      LGPoint pos = MakePoint(_current_view->abs_x + RectWidth(_current_view->r)/2,
                            _current_view->abs_y + RectHeight(_current_view->r)/2 + (RectHeight(_current_view->r)>>4));

      ui_mouse_put_xy(pos.x,pos.y);
#ifdef SVGA_SUPPORT
      ss_point_convert(&(pos.x),&(pos.y),FALSE);
#endif
	   fire_player_weapon(&pos,_current_view,pull);
   }
}

#define inp_weapon_junk() weap_time=0

void inp_use_sidestep_button()
{
   if (use_but_time)
   {  // if long enough, go to sidestep....
      if (*tmd_ticks>use_but_time+USE_UPPER_BOUND)
         { inp_sidestep=TRUE; use_but_time=0; }
   }
   else
      if (!inp_sidestep)
	      use_but_time=*tmd_ticks;
}

// teach this that if you have obj on cursor it knows whether to throw or 
//  put in your inventory.  do this by storing when you pick up w/joystick
//  and if mouse moves cancelling but otherwise when joyclicking again with
//  an obj already on cursor the put it in inventory

void inp_use_sidestep_junk()
{
   if (use_but_time)
      {
         LGPoint pos;
         
         if (input_cursor_mode == INPUT_OBJECT_CURSOR)
         {
            extern void absorb_object_on_cursor(short keycode, ulong context, void* data);
            absorb_object_on_cursor(0,0,0);
         }
         else
         {
            pos = MakePoint(_current_view->abs_x + RectWidth(_current_view->r)/2,
                              _current_view->abs_y + RectHeight(_current_view->r)/2);
   #ifdef SVGA_SUPPORT
            ss_point_convert(&pos.x,&pos.y,FALSE);                          
   #endif
            mouse_put_xy(pos.x,pos.y);
            view3d_dclick(pos,NULL);
         }
         use_but_time=0;
      }
   else
      inp_sidestep=FALSE;
}


#ifdef VFX1_SUPPORT

#define VFX1_MAX_VAL      (1<<15)
#define VFX1_TRA_TOL      (MAX_VAL>>5)
#define VFX1_Tra_Scale(x) (((x)*CONTROL_MAX_VAL)/(VFX1_MAX_VAL-VFX1_TRA_TOL))

// total hack lame guess at a vfx1 function...
void vfx1_chk(void)
{
   static uchar last_but;
//   static LGPoint targ_loc={160,100};
   i6s_event *inp6d_in;
   int xp, yp, xp1, xp2, zv;
   short *angs;

   if (game_paused) return;
   inp6d_in=i6_poll();
   if (inp6d_in==NULL) return;

   xp=inp6d_in->x;

   xp+=(xp>>1);   // add fifty percent
   if (abs(xp)>VFX1_TRA_TOL)
      if (xp>VFX1_TRA_TOL) xp=VFX1_Tra_Scale(xp-VFX1_TRA_TOL); else xp=VFX1_Tra_Scale(xp+VFX1_TRA_TOL); else xp=0;

   yp=-inp6d_in->y;        // flip it so + is forward motion
   yp*=2;   // since the bat, forward 90 degrees is full speed
   if (abs(yp)>VFX1_TRA_TOL)
      if (yp>VFX1_TRA_TOL) yp= CONTROL_MAX_VAL/4 + VFX1_Tra_Scale(yp-VFX1_TRA_TOL); else yp=VFX1_Tra_Scale(yp+VFX1_TRA_TOL); else yp=0;
   xp1=xp; xp2=0;
   zv=0;

   angs=set_abs_head(inp6d_in);

   if (inp6d_in->but)
   {
      if (joystick_mouse_emul)
      {
         extern void joystick_emulate_mouse(int x, int y,uchar bstate,uchar last_bstate);
         joystick_emulate_mouse(xp,-yp,inp6d_in->but>>1,last_but>>1);
		   xp2=xp1=yp=zv=0;
      }
      else
      {
         if (inp6d_in->but&1)
            inp_weapon_button((last_but&1)!=(inp6d_in->but&1));
         if (inp6d_in->but&2)
            zv=100;                       // jump jump jump
         if (inp6d_in->but&4)
            inp_use_sidestep_button();
      }
   }

   if (inp_sidestep)
     { xp2=xp1*2; xp1=0; }      // switch bat X from heading to sidestep


   if (!joystick_mouse_emul)
   {
	   if ((inp6d_in->but&4)==0)
	      inp_use_sidestep_junk();

	   if ((inp6d_in->but&1)==0 && (last_but&1)==1) {
	      inp_weapon_junk();
      }
   }

#ifdef PLAYTEST
   if (inp6d_dbg)
	   mprintf("Parsed %04x %04x %04x %04x %04x %04x to %d %d %d %d %d %d\n",
	     inp6d_in->x,inp6d_in->y,inp6d_in->z,inp6d_in->rx,inp6d_in->ry,inp6d_in->rz,
	      xp,yp,zv,angs[ANG_P],angs[ANG_B],angs[ANG_H]);
#endif

#ifdef PLAYTEST
   if (!inp6d_player)
   {
	   fr_camera_slewcam(NULL,EYE_X,xp/5);
	   fr_camera_slewcam(NULL,EYE_Y,yp/5);
//	   fr_camera_slewcam(NULL,EYE_Z,zv/5);
//	   fr_camera_setone(NULL,EYE_HEADH,h);
	   fr_camera_setone(NULL,EYE_H,angs[ANG_H]);
	   fr_camera_setone(NULL,EYE_P,angs[ANG_P]);
	   fr_camera_setone(NULL,EYE_B,angs[ANG_B]);
	}
	else
#endif      // PLAYTEST
   {
      // combine the two when in cyberspace
      if (global_fullmap->cyber) {
         xp1 += deparse_angle_region(angs[ANG_H],0x0400,0x1400,0x2000);
         yp  -= deparse_angle_region(angs[ANG_P],0x0400,0x1400,0x2000)/2;
         xp2 += deparse_angle_region(angs[ANG_B],0x0300,0x0B00,0x1400);

         if (xp1>CONTROL_MAX_VAL) xp1 = CONTROL_MAX_VAL;
         else if (xp1<-CONTROL_MAX_VAL) xp1 = -CONTROL_MAX_VAL;

         if (yp>CONTROL_MAX_VAL) yp = CONTROL_MAX_VAL;
         else if (yp<-CONTROL_MAX_VAL) yp = -CONTROL_MAX_VAL;

         if (xp2>CONTROL_MAX_VAL) xp2 = CONTROL_MAX_VAL;
         else if (xp2<-CONTROL_MAX_VAL) xp2 = -CONTROL_MAX_VAL;

      } else {
         slam_head(angs);
      }

      // b+xp2, -p+yp, 0+zv, h+xp1, 0, 0 
	   physics_set_player_controls(INP6D_CONTROL_BANK, xp2, yp, zv, xp1, 0, 0);
   }
   last_but = inp6d_in->but;
}
#endif      // VFX1_SUPPORT

#ifdef CTM_SUPPORT

// note secret filtering code these days....
void ctm_chk(void)
{
   i6s_event *inp6d_in;
   short *angs;

   if (game_paused) return;
   inp6d_in=i6_poll();
   if (inp6d_in==NULL) return;

   angs=set_abs_head(inp6d_in);

#ifdef PLAYTEST
   if (inp6d_dbg)
	   mprintf("Parsed %04x %04x %04x to %d %d %d\n",
	     inp6d_in->rx,inp6d_in->ry,inp6d_in->rz,angs[ANG_H],angs[ANG_P],angs[ANG_B]);
#endif      // PLAYTEST

#ifdef PLAYTEST
   if (!inp6d_player)
   {
      if (!inp6d_link)
		   fr_camera_setone(NULL,EYE_HEADH,angs[ANG_H]);
      else
		   fr_camera_setone(NULL,EYE_H,angs[ANG_H]);
	   fr_camera_setone(NULL,EYE_P,angs[ANG_P]);
	   fr_camera_setone(NULL,EYE_B,angs[ANG_B]);
	}
	else
#endif      // PLAYTEST
   {
      // combine the two when in cyberspace
      if (global_fullmap->cyber) {
         short h,p,b;
         h = deparse_angle_region(angs[ANG_H],0x0400,0x1400,0x2000);
         p = deparse_angle_region(angs[ANG_P],0x0400,0x1400,0x2000)/2;
         b = deparse_angle_region(angs[ANG_B],0x0300,0x0B00,0x1400);
	      physics_set_player_controls(INP6D_CONTROL_BANK, b,-p, 0, h, 0, 0);

      } else {
         slam_head(angs);
      }

   }
}
#endif      // CTM_SUPPORT

void swift_chk(void)
{
   i6s_event *inp6d_in;
   int our_vals[6], i, tmp;

   // we'll want to put in code here to check for mouse_button_emulation
//   if (checking_mouse_button_emulation && <SOME CODE TO CHECK FOR BUTTON PRESS>)
//      mouse_button_emulated = TRUE;

   if (game_paused) return;
   inp6d_in=i6_poll();
   if (inp6d_in==NULL) return;

   inp6d_in->y=-inp6d_in->y; // doug cheats, film at 11

   // translation
   for (i=0; i<3; i++)
   {
      tmp=inp6d_in->els[i];
      if (abs(tmp)>TRA_TOL) if (tmp>TRA_TOL) tmp=Tra_Scale(tmp-TRA_TOL); else tmp=Tra_Scale(tmp+TRA_TOL); else tmp=0;
      our_vals[i]=tmp;
   }

   // rotation
   for (i=3; i<6; i++)
   {
      tmp=inp6d_in->els[i];
      if (tmp>ROT_TOL) tmp=Rot_Scale(tmp-ROT_TOL);
      else if (tmp<-ROT_TOL) tmp=Rot_Scale(tmp+ROT_TOL);
      else tmp=0;
      our_vals[i]=tmp;
   }  // was h = -ry, p = rx, b = -rz

#ifdef PLAYTEST
   if (inp6d_dbg)
      mprintf("Parsed %04x %04x %04x %04x %04x %04x to %d %d %d %d %d %d\n",
         inp6d_in->els[0],inp6d_in->els[1],inp6d_in->els[2],inp6d_in->els[3],inp6d_in->els[4],inp6d_in->els[5],
         our_vals[0],our_vals[1],our_vals[2],our_vals[3],our_vals[4],our_vals[5]);
#endif

#ifdef PLAYTEST
   if (!inp6d_player)
   {
      for (i=0; i<3; i++)
   	   fr_camera_slewcam(NULL,i,our_vals[i]/5);  // cheat cheat cheat
	   fr_camera_slewcam(NULL,EYE_H,our_vals[ANG_H]/3);
	   fr_camera_slewcam(NULL,EYE_P,our_vals[ANG_P]/3);
	   fr_camera_slewcam(NULL,EYE_B,our_vals[ANG_B]/3);
	}
	else
#endif
   {
	   physics_set_player_controls(INP6D_CONTROL_BANK, our_vals[0], our_vals[1], our_vals[2], our_vals[3], our_vals[4], our_vals[5]);
   }
}

// various hacked hotkey functions....
#pragma disable_message(202)
#if defined(VFX1_SUPPORT)||defined(CTM_SUPPORT)
bool recenter_headset(short keycode, ulong context, void* data)
{
   long start_time=*tmd_ticks;
   i6s_event *inp6d_geth;
   do {
	   inp6d_geth=i6_poll();
   } while ((inp6d_geth==NULL)&&(*tmd_ticks<start_time+(280/4)));
   if (inp6d_geth!=NULL)
   {
      tracker_initial_pos[0]= inp6d_geth->ry;
	   tracker_initial_pos[1]= inp6d_geth->rx;
	   tracker_initial_pos[2]=-inp6d_geth->rz;
//    mprintf("Tip %x %x %x\n",tracker_initial_pos[0],tracker_initial_pos[1],tracker_initial_pos[2]);
      message_info("Headset ReCentered");
   }  // should message_info here with no headset found
   return FALSE;
}
#endif

bool recenter_joystick(short keycode, ulong context, void* data)
{
   joy_center();
   string_message_info(REF_STR_CenterJoyDone);
   return FALSE;
}

bool change_gamma(short keycode, ulong context, void* data)
{
   static fix cit_gamma=fix_make(1,0);
   int dir=(int)data;
   if ((dir<0)&&(cit_gamma>fix_make(0,0x6000)))      cit_gamma-=fix_make(0,0x0400);
   else if ((dir>0)&&(cit_gamma<fix_make(1,0x6000))) cit_gamma+=fix_make(0,0x0800);
   else dir=0;
   if (dir!=0) gr_set_gamma_pal(0,256,cit_gamma);
   // here, should do message_info ResXXX+1-dir, have 3 strings in file, Lowered, Maxed, Raised
   // instead, ill do a switch for now...
   switch (dir)
   {
   case -1: message_info("Gamma Lowered"); break;
   case  0: message_info("Gamma Maxed Out"); break;
   case  1: message_info("Gamma Raised"); break;
	}
   return FALSE;
}
#pragma enable_message(202)

#define CYBERMAN_MOTION (1<<1)
#define CYBERMAN_FIRE   (1<<0)      // either other button

#define CYB_MAX_VAL      (1<<15)
#define CYB_TOL          (CYB_MAX_VAL>>14)
#define CYB_Scale(x)     (((x)*CONTROL_MAX_VAL)/(CYB_MAX_VAL-CYB_TOL))

// total hack lame inp6d function for now....
void cyberman_chk(void)
{
//   static inp6d_raw_event last_swift;
   i6s_event *inp6d_in;
   int xp, yp, zv, h, p, b;
   static bool cyb_mouse_around=TRUE, pchange=FALSE;
   static int p_vel=0, b_vel=0; // , h_vel=0;

   // we'll want to put in code here to check for mouse_button_emulation
//   if (checking_mouse_button_emulation && <SOME CODE TO CHECK FOR BUTTON PRESS>)
//      mouse_button_emulated = TRUE;

   if (game_paused) return;
   inp6d_in=i6_poll();
   if (inp6d_in==NULL) return;

   // should wait for a motion prior to flipping over
   if ((inp6d_in->but&CYBERMAN_MOTION)==0)
   {
      if (!cyb_mouse_around)
      {
#ifdef PLAYTEST
         mprintf("CMA punt, zero controls\n");
#endif
   	   physics_set_player_controls(INP6D_CONTROL_BANK, 0, 0, 0, 0, 0, 0);
         uiGlobalEventMask|=(UI_EVENT_MOUSE|UI_EVENT_MOUSE_MOVE);
         uiSetMouseMotionPolling(TRUE);
         uiShowMouse(NULL);
         cyb_mouse_around=TRUE;
      }
      return;  // should allow mouse in this case, since we really need to do that
   }
   else
   {
      if (cyb_mouse_around)
      {
	      uiGlobalEventMask&=~(UI_EVENT_MOUSE|UI_EVENT_MOUSE_MOVE);
         uiSetMouseMotionPolling(FALSE);
	      uiHideMouse(NULL);
	      cyb_mouse_around=FALSE;
      }           
   }

   if (inp6d_in->but&CYBERMAN_FIRE)
   {
      LGPoint pos = MakePoint(_current_view->abs_x + RectWidth(_current_view->r)/2,
                            _current_view->abs_y + RectHeight(_current_view->r)/2 + (RectHeight(_current_view->r)>>5));
   
#ifdef SVGA_SUPPORT
      ss_point_convert(&(pos.x),&(pos.y),FALSE);
#endif
      fire_player_weapon(&pos,_current_view,TRUE);	
   }

   // fire hack, mouse hack
   xp=inp6d_in->x;
   if (abs(xp)>CYB_TOL) if (xp>CYB_TOL) xp=CYB_Scale(xp-CYB_TOL); else xp=CYB_Scale(xp+CYB_TOL); else xp=0;
   yp=-inp6d_in->y;
   if (abs(yp)>CYB_TOL) if (yp>CYB_TOL) yp=CYB_Scale(yp-CYB_TOL); else yp=CYB_Scale(yp+CYB_TOL); else yp=0;
   zv=inp6d_in->z;
   if (abs(zv)>CYB_TOL) if (zv>CYB_TOL) zv=CYB_Scale(zv-CYB_TOL); else zv=CYB_Scale(zv+CYB_TOL); else zv=0;

   h=-(short)inp6d_in->ry; if (h>ROT_TOL) h=Rot_Scale(h-ROT_TOL); else if (h<-ROT_TOL) h=Rot_Scale(h+ROT_TOL); else h=0;
   p= (short)inp6d_in->rx; if (p>ROT_TOL) p=Rot_Scale(p-ROT_TOL); else if (p<-ROT_TOL) p=Rot_Scale(p+ROT_TOL); else p=0;
   b=-(short)inp6d_in->rz; if (b>ROT_TOL) b=Rot_Scale(b-ROT_TOL); else if (b<-ROT_TOL) b=Rot_Scale(b+ROT_TOL); else b=0;

#ifdef PLAYTEST
   if (inp6d_dbg)
	   mprintf("Parsed %04x %04x %04x %04x %04x %04x b%x to %d %d %d %d %d %d\n",
	      inp6d_in->x,inp6d_in->y,inp6d_in->z,inp6d_in->rx,inp6d_in->ry,inp6d_in->rz,inp6d_in->but,
	      xp,yp,zv,h,p,b);
#endif

#ifdef PLAYTEST
   if (!inp6d_player)
   {
	   fr_camera_slewcam(NULL,EYE_X,xp/5);
	   fr_camera_slewcam(NULL,EYE_Y,yp/5);
	   fr_camera_slewcam(NULL,EYE_Z,zv/5);
	   fr_camera_slewcam(NULL,EYE_H,h/3);
	   fr_camera_slewcam(NULL,EYE_P,p/3);
	   fr_camera_slewcam(NULL,EYE_B,b/3);
	}
	else
#endif
   {
      if (abs(xp)<12) xp=0;
      if (abs(yp)<12) yp=0;
//      if ((abs(xp)>80)||(abs(yp)>80)) 
//         p=b=0;     // how bout that cyberman, eh?
      if (b!=0)
         xp=yp=zv=p=0;
      if (b>0)
      {
         if (b_vel>0) b_vel+=4; else b_vel=4;
         if (b_vel>CONTROL_MAX_VAL) b_vel=CONTROL_MAX_VAL;
      }
      else if (b<0) 
      {
         if (b_vel<0) b_vel-=4; else b_vel=-4;
         if (b_vel<-CONTROL_MAX_VAL) b_vel=-CONTROL_MAX_VAL;
      }
      else b_vel=0;
      b=b_vel;

      if (p>0)
      {
         if (p_vel>0) p_vel+=6; else p_vel=16;
         if (p_vel>CONTROL_MAX_VAL) p_vel=CONTROL_MAX_VAL;
      }
      else if (p<0) 
      {
         if (p_vel<0) p_vel-=6; else p_vel=-16;
         if (p_vel<-CONTROL_MAX_VAL) p_vel=-CONTROL_MAX_VAL;
      }
      else p_vel=0;
      p=p_vel;
      
//      if ((abs(b_vel)>80)||(abs(p_vel)>80))
//         zv=0; // no posture changes while zipping along in pitch
      if ((b_vel|p_vel)||(abs(xp)>10)||(abs(yp)>10))
         zv=0;       // no longer can posture change while pitch or banking
         // but should be able to jump........ arrrrghghghhh

      if (zv>0)
      {
         if (player_struct.posture!=POSTURE_STAND)
         {
            if (pchange==FALSE)
               player_struct.posture--;
            pchange=TRUE;
	         zv=0;
         }
         else if (pchange)
            zv=0; // dont jump as you stand
//         else
//            pchange=TRUE;  // treat a jump like a posture change...
      }
      else if (zv<0)
      {
         if (player_struct.posture!=POSTURE_PRONE)
            if (pchange==FALSE)
               player_struct.posture++;
         pchange=TRUE;
         zv=0;
      }
      else pchange=FALSE;

#ifdef PLAYTEST
      if (inp6d_link) p=b=0;     // no p or b
#endif
      physics_set_player_controls(INP6D_CONTROL_BANK, xp, yp, zv, h, p, b);
   }
}

#define JOY_USE_CONTROL_MAX_VAL (128)
#define JOY_MAX_VAL      (128)
#define JOY_TOL          (30)

#define SENSITIVITY_CONTROL
#ifdef SENSITIVITY_CONTROL
#define JOY_Scale(x)     fix_mul(inpJoystickSens,(((x)*JOY_USE_CONTROL_MAX_VAL)/(JOY_MAX_VAL-JOY_TOL)))
#else
#define JOY_Scale(x)     (((x)*JOY_USE_CONTROL_MAX_VAL)/(JOY_MAX_VAL-JOY_TOL))
#endif

void joystick_emulate_mouse(int x, int y,uchar bstate,uchar last_bstate)
{
   mouse_add_velocity((x)*abs(x)  << (MOUSE_VEL_UNIT_SHF - APPROX_CIT_CYCLE_SHFT-4),
                      -y*abs(y) << (MOUSE_VEL_UNIT_SHF - APPROX_CIT_CYCLE_SHFT-4));
   // if any button states are changed, generate
   // a low level mouse event.  
   if ((bstate) != (last_bstate))
   {
      // sadly, there is no good api to get at this mouse library
      // variable, so we will employ gnosis for now.  
      extern short mouseInstantButts;
      mouse_event me;

      ui_mouse_get_xy(&me.x,&me.y);
#ifdef SVGA_SUPPORT
      ss_point_convert(&me.x,&me.y,FALSE);
#endif
      me.type = 0;
      me.buttons = (uchar)mouseInstantButts;
      if ((bstate&1) != (last_bstate&1))
      {
         me.type |= (bstate&1) ? MOUSE_LDOWN : MOUSE_LUP;
         if (bstate&1)
            me.buttons |= (1 << MOUSE_LBUTTON);
         else 
            me.buttons &= ~(1 << MOUSE_LBUTTON);
      }
      if ((bstate&2) != (last_bstate&2))
      {
         me.type |= (bstate&2) ? MOUSE_RDOWN : MOUSE_RUP;
         if (bstate&2)
            me.buttons |= (1 << MOUSE_RBUTTON);
         else 
            me.buttons &= ~(1 << MOUSE_RBUTTON);
      }
      me.timestamp = *tmd_ticks;
      mouse_generate(me);
   }
}


void joystick_chk(void)
{
   static uchar last_bstate;
   int xp, yp, zv=0, h, p, b=0;
   char pot_vals[4];
   uchar bstate;

   // this has to be fixed...
   { static bool once=0; if (!once) { once=1; joy_center(); } }   // i wonder if we can punt this???

   bstate=joy_read_buttons();
   if (checking_mouse_button_emulation && (last_bstate != bstate))
   {
      mouse_button_emulated = TRUE;
      last_bstate = bstate;
   }

   if (game_paused)
      return;

   joy_read_pots(pot_vals);

   yp=-((int)pot_vals[1]);
//   mprintf("have %d from %d...",yp,pot_vals[1]);
   if (abs(yp)>JOY_TOL) if (yp>JOY_TOL) yp=JOY_Scale(yp-JOY_TOL); else if (yp<-JOY_TOL) yp=JOY_Scale(yp+JOY_TOL); else yp=0;
   h=pot_vals[0];
//   mprintf("have %d from %d...",h,pot_vals[0]);
   if (abs(h)>JOY_TOL)  if (h>JOY_TOL)  h=JOY_Scale(h-JOY_TOL);   else if (h<-JOY_TOL)  h=JOY_Scale(h+JOY_TOL);   else h=0;

   if (h>CONTROL_MAX_VAL) h=CONTROL_MAX_VAL; else if (h<-CONTROL_MAX_VAL) h=-CONTROL_MAX_VAL;
   if (yp>CONTROL_MAX_VAL) yp=CONTROL_MAX_VAL; else if (yp<-CONTROL_MAX_VAL) yp=-CONTROL_MAX_VAL;
//   mprintf("final %d and %d...",h,yp);

// pitch with throttle here, maybe pedal heading or something
//   if (joystick_count==4)

   if (inp_sidestep)  // toggle h/sidestep, forward/pitch
    { xp=h; p=-yp; h=yp=0; }
   else
      p=xp=0;
//   mprintf("net %d %d %d %d\n",xp,yp,h,p);

#ifdef PLAYTEST
   if (inp6d_dbg)
   {
      if (bstate!=0) mprintf("Yo buttons %2.2x..",bstate); else mprintf("               ");
      mprintf("cntrl %d %d %d %d from pots %d %d %d %d\n",xp,yp,h,p,pot_vals[0],pot_vals[1],pot_vals[2],pot_vals[3]);
   }
#endif

   if (joystick_mouse_emul)
   {
      joystick_emulate_mouse(h,yp,bstate,last_bstate);
      h = yp = xp =  p = 0;
   }
   else if (_current_loop <= FULLSCREEN_LOOP)
   {
      if (bstate&1)
         inp_weapon_button((bstate&1)!=(last_bstate&1));
      // reset on release
      else if ((last_bstate&1) == 1) {
         inp_weapon_junk();
      }
      if (bstate&2)
         inp_use_sidestep_button();
      else
         inp_use_sidestep_junk();
      if (bstate&4)
         zv=MAX_JUMP_CONTROL;

//      if (bstate&8)      // ??, who knows

      if (bstate>=16)   // hat behavior
      {  // coolie... HAT
         switch (bstate&JOY_HAT_MASK)
         {
         case JOY_HAT_N: if (p==0)  p= -CONTROL_MAX_VAL; break;
         case JOY_HAT_E: if (b==0)  b= CONTROL_MAX_VAL; break;
         case JOY_HAT_S: if (p==0)  p= CONTROL_MAX_VAL; break;
         case JOY_HAT_W: if (b==0)  b=-CONTROL_MAX_VAL; break;
	      }
      }
   }

   if (_current_loop <= FULLSCREEN_LOOP)
      physics_set_player_controls(JOYST_CONTROL_BANK, xp, yp, zv, h, p, b);
   last_bstate = bstate;
}

#ifdef PLAYTEST
#include <wsample.h>
#pragma disable_message(202)
bool toggle_profile(short keycode, ulong context, void* data)
{
   static bool UserProf=FALSE;
   if (UserProf)
	   _MARK_("User Off");
   else
	   _MARK_("User On");
   UserProf=!UserProf;
   return TRUE;
}
#pragma enable_message(202)
#endif

#endif //NOT_YET

// ---------
// EXTERNALS
// ---------

void install_motion_mouse_handler(LGRegion* r,frc* fr)
{
   int	cid;
   view3d_data *data = (view3d_data *)NewPtr(sizeof(view3d_data));
   data->ldown = FALSE;
   data->rdown = FALSE;
   data->fr = fr;
   uiInstallRegionHandler(r,UI_EVENT_MOUSE|UI_EVENT_MOUSE_MOVE|UI_EVENT_USER_DEFINED,(uiHandlerProc)view3d_mouse_handler,data,&cid);

   // Yeah, yeah, I know, it's not a mouse handler...
//KLC   uiInstallRegionHandler(r,UI_EVENT_KBD_COOKED, (uiHandlerProc)view3d_key_handler,    NULL, &cid);
   uiSetRegionDefaultCursor(r,NULL);
}


extern void motion_keycheck_handler(uiEvent*,LGRegion*,void*);

void install_motion_keyboard_handler(LGRegion* r)
{
   int cid;
   uiInstallRegionHandler(r,UI_EVENT_KBD_POLL, (uiHandlerProc)motion_keycheck_handler, NULL, &cid);
}


void pop_cursor_object(void)
{
   if (input_cursor_mode != INPUT_OBJECT_CURSOR)
      return;
   object_on_cursor = OBJ_NULL;
   uiPopSlabCursor(&fullscreen_slab);
   uiPopSlabCursor(&main_slab);
   input_cursor_mode = INPUT_NORMAL_CURSOR;
}

extern void push_live_grenade_cursor(ObjID obj);

void push_cursor_object(short obj)
{
   LGPoint hotspot;
   grs_bitmap *bmp;
#ifdef CURSOR_BACKUPS
   extern LGCursor backup_object_cursor;
#endif
   if (objs[obj].obclass == CLASS_GRENADE && objGrenades[objs[obj].specID].flags & GREN_ACTIVE_FLAG)
   {
      push_live_grenade_cursor(obj);
      return;
   }
   uiHideMouse(NULL);
   if ((ID2TRIP(obj) == HEAD_TRIPLE) || (ID2TRIP(obj) == HEAD2_TRIPLE))
      bmp = bitmaps_3d[BMAP_NUM_3D(ObjProps[OPNUM(obj)].bitmap_3d) + objs[obj].info.current_frame];
   else
      bmp = bitmaps_2d[OPNUM(obj)];
   object_on_cursor = obj;
   input_cursor_mode = INPUT_OBJECT_CURSOR;
#ifdef SVGA_SUPPORT
   if (convert_use_mode != 0)
   {
      grs_canvas temp_canv;
      // Get a new bigger bitmap
      gr_init_bm(&svga_cursor_bmp,svga_cursor_bits,BMT_FLAT8,BMF_TRANS,min(MODE_SCONV_X(bmp->w,2),SVGA_CURSOR_WIDTH),
         min(MODE_SCONV_Y(bmp->h,2),SVGA_CURSOR_HEIGHT));
      gr_make_canvas(&svga_cursor_bmp,&temp_canv);

      // Draw into it
      gr_push_canvas(&temp_canv);
      gr_clear(0);
      gr_scale_bitmap(bmp,0,0,svga_cursor_bmp.w,svga_cursor_bmp.h);
      gr_pop_canvas();

      // use it
      bmp = &svga_cursor_bmp;
   }
#endif   
   hotspot.x = bmp->w/2;
   hotspot.y = bmp->h/2;
   uiMakeBitmapCursor(&object_cursor,bmp,hotspot);
#ifdef CURSOR_BACKUPS
   uiMakeBitmapCursor(&backup_object_cursor,bmp,hotspot);
#endif
   uiPushSlabCursor(&fullscreen_slab,&object_cursor);
   uiPushSlabCursor(&main_slab,&object_cursor);
   uiShowMouse(NULL);
   look_at_object(obj);
} 
