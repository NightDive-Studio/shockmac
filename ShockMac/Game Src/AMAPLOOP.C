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
 * $Source: r:/prj/cit/src/RCS/amaploop.c $
 * $Revision: 1.42 $
 * $Author: tjs $
 * $Date: 1994/11/20 18:02:29 $
 *
 * full screen map stuff
 */

#include <string.h>

#include "ShockBitmap.h"

#include "audiolog.h"
#include "criterr.h"
#include "tools.h"
#include "gamescr.h"
#include "rcolors.h"
#include "mainloop.h"
#include "amaploop.h"
#include "lvldata.h"
#include "faketime.h"
#include "map.h"
#include "otrip.h"
#include "objgame.h"
#include "objsim.h"
#include "player.h"
#include "wares.h"
#include "cybstrng.h"
#include "gamestrn.h"
#include "musicai.h"
#include "cit2d.h"

#include "gr2ss.h"
#include "fullscrn.h" // for the full screen frame buffer canvas
#include "frprotox.h"
#include "frflags.h"
#include "screen.h"

// octant-wise, that is...
#define NORTH 0
#define EAST 2
#define SOUTH 4
#define WEST 6

// room for a message line on the bottom of the screen...

#define AMAP_BUTTON_WIDTH	92
#define AMAP_HEADER_HGT		11
#define AMAP_BORDER				4
#define AMAP_TOP(h)				(AMAP_HEADER_HGT+AMAP_BORDER)
#define AMAP_LFT(w)				(AMAP_BORDER)
#define AMAP_BOT(h)        			(h-1-AMAP_BORDER-AMAP_HEADER_HGT)
#define AMAP_RGT(w)        			(w-1-AMAP_BUTTON_WIDTH-AMAP_BORDER)
#define AMAP_HGT(h)        			(AMAP_BOT(h)-AMAP_TOP(h))
#define AMAP_WID(w)        		(AMAP_RGT(w)-AMAP_LFT(w))

#define FSMAP_MAX_MSG			40

#define MOUSE_UPS   (MOUSE_LUP|MOUSE_RUP|MOUSE_CUP)
#define MOUSE_DOWNS (MOUSE_LDOWN|MOUSE_RDOWN|MOUSE_CDOWN)

// spoofs for calling kb callback to do stuff
#define DO_ZOOMOUT   KEY_PGUP
#define DO_ZOOMIN    KEY_PGDN
#define DO_RIGHT     KEY_RIGHT
#define DO_LEFT      KEY_LEFT
#define DO_UP        KEY_UP
#define DO_DOWN      KEY_DOWN
#define DO_QUIT      KEY_ESC
//#define DO_MSG       KEY_ENTER
#define DO_CHEAT     'f'
#define DO_SCAN      'r'
#define DO_CRITTER   'c'
#define DO_SECUR     's'
#define DO_RECENTER  KEY_HOME
#define DO_FULLMSG   'm'


#define BTN_RECENTER  2
#define BTN_FULLMSG   3
#define BTN_CHEAT     6
#define BTN_SCAN      6
#define BTN_CRITTER   5
#define BTN_SECURE    4
#define BTN_ZOOMIN    0
#define BTN_ZOOMOUT   1
#define BTN_PANREG    8

#define BTN_NUM_TOT   9
#define BTN_NUM_REAL  7

#define BTN_TALK  0xdead   // hack code for todo
#define BTN_PEND  0xbeef

extern void amap_pixratio_set(fix);

// limit framerate while scrolling to avoid flicker?
// should really do something real about flicker anyway.
#define SCROLL_FRATE 90

frc* full_map_context;
#define FULLMAP_CANVAS ((grs_canvas*)fr_get_canvas(full_map_context))
//was full_game_fr_context

#define FSMAP_OPP 0x8000
// note this makes the init code 0b00010100000101, or 0x505
static ushort btn_to_code[]={DO_ZOOMIN,DO_ZOOMOUT,DO_RECENTER,DO_FULLMSG,DO_SECUR,DO_CRITTER,DO_SCAN,0};

static ushort btn_to_amap[]={0,0,FSMAP_OPP|AMAP_TRACK_OBJ,AMAP_FULL_MSG,AMAP_SHOW_SEC,AMAP_SHOW_CRIT|AMAP_SHOW_ROB,AMAP_SHOW_SENS,0};

#define NUM_SIDE_BUTTONS 7

#define BOTTOM_BUTTONS_INDEX 7

