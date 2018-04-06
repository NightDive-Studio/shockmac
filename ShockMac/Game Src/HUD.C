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
 * $Source: r:/prj/cit/src/RCS/hud.c $
 * $Revision: 1.67 $
 * $Author: mahk $
 * $Date: 1994/11/15 12:27:09 $
 */

#define __HUD_SRC

#include <string.h>
#include <stdio.h>

#include "hud.h"
#include "hudobj.h"
#include "player.h"
#include "gr2ss.h"
#include "colors.h"
#include "screen.h"
#include "tools.h"
#include "gamescr.h"
#include "effect.h"
#include "objprop.h"
#include "objwpn.h"
#include "objsim.h"
#include "objgame.h"
#include "gamestrn.h"
#include "frtypes.h"
#include "faketime.h"
#include "fullscrn.h"
#include "olhext.h"
#include "cit2d.h"
#include "cybmem.h"
#include "damage.h"

#include "cybstrng.h"
#include "otrip.h"
#include "lvldata.h"
#include "diffq.h"


LGRect target_screen_rect;

// ---------------
// HUD COLOR BANKS
// ---------------
#define RED_CYCLING_COLOR  0x7

ubyte hud_color_bank = 0;
ubyte hud_colors[HUD_COLOR_BANKS][HUD_COLORS_PER_BANK] =
{
   { WHITE, RED_BASE + 3, GREEN_BASE, 0x41, RED_CYCLING_COLOR },
   { 0x38, GREEN_BASE, 0x41,  WHITE, RED_CYCLING_COLOR },
   { GREEN_BASE, 0x41, WHITE, RED_BASE + 3, RED_CYCLING_COLOR },
}; 


#ifndef STORE_CLIP 
#define STORE_CLIP(a,b,c,d) a = gr_get_clip_l(); \
   b = gr_get_clip_t();  c = gr_get_clip_r(); d = gr_get_clip_b()
#endif // !STORE_CLIP

#ifndef RESTORE_CLIP 
#define RESTORE_CLIP(a,b,c,d) gr_set_cliprect(a,b,c,d)
#endif // !RESTORE_CLIP

#define HUDBUFSZ 64

// ---------------------------------------
// HEY, LETS CREATE A HUD LINE ABSTRACTION

typedef struct _hudline
{
   int strid;           // String ID to load string from if null.  
   ubyte color;         // what color should it be?
   ulong mask;          // when should I draw?
   char hudvar_id;      // hud variable to display
   short x;             // coords to display at, default ordering if 0
   short y;             // ditto
   ubyte text;          // What text should I have?
   ulong time;          // how long should I stay around? (0 = forever)
} HudLine;


#define HUD_STRING_SIZE 48
#define NUM_HUDLINE_BUFFERS 6
char hudline_text[NUM_HUDLINE_BUFFERS][HUD_STRING_SIZE+1];

#define X_MARGIN  10
#define Y_MARGIN 7
#define Y_STEP 7
#define HUDLINE_HAS_TEXT(line) ((line)->text != 0)
#define HUDLINE_TEXT(line)     (((line)->text)-1)
#define HUDLINE_SET_TEXT(line,num)  ((line)->text = (ubyte)((num)+1))
#define HUDLINE_X_CENTER -1


