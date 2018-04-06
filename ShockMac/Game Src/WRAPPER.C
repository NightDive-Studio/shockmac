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
 * $Source: r:/prj/cit/src/RCS/wrapper.c $
 * $Revision: 1.146 $
 * $Author: dc $
 * $Date: 1994/11/28 06:40:50 $
 */

#define __WRAPPER_SRC

#include <limits.h>

#include "wrapper.h"
#include "tools.h"
#include "invent.h"
#include "invpages.h"
#include "gamescr.h"
#include "mainloop.h"
#include "hkeyfunc.h"
#include "gamewrap.h"
#include "saveload.h"
#include "colors.h"
#include "cybstrng.h"
#include "status.h"
#include "fullscrn.h"
#include "render.h"
#include "gametime.h"
#include "musicai.h"
#include "input.h"
#include "gamestrn.h"
#include "miscqvar.h"
#include "cit2d.h"
#include "cybmem.h"
#include "citres.h"
#include "sfxlist.h"
#include "criterr.h"
#include "gr2ss.h"
/*���
#include <olhext.h>
#include <inp6d.h>
#include <i6dvideo.h>
#include <lgsndx.h>
#include <joystick.h>
#include <config.h>
*/

#ifdef AUDIOLOGS
#include "audiolog.h"
#endif

#include "mfdart.h" // for the slider bar


extern void text_button(char *text, int xc, int yc, int col, int shad, int w, int h);

#define LOAD_BUTTON 0
#define SAVE_BUTTON 1
#define AUDIO_BUTTON 2
#define INPUT_BUTTON 3
#define OPTIONS_BUTTON 4
#define VIDEO_BUTTON 5
#define RETURN_BUTTON 6
#define QUIT_BUTTON 7
#define AUDIO_OPT_BUTTON 8
#define SCREENMODE_BUTTON 9
#define HEAD_RECENTER_BUTTON  10
#define HEADSET_BUTTON 11

#define MOUSE_DOWN (MOUSE_LDOWN|MOUSE_RDOWN|UI_MOUSE_LDOUBLE)
#define MOUSE_UP (MOUSE_LUP|MOUSE_RUP)
#define MOUSE_LEFT (MOUSE_LDOWN|UI_MOUSE_LDOUBLE)

LGCursor option_cursor;
grs_bitmap option_cursor_bmap;

extern Region *inventory_region;
int wrap_id = -1, wrapper_wid, wrap_key_id;
bool clear_panel = TRUE, wrapper_panel_on = FALSE;
grs_font* opt_font;
bool olh_temp;
extern bool sfx_on;
extern bool digi_gain;
errtype (*wrapper_cb)(int num_clicked);
errtype (*slot_callback)(int num_clicked);
static bool cursor_loaded = FALSE;
#if defined(VFX1_SUPPORT)||defined(CTM_SUPPORT)
bool headset_track = TRUE;
#define HEADSET_FOV_MIN 30
#define HEADSET_FOV_MAX 180
// these 3 should all be initialized for real elsewhere...
int inp6d_real_fov = 60;
int hack_headset_fov = 30;
#endif
int inp6d_curr_fov = 60;

errtype music_slots();
errtype wrapper_do_save();
extern errtype inventory_clear(void);
errtype wrapper_panel_close(bool clear_message);
errtype do_savegame_guts(uchar slot);
void wrapper_start(void (*init)(void));
void quit_verify_pushbutton_handler(uchar butid);
bool quit_verify_slorker(uchar butid);
void save_verify_pushbutton_handler(uchar butid);
bool save_verify_slorker(uchar butid);
errtype make_options_cursor(void);
void free_options_cursor(void);

uint multi_get_curval(uchar type, void* p);
void multi_set_curval(uchar type, void* p, uint val, void* deal);

extern Cursor slider_cursor;
extern grs_bitmap slider_cursor_bmap;
extern char which_lang;

extern void mouse_unconstrain(void);

void verify_screen_init(void (*verify)(uchar butid), void (*slork)(uchar butid));
void options_screen_init(void);
void wrapper_init(void);
void load_screen_init(void);

void draw_button(uchar butid);

#define SLOTNAME_HEIGHT  6
#define PANEL_MARGIN_Y 3
#define WRAPPER_PANEL_HEIGHT (INVENTORY_PANEL_HEIGHT-2*PANEL_MARGIN_Y)

#define OPTIONS_FONT RES_tinyTechFont

errtype (*verify_callback)(int num_clicked) = NULL;
char savegame_verify;
char comments[NUM_SAVE_SLOTS+1][SAVE_COMMENT_LEN];
bool pause_game_func(short keycode, ulong context, void* data);
bool really_quit_key_func(short keycode, ulong context, void* data);

// separate mouse region for regular-screen and fullscreen.
#define NUM_MOUSEREGION_SCREENS 2
Region options_mouseregion[NUM_MOUSEREGION_SCREENS];
uchar free_mouseregion=0;

bool popup_cursors = TRUE;

char save_game_name[]="savgam00.dat";


extern grs_canvas* pinv_canvas;
extern grs_canvas  inv_norm_canvas;
extern grs_canvas  inv_fullscrn_canvas;
extern grs_canvas  inv_view360_canvas;

#define FULL_BACK_X (GAME_MESSAGE_X-INVENTORY_PANEL_X)
#define FULL_BACK_Y (GAME_MESSAGE_Y-INVENTORY_PANEL_Y)

#define BUTTON_COLOR GREEN_BASE+2
#define BUTTON_SHADOW 7

#define BUTTON_USERDATA_SIZ 32

typedef struct {
   Rect     rect;
   uchar    user[BUTTON_USERDATA_SIZ];
   ulong    evmask;
   void (*drawfunc)(uchar butid);
   bool (*handler)(uiEvent* ev, uchar butid);
} opt_button;

// SLIDER WIDGETS:
// structure for slider widget.  The slider has a pointer to a uchar,
// ushort, or uint, which it sets to a value in the range [0,maxval].
// It recalculates this value based on interpolation from the actual
// size of the slider.  The function dealfunc (if not NULL) is called
// when the value changes, and is passed the new value.  The value is
// updated continuously if smooth==TRUE; otherwise it is updated upon
// mouse-up.
// 
typedef struct {
   uchar color;
   uchar bvalcol;
   uchar sliderpos;
   bool  active;
   Ref   descrip;
   uint  maxval;
   uchar baseval;
   uchar type;
   bool  smooth;
   void* curval;
   void* dealfunc;
} opt_slider_state;

// PUSHBUTTON WIDGETS:
// the simplest widgets.  Calls pushfunc, passing in its own button ID,
// upon mouse left-click upon it, or on a keyboard event corresponding
// to keyeq.
//
typedef struct {
   uchar keyeq;
   Ref   descrip;
   uchar fcolor;
   uchar shadow;
   void  (*pushfunc)(uchar butid);
} opt_pushbutton_state;

// MULTI_STATE WIDGETS:
// these are much like pushbuttons, but also have a pointer to a uchar,
// ushort, or uint, which takes on a value in the range [0,num_opts-1].
// The button is labelled both with its description string (descrip) and
// with a string offset from optbase by an amount equal to the current
// value of its associated variable.  Whenever its value changes, it calls
// dealfunc, and message-lines a string offset from feedbackbase by an
// amount equal to its current value.
//
typedef struct {
   uchar keyeq;
   uchar type;
   uchar num_opts;
   Ref   optbase;
   Ref   descrip;
   Ref   feedbackbase;
   void* curval;
   void* dealfunc;
} opt_multi_state;

// TEXT WIDGET
// nothing but a piece of text, folks.  No handler, simple draw func.
//
typedef struct {
   Ref   descrip;
   uchar color;
} opt_text_state;

// TEXTLIST WIDGET
// used for editing and selecting (and responding to the editing and
// selecting of) a list of text strings.  You provide a block of text,
// which is assumed to be a 2-D array of chars (you inform the widget
// of the dimension of the subarrays).  The widget may either be edit-
// allowing or not.  If not, then it calls its dealfunc whenever a
// text string is selected (by mouse-clicking on it or by using the
// keyboard to move the highlight to it and hitting ENTER).  If the
// strings are editable, it calls its dealfunc only when you are done
// selecting and editing one.  A mask may be provided of what entries
// on the list are valid candidates for selection, and a string resource
// is given to display in the place of uninitialized selections.  Different
// colors are provided for selectable text, currently selected text, 
// and non-selectable text.  Note that the user is responsible for
// providing space for one more line of text than the widget uses, for
// the purposes of saving string information.
//
typedef struct {
   char*    text;
   uchar    numblocks;
   uchar    blocksiz;

   char     currstring;
   char     index;
   bool     modified;

   bool     editable;
   ushort   editmask;
   ushort   selectmask;
   ushort   initmask;
   Ref      invalidstr;

   Ref      selectprompt;

   uchar    validcol;
   uchar    selectcol;
   uchar    invalidcol;

   void (*dealfunc)(uchar butid,uchar index);
} opt_textlist_state;

// SLORKER WIDGET
// used to implement default actions in the keyboard interface to options
// screens, slorker widgets respond to no mouse events, but will respond
// to any keyboard events which actually reach them by calling their function
// with their button id as an argument.  Thus, any keypress which is not
// handled by another gadget is taken by the slorker.
//                                                                                
typedef bool (*slorker)(uchar butid);

#define OPT_SLIDER_BAR REF_IMG_BeamSetting

#define MAX_OPTION_BUTTONS 12
#define BR(i) (OButtons[i].rect)

#ifdef STATIC_BUTTON_STORE
opt_button OButtons[MAX_OPTION_BUTTONS];
#else
extern grs_canvas _offscreen_mfd;
opt_button* OButtons;
uchar fv;
#endif

#define OPTIONS_COLOR RED_BROWN_BASE + 4

// Source Code for wrapper interface and functions

#define STORE_CLIP(a,b,c,d) a = gr_get_clip_l(); \
   b = gr_get_clip_t();  c = gr_get_clip_r(); d = gr_get_clip_b()