// should alloc and dealloc these?, or take them out of some memory pool
static grs_canvas fsmap_actual, fsmap_bregion;
static ushort     fsmap_buttons, fsmap_btn_pending;
static uchar      bcolor[]={GREEN_8_BASE+7,GREEN_8_BASE+3,GREEN_8_BASE,GREEN_8_BASE+1};
static int        cur_btn_hgt;
static char      *cur_mapnote_base=NULL;
static char      *cur_mapnote_ptr=NULL;
static bool       last_msg_ok=TRUE;
static ulong      map_scrolltime=0L;
static int      map_scroll_d=0;
static char       map_scroll_code=0;

bool pend_check(void);

// default units per second
// in defiance of Rob, I use the number 13.
#define MAP_SCROLL_SPEED 13
                                     
#define clear_cur_mapnote() cur_mapnote_base=cur_mapnote_ptr=NULL

// --------------------
//  INTERNAL PROTOTYPES
// --------------------
void trail_sp_punt(void);
void fsmap_button_redraw(void);
char *fsmap_get_lev_str(char* buf, int siz);
void fsmap_interface_draw(void);
void fsmap_message_redraw(void);
void fsmap_draw_map(void);
void fsmap_draw_screen(uint chng);
int s_bf(int btn_id,int todo);
void fsmap_new_msg(curAMap *amptr);
bool amap_scroll_handler(uiEvent* ev, LGRegion *r, void* user_data);
void edit_mapnote(curAMap* amptr);
bool amap_ms_callback(curAMap *amptr,int x,int y,short action,ubyte but);
bool zoom_deal(curAMap *amptr, int btn);
bool flags_deal(curAMap *amptr, int btn, int todo);
void btn_init(curAMap *amptr);


// The devil drives a Buick
// He sits inside and eats lunch
// Then he sticks his pitchfork through the trunk and into the spare
// and he pull out True Love

void fsmap_startup(void)
{
	void btn_init(curAMap *amptr);
	int 			i, n=0, f, b, todo;
	grs_font	*fsmap_font;

	// Do appropriate stuff to enter into amap mode here....
	automap_init(player_struct.hardwarez[HARDWARE_AUTOMAP],MFD_FULLSCR_MAP);
	f=oAMap(MFD_FULLSCR_MAP)->flags;
	oAMap(MFD_FULLSCR_MAP)->flags = 0;
	oAMap(MFD_FULLSCR_MAP)->zoom = 3;			//KLC - added for 640x480 Mac fullscreen map
	for(i=0;i<NUM_MFDS;i++)
	{
		if(oAMap(i))
		{
			oAMap(MFD_FULLSCR_MAP)->flags|=oAMap(i)->flags;
			n++;
		}
	}
	if((n==0) || (oAMap(MFD_FULLSCR_MAP)->flags == 0))
		oAMap(MFD_FULLSCR_MAP)->flags = f;
	
	// Get the graphics system setup for fullscreen drawing.
	full_map_context = fr_place_view(FR_NEWVIEW, FR_DEFCAM, gMainOffScreen.Address, 
														FR_DOUBLEB_MASK|FR_WINDOWD_MASK, 0, 0, 
														0, 0, 640, 480);
	gr_set_canvas(FULLMAP_CANVAS);
	gr_clear(0xff);
	amap_pixratio_set(FIX_UNIT);
	fsmap_font=(grs_font *)ResLock(RES_largeTechFont);		// KLC - was RES_mfdFont
	gr_set_font(fsmap_font);
	gr_init_sub_canvas(FULLMAP_CANVAS,&fsmap_actual,AMAP_LFT(grd_bm.w),AMAP_TOP(grd_bm.h),AMAP_WID(grd_bm.w),AMAP_HGT(grd_bm.h));
	gr_init_sub_canvas(FULLMAP_CANVAS,&fsmap_bregion,AMAP_RGT(grd_bm.w)+AMAP_BORDER,AMAP_TOP(grd_bm.h),AMAP_BUTTON_WIDTH-2*AMAP_BORDER,AMAP_HGT(grd_bm.h));
	fsmap_actual.gc.font=fsmap_font;
	fsmap_bregion.gc.font=fsmap_font;
	fsmap_buttons=(1<<(2*BTN_ZOOMIN))|(1<<(2*BTN_ZOOMOUT));
	for(i=0;i<NUM_SIDE_BUTTONS;i++)
	{
		b=btn_to_amap[i];
		if(b & oAMap(MFD_FULLSCR_MAP)->flags)
		{
			if(i==BTN_RECENTER)
				todo=AMAP_UNSET;
			else
				todo=AMAP_SET;
			s_bf(i,todo);
			pend_check();
		}
	}
	fsmap_btn_pending=0;
//KLC - seems to be causing a problem		btn_init(oAMap(MFD_FULLSCR_MAP));
	
	cur_btn_hgt=(((AMAP_HGT(grd_bm.h))*3)>>5);
	
	chg_set_flg(LL_CHG_MASK);
}