HudLine hud_lines[] =
{
   { REF_STR_GametimeLeft,       0, HUD_GAMETIME,        6 },
   { REF_STR_Null,               0, HUD_MSGLINE,         5, HUDLINE_X_CENTER },
   { REF_STR_InfraredOn,         0, HUD_INFRARED,        0 }, 
   { REF_STR_RadiationZone,      0, HUD_RADIATION,       0 }, 
   { REF_STR_BiohazardZone,      0, HUD_BIOHAZARD,       0 },
   { REF_STR_Null,               0, HUD_SHODOMETER,      1, X_MARGIN, 90},
   { REF_STR_HighFatigue,        1, HUD_FATIGUE,         0},
   { REF_STR_ShieldAbsorb,       0, HUD_SHIELD,          2, X_MARGIN, 80},
   { REF_STR_AbnormalGravity1,   0, HUD_ZEROGRAV,        3},
   { REF_STR_CyberFakeID,        0, HUD_FAKEID,          0},
   { REF_STR_CyberDecoy,         0, HUD_DECOY,           0},
   { REF_STR_CyberTurbo,         0, HUD_TURBO,           0},
   { REF_STR_CyberTime,          0, HUD_CYBERTIME,       4, X_MARGIN, 90},
   { REF_STR_CyberDanger,        4, HUD_CYBERDANGER,     0},
   { REF_STR_RadPoison,          1, HUD_RADPOISON,       19},
   { REF_STR_BioPoison,          1, HUD_BIOPOISON,       23},
   { REF_STR_Null,               1, HUD_ENVIROUSE,       8},
//   { REF_STR_Null,               1, HUD_ENVIROUSE,       9},
   { REF_STR_EnergyCritical,     1, HUD_BEAMHOT,         0},
   { REF_STR_EnergyUsage,        0, HUD_ENERGYUSE,       7},
};

#define HUD_LINES (sizeof(hud_lines)/sizeof(hud_lines[0]))
#define FULLSCREEN_Y_OFFSET   40

#define HUDLINE_BUFFER(i) (hudline_text[HUDLINE_TEXT(&hud_lines[i])])

extern Boolean	DoubleSize;


// --------------
//  PROTOTYPES
// --------------
bool hud_color_bank_cycle(short keycode, ulong context, void* data);
void hud_free_line(int i);
void hud_delete_line(int i);
void compute_hud_var(HudLine* hl);
void hud_update_lines(short x, short* y, short xwid, short ywid);
void hud_shutdown_lines(void);



// --------------
//  FUNCTIONS
// --------------
bool hud_color_bank_cycle(short , ulong , void* )
{
   hud_color_bank=(hud_color_bank+1)%HUD_COLOR_BANKS;
   return TRUE;
}

void hud_free_line(int i)
{
   if (HUDLINE_HAS_TEXT(&hud_lines[i]))
   {
      HUDLINE_BUFFER(i)[0] = '\0';
      hud_lines[i].text = 0;
   }
}

void hud_delete_line(int i)
{
   hud_free_line(i);
   hud_lines[i].mask = 0;
}

void compute_hud_var(HudLine* hl)
{
   extern short shield_absorb_perc;
   extern ulong time_until_shodan_avatar;
   extern void second_format(int sec_remain, char *s);

   char* text = hudline_text[HUDLINE_TEXT(hl)];
   int len = strlen(text);
   char* s = text + len;

   switch (hl->hudvar_id)
   {
      case 1:
         lg_sprintf(text,get_temp_string(REF_STR_ShodanHud),
            QUESTVAR_GET(0x10 + player_struct.level) * 100 / player_struct.initial_shodan_vals[player_struct.level]);
         break;
      case 2:
         {
            short use_perc = min(shield_absorb_perc, 95);
            numtostring(use_perc, s);
            strcat(s, " %");
         }
         break;
      case 3:
         numtostring(level_gamedata.hazard.bio * 100 / 4, s);
         len += strlen(s); s+= strlen(s);
         get_string(REF_STR_AbnormalGravity2, s, HUD_STRING_SIZE - len);
         break;
      case 4:
         if (time_until_shodan_avatar > player_struct.game_time)
            second_format((time_until_shodan_avatar - player_struct.game_time) / CIT_CYCLE, s);
         else
         {
            get_string(REF_STR_ShodanNow, s, HUD_STRING_SIZE - len);
            gr_set_fcolor(RED_CYCLING_COLOR);  // a good cycling color
         }
         break;
      case 5: // message line hud.
         {
            extern char last_message[];
            strncpy(s,last_message,HUD_STRING_SIZE);
            break;
         }
      case 6: // game time remaining hud
      {
         int secs=(MISSION_3_TICKS-player_struct.game_time)/CIT_CYCLE;
         if(secs<0) secs=0;
         second_format(secs, s);
         break;
      }
      case 7: // energy usage hud
      {
         extern short enviro_edrain_rate;
         numtostring(player_struct.energy_spend+enviro_edrain_rate,s);
         len+=strlen(s); s+=strlen(s);
         get_string(REF_STR_EnergyUnit,s,HUD_STRING_SIZE-len);
         break;
      }
      case 8: // enviro suit absorption
      {
         extern short enviro_edrain_rate,enviro_absorb_rate;
         lg_sprintf(s,get_temp_string(REF_STR_EnviroAbsorb),enviro_absorb_rate);
         len += strlen(s);
         break;
      }
     case 9: // enviro suit energy drain
      {
         extern short enviro_edrain_rate,enviro_absorb_rate;
         lg_sprintf(s,get_temp_string(REF_STR_EnviroDrain),enviro_edrain_rate);
         len += strlen(s);
         break;
      }
         

      case 16: // 16-23 are reserved for damage exposure notices
      case 19:
      case 23:
         {
            int lvl = player_struct.hit_points_lost[hl->hudvar_id - 16] >> 1; 
            numtostring(lvl,s);
            len += strlen(s); s+= strlen(s);
            get_string(REF_STR_ExposureUnit,s,HUD_STRING_SIZE - len);
         }
   }
}

