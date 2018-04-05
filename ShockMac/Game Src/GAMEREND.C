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
 * $Source: u://RCS/gamerend.c $
 * $Revision: 1.77 $
 * $Author: xemu $
 * $Date: 1994/11/25 08:22:19 $
 */

#define __GAMEREND_SRC

#include <stdlib.h>

#include "tools.h"

#include "gamerend.h"
#include "weapons.h"
#include "colors.h"
#include "rcolors.h"
#include "cybstrng.h"   // for resurrect text
#include "gamestrn.h"
#include "citres.h"
#include "gamescr.h"
#include "mainloop.h"
#include "game_screen.h"
#include "player.h"
#include "shodan.h"
#include "criterr.h"

#include "cit2d.h"
#include "diffq.h" // for time limit
#include "newmfd.h"
#include "fullscrn.h"

#include "grenades.h"
#include "hud.h"
#include "hand.h"
#include "wares.h"
#include "rendfx.h"
#include "hudobj.h"     // for beam effect following

// maybe we can not have this
#include "frtypes.h"    // has to be before frprotox so proto gets the context for real
#include "frprotox.h"
#include "frflags.h"
#include "gr2ss.h"

#include "faketime.h"
#include "hkeyfunc.h"

#include "status.h"
#include "statics.h"

#ifdef AUDIOLOGS
#include "audiolog.h"
#endif

extern uchar  tmap_big_buffer[];


// prototypes
void set_shield_raisage(bool going_up);
void begin_shodan_conquer_fx(bool begin);
void set_dmg_percentage(int which, ubyte percent);
void do_secret_fx(void);
void gamesys_render_effects(void);
bool use_ir_hack(void);
void draw_single_static_line(uchar *line_base, int lx, int rx, int c_base);
void draw_line_static(grs_bitmap *stat_dest, int dens1, int color1);
void draw_full_static(grs_bitmap *stat_dest, int c_base);
bool gamesys_draw_func(void *fake_dest_canvas, void *fake_dest_bm, int x, int y, int flags);
void gamesys_render_func(void *fake_dest_bitmap, int flags);

errtype gamerend_init(void)
{
   handart_show = 1;
   return(OK);
}

// note these are now 0-255, like all computer percentages should be...
static ubyte fr_sfx_color;
static ubyte dmg_percentage;
static char shield_raisage;

void set_shield_raisage(bool going_up)
{
   if (going_up) shield_raisage=1; else shield_raisage=-15;
   fr_global_mod_flag(FR_SFX_SHIELD, FR_SFX_MASK);
}

void begin_shodan_conquer_fx(bool begin)
{
   if (begin)
      fr_global_mod_flag(FR_OVERLAY_SHODAN, FR_OVERLAY_MASK);
   else
      fr_global_mod_flag(0, FR_OVERLAY_SHODAN);
}

uchar color_base[]={BLUE_8_BASE,RED_8_BASE,GREEN_8_BASE};

#define MIN_STATIC 10

void set_dmg_percentage(int which, ubyte percent)
{
   if (percent>dmg_percentage)
   {
	   fr_sfx_color=color_base[which];
	   dmg_percentage=percent;
      if (dmg_percentage < MIN_STATIC)
         dmg_percentage = MIN_STATIC;
	   fr_global_mod_flag(FR_SFX_STATIC, FR_SFX_MASK);
   }
}


// -----------------------------------------------
// beam_effect_update()
//

extern ObjID beam_effect_id;

#define PRE_FINAL_DEATH       0x1A
#define DYING_FRAMES          0x20
#define REBORN_VISION_TICK    0x10
#define REBORN_FRAMES         0x18
#define FAKEWIN_FRAMES        0x30

#define FAKEWIN_NUM_FONT      RES_bigLEDFont
#define FAKEWIN_TEXT_FONT     RES_readingFont
#define FAKEWIN_EMAIL_MUNGE   0x01c
#define FAKEWIN_PAPER         0x8
#define FAKEWIN_STRING_BASE   REF_STR_fakewinStrings

extern bool kill_player(void);

// about a quarter second, really
#define V_CLOCK 0x040
#define V_MASK  (~0x03f)

#define build_systems_y_coor(i) (5+(14*i))