void fsmap_free(void)
{
   int i;
   for(i=0;i<NUM_MFDS;i++) {
      oAMap(i)->flags=oAMap(MFD_FULLSCR_MAP)->flags;
      oAMap(i)->flags |= AMAP_TRACK_OBJ;
   }
   ResUnlock(RES_largeTechFont);				//KLC - was RES_mfdFont
   gr_set_canvas(grd_screen_canvas);
}

void trail_sp_punt(void)
{
   if(cur_mapnote_ptr==NULL) return;
   if (*cur_mapnote_ptr=='\0') {
      while (cur_mapnote_ptr>cur_mapnote_base)
         if ((*(cur_mapnote_ptr-1))!=' ')
            break;
         else cur_mapnote_ptr--;
      *cur_mapnote_ptr='\0';
   }
}

#define BTN_HGT_MUL    (cur_btn_hgt)
#define GET_BTN_TOP(x) ((x)*BTN_HGT_MUL)
#define GET_BTN_BOT(x) (GET_BTN_TOP(x+1)-AMAP_BORDER)
#define BUTTON_BUF_SIZE    10
#define AMAP_BUTTON_BASE   REF_STR_AutomapButtons

void fsmap_button_redraw(void)
{
   int i, cb, bsx, bsy, cx, cy;
   char button_buf[BUTTON_BUF_SIZE];

   gr_push_canvas(&fsmap_bregion);
   for (cb=fsmap_buttons,i=0; i<NUM_SIDE_BUTTONS; i++,cb>>=2)
   {
      gr_set_fcolor(bcolor[0]);
      ss_box(AMAP_BORDER,GET_BTN_TOP(i),grd_bm.w-AMAP_BORDER+3,GET_BTN_BOT(i));
      gr_set_fcolor(bcolor[cb&3]);
      ss_string(get_string(AMAP_BUTTON_BASE + i, button_buf, BUTTON_BUF_SIZE),2*AMAP_BORDER,GET_BTN_TOP(i)+AMAP_BORDER+10);
   }

   i=BOTTOM_BUTTONS_INDEX;

   gr_set_fcolor(bcolor[0]);

   // draw the pan controller... or have mini map...
   cx=AMAP_BORDER+1;
   cy=GET_BTN_TOP(i);
   bsx=grd_bm.w-AMAP_BORDER+3;
   bsy=grd_bm.h-BTN_HGT_MUL-AMAP_BORDER;

   ss_box(cx,cy,bsx,bsy);
   bsx-=cx; bsy-=cy;

   ss_int_line(cx,cy,cx+bsx-1,cy+bsy-1);
   ss_int_line(cx+bsx-1,cy,cx,cy+bsy-1);
   ss_string(get_temp_string(REF_STR_DirectionAbbrev+NORTH),cx+(bsx>>1)-3, cy+(bsy>>2)-3);
   ss_string(get_temp_string(REF_STR_DirectionAbbrev+EAST),cx+(bsx>>1)+(bsx>>2)-2+AMAP_BORDER, cy+(bsy>>1)-3);
   ss_string(get_temp_string(REF_STR_DirectionAbbrev+SOUTH),cx+(bsx>>1)-3, cy+bsy-(bsy>>2)-3);
   gr_string(get_temp_string(REF_STR_DirectionAbbrev+WEST),cx+(bsx>>3)-2+AMAP_BORDER, cy+(bsy>>1)-3);

   // done button
   ss_box(AMAP_BORDER,grd_bm.h-BTN_HGT_MUL,grd_bm.w-AMAP_BORDER+3,grd_bm.h);
   gr_set_fcolor(bcolor[cb&3]);
   ss_string(get_string(AMAP_BUTTON_BASE + i, button_buf, BUTTON_BUF_SIZE),2*AMAP_BORDER,(grd_bm.h)-BTN_HGT_MUL+AMAP_BORDER+12);

   gr_pop_canvas();
   chg_unset_flg(AMAP_BUTTON_EV);
}

char *fsmap_get_lev_str(char* buf, int siz)
{
   extern char *level_to_floor(int lev_num, char *buf);
   int l;
   
   get_string(REF_STR_Level,buf,siz);
   l=strlen(buf);
   if(l+3 < siz) {
      buf[l]=' ';
      level_to_floor(player_struct.level,buf+l+1);
   }
   return buf;
}