void hud_update_lines(short x, short* y, short , short )
{
   int i;
   short use_x, use_y;
   extern void strip_newlines(char* buf);

   for (i = 0; i < HUD_LINES; i++)
      if (hud_lines[i].mask & player_struct.hud_modes)
      {
         bool compute_text = FALSE;
         if ((hud_lines[i].time != 0) && (hud_lines[i].time < player_struct.game_time))
         {
            hud_unset(hud_lines[i].mask);
            continue;
         }
         gr_set_fcolor(hud_colors[hud_color_bank][hud_lines[i].color]);
         if (!HUDLINE_HAS_TEXT(&hud_lines[i]))
         {
            int j;
            if (hud_lines[i].strid == 0)
            {
               hud_delete_line(i);
               continue;
            }
            for (j = 0; j < NUM_HUDLINE_BUFFERS; j++)
               if (hudline_text[j][0] == '\0')
               {
                  HUDLINE_SET_TEXT(&hud_lines[i],j);
                  compute_text = TRUE;
                  break;
               }
            if (j >= NUM_HUDLINE_BUFFERS)
            {
               Warning (("No room for one more hudline\n"));
               continue;
            }

         }
         else if (hud_lines[i].hudvar_id != 0)
            compute_text = TRUE;
         if (compute_text)
         {
            get_string(hud_lines[i].strid, HUDLINE_BUFFER(i), HUD_STRING_SIZE);
            compute_hud_var(&hud_lines[i]);
         }
         strip_newlines(HUDLINE_BUFFER(i));
         if (hud_lines[i].x == 0)
            use_x = x;
         else if (hud_lines[i].x == HUDLINE_X_CENTER)
         {
            use_x = x+ (SCREEN_VIEW_WIDTH - gr_string_width(HUDLINE_BUFFER(i)))/2;
         }
         else
            use_x = hud_lines[i].x;
         if (hud_lines[i].y == 0)
         {
            use_y = *y;
            *y += Y_STEP;
         }
         else
         {
            extern bool full_game_3d;
            use_y = hud_lines[i].y;
            if (full_game_3d)
               use_y += FULLSCREEN_Y_OFFSET;
         }
#ifdef STEREO_SUPPORT
         {   
            short temp;
            if (convert_use_mode == 5)
               use_x = 12;
            ss_set_hack_mode(2,&temp);
#endif
            res_draw_text_shadowed(RES_tinyTechFont, HUDLINE_BUFFER(i),use_x,use_y,TRUE);
#ifdef STEREO_SUPPORT
            ss_set_hack_mode(0, &temp);
         }
#endif
      }
      else
         hud_free_line(i);
}

// -----------------------------------------
// HUD COMPASS

void hud_update_compass(short* y,short xmin, short xwid);

#define HUD_COMPASS_ARC    (80*256/360)
#define HALF_COMPASS_ARC   (40*256/360)
#define HUD_COMPASS_OCT    32
#define HUD_COMPASS_STEP   8
#define COMPASS_TICKSCALE  4
#define COMPASS_COLOR 2