#define RESTORE_CLIP(a,b,c,d) gr_set_cliprect(a,b,c,d)

// decides on a "standard" width for our widgets based on column count
// of current screen.  Our desire is that uniform widgets of this size
// should have certain margins between them independent of column count.
#define CONSTANT_MARGINS

#ifdef HALF_BUTTON_MARGINS
#define widget_width(t,m) (2*INVENTORY_PANEL_WIDTH/(3*(t)+1))
#define widget_x(c,t,m) ((3*(t)+1)*INVENTORY_PANEL_WIDTH/(3*(t)+1))
#endif
#ifdef CONSTANT_MARGINS
#define widget_width(t,m) ((INVENTORY_PANEL_WIDTH-((m)*((t)+1)))/(t))
#define widget_x(c,t,m) ((m)*((c)+1)+widget_width(t,m)*(c))
#endif


#ifdef NOT_YET //���

void draw_button(uchar butid)
{
   if(OButtons[butid].drawfunc) {
#ifdef SVGA_SUPPORT
      uchar old_over;
      old_over = gr2ss_override;
      gr2ss_override = OVERRIDE_ALL;
#endif
      uiHideMouse(NULL);
      gr_push_canvas(&inv_norm_canvas);
      gr_set_font(opt_font);
      OButtons[butid].drawfunc(butid);
      gr_pop_canvas();
      uiShowMouse(NULL);
#ifdef GR2SS_OVERRIDE
      gr2ss_override = old_over;
#endif
   }
}

void wrapper_draw_background(short ulx, short uly, short lrx, short lry)
{
   short cx1, cx2, cy1, cy2;
   extern grs_bitmap inv_backgnd;
   short a1,a2,a3,a4;

#ifdef SVGA_SUPPORT
   uchar old_over;
   old_over = gr2ss_override;
   gr2ss_override = OVERRIDE_ALL;
#endif
   // draw background behind the slider.
   STORE_CLIP(cx1,cy1,cx2,cy2);
   ss_safe_set_cliprect(ulx,uly,lrx,lry);
   if (full_game_3d)
   {
//      gr_bitmap(&inv_view360_canvas.bm,FULL_BACK_X,FULL_BACK_Y);
      gr_get_cliprect(&a1,&a2,&a3,&a4);
      ss_noscale_bitmap(&inv_view360_canvas.bm,FULL_BACK_X,FULL_BACK_Y);
   }
   else
      ss_bitmap(&inv_backgnd,0,0);
   RESTORE_CLIP(cx1,cy1,cx2,cy2);
#ifdef SVGA_SUPPORT
   gr2ss_override = old_over;
#endif
}

void slider_draw_func(uchar butid)
{
   opt_slider_state* st=(opt_slider_state*)&(OButtons[butid].user);
   short w,h,sw;
   char *title;

#ifdef SVGA_SUPPORT
   uchar old_over;
   old_over = gr2ss_override;
   gr2ss_override = OVERRIDE_ALL;
#endif

   sw=res_bm_width(OPT_SLIDER_BAR);
   gr_set_fcolor(st->color);
   title=get_temp_string(st->descrip);
   gr_string_size(title,&w,&h);

   // draw background behind the slider
   wrapper_draw_background(BR(butid).ul.x-sw/2,BR(butid).ul.y-h,BR(butid).lr.x+sw/2,BR(butid).lr.y);
   draw_shadowed_string(title,BR(butid).ul.x,BR(butid).ul.y-h,full_game_3d);

   gr_set_fcolor(st->bvalcol);
   ss_vline(BR(butid).ul.x+st->baseval,BR(butid).ul.y,BR(butid).lr.y-1);

   gr_set_fcolor(st->color);
   ss_box(BR(butid).ul.x,BR(butid).ul.y,BR(butid).lr.x,BR(butid).lr.y);
   
   if(!(st->active))
      draw_raw_resource_bm(OPT_SLIDER_BAR,BR(butid).ul.x+st->sliderpos+1-sw/2,BR(butid).ul.y);

#ifdef SVGA_SUPPORT
   gr2ss_override = old_over;
#endif

}

void slider_deal(uchar butid, bool deal)
{
   opt_slider_state* st=(opt_slider_state*)&(OButtons[butid].user);
   uint val;
   
   deal=deal||st->smooth;

   val=(st->sliderpos*st->maxval)/(BR(butid).lr.x-BR(butid).ul.x-3);

   multi_set_curval(st->type,st->curval,val,deal?st->dealfunc:NULL);
}

//
// every time you find yourself,
// you lose a little bit of me, from within
//

bool slider_handler(uiEvent* ev, uchar butid)
{
   opt_slider_state* st=(opt_slider_state*)&(OButtons[butid].user);
   uiMouseEvent* mev=(uiMouseEvent*)ev;

   switch(ev->type) {
      case UI_EVENT_MOUSE_MOVE:
         if(st->active) {
            st->sliderpos=mev->pos.x-(BR(butid).ul.x+1);
            if(st->smooth)
               slider_deal(butid,FALSE);
         }
         break;
      case UI_EVENT_MOUSE:
         if(st->active && !(mev->buttons)) {
            st->sliderpos=mev->pos.x-(BR(butid).ul.x+1);
            st->active=FALSE;
            slider_deal(butid,TRUE);
            uiPopGlobalCursor();
            mouse_unconstrain();
            draw_button(butid);
         }
         else if(!st->active && (ev->subtype & MOUSE_DOWN)) {
            short tmpy;

            st->active=TRUE;
            st->sliderpos=mev->pos.x-(BR(butid).ul.x+1);
            slider_cursor.hotspot.x=slider_cursor_bmap.w/2;
            tmpy=mev->pos.y-((BR(butid).ul.y+BR(butid).lr.y)/2);
#ifdef SVGA_SUPPORT
             { short duh; ss_point_convert(&duh,&tmpy,FALSE); }
#endif
            slider_cursor.hotspot.y=(slider_cursor_bmap.h/2)+tmpy;
            uiPushGlobalCursor(&slider_cursor);
            draw_button(butid);
            // -2's are because our lr coorodinate is immediately OUTSIDE the box
            // we draw, so two pixels up and left is one pixel INSIDE the box.
            ui_mouse_constrain_xy(BR(butid).ul.x+inventory_region->r->ul.x+1,
                               mev->pos.y+inventory_region->r->ul.y,
                               BR(butid).lr.x+inventory_region->r->ul.x-2,
                               mev->pos.y+inventory_region->r->ul.y);
         }
         return TRUE;
      default:
         break;
   }
   return FALSE;
}

void slider_init(uchar butid, Ref descrip, uchar type, bool smooth, void* var, uint maxval, uchar baseval, void* dealfunc, Rect* r)
{
   opt_slider_state* st=(opt_slider_state*)&OButtons[butid].user;
   uint val;

   val=((r->lr.x-r->ul.x-3)*multi_get_curval(type,var))/maxval;

   st->color=BUTTON_COLOR;
   st->bvalcol=GREEN_YELLOW_BASE+1;
   st->sliderpos=val;
   st->baseval=baseval;
   st->maxval=maxval;
   st->active=FALSE;
   st->descrip=descrip;
   st->type=type;
   // note that in these settings, we don't care what size of
   // variable we're dealing with, 'cause we secretly know that
   // all pointers are represented the same and we don't
   // have to actually dereference these.
   st->dealfunc=dealfunc;
   st->curval=var;
   st->smooth=smooth;
   
   OButtons[butid].evmask=UI_EVENT_MOUSE|UI_EVENT_MOUSE_MOVE;
   OButtons[butid].drawfunc=slider_draw_func;
   OButtons[butid].handler=slider_handler;
   OButtons[butid].rect=*r;
}

void pushbutton_draw_func(uchar butid)
{
   char* btext;
   short w,h;
   opt_pushbutton_state* st=(opt_pushbutton_state*)&OButtons[butid].user;

   w=BR(butid).lr.x-BR(butid).ul.x;
   h=BR(butid).lr.y-BR(butid).ul.y;

   btext=get_temp_string(st->descrip);
   wrap_text(btext,BR(butid).lr.x-BR(butid).ul.x-3);
   text_button(btext,BR(butid).ul.x,BR(butid).ul.y,st->fcolor,st->shadow,-w,-h);
   unwrap_text(btext);
}

bool pushbutton_handler(uiEvent* ev, uchar butid)
{
   if(((ev->type==UI_EVENT_MOUSE) && (ev->subtype & MOUSE_DOWN)) ||
      ((ev->type==UI_EVENT_KBD_COOKED) && ((((uiCookedKeyEvent*)ev)->code & 0xFF)==((opt_pushbutton_state*)(&OButtons[butid].user))->keyeq)))
   {
      ((opt_pushbutton_state*)&OButtons[butid].user)->pushfunc(butid);
      return TRUE;
   }
   return FALSE;
}

void pushbutton_init(uchar butid, uchar keyeq, Ref descrip, void (*pushfunc)(uchar butid), Rect* r)
{
   opt_pushbutton_state* st=(opt_pushbutton_state*)&OButtons[butid].user;

   OButtons[butid].rect=*r;
   OButtons[butid].evmask=UI_EVENT_MOUSE|UI_EVENT_KBD_COOKED;
   OButtons[butid].drawfunc=pushbutton_draw_func;
   OButtons[butid].handler=pushbutton_handler;
   st->fcolor=BUTTON_COLOR;
   st->shadow=BUTTON_SHADOW;
   st->keyeq=keyeq;
   st->descrip=descrip;
   st->pushfunc=pushfunc;
}

void dim_pushbutton(uchar butid)
{
   opt_pushbutton_state* st=(opt_pushbutton_state*)&OButtons[butid].user;
   OButtons[butid].evmask=0;
   st->fcolor+=4;
   st->shadow-=3;
}

void bright_pushbutton(uchar butid)
{
   opt_pushbutton_state* st=(opt_pushbutton_state*)&OButtons[butid].user;
   OButtons[butid].evmask=0;
   st->fcolor-=2;
   st->shadow+=2;
}