#define TRIOP_BUF_SIZE     50
#define TRIOP_STRING_BASE  REF_STR_AutomapSpew
void fsmap_interface_draw(void)
{
   char buf[TRIOP_BUF_SIZE];
   gr_set_fcolor(GREEN_8_BASE);
   ss_box(0,0,grd_bm.w-1,grd_bm.h-1);
   ss_box(AMAP_LFT(grd_bm.w)-1,AMAP_TOP(grd_bm.h)-1,AMAP_RGT(grd_bm.w)+1,AMAP_BOT(grd_bm.h)+1);

   gr_set_fcolor(RED_8_BASE);
   ss_string(get_string(TRIOP_STRING_BASE, buf, TRIOP_BUF_SIZE),AMAP_LFT(grd_bm.w),AMAP_BORDER);

   ss_string(fsmap_get_lev_str(buf,TRIOP_BUF_SIZE),AMAP_RGT(grd_bm.w)+2*AMAP_BORDER,AMAP_BORDER);

   chg_unset_flg(AMAP_FULLEXPOSE);
}

#define MSG_BUF2_SIZE   25 
void fsmap_message_redraw(void)
{
   char buf[FSMAP_MAX_MSG];
   char buf2[MSG_BUF2_SIZE];
   short x,y,w,dummy;

   gr_set_font(fsmap_actual.gc.font);  	// so we dont need this as another global, ick
   gr_set_fcolor(0xFF);                   		// and someone keeps secretly reseting the font to ickiness
   ss_rect(AMAP_LFT(grd_bm.w),AMAP_BOT(grd_bm.h)+1+2,grd_bm.w-2,grd_bm.h-2);     // -2 to miss the border
   x=AMAP_LFT(grd_bm.w);
   y=AMAP_BOT(grd_bm.h)+1+2;
   if (last_msg_ok)
   {
      if (cur_mapnote_base==NULL) {
         gr_set_fcolor(GREEN_8_BASE+5);
      }
      else {
         gr_set_fcolor(RED_8_BASE+2);
         strcpy(buf,"> ");
         gr_string_size(buf,&w,&dummy);
         ss_string(buf,x,y);
         ss_string(buf,x+1,y);
         x+=w+1;
      }
      amap_get_note(oAMap(MFD_FULLSCR_MAP),buf);
      ss_string(buf,x,y);      // +1 to get out of box, 2 for pad?
   }
   else
    { gr_set_fcolor(RED_8_BASE+4); ss_string(get_string(REF_STR_NoMessage, buf2, MSG_BUF2_SIZE),AMAP_LFT(grd_bm.w),AMAP_BOT(grd_bm.h)+1+2); }
   if (cur_mapnote_base!=NULL)
   {
      gr_set_fcolor(PULSE_RED);
      ss_vline(x+gr_string_width(buf)+2  ,AMAP_BOT(grd_bm.h)+3,AMAP_BOT(grd_bm.h)+8);
      ss_vline(x+gr_string_width(buf)+2+1,AMAP_BOT(grd_bm.h)+3,AMAP_BOT(grd_bm.h)+8);
   }
   chg_unset_flg(AMAP_MESSAGE_EV);
}

void fsmap_draw_map(void)
{
	FrameDesc	*f;
//   short w,h;

//   w=res_bm_width(REF_IMG_bmTriLogoBack);
//   h=res_bm_height(REF_IMG_bmTriLogoBack);
   gr_push_canvas(&fsmap_actual);
   gr_clear(0xff);

//KLC - changed to draw the background logo double size
//   draw_res_bm(REF_IMG_bmTriLogoBack,(grd_bm.w-w)/2,(grd_bm.h-h)/2);
   f = (FrameDesc *)RefLock(REF_IMG_bmTriLogoBack);
   if (f == NULL)
      critical_error(CRITERR_MEM|9);
   f->bm.bits = (uchar *)(f+1);
   gr_scale_bitmap(&f->bm, (grd_bm.w - (f->bm.w*2))/2, (grd_bm.h - (f->bm.h*2))/2,
   							f->bm.w*2, f->bm.h*2);
   RefUnlock(REF_IMG_bmTriLogoBack);

   amap_draw(oAMap(MFD_FULLSCR_MAP),0);
   gr_pop_canvas();
   chg_unset_flg(AMAP_MAP_EV);
}

#define AMAP_ALLEVS (AMAP_FULLEXPOSE|AMAP_MAP_EV|AMAP_BUTTON_EV|AMAP_MESSAGE_EV)