static uchar systems_line_colors[]=
  {RED_8_BASE,RED_8_BASE+3,ORANGE_8_BASE,ORANGE_8_BASE+3,GREEN_8_BASE};

//#define NUM_SYS_LINES ((sizeof(systems_lines)/sizeof(systems_lines[0])))
// There is probably a clever way to get this out of the resource, but this is much simpler...
#define NUM_SYS_LINES   5
#define SYSTEM_BASE     REF_STR_ResurrectBase
#define LINE_BUF_SIZE   40

#define CURRENT_VIEW_W (_current_view->r->lr.x-_current_view->r->ul.x)
#define CURRENT_VIEW_H (_current_view->r->lr.y-_current_view->r->ul.y)

extern  void  regenerate_player(void);
ulong secret_sfx_time;
void do_secret_fx(void)
{     // boy is this a hack....
   static char dot_buf[]="........";
   static char tmp_buf[]="99";
   static grs_font *fx_font=NULL;
   static long sfx_time=0;
   int c_val=secret_render_fx&VAL_REND_SFX, i, cap;
   char line_buf[LINE_BUF_SIZE];
   Ref str;

   if (fx_font==NULL)
	   fx_font=(grs_font *)ResLock(RES_mfdFont);
   switch (secret_render_fx&TYPE_REND_SFX)
   {
   case DYING_REND_SFX:          // chevron drain, perhaps dim out, view rock at end.....
      secret_render_fx++;
      flatline_heart=TRUE;
      chi_amp=STATUS_CHI_AMP*(DYING_FRAMES-c_val)/DYING_FRAMES;
      if (c_val==DYING_FRAMES)
      {
         if (kill_player())
         {
            secret_render_fx=0;
            player_struct.dead = FALSE;
            flatline_heart=FALSE;
            chi_amp=STATUS_CHI_AMP;
         }
         else
         {
            secret_render_fx=REBORN_REND_SFX;
		      fr_global_mod_flag(FR_SOLIDFR_SLDKEEP, FR_SOLIDFR_MASK|FR_SFX_MASK);
            fr_solidfr_color=0xff;
            dmg_percentage=0;
            regenerate_player();
         }
      }
      else
      {
         if (c_val>=PRE_FINAL_DEATH)
   		   fr_global_mod_flag(FR_SOLIDFR_STATIC, FR_SOLIDFR_MASK);
		   dmg_percentage = 20+(c_val<<2);
		   fr_global_mod_flag(FR_SFX_STATIC, FR_SFX_MASK);
      }
      break;
   case REBORN_REND_SFX:         // add chevron growth, lighting and dimming, starting on back...
      gr_set_font(fx_font);
      cap=c_val>>2; if (cap>=NUM_SYS_LINES) cap=NUM_SYS_LINES;
      for (i=0; i<cap; i++)
      {
         gr_set_fcolor(systems_line_colors[i]);
         ss_string(dot_buf,30,build_systems_y_coor(i));
         ss_string(get_string(str=(SYSTEM_BASE + i), line_buf, LINE_BUF_SIZE),56,build_systems_y_coor(i));
      }
      if(str==REF_STR_StartHeartString)
         flatline_heart=FALSE;
      else if(str==REF_STR_StartBrainString)
         chi_amp=STATUS_CHI_AMP;
      if (c_val<REBORN_VISION_TICK)
      {
         gr_set_fcolor(systems_line_colors[i]);
	      dot_buf[c_val&7]='\0';
	      ss_string(dot_buf,30,build_systems_y_coor(i));
	      dot_buf[c_val&7]='.';
      }

      if (*tmd_ticks-sfx_time>V_CLOCK)   
      {
         secret_render_fx++;
	      if (c_val==REBORN_FRAMES)
         {
            extern errtype spoof_mouse_event(void); 
            extern bool music_on;
            secret_render_fx=0;

            // look - we should say the player is no longer dead!
            player_struct.dead = FALSE;
            uiFlush();
//KLC           spoof_mouse_event();
            if (music_on)
               start_music();
            break;
         }
	      else if (c_val==REBORN_VISION_TICK)
				fr_global_mod_flag(0, FR_SOLIDFR_MASK);

         sfx_time=(*tmd_ticks)&V_MASK;
      }
      break;
   case TIMELIMIT_REND_SFX:
   {
      grs_font* f;
      short w,h;
      int stage = (MISSION_3_TICKS-player_struct.game_time)/CIT_CYCLE;

      tmp_buf[0]=tmp_buf[1]='0';
      f=(grs_font*)ResLock(FAKEWIN_NUM_FONT);
      gr_set_font(f);
      gr_string_size(tmp_buf,&w,&h);

      if(stage<0 || stage>99) stage=0;
      tmp_buf[0]='0'+stage/10;
      tmp_buf[1]='0'+stage%10;
      draw_shadowed_string(tmp_buf, (CURRENT_VIEW_W-w)/2, (CURRENT_VIEW_H-h)/2, TRUE);
      ResUnlock(FAKEWIN_NUM_FONT);
      break;
   }
   case FAKEWIN_REND_SFX:        // do stuff
      {
         char stage = (*tmd_ticks - secret_sfx_time) / CIT_CYCLE;
         gr_set_fcolor(RED_BASE + 6);
         if (stage < 2)
            break;
         if (stage < 5)
         {
            res_draw_string(FAKEWIN_TEXT_FONT, FAKEWIN_STRING_BASE, 30, 20);
            res_draw_string(FAKEWIN_TEXT_FONT, FAKEWIN_STRING_BASE + 1, 50, 45);
            break;
         }
         if (stage < 18)
         {
            if ((stage > 10) && (c_val == 0))
            {
               fr_global_mod_flag(FR_SFX_SHAKE, FR_SFX_MASK);
               secret_render_fx++;
            }
            res_draw_string(FAKEWIN_TEXT_FONT, FAKEWIN_STRING_BASE + 2, 50, 30);
            tmp_buf[0] = '0' + ((20-stage) / 10);
            tmp_buf[1] = '0' + ((20-stage) % 10);
            res_draw_text(FAKEWIN_NUM_FONT, tmp_buf, 85, 45);
            res_draw_string(FAKEWIN_TEXT_FONT, FAKEWIN_STRING_BASE + 3, 95, 80);
            break;
         }
         if (stage < 22)
         {
            if (c_val == 1)
            {
               extern void long_bark(ObjID speaker_id, uchar mug_id, int string_id, ubyte color);
               extern void add_email_datamunge(short mung,bool select);
               extern void read_email(Id new_base, int num);
               fr_global_mod_flag(FR_SFX_SHAKE, FR_SFX_MASK);
               long_bark(OBJ_NULL, FIRST_SHODAN_MUG + 3, REF_STR_Null, 0);
               mfd_change_slot(MFD_LEFT, MFD_INFO_SLOT);
               mfd_change_slot(MFD_RIGHT, MFD_INFO_SLOT);
//KLC-duplicate mail               add_email_datamunge(FAKEWIN_EMAIL_MUNGE, FALSE);
#ifdef AUDIOLOGS 
               if (audiolog_setting)
                  audiolog_play(FAKEWIN_EMAIL_MUNGE);
               if (audiolog_setting != 1)
#endif
                  read_email(RES_paper0,FAKEWIN_PAPER);
               fr_global_mod_flag(0, FR_SFX_MASK);
               secret_render_fx++;
            }
            gr_set_fcolor(RED_BASE + 6);
            res_draw_string(FAKEWIN_TEXT_FONT, FAKEWIN_STRING_BASE + 4, 75, 50);
            break;
         }
         else
         {
            secret_render_fx = 0;
         }
      }
      break;
   }
   if (secret_render_fx==0)         // why must we unset both
	 { ResUnlock(RES_mfdFont); fx_font=NULL; chg_unset_sta(GL_CHG_2); chg_unset_flg(GL_CHG_2); }
}