// text widget
void text_draw_func(uchar butid)
{
   opt_text_state* st=(opt_text_state*)&OButtons[butid].user;
   char *s=get_temp_string(st->descrip);

   wrap_text(s,BR(butid).lr.x-BR(butid).ul.x);
   gr_set_fcolor(st->color);
   draw_shadowed_string(s,BR(butid).ul.x,BR(butid).ul.y,full_game_3d);
   unwrap_text(s);
}

void textwidget_init(uchar butid, uchar color, Ref descrip, Rect* r)
{
   opt_text_state* st=(opt_text_state*)&OButtons[butid].user;

   OButtons[butid].rect=*r;
   st->descrip=descrip;
   st->color=color;
   OButtons[butid].drawfunc=text_draw_func;
   OButtons[butid].handler=NULL;
   OButtons[butid].evmask=0;
}
   
// a keywidget is just like a pushbutton, but invisible.
//
void keywidget_init(uchar butid, uchar keyeq, void (*pushfunc)(uchar butid))
{
   opt_pushbutton_state* st=(opt_pushbutton_state*)&OButtons[butid].user;

   OButtons[butid].evmask=UI_EVENT_KBD_COOKED;
   OButtons[butid].drawfunc=NULL;
   OButtons[butid].handler=pushbutton_handler;
   st->keyeq=keyeq;
   st->pushfunc=pushfunc;
}

// gets the current "value" of a multi-option widget, whatever size
// thing that may be.
uint multi_get_curval(uchar type, void* p)
{
   uint val=0;

   switch(type) {
      case sizeof(uchar):
         val=*((uchar*)p);
         break;
      case sizeof(ushort):
         val=*((ushort*)p);
         break;
      case sizeof(uint):
         val=*((uint*)p);
         break;
   }
   return val;
}

// sets the current value pointed to by a multi-option widget.
void multi_set_curval(uchar type, void* p, uint val, void* deal)
{
   switch(type) {
      case sizeof(uchar):
         *((uchar*)p)=(uchar)val;
         if(deal)
            ((void(*)(uchar))deal)((uchar)val);
         break;
      case sizeof(ushort):
         *((ushort*)p)=(ushort)val;
         if(deal)
            ((void(*)(ushort))deal)((ushort)val);
         break;
      case sizeof(uint):
         *((uint*)p)=(uint)val;
         if(deal)
            ((void(*)(uint))deal)((uint)val);
         break;
   }
}

void multi_draw_func(uchar butid)
{
   char* btext;
   short w,h,x,y;
   uint val=0;
   opt_multi_state* st=(opt_multi_state*)&OButtons[butid].user;

   gr_set_fcolor(BUTTON_COLOR);
   ss_rect(BR(butid).ul.x,BR(butid).ul.y,BR(butid).lr.x,BR(butid).lr.y);
   gr_set_fcolor(BUTTON_COLOR+BUTTON_SHADOW);
   ss_rect(BR(butid).ul.x+1,BR(butid).ul.y+1,BR(butid).lr.x-1,BR(butid).lr.y-1);
   gr_set_fcolor(BUTTON_COLOR);
   x=(BR(butid).lr.x+BR(butid).ul.x)/2;
   y=(BR(butid).lr.y+BR(butid).ul.y)/2;
   btext=get_temp_string(st->descrip);
   gr_string_size(btext,&w,&h);
   ss_string(btext,x-w/2,y-h);
   val=multi_get_curval(st->type,st->curval);
   btext=get_temp_string(st->optbase+val);
   gr_string_size(btext,&w,&h);
   ss_string(btext,x-w/2,y);
}

bool multi_handler(uiEvent* ev, uchar butid)
{
   uint val=0, delta=0;
   opt_multi_state* st=(opt_multi_state*)&OButtons[butid].user;

   if(ev->type==UI_EVENT_MOUSE) {
      if(ev->subtype & MOUSE_LEFT)
         delta=1;
      else if(ev->subtype & MOUSE_RDOWN)
         delta=st->num_opts-1;
   }
   else if (ev->type==UI_EVENT_KBD_COOKED) {
      uiCookedKeyEvent* kev=(uiCookedKeyEvent*)ev;
      
      if(tolower(kev->code & 0xFF)==((opt_multi_state*)(&OButtons[butid].user))->keyeq) {
         if(isupper(kev->code & 0xFF))
            delta=st->num_opts-1;
         else
            delta=1;
      }
   }

   if(delta) {
      val=multi_get_curval(st->type,st->curval);
      val=(val+delta)%(st->num_opts);
      multi_set_curval(st->type,st->curval,val,st->dealfunc);
      draw_button(butid);
      if(st->feedbackbase) {
         string_message_info(st->feedbackbase+val);
      }
      return TRUE;
   }
   return FALSE;
}

void multi_init(uchar butid, uchar key, Ref descrip, Ref optbase, Ref feedbase,
   uchar type, void* var, uchar num_opts, void* dealfunc, Rect* r)
{
   opt_multi_state* st=(opt_multi_state*)&OButtons[butid].user;

   OButtons[butid].rect=*r;
   OButtons[butid].drawfunc=multi_draw_func;
   OButtons[butid].handler=multi_handler;
   OButtons[butid].evmask=UI_EVENT_MOUSE|UI_EVENT_KBD_COOKED;
   st->descrip=descrip;
   st->optbase=optbase;
   st->feedbackbase=feedbase;
   st->type=type;
   st->keyeq=key;
   st->num_opts=num_opts;
   // note that in these settings, we don't care what size of
   // variable we're dealing with, 'cause we secretly know that
   // all pointers are represented the same and we don't
   // have to actually dereference these.
   st->dealfunc=dealfunc;
   st->curval=var;
}

#pragma disable_message(202)
bool keyslork_handler(uiEvent* ev, uchar butid)
{
   slorker* slork=(slorker*)(&OButtons[butid].user);

   return((*slork)(butid));
}
#pragma enable_message(202)

void slork_init(uchar butid, bool (*slork)(short code))
{
   LG_memset(&OButtons[butid].rect,0,sizeof(Rect));
   *((slorker*)&(OButtons[butid].user))=slork;
   OButtons[butid].evmask=UI_EVENT_KBD_COOKED;
   OButtons[butid].drawfunc=NULL;
   OButtons[butid].handler=keyslork_handler;
}
 
char* textlist_string(opt_textlist_state* st, int ind)
{
   return(st->text+ind*(st->blocksiz));
}

void textlist_draw_line(opt_textlist_state* st,int line,uchar butid)
{
   short w,h;
   Rect scrrect;
   Rect r;
   char* s;
   uchar col;
#ifdef SVGA_SUPPORT
   uchar old_over;
#endif

   scrrect=BR(butid);
   scrrect.ul.x += INVENTORY_PANEL_X;
   scrrect.ul.y += INVENTORY_PANEL_Y;
   scrrect.lr.x += INVENTORY_PANEL_X;
   scrrect.lr.y += INVENTORY_PANEL_Y;

   if(((1<<line)&(st->initmask)) || (line==st->currstring && st->index>=0))
      s=textlist_string(st,line);
   else
      s=get_temp_string(st->invalidstr);

   if(line==st->currstring)
      col=st->selectcol;
   else if(st->selectmask & (1<<line))
      col=st->validcol;
   else
      col=st->invalidcol;
   gr_push_canvas(&inv_norm_canvas);
   gr_set_fcolor(col);

   gr_set_font(opt_font);
   gr_string_size(s,&w,&h);
   r.ul.x=BR(butid).ul.x;
   r.ul.y=BR(butid).ul.y+h*line;
   r.lr.x=BR(butid).lr.x;
   r.lr.y=r.ul.y+h;

   uiHideMouse(&scrrect);
#ifdef SVGA_SUPPORT
   old_over = gr2ss_override;
   gr2ss_override = OVERRIDE_ALL;
#endif
   wrapper_draw_background(r.ul.x,r.ul.y,r.lr.x,r.lr.y);
   draw_shadowed_string(s,r.ul.x,r.ul.y,full_game_3d);
#ifdef SVGA_SUPPORT
   gr2ss_override = old_over;
#endif
   uiShowMouse(&scrrect);
   gr_pop_canvas();
}

void textlist_draw_func(uchar butid)
{
   int i;
   opt_textlist_state* st=(opt_textlist_state*)&OButtons[butid].user;

   for(i=0;i<st->numblocks;i++) {
      textlist_draw_line(st,i,butid);
   }
}

void textlist_cleanup(opt_textlist_state* st)
{
   if(st->editable && st->currstring>=0 && st->index>=0) {
      strcpy(textlist_string(st,st->currstring),textlist_string(st,st->numblocks));
      st->index=-1;
   }
}

#ifdef WE_USED_THIS
void textlist_edit_line(opt_textlist_state* st, uchar butid, uchar line, bool end)
{
   char* s, * bak;
   char tmp;

   gr_push_canvas(&inv_norm_canvas);
   s=textlist_string(st,line);
   bak=textlist_string(st,st->numblocks);
   tmp=st->currstring;
   st->currstring=line;
   if(tmp>=0) {
      strcpy(textlist_string(st,tmp),bak);
      textlist_draw_line(st,tmp,butid);
   }
   strcpy(bak,s);
   st->index=end?strlen(s):0;
   s[0]='\0';
   textlist_draw_line(st,line,butid);
   gr_pop_canvas();
}
#endif

void textlist_select_line(opt_textlist_state* st, uchar butid, uchar line, bool deal)
{
   char tmp;

   gr_push_canvas(&inv_norm_canvas);
   tmp=st->currstring;
   st->currstring=line;
   st->index=-1;
   if(tmp>=0)
      textlist_draw_line(st,tmp,butid);
   textlist_draw_line(st,line,butid);
   gr_pop_canvas();
   if(deal)
      st->dealfunc(butid,line);
}