void hud_update_compass(short* y,short xmin, short xwid)
{
   short ang, betw;
   ubyte ver = player_struct.hardwarez[CPTRIP(NAV_HARD_TRIPLE)];
   ubyte pang = objs[player_struct.rep].loc.h - HALF_COMPASS_ARC;
   gr_set_fcolor(hud_colors[hud_color_bank][COMPASS_COLOR]);
   for (ang = 0; ang <= 255; ang+=HUD_COMPASS_STEP)
   {
      ubyte adj = ang-pang;
      short x = (int)adj*xwid/HUD_COMPASS_ARC;
      short w,h;
      if (x >= xwid) continue;
      if (ang%HUD_COMPASS_OCT == 0) // draw an octant string
      {
         char  s[4];
         get_string(REF_STR_DirectionAbbrev+ang/HUD_COMPASS_OCT, s, 4);
         gr_string_size(s,&w,&h);
         gr_set_fcolor(hud_colors[hud_color_bank][COMPASS_COLOR]);
         draw_shadowed_string(s,x-w/2+xmin,*y,TRUE);
      }
      else if(ver>1)
      {
         // draw line of height based one between-ness;
         // find lowest set bit in ang
         betw=(ang^(ang&(ang-1)))/COMPASS_TICKSCALE;
         gr_string_size(get_temp_string(REF_STR_DirectionAbbrev),&w,&h);
//KLC         gr_set_fcolor(BLACK);
//KLC         ss_vline(x+1+xmin,*y+((h-betw)/2),*y+((h+betw)/2));
         gr_set_fcolor(hud_colors[hud_color_bank][COMPASS_COLOR]);
         ss_vline(x+xmin,*y+((h-betw)/2)-1,*y+((h+betw)/2)-1);
      }
   }
   *y += Y_STEP;
}

//----------------------------------------------
// critter damage reports
//

static struct _damage_report
{
   short damage;
   ulong tstamp;
   ObjID id;
} hud_critters[4];

#define NUM_HUD_CRITTERS (sizeof(hud_critters)/sizeof(struct _damage_report))

#define TSTAMP_CUTOFF   (CIT_CYCLE/2) // how old can a damage report get before we nuke it. 

#define TARG_EFF_VERSION   3

void hud_report_damage(ObjID target, byte dmglvl);
void draw_target_box(short xl,short yl,short xh,short yh);
void update_damage_report(struct _hudobj_data* dat,bool reverse);


void hud_report_damage(ObjID target, byte dmglvl)
{
   short i, best = 0;
   ulong best_tstamp = 0xFFFFFFFF;
   ubyte ver = player_struct.hardwarez[CPTRIP(TARG_GOG_TRIPLE)];
   if ((dmglvl != DAMAGE_NONE)
      && (dmglvl != DAMAGE_INEFFECTIVE)      // ineffective flag - minman
      && (dmglvl != DAMAGE_STUN)
      && (dmglvl != DAMAGE_TRANQ)
      && (ver < TARG_EFF_VERSION))
      {
         if (ver ==  0)
            return;
         dmglvl = -1;
     }
   for (i = 0; i < NUM_HUD_CRITTERS; i++)
   {
      if (hud_critters[i].id == target
         || hud_critters[i].id == OBJ_NULL)
      {
         best = i;
         break;
      }
      if (hud_critters[i].tstamp < best_tstamp)
      {
         best = i;
         best_tstamp = hud_critters[i].tstamp;
      }
   }
   hudobj_set_id(target,TRUE);
   hud_critters[best].tstamp = player_struct.game_time;
   hud_critters[best].id = target;
   hud_critters[best].damage = dmglvl;
}

void draw_target_box(short xl,short yl,short xh,short yh)
{
	if (DoubleSize)
	{
		xl *= 2;
		yl *= 2;
		xh *= 2;
		yh *= 2;
	}
   short w = (xh-xl)/5;
   short h = (yh-yl)/5;
   gr_vline(xl,yl,yl+h);
   gr_hline(xl,yl,xl+w);
   gr_vline(xl,yh,yh-h);
   gr_hline(xl,yh,xl+w);
   gr_vline(xh,yh,yh-h);
   gr_hline(xh,yh,xh-w);
   gr_vline(xh,yl,yl+h);
   gr_hline(xh,yl,xh-w);
}