extern short mouse_attack_x;
extern short mouse_attack_y;
extern ulong next_fire_time;
extern bool overload_beam;
extern bool saveload_static;
extern Boolean DoubleSize;

byte beam_offset[NUM_BEAM_GUN]={-12,-8,-4};
#define DRAW_BEAM_LINE(c1,c2,c3,c4) { \
     a = mx+(c1);\
     b = my+(c2);\
     ss_point_convert(&a,&b,TRUE);\
     ss_thick_fix_line(fix_make(a,0),fix_make(b,0),fix_make(deltax+(c3)+boff,0),fix_make(deltay+(c4)+boff,0));\
}

/*
#define DRAW_BEAM_LINE(c1,c2,c3,c4) { \
     a = mx+(c1);\
     b = my+(c2);\
     if (DoubleSize) \
        gr_fix_line(fix_make(a,0),fix_make(b,0),fix_make(deltax+(c3)+boff,0), fix_make(deltay+22,0));\
     else \
     { \
        ss_point_convert(&a,&b,TRUE);\
        ss_thick_fix_line(fix_make(a,0),fix_make(b,0),fix_make(deltax+(c3)+boff,0),fix_make(deltay+(c4)+boff,0));\
     } \
}
*/

void gamesys_render_effects(void)
{
	Ref      temp;
	int deltax, deltay;
	short    mx,my;
	extern bool full_game_3d;
	
	if ((!global_fullmap->cyber)&&(!secret_render_fx))
	{
		ubyte active = player_struct.actives[ACTIVE_WEAPON];
		extern bool hack_takeover;
		extern ulong player_death_time;
		
		// check to make sure we have an active weapon before drawing handart
		if ((player_struct.weapons[active].type != EMPTY_WEAPON_SLOT) && !hack_takeover && !saveload_static)
		{
			// For hand-to-hand weapons, draw them in the center of the screen.
			if (player_struct.weapons[active].type == GUN_SUBCLASS_HANDTOHAND)
			{
				mx = (SCREEN_VIEW_WIDTH/2)+SCREEN_VIEW_X;
				my = (SCREEN_VIEW_Y);
				ss_point_convert(&mx,&my,TRUE);
			}
			else if (handart_show != 1)					// Use mouse position for other weapons
			{
				mx = mouse_attack_x;
				my = mouse_attack_y;
				if (!DoubleSize)
					ss_point_convert(&mx,&my,TRUE);
				else
					ui_mouse_get_xy(&mx,&my);
			}
			else													// Not showing a weapon
			{
				ui_mouse_get_xy(&mx,&my);
			}
			
			// Get the weapon art to draw.
			temp = get_handart(&deltax, &deltay,mx, my);
			if (temp != NULL)
			{
				extern bool ready_to_draw_handart(void);
				
				if (handart_show != 1)     // are we showing an attack frame?
				{
					// If this is a beam weapon, we need to draw the beam during attack.
					if (player_struct.weapons[active].type == GUN_SUBCLASS_BEAM)
					{
						short    base_color = (overload_beam) ? BLUE_BASE : TURQUOISE_BASE;
						byte     boff = beam_offset[player_struct.weapons[active].subtype];
						if (beam_effect_id)
						{
							int i;
							bool draw_beam=FALSE;
							
							for (i=0;i<current_num_hudobjs;i++)
							{
								struct _hudobj_data *dat = &hudobj_vec[i];
								if ((dat->id == beam_effect_id) && beam_effect_id)
								{
									mx = (dat->xl + dat->xh)/2;
									my = (dat->yl + dat->yh)/2;
									if (DoubleSize)
									{
										mx *= 2;
										my *= 2;
									}
									draw_beam = TRUE;
								}
 							}
							if (draw_beam)
							{
								short a,b;
								
								gr_set_fcolor(base_color+5);
								if (overload_beam)
								{
									DRAW_BEAM_LINE(-2,0,17,12);
									gr_set_fcolor(base_color+1);
								}
								DRAW_BEAM_LINE(-1,0,18,12);
								gr_set_fcolor(base_color+1);
								DRAW_BEAM_LINE(0,0,19,12);
								if (!overload_beam)
									gr_set_fcolor(base_color+5);
								DRAW_BEAM_LINE(1,0,20,12);
								
								if (overload_beam)
								{
									gr_set_fcolor(base_color+5);
									DRAW_BEAM_LINE(2,0,21,12);
								}
							}
  						}
  					}
  				}
				if ((handart_show != 1) || ready_to_draw_handart())
				{
					draw_hires_resource_bm(temp, SCONV_X(deltax), SCONV_Y(deltay));
					notify_draw_handart();
				}
			}
		}
	}
   
   // Redraw hud displays as appropriate
   // HOW ABOUT A FLAG HERE, NOT HARDCODED LOOP NUMBERS
   if (!secret_render_fx && _current_loop <= FULLSCREEN_LOOP) {
         hud_update(FALSE,_current_fr_context);
   }

   if (secret_render_fx)
      do_secret_fx();

   // draw the gamescreen border
   if (!full_game_3d)
   {
      draw_hires_resource_bm(REF_IMG_bm3dBackground1,0,-2);
      draw_hires_resource_bm(REF_IMG_bm3dBackground2,226,-1);
      draw_hires_resource_bm(REF_IMG_bm3dBackground2,270,-1);
      draw_hires_resource_bm(REF_IMG_bm3dBackground3,522,-2);
      draw_hires_resource_bm(REF_IMG_bm3dBackground4,0,197);

      // whoop whoop whoop!
      // hack alert!  hack alert!
      if (convert_use_mode == 3)
      {
         draw_hires_resource_bm(REF_IMG_bm3dBackground5,27,257);
         draw_hires_resource_bm(REF_IMG_bm3dBackground6,415,257);
      }
   }
   else
      fullscreen_overlay();
}