void fsmap_draw_screen(uint chng)
{
   int l,t,r,b;
   LGRect cr;

   gr_push_canvas(grd_screen_canvas);
   if((chng&AMAP_ALLEVS)==AMAP_MAP_EV) {
      cr.ul.x=AMAP_LFT(grd_bm.w);
      cr.ul.y=AMAP_TOP(grd_bm.h);
      cr.lr.x=AMAP_LFT(grd_bm.w)+AMAP_WID(grd_bm.w);
      cr.lr.y=AMAP_TOP(grd_bm.h)+AMAP_HGT(grd_bm.h);
      gr_get_cliprect(&l,&t,&r,&b);
      ss_safe_set_cliprect(cr.ul.x,cr.ul.y,cr.lr.x,cr.lr.y);
      uiHideMouse(&cr);
      ss_bitmap(&(FULLMAP_CANVAS->bm),0,0);
      uiShowMouse(&cr);
      ss_safe_set_cliprect(l,t,r,b);
   }
   else {
      uiHideMouse(NULL);
      ss_bitmap(&(FULLMAP_CANVAS->bm),0,0);
      uiShowMouse(NULL);
   }
   gr_pop_canvas();
}

extern void mlimbs_do_ai();

void automap_loop(void)
{
   extern void loop_debug(void);
   uint cf = _change_flag;

//KLC - does nothing      loopLine(GL|0x1D,synchronous_update());
   if (music_on)
      loopLine(GL|0x1C,mlimbs_do_ai());
   if (localChanges)
   {
      if (_change_flag&AMAP_FULLEXPOSE) { loopLine(AL|0x1, fsmap_interface_draw()); }
      if (_change_flag&AMAP_MAP_EV)     { loopLine(AL|0x2, fsmap_draw_map()); }
      if (_change_flag&AMAP_BUTTON_EV)  { loopLine(AL|0x3, fsmap_button_redraw()); }
      if (_change_flag&AMAP_MESSAGE_EV) { loopLine(AL|0x4, fsmap_message_redraw()); }
   }
//   if (pal_fx_on) {
//      loopLine(AL|0x41,palette_advance_all_fx(*tmd_ticks));
//   }
   audiolog_loop_callback();	
   if(cf&(AMAP_FULLEXPOSE|AMAP_MAP_EV|AMAP_BUTTON_EV|AMAP_MESSAGE_EV)) {
      fsmap_draw_screen(cf);
   }
}

int s_bf(int btn_id,int todo)
{
   int prtlmask=1<<(btn_id<<1);
   int fullmask=prtlmask+(prtlmask<<1);
   switch (todo)
   {
      case AMAP_TOGGLE: fsmap_buttons=(fsmap_buttons&~fullmask)+((fsmap_buttons&fullmask)^prtlmask); break; // xor on/off
      case AMAP_SET:    fsmap_buttons=(fsmap_buttons&~fullmask)+prtlmask;                            break; // set on/off to on
      case AMAP_UNSET:  fsmap_buttons=(fsmap_buttons&~fullmask);                                     break; // set on/off to off
      case BTN_TALK:    return fsmap_buttons&prtlmask;           // return whether this is on or off
      case BTN_PEND:    break;
   }
   fsmap_btn_pending|=(prtlmask<<1);
   fsmap_buttons|=fsmap_btn_pending;
   chg_set_flg(AMAP_BUTTON_EV);
   return fsmap_buttons&fullmask;                     // heck, why not
}

void fsmap_new_msg(curAMap *amptr)
{
   cur_mapnote_base=amap_str_next();
   amap_note_value(amptr->note_obj)=amap_str_deref(cur_mapnote_base);
   cur_mapnote_ptr=cur_mapnote_base;
   *cur_mapnote_ptr='\0';
   chg_set_flg(AMAP_MAP_EV);
}

bool pend_check(void)
{
   if (fsmap_btn_pending)
   {
//      mprintf("UnPend %x (c %x)...",fsmap_btn_pending,fsmap_buttons);
      fsmap_buttons&=~fsmap_btn_pending;
      fsmap_btn_pending=0;
      chg_set_flg(AMAP_BUTTON_EV);
//      mprintf("done %x\n",fsmap_buttons);
      return TRUE;
   }
   return FALSE;
}


#define hack_kb_callback(am,k) amap_kb_callback(am,k|KB_FLAG_DOWN)

#define UP_ARROW_CODE  0xc8
#define DOWN_ARROW_CODE  0xd0
#define LEFT_ARROW_CODE  0xcb
#define RIGHT_ARROW_CODE  0xcd