bool textlist_handler(uiEvent* ev, uchar butid)
{
   uchar line;
   opt_textlist_state* st=(opt_textlist_state*)&OButtons[butid].user;

   if((ev->type==UI_EVENT_MOUSE) && (ev->subtype & MOUSE_DOWN)) {
      uiMouseEvent* mev=(uiMouseEvent*)ev;
      short w,h;

      gr_set_font(opt_font);
      gr_char_size('X',&w,&h);

      line=(mev->pos.y-BR(butid).ul.y)/h;

      if(st->editable && (st->editmask & (1<<line))) {
// this is how you would do this if you wanted right-click to select
// a line w/ confirm, which would be the right thing to do, instead
// of confirm without selection, which is what Harvey at Origin wants.
//
//          if(st->selectprompt)
//             textlist_select_line(st,butid,line,FALSE);
//          textlist_select_line(st,butid,line,(ev->subtype&MOUSE_RDOWN)!=0);
//
         if(ev->subtype&MOUSE_RDOWN) {
            if(st->currstring>=0)
               st->dealfunc(butid,st->currstring);
         }
         else if (!st->modified){
            string_message_info(st->selectprompt);
            if(st->selectprompt)
               textlist_select_line(st,butid,line,FALSE);
         }
      }
      else if(st->selectmask & (1<<line)) {
         textlist_select_line(st,butid,line,TRUE);
      }
      return TRUE;
   }
   else if (ev->type==UI_EVENT_KBD_COOKED) {
      uiCookedKeyEvent* kev=(uiCookedKeyEvent*)ev;
      char k=(kev->code&0xFF);
      uint keycode=kev->code & ~KB_FLAG_DOWN;
      bool special=((kev->code & KB_FLAG_SPECIAL)!=0);
      char* s;
      char upness=0;
      char cur=st->currstring;

      // explicitly do not deal with alt-x, but leave
      // it to more capable hands.
      if(keycode==(KB_FLAG_ALT|'x'))
         return FALSE;
      if(cur>=0)
         s=textlist_string(st,cur);
      if(st->editable && cur>=0 && !special && kb_isprint(keycode)) {
         if(st->index<0) {
            strcpy(textlist_string(st,st->numblocks),textlist_string(st,st->currstring));
            st->index=0;
         }
         if(st->index+1<st->blocksiz) {
            s[st->index]=k;
            st->index++;
            s[st->index]='\0';
            textlist_draw_line(st,cur,butid);
         }
         st->modified = TRUE;
         return TRUE;
      }        
      switch(keycode) {
         case KEY_BS:
            if(st->editable && cur>=0) {
               if(st->index<0) {
                  strcpy(textlist_string(st,st->numblocks),textlist_string(st,st->currstring));
                  st->index=strlen(s);
               }
               if(st->index>0)
                  st->index--;
               s[st->index]='\0';
               textlist_draw_line(st,cur,butid);
            }
            break;
         case KEY_UP:
            upness=st->numblocks-1;
            break;
         case KEY_DOWN:
            upness=1;
            break;
         case KEY_ENTER:
            if(st->currstring>=0) {
               st->dealfunc(butid,cur);
               return TRUE;
            }
            break;
         case KEY_ESC:
            // on ESC, clean up but pass the event through.
            textlist_cleanup(st);
            wrapper_panel_close(TRUE);
            return FALSE;
      }
      if(upness!=0) {
         char newstring;
         uchar safety=0;

         newstring=cur;
         if(newstring<0)
            newstring=(upness==1)?st->numblocks-1:0;
         do {
            newstring=(newstring+upness)%st->numblocks;
            safety++;
         } while(safety<st->numblocks && !((1<<newstring)&st->selectmask));
         if(safety>=st->numblocks)
            newstring=cur;
         if(newstring!=cur) {
            textlist_cleanup(st);
            st->currstring=newstring;
            if(cur>=0 && cur<st->numblocks)
               textlist_draw_line(st,cur,butid);
            textlist_draw_line(st,newstring,butid);
         }
      }
      return TRUE;
   }
   return TRUE;
}

void textlist_init(uchar butid,char* text,uchar numblocks,uchar blocksiz,
   bool editable,ushort editmask,ushort selectmask,ushort initmask,
   Ref invalidstr,uchar validcol,uchar selectcol,uchar invalidcol,
   Ref selectprompt, void (*dealfunc)(uchar butid,uchar index), Rect* r)
{
   opt_textlist_state* st=(opt_textlist_state*)&OButtons[butid].user;

   if(r==NULL) {
      BR(butid).ul.x=2;
      BR(butid).ul.y=2;
      BR(butid).lr.x=INVENTORY_PANEL_WIDTH;
      BR(butid).lr.y=INVENTORY_PANEL_HEIGHT;
   }
   else
      OButtons[butid].rect=*r;
   OButtons[butid].drawfunc=textlist_draw_func;
   OButtons[butid].handler=textlist_handler;
   OButtons[butid].evmask=UI_EVENT_MOUSE|UI_EVENT_KBD_COOKED;
   st->text=text;
   st->numblocks=numblocks;
   st->blocksiz=blocksiz;
   st->editable=editable;
   st->editmask=editmask;
   st->selectmask=selectmask;
   st->initmask=initmask;
   st->invalidstr=invalidstr;
   st->validcol=validcol;
   st->selectcol=selectcol;
   st->invalidcol=invalidcol;
   st->dealfunc=dealfunc;
   st->selectprompt=selectprompt;

   st->currstring=-1;
   st->index=-1;
   st->modified=FALSE;
}

// One, true mouse handler for all options panel mouse events.
// checks all options panel widgets which enclose point of mouse
// event to see if they want to deal with it.
//
#pragma disable_message(202)
bool opanel_mouse_handler(uiEvent *ev, Region *r, void *user_data)
{
   uiMouseEvent mev;
   int b;

   mev=*((uiMouseEvent*)ev);

   if(!ev->type && (UI_EVENT_MOUSE|UI_EVENT_MOUSE_MOVE))
      return FALSE;
   if(ev->type==UI_EVENT_MOUSE && !(ev->subtype & (MOUSE_DOWN|MOUSE_UP)))
      return FALSE;

   mev.pos.x-=inventory_region->r->ul.x;
   mev.pos.y-=inventory_region->r->ul.y;

   for(b=0;b<MAX_OPTION_BUTTONS;b++) {
      if(RECT_TEST_PT(&BR(b),mev.pos) && (ev->type & OButtons[b].evmask)) {
         if(OButtons[b].handler && OButtons[b].handler((uiEvent*)(&mev),b))
            return TRUE;
      }
   }
   return TRUE;
}

// One, true keyboard handler for all options mode events.
// checks all options panel widgets to see if they want to deal.
//
bool opanel_kb_handler(uiEvent *ev, Region *r, void* user_data)
{
   uiCookedKeyEvent* kev=(uiCookedKeyEvent*)ev;
   int b;

   if(!(kev->code & KB_FLAG_DOWN)) return TRUE;

   for(b=0;b<MAX_OPTION_BUTTONS;b++) {
      if((ev->type & OButtons[b].evmask) && OButtons[b].handler && OButtons[b].handler(ev,b))
         return TRUE;
   }
   // if no-one else has hooked KEY_ESC, it defaults to closing
   // the wrapper panel.
   //
   if((kev->code & 0xFF)==KEY_ESC)
      wrapper_panel_close(TRUE);
   return TRUE;
}
#pragma enable_message(202)

void clear_obuttons()
{
   uiCursorStack* cs;
   extern uiSlab* uiCurrentSlab;

   uiGetSlabCursorStack(uiCurrentSlab,&cs);
   uiPopCursorEvery(cs,&slider_cursor);
   mouse_unconstrain();
   LG_memset(OButtons,0,MAX_OPTION_BUTTONS*sizeof(opt_button));
}

void opanel_redraw(bool back)
{
   extern grs_bitmap inv_backgnd;
   int but;
   Rect r = { { INVENTORY_PANEL_X, INVENTORY_PANEL_Y },
      { INVENTORY_PANEL_X + INVENTORY_PANEL_WIDTH,
        INVENTORY_PANEL_Y + INVENTORY_PANEL_HEIGHT}};
#ifdef SVGA_SUPPORT
   uchar old_over = gr2ss_override;
   gr2ss_override = OVERRIDE_ALL;     // Since we are really going straight to screen in our heart of hearts
#endif
   if (!full_game_3d)
      inventory_clear();
   gr_push_canvas(&inv_norm_canvas);
   uiHideMouse(&r);
   gr_set_font(opt_font);
   if(back) {
      if (full_game_3d)
         ss_noscale_bitmap(&inv_view360_canvas.bm,FULL_BACK_X,FULL_BACK_Y);
      else
         ss_bitmap(&inv_backgnd,0,0);
   }

   for(but=0;but<MAX_OPTION_BUTTONS;but++) {
      if(OButtons[but].drawfunc) {
         OButtons[but].drawfunc(but);
      }
   }
   uiShowMouse(&r);
   gr_pop_canvas();
#ifdef SVGA_SUPPORT
   gr2ss_override = old_over;
#endif
}

// fills in the Rect r with one of the "standard" button rects,
// assuming buttons in three columns, ro rows, high enough for
// a specified number of lines of text.
//
void standard_button_rect(Rect* r, uchar butid, uchar lines, uchar ro, uchar mar)
{
   short w,h;
   char i=butid;

   gr_set_font(opt_font);
   gr_string_size("X",&w,&h);

   h*=lines;

   r->ul.x=widget_x(i%3,3,mar);
   r->lr.x=r->ul.x+widget_width(3,mar);
   r->ul.y=INVENTORY_PANEL_HEIGHT*(i/3+1)/(ro+1)-h/2;
   if(ro>2) r->ul.y+=(3*((i/3)-1));
   r->lr.y=r->ul.y+h+2;
}