bool use_ir_hack(void)
{
   return(WareActive(player_struct.hardwarez_status[HARDWARE_GOGGLE_INFRARED]));
}

//���#pragma aux c_ror_by_5 = "ror eax,5" parm [eax] modify exact [eax];
// MLA- optimize this
#define c_ror_by_5(v_to_ror) ((v_to_ror>>5 & 0x07ffffff) | (v_to_ror<<27))


// stolen from RND.LIB, without shift gruesomeness...
#define LC16_MULT 2053
#define LC16_ADD 13849

void draw_single_static_line(uchar *line_base, int lx, int rx, int c_base)
{
   uchar *cur_pix;
   int our_seed=rand();
   for (cur_pix=line_base+lx; lx<rx; lx++, cur_pix++)
   {
#ifdef SIMPLE_LC_WAY
		our_seed = (our_seed * LC16_MULT) + LC16_ADD;
      if (our_seed&0x300)  // 3/4 are colored
         *cur_pix=c_base+(our_seed&0x7);
      else           // 1/4 black
         *cur_pix=0;
#else
      if (our_seed&0x300)
       { *cur_pix=c_base+(our_seed&0x7); our_seed+=(long)cur_pix; c_ror_by_5(our_seed); }
      else
       { *cur_pix=0; our_seed+=(our_seed * LC16_MULT) + LC16_ADD; }
//       { *cur_pix=0; our_seed+=rand(); }
#endif
   }
}