void update_damage_report(struct _hudobj_data* dat,bool reverse)
{
   short i;
   for (i = 0; i < NUM_HUD_CRITTERS; i++)
   {
      if (dat->id == hud_critters[i].id)
      {
         short w,h;
         short x,y;

         char buf[80];
         struct _damage_report* rpt = &hud_critters[i];
         if (player_struct.game_time - rpt->tstamp > TSTAMP_CUTOFF)
         {
            rpt->id = OBJ_NULL;
            if (dat->id != player_struct.curr_target)
            {
               hudobj_set_id(dat->id,FALSE);
               dat->id = OBJ_NULL;
            }
         }
         if (dat->id != player_struct.curr_target)
         {
            draw_target_box(dat->xl,dat->yl,dat->xh,dat->yh);
         }
         get_string(REF_STR_TargetDamageBase + rpt->damage,buf,sizeof(buf));
         gr_string_size(buf,&w,&h);
         x = (dat->xl + dat->xh - w)/2;
         y = dat->yl - h;
         if (reverse)
         {
/*���  shock_hflip_in_place is mostly ASM.
            extern void shock_hflip_in_place(grs_bitmap* bm);
            grs_canvas gc;
            grs_font* font = gr_get_font();
            gr_init_canvas(&gc,big_buffer,BMT_FLAT8,w+4,h+4);
            gr_push_canvas(&gc);
            gr_set_font(font);
            gr_clear(0);
            draw_shadowed_string(buf,2,2,TRUE);
            gr_pop_canvas();
            shock_hflip_in_place(&gc.bm);
            gc.bm.flags |= BMF_TRANS;
            ss_bitmap(&gc.bm,x-1,y-2);
*/
         }
         else
         {
#ifdef SVGA_SUPPORT
            extern bool shadow_scale;
            bool old_scale = shadow_scale;
            shadow_scale = FALSE;
#endif
            if (DoubleSize)
            {
               x *= 2;
               y = 2*y + 1;		// Text needed to come down a bit.
            }
            draw_shadowed_string(buf,x,y,TRUE);
#ifdef SVGA_SUPPORT
            shadow_scale =old_scale;
#endif
         }
         break;
      }
   }
}

//-------------------------------------------
// hud_do_objs()
// 
// Deals with all hudobjs.

#define NUM_TARG_FRAMES 5

ubyte targ_frame = NUM_TARG_FRAMES;

#define TARG_COLOR 2

void hud_do_objs(short xtop, short ytop, short xwid, short ywid, bool reverse);


void hud_do_objs(short xtop, short ytop, short , short , bool reverse)
{
   int i;
   extern ObjID beam_effect_id;
//KLC   short a,b,c,d;
//KLC   STORE_CLIP(a,b,c,d);
   PointSetNull(target_screen_rect.ul);
   gr_set_fcolor(hud_colors[hud_color_bank][TARG_COLOR]);
   if (player_struct.curr_target == OBJ_NULL)
      targ_frame = NUM_TARG_FRAMES;

//KLC   safe_set_cliprect(0,0,xwid,ywid);
   gr_set_font((grs_font*)ResLock(RES_tinyTechFont));
   for (i = 0; i < current_num_hudobjs; i++)
   {
      struct _hudobj_data *dat = &hudobj_vec[i];
      if (dat->id == OBJ_NULL) continue;
      update_damage_report(dat,reverse);
      if (dat->id == OBJ_NULL) continue;
      if (dat->id == player_struct.curr_target)
      {
         short w = (dat->xh-dat->xl)/5;
         short h = (dat->yh-dat->yl)/5;
         draw_target_box(dat->xl-targ_frame*w,dat->yl-targ_frame*h,
            dat->xh+targ_frame*w,dat->yh+targ_frame*h);
         if (targ_frame > 0) targ_frame--;
         dat->id = OBJ_NULL;
         target_screen_rect.ul = MakePoint(dat->xl,dat->yl);
         target_screen_rect.lr = MakePoint(dat->xh,dat->yh);
         RECT_MOVE(&target_screen_rect,MakePoint(xtop,ytop));
      // Do other targeting stuff here. 
      }
   }
   current_num_hudobjs = 0;
   if (player_struct.curr_target != OBJ_NULL)
      hudobj_set_id(player_struct.curr_target,TRUE);
   ResUnlock(RES_tinyTechFont);
//KLC   RESTORE_CLIP(a,b,c,d);
}