void standard_slider_rect(Rect* r, uchar butid, uchar ro, uchar mar)
{
   short sh,sw;

   standard_button_rect(r,butid,2,ro,mar);

   sh=res_bm_height(OPT_SLIDER_BAR);
   sw=res_bm_height(OPT_SLIDER_BAR);
   r->ul.x+=sw/2;
   r->lr.x-=sw/2;
   r->ul.y=r->lr.y-sh;
}

errtype wrapper_panel_close(bool clear_message)
{
   uiCursorStack* cs;
   extern uiSlab* uiCurrentSlab;
   int i;
   extern void mfd_force_update_single(int which_mfd);
#ifdef SVGA_SUPPORT
   extern errtype mfd_clear_all();
#endif

   if (!wrapper_panel_on) return ERR_NOEFFECT;
   mouse_unconstrain();
   if (clear_message)
      message_info("");
   wrapper_panel_on = FALSE;
   inventory_page = inv_last_page;
   if (inventory_page < 0 && inventory_page != INV_3DVIEW_PAGE)
      inventory_page = 0;
   pause_game_func(0,0,0);
   uiGetSlabCursorStack(uiCurrentSlab,&cs);
   uiPopCursorEvery(cs,&slider_cursor);
   uiReleaseFocus(inventory_region, UI_EVENT_KBD_COOKED|UI_EVENT_MOUSE);
   uiRemoveRegionHandler(inventory_region, wrap_id);
   uiRemoveRegionHandler(inventory_region, wrap_key_id);
#ifndef STATIC_BUTTON_STORE
   full_visible=fv;
#endif
   inventory_clear();
   inventory_draw();
#ifdef SVGA_SUPPORT
   mfd_clear_all();
#endif
   for(i=0;i<NUM_MFDS;i++)
      mfd_force_update_single(i);
   ResUnlock(OPTIONS_FONT);
   resume_game_time();
   return(OK);
}

extern bool game_paused;

bool can_save()
{
   bool gp=game_paused;
   if (global_fullmap->cyber)
   {
      // spoof the game as not being paused so that the message won't go to the 
      // phantom message line in full screen mode, where it will stay only for a frame.
      game_paused=FALSE;              
      string_message_info(REF_STR_NoCyberSave);
      game_paused=gp;
      return(FALSE);
   }
   if (input_cursor_mode == INPUT_OBJECT_CURSOR)
   {
      string_message_info(REF_STR_CursorObjSave);
      return(FALSE);
   }
   return(TRUE);
}

//
// THE TOP LEVEL OPTIONS: Initialization, handler
//

void wrapper_pushbutton_func(uchar butid)
{
   switch(butid) {
      case LOAD_BUTTON: // Load Game
#ifdef DEMO
         wrapper_panel_close(FALSE);
#else
         load_screen_init();
         string_message_info(REF_STR_LoadSlot);
#endif
         break;
      case SAVE_BUTTON: // Save Game
#ifdef DEMO
         wrapper_panel_close(FALSE);
#else
         if(can_save()) {
            save_screen_init();
            string_message_info(REF_STR_SaveSlot);
         }
         else
            wrapper_panel_close(FALSE);
#endif
         break;
      case AUDIO_BUTTON: // Audio
         sound_screen_init();
         break;
      case INPUT_BUTTON: // Input
         input_screen_init();
         break;
      case VIDEO_BUTTON: // Input
         video_screen_init();
         break;
#ifdef SVGA_SUPPORT
      case SCREENMODE_BUTTON: // Input
         screenmode_screen_init();
         break;
      case HEAD_RECENTER_BUTTON: // Input
         {
            extern bool recenter_headset(short keycode, ulong context, void* data);
            recenter_headset(0,0,0);
         }
         break;
      case HEADSET_BUTTON:
         headset_screen_init();
         break;
#endif
      case AUDIO_OPT_BUTTON:
         soundopt_screen_init();
         break;
      case OPTIONS_BUTTON: // Options
         options_screen_init();
         break;
      case RETURN_BUTTON: // Return
         wrapper_panel_close(TRUE);
         break;
      case QUIT_BUTTON: // Quit
         verify_screen_init(quit_verify_pushbutton_handler,quit_verify_slorker);
         string_message_info(REF_STR_QuitConfirm);
         break;
   }
   return;
}

void wrapper_init(void)
{
   Rect r;
   int i;
   char* keyequivs;
   
   keyequivs=get_temp_string(REF_STR_KeyEquivs0);

   clear_obuttons();
   for(i=0;i<8;i++) {
      standard_button_rect(&r,i,2,3,5);
      pushbutton_init(i,keyequivs[i],REF_STR_WrapperText+i,wrapper_pushbutton_func,&r);
   }   
   if(!music_card && !sfx_card) {
      dim_pushbutton(AUDIO_BUTTON);
   }
#ifdef DEMO
   dim_pushbutton(LOAD_BUTTON);
   dim_pushbutton(SAVE_BUTTON);
#endif
   opanel_redraw(TRUE);
}

// 
// THE VERIFY SCREEN: Initialization, handlers
//

#pragma disable_message(202)
void quit_verify_pushbutton_handler(uchar butid)
{
   really_quit_key_func(0,0,0);
}

bool quit_verify_slorker(uchar butid)
{
   wrapper_panel_close(TRUE);
   return TRUE;
}

void save_verify_pushbutton_handler(uchar butid)
{
   do_savegame_guts(savegame_verify);
}

bool save_verify_slorker(uchar butid)
{
   strcpy(comments[savegame_verify],comments[NUM_SAVE_SLOTS]);
   wrapper_panel_close(TRUE);
   return TRUE;
}
#pragma enable_message(202)

void verify_screen_init(void (*verify)(uchar butid), slorker slork)
{
   Rect r;

   clear_obuttons();

   standard_button_rect(&r,1,2,2,5);
   pushbutton_init(0,tolower(get_temp_string(REF_STR_VerifyText)[0]),REF_STR_VerifyText,verify,&r);
   
   standard_button_rect(&r,4,2,2,5);
   pushbutton_init(1,0,tolower(REF_STR_VerifyText+1),slork,&r);

   slork_init(2,slork);

   opanel_redraw(TRUE);
}

void quit_verify_init(void)
{
   verify_screen_init(quit_verify_pushbutton_handler,quit_verify_slorker);
}

//
// THE SOUND OPTIONS SCREEN: Initialization, update funcs

uchar curr_vol_lev = 100;
uchar curr_sfx_vol = 100;
uchar curr_alog_vol = 100;

void recompute_music_level(ushort vol)
{
//   curr_vol_lev=long_sqrt(100*vol);
   curr_vol_lev=QVAR_TO_VOLUME(vol);
   if(vol==0) {
      music_on=FALSE;
      stop_music_func(0,0,0);
   }
   else {
      if(!music_on) {
         music_on=TRUE;
         start_music_func(0,0,0);
      }
      mlimbs_change_master_volume(curr_vol_lev);
   }
}

void recompute_digifx_level(ushort vol)
{
   if (sfx_card)
      sfx_on=(vol!=0);
   else
      sfx_on=FALSE;
   curr_sfx_vol=QVAR_TO_VOLUME(vol);
   if (sfx_on)
   {
#ifdef DEMO
      play_digi_fx(73,1);
#else
      play_digi_fx(SFX_NEAR_1,1);
#endif
   }
   else
   {
#ifdef AUDIOLOGS
      audiolog_stop();
#endif
      stop_digi_fx();
   }
}

#ifdef AUDIOLOGS
void recompute_audiolog_level(ushort vol)
{
   curr_alog_vol=QVAR_TO_VOLUME(vol);
}
#endif

#pragma disable_message(202)
void digi_toggle_deal(bool offon)
{
   int vol;
   vol=(sfx_on)?100:0;
   recompute_digifx_level(vol);
   QUESTVAR_SET(SFX_VOLUME_QVAR,vol);
}

#ifdef AUDIOLOGS
void audiolog_dealfunc(short val)
{
   if (!val)
      audiolog_stop();
   QUESTVAR_SET(ALOG_OPT_QVAR,audiolog_setting);
}
#endif

char hack_digi_channels = 1;

void digichan_dealfunc(short val)
{
   hack_digi_channels = val;
   switch(hack_digi_channels)
   {
      case 0:
         cur_digi_channels = 2;
         break;
      case 1:
         cur_digi_channels = 4;
         break;
      case 2:
         cur_digi_channels = 8;
         break;
   }
   QUESTVAR_SET(DIGI_CHANNELS_QVAR, hack_digi_channels);
   snd_set_digital_channels(cur_digi_channels);
}

#pragma enable_message(202)

#define SLIDER_OFFSET_3 0
void soundopt_screen_init()
{
   Rect r;
   char retkey;
   int i = 0;

   clear_obuttons();

   standard_button_rect(&r,i,2,2,5);
   retkey=tolower(get_temp_string(REF_STR_AilThreeText)[0]);
   multi_init(i, retkey, REF_STR_AilThreeText, REF_STR_DigiChannelState, NULL,
      sizeof(hack_digi_channels), &hack_digi_channels, 3, digichan_dealfunc, &r);
   i++;

   standard_button_rect(&r,i,2,2,5);
   retkey=tolower(get_temp_string(REF_STR_AilThreeText+1)[0]);
   multi_init(i, retkey, REF_STR_AilThreeText+1, REF_STR_StereoReverseState, NULL,
      sizeof(snd_stereo_reverse), &snd_stereo_reverse, 2, NULL, &r);
   i++;

#ifdef AUDIOLOGS
   standard_button_rect(&r,i,2,2,5);
   retkey=tolower(get_temp_string(REF_STR_MusicText+3)[0]);
   multi_init(i, retkey, REF_STR_MusicText+3, REF_STR_AudiologState, NULL,
      sizeof(audiolog_setting), &audiolog_setting, 3, audiolog_dealfunc, &r);
   i++;
#endif

   standard_button_rect(&r,5,2,2,5);
   retkey=tolower(get_temp_string(REF_STR_MusicText+2)[0]);
   pushbutton_init(RETURN_BUTTON,retkey,REF_STR_MusicText+2,wrapper_pushbutton_func,&r);

   keywidget_init(QUIT_BUTTON,KB_FLAG_ALT|'x',wrapper_pushbutton_func);
   opanel_redraw(TRUE);
}