#define LAST_INITIAL 32
// probably have to split it up, and then have a static pass and a translucency pass...
void draw_line_static(grs_bitmap *stat_dest, int dens1, int color1)
{
   int y,last=0,lx,rx,cwid=stat_dest->w;
   uchar *line_base;

   dens1>>=1;
   for (line_base=stat_dest->bits,y=0; y<stat_dest->h; y++,line_base+=stat_dest->row)
   {
      if ((last==LAST_INITIAL)||((rand()&0xff)<(dens1+last)))
      {
         lx=(rand()&0xff)-0x80; if (lx>cwid) lx=cwid-(lx&0x1f); if (lx<0) lx=0;
         rx=(rand()&0xff)-0x80; if (rx>cwid) rx=cwid-(rx&0x1f); if (rx<0) rx=0; rx=cwid-rx;
         if (rx<lx) if (cwid-lx>rx) lx=0; else { lx=rx; rx=cwid; } // gnosis move
         draw_single_static_line(line_base,lx,rx,color1);
         if (last==0) last=LAST_INITIAL; else if (last>1) last>>=1;   // decay repeat freq, stop at 1
      }
      else last=0;
   }
}

void draw_full_static(grs_bitmap *stat_dest, int c_base)
{  // note we do this as a for, not a big fill, so it will work with row hacks, full screen, so on....
   uchar *line_base;
   int y;

   for (line_base=stat_dest->bits,y=0; y<stat_dest->h; y++,line_base+=stat_dest->row)
      draw_single_static_line(line_base,0,stat_dest->w,c_base);
}