bool amap_scroll_handler(uiEvent* ev, LGRegion *, void* )
{
   int elapsed, now;
   short code;
   curAMap* amptr=oAMap(MFD_FULLSCR_MAP);

   if(map_scroll_code==0) return FALSE;

   if(ev->type==UI_EVENT_KBD_POLL) {
      map_scroll_code=0;
      code=((uiRawKeyEvent*)ev)->scancode;
      switch(code) {
         case UP_ARROW_CODE: map_scroll_code=AMAP_PAN_N; break;
         case DOWN_ARROW_CODE: map_scroll_code=AMAP_PAN_S; break;
         case LEFT_ARROW_CODE: map_scroll_code=AMAP_PAN_W; break;
         case RIGHT_ARROW_CODE: map_scroll_code=AMAP_PAN_E; break;
      }
      if(map_scroll_code==0) return FALSE;
   }

   now=*tmd_ticks;
   elapsed=now-map_scrolltime;
   if(elapsed<(CIT_CYCLE/SCROLL_FRATE))
      return TRUE;
   map_scrolltime=now;
   map_scroll_d += (elapsed*MAP_SCROLL_SPEED*AMAP_DEF_DST)/CIT_CYCLE;
   amap_pan(amptr,map_scroll_code,&map_scroll_d);
   s_bf(BTN_RECENTER,AMAP_SET);
   pend_check();
   chg_set_flg(AMAP_MAP_EV);
   return TRUE;
}

void edit_mapnote(curAMap* amptr)
{
   if ((cur_mapnote_ptr==NULL)&&(amptr->note_obj)) {
      char buf[FSMAP_MAX_MSG];
      strcpy(buf,amap_note_string(amptr->note_obj));
      amap_str_delete(amap_note_string(amptr->note_obj));
      fsmap_new_msg(amptr);
      strcpy(cur_mapnote_base,buf);
      cur_mapnote_ptr=cur_mapnote_base+strlen(cur_mapnote_base);
      chg_set_flg(AMAP_MESSAGE_EV);
   }
}

bool amap_ms_callback(curAMap *amptr,int x,int y,short action,ubyte )
{
	extern void mouse_unconstrain(void);
	int scregion=0;
	
	if(action&MOUSE_UPS)
	{
		mouse_unconstrain();
		if(map_scroll_code!=0)
		{
			map_scrolltime=0;
			map_scroll_code=0;
		}
		pend_check();
		return TRUE;
	}

	if (!(action&MOUSE_DOWNS))
		return FALSE;

	ui_mouse_constrain_xy(x,y,x,y);
	
	y -= AMAP_TOP(grd_bm.h);
	
	if (y > AMAP_HGT(grd_bm.h))				  							// if click in the message line
	{
		scregion=AMAP_MESSAGE_EV;
		edit_mapnote(amptr);													// start editing.
	}
	
	else if (x>AMAP_RGT(grd_bm.w))									// If in the button region…
	{
		if (y<GET_BTN_TOP(7)-AMAP_BORDER)   					// If in one of the upper buttons (above pan)
		{
			// KLC - changed to treat clicks and keyboard equivalents a little differently
			// 		  in the Mac version.  Doesn't call the hack_kb_callback anymore.
			
			int 	btn = y/BTN_HGT_MUL;
   			char	todo = AMAP_TOGGLE;
			
			if (btn == BTN_ZOOMIN || btn == BTN_ZOOMOUT)
			{
				if (zoom_deal(amptr, btn))
         				chg_set_flg(AMAP_MAP_EV);
			}
			else
			{
				if (btn == BTN_RECENTER)
					todo=AMAP_UNSET;
         			if (flags_deal(amptr, btn, todo))
         				chg_set_flg(AMAP_MAP_EV);
			}
		}

		else if (y>AMAP_HGT(grd_bm.h)-BTN_HGT_MUL)			// If in the "Done" button
		{
			if (y < AMAP_HGT(grd_bm.h)) 								// quit automap... that was the done button kids.
				hack_kb_callback(amptr,DO_QUIT);
		}

		else  																			// Else we must be in the pan region
		{
			x -= AMAP_RGT(grd_bm.w)+2*AMAP_BORDER+1; 	// normalize to middle of pan region
			x -= 38;
			x *= 2;
			y -= GET_BTN_TOP(7);
			y -= 54;
			y *= 3;
			
			map_scrolltime=*tmd_ticks;
			
			if ((abs(abs(x)-abs(y)))<3)
				return TRUE; // null pan area...
			if (abs(x)>abs(y))   // ew
				if (x>0)
					map_scroll_code=AMAP_PAN_E;
				else
					map_scroll_code=AMAP_PAN_W;
			else
				if (y>0)
					map_scroll_code=AMAP_PAN_S;
				else
					map_scroll_code=AMAP_PAN_N;
		}
   }
   else
   {
      void *deal_data;
      ObjID prev_note;

      scregion=AMAP_MAP_EV;
      x-=AMAP_LFT(grd_bm.w);
      if ((x<=0)||(y<=0)) return TRUE;  // not really in map
      prev_note=amptr->note_obj;
      if ((deal_data=amap_deal_with_map_click(amptr,&x,&y))!=NULL)
      {
         if (amptr->note_obj!=prev_note) {
            trail_sp_punt();
            if(cur_mapnote_base!=NULL) {
               if(*cur_mapnote_base=='\0') {
                  obj_destroy(prev_note);
               }
               else
                  amap_str_grab(cur_mapnote_base);
            }
            clear_cur_mapnote();
         }
         last_msg_ok=TRUE;
         chg_set_flg(AMAP_MESSAGE_EV|AMAP_MAP_EV);
         if (deal_data!=AMAP_NOTE_HACK_PTR)
         {
            trail_sp_punt();
            if ((amptr->note_obj=object_place(MAPNOTE_TRIPLE,MakePoint(x,y)))!=OBJ_NULL)
               fsmap_new_msg(amptr);
            else
            {
               last_msg_ok=FALSE;
               clear_cur_mapnote();
               Warning(("No more mapnote space\n"));
            }
         }
// if we want to do things when you click on old map notes...
//         else
//         {
//            mprintf("Clicked on old map note\n");
//         }
      }
      else {
         last_msg_ok=FALSE;
         chg_set_flg(AMAP_MESSAGE_EV);
      }
   }
   if(scregion!=AMAP_MAP_EV && scregion!=AMAP_MESSAGE_EV && amptr->note_obj!=OBJ_NULL) {
      trail_sp_punt();
      if(cur_mapnote_base!=NULL) {
         if(*cur_mapnote_base=='\0') {
            obj_destroy(amptr->note_obj);
            chg_set_flg(AMAP_MAP_EV);
         }
         else
            amap_str_grab(cur_mapnote_base);
      }
      clear_cur_mapnote();
      last_msg_ok=TRUE;
      chg_set_flg(AMAP_MESSAGE_EV);
   }
   return TRUE;
}