void sound_screen_init(void)
{
   Rect r;
   uchar sliderbase;
   char retkey;
   char slider_offset = 0;
#ifdef AUDIOLOGS
   if (sfx_card)
      slider_offset = 10;
#endif

   clear_obuttons();

   if(music_card) {
      standard_slider_rect(&r,0,2,5);
      // let's double the width of these things, eh?
      r.lr.x+=(r.lr.x-r.ul.x);
      r.ul.y-=slider_offset;
      r.lr.y-=slider_offset;
      sliderbase=r.lr.x-r.ul.x-2;
      slider_init(0,REF_STR_MusicText,sizeof(ushort),
         TRUE,&player_struct.questvars[MUSIC_VOLUME_QVAR],100,sliderbase,recompute_music_level,&r);
   }
   else {
      standard_button_rect(&r,0,2,2,5);
      r.lr.x+=(r.lr.x-r.ul.x);
      r.ul.y-=slider_offset/2;
      r.lr.y-=slider_offset/2;
      textwidget_init(0,BUTTON_COLOR,REF_STR_MusicFeedbackText+2,&r);
   }

   if(sfx_card) {
      if(digi_gain) {
         standard_slider_rect(&r,3,2,5);
         r.lr.x+=(r.lr.x-r.ul.x);
         r.ul.y-=slider_offset;
         r.lr.y-=slider_offset;
         slider_init(1,REF_STR_MusicText+1,sizeof(ushort),
            FALSE,&player_struct.questvars[SFX_VOLUME_QVAR],100,sliderbase,recompute_digifx_level,&r);
      }
      else {
         standard_button_rect(&r,3,2,2,5);
         r.ul.y-=slider_offset;
         r.lr.y-=slider_offset;
         multi_init(1,get_temp_string(REF_STR_MusicText+1)[0],
            REF_STR_MusicText+1,REF_STR_OffonText,REF_STR_MusicFeedbackText+5,
            sizeof(sfx_on),&sfx_on, 2, digi_toggle_deal, &r);
      }
   }
   else {
      standard_button_rect(&r,3,2,2,5);
      r.lr.x+=(r.lr.x-r.ul.x);
      textwidget_init(1,BUTTON_COLOR,REF_STR_MusicFeedbackText+7,&r);
   }

#ifdef AUDIOLOGS
   if(sfx_card) {
      standard_slider_rect(&r,6,2,5);
      r.lr.x+=(r.lr.x-r.ul.x);
      r.ul.y-=slider_offset;
      r.lr.y-=slider_offset;
      slider_init(2,REF_STR_MusicText+4,sizeof(ushort),
         FALSE,&player_struct.questvars[ALOG_VOLUME_QVAR],100,sliderbase,recompute_audiolog_level,&r);
   }
#endif

   if (sfx_card)
   {
      standard_button_rect(&r,2,2,2,5);
      retkey=tolower(get_temp_string(REF_STR_AilThreeText+2)[0]);
      pushbutton_init(AUDIO_OPT_BUTTON, retkey, REF_STR_AilThreeText+2, wrapper_pushbutton_func, &r);
   }

   standard_button_rect(&r,5,2,2,5);
   retkey=tolower(get_temp_string(REF_STR_MusicText+2)[0]);
   pushbutton_init(RETURN_BUTTON,retkey,REF_STR_MusicText+2,wrapper_pushbutton_func,&r);

   keywidget_init(QUIT_BUTTON,KB_FLAG_ALT|'x',wrapper_pushbutton_func);

   opanel_redraw(TRUE);
}

//
// THE OPTIONS SCREEN: Initialization, update funcs
//

void gamma_dealfunc(ushort gamma_qvar)
{
   fix gamma;

//   gamma=FIX_UNIT-fix_make(0,gamma_qvar);
//   gamma=fix_mul(gamma,gamma)+(FIX_UNIT/2);
   gamma=QVAR_TO_GAMMA(gamma_qvar);
   gr_set_gamma_pal(0,256,gamma);
}

#ifdef SVGA_SUPPORT
bool wrapper_screenmode_hack = FALSE;
void screenmode_change(new_mode)
{
   extern short mode_id;
   mode_id = new_mode;
   QUESTVAR_SET(SCREENMODE_QVAR, new_mode);
   change_mode_func(0,0,(void *)_current_loop);
   wrapper_screenmode_hack = TRUE;
}
#endif

void language_change(uchar lang)
{
   extern int string_res_file, mfdart_res_file;
   extern char* mfdart_files[];
   extern char* language_files[];
   extern void invent_language_change(void);
   extern void mfd_language_change(void);
   extern void side_icon_language_change(void);

   ResCloseFile(string_res_file);
   ResCloseFile(mfdart_res_file);

   mfdart_res_file=ResOpenFile(mfdart_files[lang]);
   if(mfdart_res_file<0)
      critical_error(CRITERR_RES|2);
   
   string_res_file=ResOpenFile(language_files[lang]);
   if(string_res_file<0)
      critical_error(CRITERR_RES|0);

   QUESTVAR_SET(LANGUAGE_QVAR,lang);

   // in case we got here from interpret_qvars, and thus
   // haven't set this yet
   which_lang=lang;

   invent_language_change();
   mfd_language_change();
   side_icon_language_change();
   free_options_cursor();
   make_options_cursor();
}

void language_dealfunc(uchar lang)
{
   language_change(lang);

   render_run();
   opanel_redraw(FALSE);
}

void dclick_dealfunc(ushort dclick_qvar)
{
   uiDoubleClickDelay=QVAR_TO_DCLICK(dclick_qvar,0);
   uiDoubleClickTime=QVAR_TO_DCLICK(dclick_qvar,1);
}

void joysens_dealfunc(ushort joysens_qvar)
{
   extern fix inpJoystickSens;

   inpJoystickSens=QVAR_TO_JOYSENS(joysens_qvar);

}

#pragma disable_message(202)
void center_joy_go(uchar butid)
{
   extern bool recenter_joystick(short keycode, ulong context, void* data);

   recenter_joystick(0,0,0);
   joystick_screen_init();
}
#pragma enable_message(202)

void center_joy_pushbutton_func(uchar butid)
{
   int i;
   string_message_info(REF_STR_CenterJoyPrompt);

   // take over this button, null the other buttons
   // except for RETURN and QUIT;

   for(i=0;i<MAX_OPTION_BUTTONS;i++) {
      if(i==butid)
         keywidget_init(i,KEY_ENTER,center_joy_go);
      else if(i!=RETURN_BUTTON && i!=QUIT_BUTTON)
         OButtons[i].evmask=0;
   }
}

void detail_dealfunc(uchar det)
{
   extern errtype change_detail_level(byte new_level);

   change_detail_level(det);
   uiHideMouse(NULL);
   render_run();
   if(full_game_3d)
      opanel_redraw(FALSE);
   uiShowMouse(NULL);
}

void mousehand_dealfunc(ushort lefty)
{
   mouse_set_lefty(lefty);
}

#if defined(VFX1_SUPPORT)||defined(CTM_SUPPORT)
#pragma disable_message(202)
void headset_stereo_dealfunc(bool st_on)
{
   extern bool inp6d_headset;
   extern bool inp6d_stereo;
//   extern bool ui_stereo_on;
   if ((inp6d_headset) && (i6d_device != I6D_ALLPRO))
   {
//      ui_stereo_on = inp6d_stereo;
      if (!inp6d_stereo)
         i6_video(I6VID_CLOSEDOWN,NULL); // this will want to be I6VID_STR_CLOSE at some point
      else
      {
         if (i6_video(I6VID_STR_START,NULL))
         {
            Warning(("Headset stereo startup failed!\n"));
            return;
         }
      }
   }
}

void headset_tracking_dealfunc(bool tr_on)
{
   Warning(("tracking now %d!\n",tr_on));
   return;
}

void headset_fov_dealfunc(int hackval)
{
   inp6d_curr_fov = hack_headset_fov + HEADSET_FOV_MIN;
   Warning(("FOV now %d!\n",inp6d_curr_fov));
   return;
}
#pragma enable_message(202)
#endif

#pragma disable_message(202)
void olh_dealfunc(uchar olh)
{
   extern bool toggle_olh_func(short keycode, ulong context, void* data);

   toggle_olh_func(0,0,0);
}
#pragma enable_message(202)

#ifdef STEREO_SUPPORT
#define INITIAL_OCULAR_DIST   fix_make(3,0x4000)
#endif

ushort wrap_joy_type = 0;
ushort high_joy_flags;
void joystick_type_func(ushort new_joy_type)
{
   extern uchar joystick_count;
   joystick_count = joy_init(high_joy_flags | new_joy_type);
   config_set_single_value("joystick",CONFIG_INT_TYPE,(config_valtype)(high_joy_flags|new_joy_type));
   joystick_screen_init();
}

void joystick_screen_init(void)
{
   Rect r;
   int i = 0;
   char *keys;
   extern bool inp6d_headset;
   uchar sliderbase;

   extern uchar joystick_count;
   keys=get_temp_string(REF_STR_KeyEquivs6);
   clear_obuttons();

   standard_button_rect(&r,i,2,2,1);
   multi_init(i,keys[i],REF_STR_JoystickType,REF_STR_JoystickTypes,
      NULL, sizeof(wrap_joy_type),
      &wrap_joy_type, 4, joystick_type_func, &r);
   i++;

   standard_button_rect(&r,i,2,2,1);
   pushbutton_init(i,keys[i],REF_STR_CenterJoy,center_joy_pushbutton_func,&r);
   if(!joystick_count && !inp6d_headset) {
      dim_pushbutton(i);
   }
   i++;

   if(joystick_count) {
      standard_slider_rect(&r,i,2,1);
      sliderbase=(r.lr.x-r.ul.x-2)>>1;
      slider_init(i,REF_STR_JoystickSens,sizeof(ushort),FALSE,
         &player_struct.questvars[JOYSENS_QVAR],256,sliderbase,joysens_dealfunc,&r);
   }
   i++;

   standard_button_rect(&r,5,2,2,1);
   pushbutton_init(RETURN_BUTTON,keys[i],REF_STR_OptionsText+5,wrapper_pushbutton_func,&r);

   keywidget_init(QUIT_BUTTON,KB_FLAG_ALT|'x',wrapper_pushbutton_func);

   opanel_redraw(TRUE);
}