// ------------------------------------------
// hud_update()

errtype hud_update(bool, frc* context)
{
   extern bool fullscrn_vitals;
   extern bool fullscrn_icons;
   fauxrend_context *fc = (fauxrend_context*)context;
   short y = Y_MARGIN;
   short x = X_MARGIN;
   short xwid = fc->xwid;
//KLC   short a,b,c,d;
//KLC   STORE_CLIP(a,b,c,d);
//KLC   safe_set_cliprect(0,0,fc->xwid,fc->ywid);
   gr_set_font((grs_font*)ResLock(RES_tinyTechFont));

/* TEMP		This is where we display the frame counter, if it is on.
extern Boolean	gShowFrameCounter;
if (gShowFrameCounter)
{
static long		numFrames = 0;
static long		nextTime = 0;
static char		msg[64] = "\0\0\0";
int	x, y;

if (nextTime == 0)
	nextTime = *tmd_ticks + 560;		// Update every 2 seconds
else if (*tmd_ticks > nextTime)
{
	fix_sprint(msg, fix_div(fix_make(numFrames, 0), fix_make(2,0)));
	
	nextTime = *tmd_ticks + 560;	
	numFrames = 0;
}
else
	numFrames++;

if (msg[0])
{
	if (full_game_3d)
	{
		x = 280;
		y = 130;
	}
	else
	{
		x = 240;
		y = 100;
	}
	gr_set_fcolor(76);
	draw_shadowed_string(msg, x, y, TRUE);
}
}
//��� END TEMP
*/
   if (full_game_3d && fullscrn_vitals)
   {
      y = SCREEN_VIEW_Y+Y_MARGIN;
   }
   if (full_game_3d && fullscrn_icons)
   {
      xwid = SCREEN_VIEW_WIDTH;
      x = SCREEN_VIEW_X + X_MARGIN;
   }
   if ((!global_fullmap->cyber) && (player_struct.hud_modes & HUD_COMPASS))
      hud_update_compass(&y,x,xwid);
   hud_update_lines(x,&y,xwid,fc->ywid);

   if (olh_active)
      olh_do_hudobjs(fc->xtop,fc->ytop);
   hud_do_objs(fc->xtop,fc->ytop,fc->xwid,fc->ywid,FALSE);

   ResUnlock(RES_tinyTechFont);
//KLC   RESTORE_CLIP(a,b,c,d);
   return(OK);
}


errtype hud_set(ulong hud_modes)
{
   player_struct.hud_modes |= hud_modes;

   return(OK);
}

errtype hud_unset(ulong hud_modes)
{
   int i;
   player_struct.hud_modes &= ~hud_modes;

   // Clear any times associated
   for (i=0; i < HUD_LINES; i++)
   {
      if (hud_lines[i].mask & hud_modes)
      {
         hud_lines[i].time = 0;
      }
   }
   return(OK);
}

errtype hud_set_time(ulong hud_modes, ulong ticks)
{
   int i;
   hud_set(hud_modes);
   for (i=0; i < HUD_LINES; i++)
   {
      if (hud_lines[i].mask & hud_modes)
      {
         hud_lines[i].time = player_struct.game_time + ticks;
      }
   }
   return (OK);
}


void hud_shutdown_lines(void)
{
   int i;
   for (i = 0; i < HUD_LINES; i++)
   {
      if (hud_lines[i].time > 0)
         hud_unset(hud_lines[i].mask);
   }
}

// --------------------------------------------------
//                HUD WARE/MFD
// --------------------------------------------------