bool zoom_deal(curAMap *amptr, int btn)
{
   int zfac;
   bool res;
   // now, set up for real 
   if (btn==BTN_ZOOMIN) zfac=1; else zfac=-1;
   res=amap_zoom(amptr,FALSE,zfac);
   if (res)
   {
      if (amptr->zoom+1>=AMAP_MAX_ZOOM)
         s_bf(BTN_ZOOMIN,AMAP_UNSET);
      else if (amptr->zoom<=AMAP_MIN_ZOOM)
         s_bf(BTN_ZOOMOUT,AMAP_UNSET);
      if ((btn==BTN_ZOOMIN) &&(s_bf(BTN_ZOOMOUT,BTN_TALK)==0))
         s_bf(BTN_ZOOMOUT,AMAP_SET);
      if ((btn==BTN_ZOOMOUT)&&(s_bf(BTN_ZOOMIN, BTN_TALK)==0))
         s_bf(BTN_ZOOMIN, AMAP_SET);
   }
   s_bf(btn,BTN_PEND);
   return res;
}

bool flags_deal(curAMap *amptr, int btn, int todo)
{
   short flgs=btn_to_amap[btn];
   int todo_bf=todo;
   bool res;

//   mprintf("Think deal with %x - %d\n",flgs,todo);
   if (flgs==0) return FALSE;

   if (flgs&FSMAP_OPP)
   {
      switch (todo)
      {
      case AMAP_UNSET: todo_bf=AMAP_SET;   break;
      case AMAP_SET:   todo_bf=AMAP_UNSET; break;
      }
   }
   res=amap_flags(amptr,flgs,todo_bf);
   if (res) s_bf(btn,todo);
   return res;
}

void btn_init(curAMap *amptr)
{
   int i,j;
   for (i=3,j=(1<<(3*2)); i<BTN_NUM_REAL; i++,j<<=2)
      if (fsmap_buttons&j)
      {
         if (btn_to_amap[i]!=0)
            if (!flags_deal(amptr,i,AMAP_SET))
               fsmap_buttons&=~j;
      }
      else
         flags_deal(amptr,i,AMAP_UNSET);
}