#pragma disable_message(202)
void joystick_button_func(uchar butid)
{
   joystick_screen_init();
}
#pragma enable_message(202)

void input_screen_init(void)
{
   Rect r;
   char* keys;
   int i=0;
   uchar sliderbase;
   extern bool inp6d_headset;

   keys=get_temp_string(REF_STR_KeyEquivs1);
   clear_obuttons();

   standard_button_rect(&r,i,2,2,1);
   r.ul.x-=1;
   multi_init(i,keys[0],REF_STR_OptionsText+0,REF_STR_OffonText,
      REF_STR_PopupCursFeedback, sizeof(popup_cursors),
      &popup_cursors, 2, NULL, &r);
   i++;

   standard_button_rect(&r,i,2,2,1);
   multi_init(i,keys[1],REF_STR_OptionsText+1,REF_STR_MouseHand,
      REF_STR_HandFeedback, sizeof(player_struct.questvars[MOUSEHAND_QVAR]),
      &player_struct.questvars[MOUSEHAND_QVAR], 2, mousehand_dealfunc, &r);
   i++;

   standard_button_rect(&r,5,2,2,1);
   pushbutton_init(RETURN_BUTTON,keys[3],REF_STR_OptionsText+5,wrapper_pushbutton_func,&r);

   standard_slider_rect(&r,i,2,1);
   r.ul.x-=1;
   sliderbase=((r.lr.x-r.ul.x-3)*(FIX_UNIT/3))/USHRT_MAX;
   slider_init(i,REF_STR_DoubleClick,sizeof(ushort),
      FALSE,&player_struct.questvars[DCLICK_QVAR],USHRT_MAX,sliderbase,dclick_dealfunc,&r);
   i++;

   standard_button_rect(&r,i,2,2,1);
   r.ul.x-=1;
   pushbutton_init(i,keys[2],REF_STR_Joystick,joystick_button_func, &r);
   i++;

   keywidget_init(QUIT_BUTTON,KB_FLAG_ALT|'x',wrapper_pushbutton_func);

   opanel_redraw(TRUE);
}

void video_screen_init(void)
{
   Rect r;
   int i;
   char* keys;
#ifdef SVGA_SUPPORT
   extern short mode_id;
#endif
   uchar sliderbase;
#ifdef STEREO_SUPPORT
   extern bool inp6d_headset;
#endif

   keys=get_temp_string(REF_STR_KeyEquivs3);

   clear_obuttons();

   i = 0;

#ifdef SVGA_SUPPORT
   standard_button_rect(&r,i,2,2,2);
   pushbutton_init(SCREENMODE_BUTTON, keys[0], REF_STR_VideoText, wrapper_pushbutton_func,&r);
#endif

   i++;

   standard_button_rect(&r,i,2,2,2);
   r.lr.x+=2;
   multi_init(i,keys[1],REF_STR_OptionsText+4,REF_STR_DetailLvl,
      REF_STR_DetailLvlFeedback, sizeof(player_struct.detail_level),
      &player_struct.detail_level, 4, detail_dealfunc, &r);

   i++;

   standard_slider_rect(&r,i,2,2);
   r.ul.x=r.ul.x+1;
   sliderbase=((r.lr.x-r.ul.x-1)*((29*FIX_UNIT)/100))/USHRT_MAX;
   slider_init(i,REF_STR_OptionsText+3,sizeof(ushort),
      TRUE,&player_struct.questvars[GAMMACOR_QVAR],USHRT_MAX,sliderbase,gamma_dealfunc,&r);

#if defined(VFX1_SUPPORT)||defined(CTM_SUPPORT)
   i++;
   standard_button_rect(&r,i,2,2,2);
   pushbutton_init(HEADSET_BUTTON, keys[2], REF_STR_HeadsetText, wrapper_pushbutton_func,&r);
   if (!inp6d_headset)
      dim_pushbutton(HEADSET_BUTTON);
#endif

   i++;
   standard_button_rect(&r,5,2,2,2);
   pushbutton_init(RETURN_BUTTON,keys[3],REF_STR_OptionsText+5,wrapper_pushbutton_func,&r);

   keywidget_init(QUIT_BUTTON,KB_FLAG_ALT|'x',wrapper_pushbutton_func);

   opanel_redraw(TRUE);
}

#if defined(VFX1_SUPPORT)||defined(CTM_SUPPORT)
void headset_screen_init(void)
{
   Rect r;
   int i;
   char* keys;
#ifdef STEREO_SUPPORT
   extern bool inp6d_stereo;
   extern int inp6d_stereo_div;
#endif

   keys=get_temp_string(REF_STR_KeyEquivs5);

   clear_obuttons();

   i = 0;

   standard_button_rect(&r,i,2,2,2);
   pushbutton_init(HEAD_RECENTER_BUTTON, keys[0], REF_STR_HeadsetText + 1, wrapper_pushbutton_func,&r);

#ifdef STEREO_SUPPORT
   i++;
   standard_slider_rect(&r,i,2,2);
   r.ul.x -= 1;
   slider_init(i,REF_STR_HeadsetText + 2,sizeof(inp6d_stereo_div),
      FALSE,&inp6d_stereo_div,fix_make(10,0),INITIAL_OCULAR_DIST,NULL,&r);

   i++;
   standard_button_rect(&r,i,2,2,2);
   multi_init(i,keys[1],REF_STR_HeadsetText + 3,REF_STR_OffonText,
      NULL, sizeof(inp6d_stereo), &inp6d_stereo, 2, headset_stereo_dealfunc, &r);

   if (i6d_device == I6D_ALLPRO)
      dim_pushbutton(i);

   i++;
   standard_button_rect(&r,i,2,2,2);
   multi_init(i,keys[3],REF_STR_MoreHeadset + 1,REF_STR_OffonText,
      NULL, sizeof(headset_track), &headset_track, 2, headset_tracking_dealfunc, &r);

   i++;
   standard_slider_rect(&r,i,2,2);
   r.ul.x -= 1;
   slider_init(i,REF_STR_MoreHeadset,sizeof(hack_headset_fov),
      FALSE,&hack_headset_fov,HEADSET_FOV_MAX - HEADSET_FOV_MIN,
      inp6d_real_fov - HEADSET_FOV_MIN,headset_fov_dealfunc,&r);
#endif

   // Standard return button and other bureaucracy
   standard_button_rect(&r,5,2,2,2);
   pushbutton_init(RETURN_BUTTON,keys[2],REF_STR_OptionsText+5,wrapper_pushbutton_func,&r);
   keywidget_init(QUIT_BUTTON,KB_FLAG_ALT|'x',wrapper_pushbutton_func);
   opanel_redraw(TRUE);
}
#endif

#ifdef SVGA_SUPPORT
void screenmode_screen_init(void)
{
   Rect r;
   int i;
   char* keys;

   if (wrapper_screenmode_hack)
   {
      uiHideMouse(NULL);
      render_run();
      uiShowMouse(NULL);
      wrapper_screenmode_hack = FALSE;
   }

   keys=get_temp_string(REF_STR_KeyEquivs4);

   clear_obuttons();
   
   for (i=0; i < 4; i++)
   {
      extern short svga_mode_data[];
      bool mode_ok = FALSE;
      char j =0;
      standard_button_rect(&r,i,2,2,2);
      pushbutton_init(i,keys[i],REF_STR_ScreenModeText + i,screenmode_change,&r);
      while ((grd_info.modes[j] != -1) && !mode_ok)
      {
         if (grd_info.modes[j] == svga_mode_data[i])
            mode_ok = TRUE;
         j++;
      }
      if (!mode_ok)
         dim_pushbutton(i);
      else if (i == convert_use_mode)
         bright_pushbutton(i);
   }

   standard_button_rect(&r,5,2,2,2);
   pushbutton_init(RETURN_BUTTON,keys[2],REF_STR_OptionsText+5,wrapper_pushbutton_func,&r);

   keywidget_init(QUIT_BUTTON,KB_FLAG_ALT|'x',wrapper_pushbutton_func);

   opanel_redraw(TRUE);
}
#endif

void options_screen_init(void)
{
   Rect r;
   char* keys;
   int i=0;

   keys=get_temp_string(REF_STR_KeyEquivs2);
   clear_obuttons();

   olh_temp=(QUESTBIT_GET(OLH_QBIT)==0);

   // okay, I admit it, we're going to tweak these "standard"
   // button rects a little bit.

   standard_button_rect(&r,0,2,2,2);
   r.ul.x-=2;
   multi_init(i,keys[i],REF_STR_OptionsText+2,REF_STR_TerseText,
      REF_STR_TerseFeedback, sizeof(player_struct.terseness),
      &player_struct.terseness, 2, NULL, &r);
   i++;

   i++;

   standard_button_rect(&r,1,2,2,2);
   multi_init(i,keys[i],REF_STR_OnlineHelp,REF_STR_OffonText,
      NULL, sizeof(olh_temp), &olh_temp, 2, olh_dealfunc, &r);
   i++;

   i++;

   standard_button_rect(&r,2,2,2,2);
   multi_init(i,keys[i],REF_STR_Language,REF_STR_Languages, NULL,
      sizeof(which_lang), &which_lang, 3, language_dealfunc, &r);
   i++;

   standard_button_rect(&r,5,2,2,2);
   r.lr.x+=2;
   pushbutton_init(RETURN_BUTTON,keys[i],REF_STR_OptionsText+5,wrapper_pushbutton_func,&r);

   keywidget_init(QUIT_BUTTON,KB_FLAG_ALT|'x',wrapper_pushbutton_func);

   opanel_redraw(TRUE);
}