#define TELEPORT_COLOR        0x1C
#define VHOLD_SHIFT_AMOUNT 7
short vhold_shift = 0;

#define FULL_CONVERT_X  

// returns whether to send the bitmap out in the render
bool gamesys_draw_func(void *fake_dest_canvas, void *fake_dest_bm, int x, int y, int flags)
{ 
   extern hud_do_objs(short xtop, short ytop, short xwid, short ywid, bool rev);
   grs_canvas *dest_canvas = (grs_canvas *)fake_dest_canvas;
   grs_bitmap *dest_bm = (grs_bitmap *)fake_dest_bm;
   uchar *orig_bits;
   int orig_h, loop, orig_w;

   if (flags&FR_WINDOWD_MASK)
      gamesys_render_effects();     // static gets drawn over window dressing due to this
   else
      hud_do_objs(x,y,dest_bm->w,dest_bm->h,(flags&FR_DOHFLIP_MASK)!= 0);

	if (flags&FR_DOUBLEB_MASK)       // looks like a bug to me, eh?
	{
		switch (flags & FR_SFX_MASK)
		{
			case FR_SFX_VHOLD:
				(*fr_mouse_hide)();
				gr_set_canvas(dest_canvas);
				
				// Save off original state
				orig_bits = dest_bm->bits;
				orig_h = dest_bm->h;
				
				// KLC - adjust x and y if in doublesize mode.
				if (DoubleSize)
				{
					x *= 2;
					y *= 2;
					if (y > 0) y++;		// It's one off in slot view.
				}
				
				// Note that all of this contrivance to keep vhold_shift in original
				// 320x200 coordinates is to avoid the wacky class of bugs of
				// shifting screen mode in the middle of an EMP grenade
				{
					short vhs = vhold_shift;
					if (convert_use_mode)
						vhs = SCONV_Y(vhold_shift);

					// Draw top bitmap
					dest_bm->bits += dest_bm->row * (orig_h - vhs);
					dest_bm->h = vhs;
					if (dest_bm->h != 0)
						gr_bitmap(dest_bm, x, y); 
					
					// Draw bottom bitmap
					dest_bm->bits = orig_bits;
					dest_bm->h = orig_h - vhs;
					if (dest_bm->h != 0)
						gr_bitmap(dest_bm, x, y + vhs);

					// Restore state & increment shift
					dest_bm->h = orig_h;
					if (convert_use_mode)
						vhold_shift += SCONV_Y(VHOLD_SHIFT_AMOUNT);
					else
						vhold_shift += VHOLD_SHIFT_AMOUNT;
					if (convert_use_mode)
						vhs = SCONV_Y(vhold_shift);
					else
						vhs = vhold_shift;
					if (vhs > orig_h)
						vhold_shift = 0;
				}
 				(*fr_mouse_show)();
				return FALSE;
         case FR_SFX_STATIC:
            draw_line_static(dest_bm, dmg_percentage, fr_sfx_color);
            dmg_percentage>>=2;
            if (dmg_percentage==0)
	            fr_global_mod_flag(0,FR_SFX_MASK);
            break;
         case FR_SFX_TELEPORT:
            {
               uchar *p = dest_bm->bits;
               int count = 0;
               while (count < (dest_bm->w * dest_bm->h))
               {
                  *p = TELEPORT_COLOR + (rand()&0x3);
                  p++;
                  count++;
               }
            }
            break;
         case FR_SFX_SHIELD:
            orig_h=(dest_bm->h>>1)-(dest_bm->h>>5)*abs(shield_raisage); 
            orig_w=(dest_bm->w>>1)-(dest_bm->w>>5)*abs(shield_raisage); 
            orig_bits=dest_bm->bits;
            for (loop=1; loop<5; loop++)
	         {
               draw_single_static_line(orig_bits+(orig_h+loop)*dest_bm->w,orig_w+loop,dest_bm->w-orig_w-loop,BLUE_8_BASE);
               draw_single_static_line(orig_bits+(dest_bm->h-(orig_h+loop))*dest_bm->w,orig_w+loop,dest_bm->w-orig_w-loop,BLUE_8_BASE);
            }
            if (shield_raisage&0xf)
               shield_raisage++;
            else
	            fr_global_mod_flag(0,FR_SFX_MASK);
            break;
      }
      switch (flags & FR_OVERLAY_MASK)
      {
         case FR_OVERLAY_SHODAN:
            {
               int i;
               extern ulong time_until_shodan_avatar;
               extern uchar *shodan_bitmask;
               extern grs_bitmap shodan_draw_fs;
               extern grs_bitmap shodan_draw_normal;
               extern char thresh_fail;
               uchar *shodan_draw_bits;
               grs_bitmap *curr_shodan;
               short shodan_level = (player_struct.game_time - time_until_shodan_avatar) >> SHODAN_TIME_SHIFT;
               if (shodan_level > MAX_SHODAN_LEVEL)
                  shodan_level = MAX_SHODAN_LEVEL;
               if (full_game_3d)
               {
                  curr_shodan = &shodan_draw_fs;
                  shodan_draw_bits = shodan_draw_fs.bits;
               }
               else
               {
                  curr_shodan = &shodan_draw_normal;
                  shodan_draw_bits = shodan_draw_normal.bits;
               }
               if ((thresh_fail) || ((rand() & 0x1FF) == 1))
               {
#ifdef SVGA_SUPPORT
                  if (convert_use_mode)
                  {
                     grs_bitmap temp_bm;

                     // Note that we can use this since we know that audiologs aren't playing
                     // and that we don't want normal texture maps
                     gr_init_bitmap(&temp_bm,tmap_big_buffer,BMT_FLAT8,BMF_TRANS,curr_shodan->w,curr_shodan->h);

                     for (i=0; i < temp_bm.h * temp_bm.w; i = i + ((thresh_fail) ? 1 : 2))
                        *(temp_bm.bits + i) = *(shodan_draw_bits + i);

                     // Copy in and scale up the snowy bitmap
                     ss_bitmap(&temp_bm,0,0);
                  }
                  else
#endif
                  {
                     for (i=0; i < dest_bm->h * dest_bm->w; i = i + ((thresh_fail) ? 1 : 2))
                        *(dest_bm->bits + i) = *(shodan_draw_bits + i);
                  }
               }
               else
               {
#ifdef SVGA_SUPPORT
                  if (convert_use_mode)
                  {
                     grs_bitmap temp_bm;

                     // Note that we can use this since we know that audiologs aren't playing
                     // and that we don't want normal texture maps
                     gr_init_bitmap(&temp_bm,tmap_big_buffer,BMT_FLAT8,BMF_TRANS,curr_shodan->w,curr_shodan->h);

                     for (i=0; i < temp_bm.h * temp_bm.w; i++)
                     {
                        if (SHODAN_CONQUER_GET(shodan_bitmask, i))
                           *(temp_bm.bits + i) = *(shodan_draw_bits + i);
                        else
                           *(temp_bm.bits + i) = 0;
                     }
                     // Copy in and scale up the snowy bitmap
                     ss_bitmap(&temp_bm,0,0);
                  }
                  else
#endif
                  {
                     for (i=0; i < dest_bm->h * dest_bm->w; i++)
                     {
                        if (SHODAN_CONQUER_GET(shodan_bitmask, i))
                           *(dest_bm->bits + i) = *(shodan_draw_bits + i);
                     }
                  }
               }
            }
            break;
      }
   }

   return TRUE;         // let the renderer do the blit
}

void gamesys_render_func(void *fake_dest_bitmap, int flags)
{
   grs_bitmap *dest_bitmap = (grs_bitmap *)fake_dest_bitmap;
   grs_canvas temp_canvas;

   gr_make_canvas(dest_bitmap, &temp_canvas);
   gr_push_canvas(&temp_canvas);
   switch(flags & FR_SOLIDFR_MASK)
   {
   case FR_SOLIDFR_SLDCLR:    // if clearing, reset the mask
      fr_global_mod_flag(0,FR_SOLIDFR_MASK);
   case FR_SOLIDFR_SLDKEEP:   // else just clear to the color
      gr_clear(fr_solidfr_color);
      break;
   case FR_SOLIDFR_STATIC:
      draw_full_static(dest_bitmap,GRAY_8_BASE);
      break;
   }
   gr_pop_canvas();
}