bool amap_kb_callback(curAMap *amptr, int code)
{
//   char codewas;
   int exp=0xff; // will get zeroed in case default for codes we ignore

/*KLC - we're just forgetting most of the key equivalents for now

   if (code & KB_FLAG_DOWN)					//KLC - we'll process on key down.
   {
      codewas=map_scroll_code;
      map_scroll_code=0;
      if(pend_check()) return TRUE;
      if(!(codewas==0)) return TRUE;
   }
*/
   
   // If we're currently editing a message...
   
   if (cur_mapnote_ptr!=NULL)
   {
//KLC - we'll do keydowns      if ((code&KB_FLAG_DOWN)==KB_FLAG_DOWN) return TRUE;
      code = kb2ascii(code);
//KLC      if ((code==KEY_ENTER)||(code==KEY_DEL))
      if (code==KEY_ENTER)												// If we've pressed Enter
      {
         if (code==KEY_ENTER)
         {
            trail_sp_punt();													// clear out any trailing spaces
            if(amptr->flags&AMAP_FULL_MSG)						// switch out of editing loop
               chg_set_flg(AMAP_MAP_EV);
         }

//KLC         if ((code==KEY_DEL)||(cur_mapnote_ptr==cur_mapnote_base))
         if (cur_mapnote_ptr == cur_mapnote_base)				// If the map note string is empty
         {																			// then delete the map note
            obj_destroy(amptr->note_obj);
            amptr->note_obj=0;
            chg_set_flg(AMAP_MAP_EV);
         }
         else
            amap_str_grab(cur_mapnote_base);
         clear_cur_mapnote();
      }
      else if (isprint(code))
      {  // make sure it isnt too long
         int clen=strlen(cur_mapnote_base)+1;

         if (amap_str_deref(cur_mapnote_base)+clen<AMAP_STRING_SIZE)
	         if(clen<FSMAP_MAX_MSG) {
	            if (*cur_mapnote_ptr!='\0')
	               memcpy(cur_mapnote_ptr+1,cur_mapnote_ptr,strlen(cur_mapnote_ptr));
	            else
	               *(cur_mapnote_ptr+1)='\0';
	            *cur_mapnote_ptr++=(char)code;
	         }
      }
      else if (code==KEY_BS)
      {
         if (cur_mapnote_ptr>cur_mapnote_base)
            *--cur_mapnote_ptr='\0';
      }
      else return FALSE;
      chg_set_flg(AMAP_MESSAGE_EV);
      if(amptr->flags & AMAP_FULL_MSG)
         chg_set_flg(AMAP_MAP_EV);
      return TRUE;
   }
   
   // We're not editing.  Keyboard equivalents for buttons.
   
   else
   {
      char btn=-1, todo=AMAP_TOGGLE;
      map_scroll_code=0;
      switch (code&(~KB_FLAG_DOWN))
      {
//KLC      case KEY_PGUP:   case '[': if (!zoom_deal(amptr,BTN_ZOOMOUT)) exp=0; break;
//KLC      case KEY_PGDN:   case ']': if (!zoom_deal(amptr,BTN_ZOOMIN))  exp=0; break;
//KLC maybe put keyboard scrolling in later
//KLC      case KEY_RIGHT:  case 'l': map_scroll_code=AMAP_PAN_E; break;
//KLC      case KEY_LEFT:   case 'j': map_scroll_code=AMAP_PAN_W; break;
//KLC      case KEY_UP:     case 'i': map_scroll_code=AMAP_PAN_N; break;
//KLC      case KEY_DOWN:   case 'k': map_scroll_code=AMAP_PAN_S; break;
      case KEY_ESC:    case 'q': _new_mode=_last_mode; chg_set_flg(GL_CHG_LOOP); break;
//KLC      case KEY_BS:      edit_mapnote(amptr); break;
      case KEY_BS:
            obj_destroy(amptr->note_obj);		// Delete the map note
            amptr->note_obj=0;
         	   clear_cur_mapnote();
            last_msg_ok=TRUE;
            chg_set_flg(AMAP_MESSAGE_EV|AMAP_MAP_EV);
            break;
//KLC      case DO_FULLMSG:           btn=BTN_FULLMSG; break;  
//KLC      case DO_RECENTER:          btn=BTN_RECENTER; todo=AMAP_UNSET; break;
//KLC      case DO_SCAN:              btn=BTN_SCAN; break;
//KLC      case DO_CRITTER:           btn=BTN_CRITTER; break;
//KLC      case DO_SECUR:             btn=BTN_SECURE; break;
      default:                   exp=0; break;
      }
/* KLC - not used any more
      if (btn!=-1)
         if (!flags_deal(amptr,btn,todo)) exp=0;
      if(map_scroll_code!=0) {
         map_scrolltime=*tmd_ticks;
         s_bf(BTN_RECENTER,AMAP_SET);
      }
*/
      if (exp)
      {
         chg_set_flg(AMAP_MAP_EV);
         return TRUE;
      }
      return FALSE;
   }
}


                                                                