#pragma disable_message(202)
bool wrapper_options_func(short keycode, ulong context, void* data)
{
   wrapper_start(wrapper_init);
   return(OK);
}
#pragma enable_message(202);

//
// THE LOAD GAME SCREEN: Initialization, update funcs
//

extern void spoof_mouse_event();


#pragma disable_message(202)
void load_dealfunc(uchar butid,uchar index)
{
   begin_wait();
   Poke_SaveName(index);
   Spew(DSRC_EDITOR_Save,("attempting to load from %s\n",save_game_name));
   if(load_game(save_game_name)!=OK) {
      Warning(("Load game failed!\n"));
   }
   else {
      Spew(DSRC_EDITOR_Restore,("Game %d loaded!\n",index));
   }
   end_wait();
   spoof_mouse_event();
   wrapper_panel_close(TRUE);
}
#pragma enable_message(202)

void load_screen_init(void)
{
   extern uchar valid_save;

   clear_obuttons();

   textlist_init(0,comments,NUM_SAVE_SLOTS,SAVE_COMMENT_LEN,
      FALSE,0,valid_save,valid_save,REF_STR_UnusedSave,
      BUTTON_COLOR,WHITE,BUTTON_COLOR+2,0,load_dealfunc,NULL);

   keywidget_init(QUIT_BUTTON,KB_FLAG_ALT|'x',wrapper_pushbutton_func);

   opanel_redraw(TRUE);
}

//
// THE SAVE GAME SCREEN: Initialization, update funcs
//

#pragma disable_message(202)
void save_dealfunc(uchar butid,uchar index)
{
   if(!ObjSysOkay()) {
      string_message_info(REF_STR_ObjSysBad);
      savegame_verify=index;
      verify_screen_init(save_verify_pushbutton_handler,save_verify_slorker);
   }
   else {
      message_info("");
      do_savegame_guts(index);
   }
}

void save_screen_init(void)
{
   extern uchar valid_save;

   clear_obuttons();

   textlist_init(0,comments,NUM_SAVE_SLOTS,SAVE_COMMENT_LEN,
      TRUE,0xFFFF,0xFFFF,valid_save,REF_STR_UnusedSave,
      BUTTON_COLOR,WHITE,BUTTON_COLOR+2,REF_STR_EnterSaveString,
      save_dealfunc,NULL);

   keywidget_init(QUIT_BUTTON,KB_FLAG_ALT|'x',wrapper_pushbutton_func);

   opanel_redraw(TRUE);
}


void wrapper_start(void (*init)(void))
{
   extern void reset_input_system(void);
   extern errtype change_detail_level(byte new_level);

   if (wrapper_panel_on) return;
   inv_last_page = inventory_page;
   if (!game_paused)
      pause_game_func(0,0,0);
   if (!full_game_3d)
      message_info("");
   inventory_page = -1;
   wrapper_panel_on = TRUE;
   suspend_game_time();
   opt_font=(grs_font*)ResLock(OPTIONS_FONT);
#ifndef STATIC_BUTTON_STORE 
   OButtons=(opt_button*)(_offscreen_mfd.bm.bits);
   fv=full_visible;
   full_visible=0;
#endif
   uiHideMouse(NULL);
   if (full_game_3d)
   {
#ifdef SVGA_SUPPORT
      uchar old_over = gr2ss_override;
#endif
      render_run();
      gr_push_canvas(grd_screen_canvas);
#ifdef SVGA_SUPPORT
      gr2ss_override = OVERRIDE_ALL;
#endif
      ss_get_bitmap(&inv_view360_canvas.bm,GAME_MESSAGE_X,GAME_MESSAGE_Y);
#ifdef SVGA_SUPPORT
      gr2ss_override = old_over;
#endif
      gr_pop_canvas();
   }
   else
      inventory_clear();
   uiShowMouse(NULL);
   uiInstallRegionHandler(inventory_region, UI_EVENT_MOUSE|UI_EVENT_MOUSE_MOVE, opanel_mouse_handler, NULL, &wrap_id);
   uiInstallRegionHandler(inventory_region, UI_EVENT_KBD_COOKED, opanel_kb_handler, NULL, &wrap_key_id);
   uiGrabFocus(inventory_region, UI_EVENT_KBD_COOKED|UI_EVENT_MOUSE);
   region_set_invisible(inventory_region,FALSE);
   reset_input_system();
   init();
}

#define NEEDED_DISKSPACE   630000
errtype check_free_diskspace(int *needed)
{
   struct diskfree_t freespace;
   _dos_getdiskfree(0, &freespace);
   if (freespace.avail_clusters * freespace.sectors_per_cluster * freespace.bytes_per_sector < NEEDED_DISKSPACE)
   {
      *needed = NEEDED_DISKSPACE - (freespace.avail_clusters * freespace.sectors_per_cluster * freespace.bytes_per_sector);
      return(ERR_NOMEM);
   }
   *needed = 0;
   return(OK);
}

errtype do_savegame_guts(uchar slot)
{
   extern uchar valid_save;
   errtype retval = OK;

   begin_wait();
   if (!(valid_save & (1 << slot)))
   {
      int needed;
//      char buf1[128],buf2[128];
      if (check_free_diskspace(&needed) == ERR_NOMEM)
      {
//         lg_sprintf(buf2, get_string(REF_STR_InsufficientDisk, buf1, 128), needed);
         string_message_info(REF_STR_InsufficientDisk);
         retval = ERR_NOMEM;
      }
   }
   if (retval == OK)
   {
      Poke_SaveName(slot);
      if (save_game(save_game_name, comments[slot]) != OK)
      {
         Spew(DSRC_EDITOR_Save, ("Save game failed!\n"));
         message_info("Game save failed!");
   //      strcpy(comments[comment_mode], original_comment);
         retval = ERR_NOEFFECT;
         valid_save &= ~(1<<slot);
      }
      else
         Spew(DSRC_EDITOR_Save, ("Game %d saved!\n", slot));
      if (retval == OK)
         valid_save |= 1 << slot;
   }
   end_wait();
   spoof_mouse_event();
   if (retval == OK)
      wrapper_panel_close(TRUE);
   return(retval);
}

#pragma disable_message(202)
bool wrapper_region_mouse_handler(uiMouseEvent* ev, Region* r, void* data)
{
   if (global_fullmap->cyber)
   {
      uiSetRegionDefaultCursor(r,NULL);
      return FALSE;
   }
   else
      uiSetRegionDefaultCursor(r,&option_cursor);

   if (ev->action & MOUSE_DOWN)
   {
      wrapper_options_func(0,0,(void*)TRUE);
      return TRUE;
   }
   return FALSE;
}
#pragma enable_message(202)


#endif // NOT_YET���

errtype make_options_cursor(void)
{
   char* s;
   short w,h;
   LGPoint hot = {0,0};
   grs_canvas cursor_canv;
   short orig_w;
   extern uchar svga_options_cursor_bits[];
   uchar old_over = gr2ss_override;
   gr2ss_override = OVERRIDE_ALL;

   orig_w = w=res_bm_width(REF_IMG_bmOptionCursor);
   h=res_bm_height(REF_IMG_bmOptionCursor);
   ss_point_convert(&w,&h,FALSE);
   gr_init_bm(&option_cursor_bmap, svga_options_cursor_bits, BMT_FLAT8, BMF_TRANS, w,h);
   gr_make_canvas(&option_cursor_bmap,&cursor_canv);
   gr_push_canvas(&cursor_canv);
   gr_clear(0);
   s=get_temp_string(REF_STR_ClickForOptions);
   gr_set_font((grs_font*)ResLock(OPTIONS_FONT));
   wrap_text(s,orig_w-3);
   gr_string_size(s,&w,&h);
   gr_set_fcolor(0xB8);
   ss_rect(1,1,w+2,h+2);
   gr_set_fcolor(0xD3);
   ss_string(s,2,1);
   unwrap_text(s);
   uiMakeBitmapCursor(&option_cursor,&option_cursor_bmap,hot);
   gr_pop_canvas();
   ResUnlock(OPTIONS_FONT);
   cursor_loaded = TRUE;
   gr2ss_override = old_over;

   return OK;
}

#ifdef NOT_YET //���

void free_options_cursor(void)
{
#ifndef SVGA_SUPPORT
   if(cursor_loaded)
      Free(option_cursor_bmap.bits);
#endif
}

errtype wrapper_create_mouse_region(Region* root)
{ 
   errtype err;
   int id;
   Rect r = { { 0, 0}, {STATUS_X,STATUS_HEIGHT}};
   Region* reg = &(options_mouseregion[free_mouseregion++]);

   err = region_create(root,reg,&r,2,0,REG_USER_CONTROLLED|AUTODESTROY_FLAG,NULL,NULL,NULL,NULL);
   if (err != OK) return err;
   err = uiInstallRegionHandler(reg,UI_EVENT_MOUSE|UI_EVENT_MOUSE_MOVE,(uiHandlerProc)wrapper_region_mouse_handler,NULL,&id);
   if (err != OK) return err;
   if (!cursor_loaded)
   {
      err=make_options_cursor();
      if(err!=OK) return err;
   }
   uiSetRegionDefaultCursor(reg,&option_cursor);
   return OK;
}

#pragma disable_message(202)
bool saveload_hotkey_func(short keycode, ulong context, void* data)
{
#ifdef DEMO
   return(TRUE);
#else
   if ((!data) && (!can_save()))
      return(TRUE);
   wrapper_start(data ? load_screen_init : save_screen_init);
   string_message_info(data ? REF_STR_LoadSlot : REF_STR_SaveSlot);
   return(TRUE);
#endif
}

bool demo_quit_func(short keycode, ulong context, void* data)
{
   wrapper_start(quit_verify_init);
   string_message_info(REF_STR_QuitConfirm);
   return(TRUE);
}
#pragma enable_message(202)

#endif // NOT_YET���
