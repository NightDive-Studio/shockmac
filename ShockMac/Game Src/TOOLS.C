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
 * $Source: r:/prj/cit/src/RCS/tools.c $
 * $Revision: 1.94 $
 * $Author: dc $
 * $Date: 1994/11/25 16:58:28 $
 */

// Source code from random useful tools and utilities

#define __TOOLS_SRC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "criterr.h"
#include "gr2ss.h"
#include "tools.h"
#include "mainloop.h"
#include "gamescr.h"
#include "musicai.h"
#include "colors.h"
#include "gamestrn.h"
#include "invdims.h"
#include "fullscrn.h"
#include "frprotox.h"
#include "citres.h"
#include "hud.h"
#include "canvchek.h"
#include "cybmem.h"
#include "player.h"
#include "faketime.h"
#include "cit2d.h"

//���#include <gifd.h>


#ifndef STORE_CLIP 
#define STORE_CLIP(a,b,c,d) a = gr_get_clip_l(); \
   b = gr_get_clip_t();  c = gr_get_clip_r(); d = gr_get_clip_b()
#endif // !STORE_CLIP

#ifndef RESTORE_CLIP 
#define RESTORE_CLIP(a,b,c,d) gr_set_cliprect(a,b,c,d)
#endif // !RESTORE_CLIP

//------------
//  PROTOTYPES
//------------
int str_to_hex(char val);
void text_button(char *text, int xc, int yc, int col, int shad, int w, int h);
void simple_text_button(char *text, int xc, int yc, int col);
void Rect_gr_rect(LGRect *r);
void Rect_gr_box(LGRect *r);
char *itoa_2_10(char *s, int val);
int hyphenated_wrap_text(char *ps, char* out, short width);
void zoom_rect(LGRect* start, LGRect* end);


int str_to_hex(char val)
{
   int retval = 0;
   if ((val >= '0') && (val <= '9'))
      retval = val - '0';
   else if ((val >= 'A') && (val <= 'F'))
      retval = 10 + val - 'A';
   else if ((val >= 'a') && (val <= 'f'))
      retval = 10 + val - 'a';
   return(retval);
}

void strtoupper(char *text)
{
   for(;*text;text++) {
      if(islower(*text))
         (*text)+='A'-'a';
   }
}

#ifdef SVGA_SUPPORT
bool shadow_scale = TRUE;
#endif
void draw_shadowed_string(char* s, short x, short y, uchar shadow)
{
   LGPoint npt;
   ubyte color = gr_get_fcolor();
   npt.x = x;
   npt.y = y;
   if (shadow && FONT_IS_MONO(gr_get_font())) // draw a black box
   {
#ifdef SVGA_SUPPORT
      extern char convert_use_mode;
      extern bool perform_svga_conversion(uchar mask);
      extern void ss_scale_string(char *s, short x, short y);
      if ((convert_use_mode > 0) && (perform_svga_conversion(OVERRIDE_FONT)))
      {
         if (shadow_scale)
            ss_point_convert(&(npt.x),&(npt.y),FALSE);
         gr_set_fcolor(shadow);
         ss_scale_string(s,npt.x-1,npt.y-1);
         ss_scale_string(s,npt.x,npt.y-1);
         ss_scale_string(s,npt.x+1,npt.y-1);
         ss_scale_string(s,npt.x,npt.y+1);
         ss_scale_string(s,npt.x-1,npt.y+1);
         ss_scale_string(s,npt.x+1,npt.y+1);
         ss_scale_string(s,npt.x-1,npt.y);
         ss_scale_string(s,npt.x+1,npt.y);
         gr_set_fcolor(color);
         ss_scale_string(s,npt.x,npt.y);
      }
      else
#endif
      {
         gr_set_fcolor(shadow);
         gr_string(s,npt.x-1,npt.y-1);
         gr_string(s,npt.x,npt.y-1);
         gr_string(s,npt.x+1,npt.y-1);
         gr_string(s,npt.x,npt.y+1);
         gr_string(s,npt.x-1,npt.y+1);
         gr_string(s,npt.x+1,npt.y+1);
         gr_string(s,npt.x-1,npt.y);
         gr_string(s,npt.x+1,npt.y);
         gr_set_fcolor(color);
         gr_string(s,npt.x,npt.y);
      }
   }
   else
   {
      gr_set_fcolor(color);
//      gr_string(s,npt.x,npt.y);
      ss_string(s,npt.x,npt.y);
   }
}


void draw_hires_resource_bm(Ref id, int x, int y)
{
	FrameDesc *f = (FrameDesc *)RefLock(id);
	if (f == NULL)
		critical_error(CRITERR_MEM|9);
	f->bm.bits = (uchar *)(f+1);
	gr_bitmap(&f->bm, x, y);
	RefUnlock(id);
}

void draw_hires_halfsize_bm(Ref id, int x, int y)
{
	FrameDesc *f = (FrameDesc *)RefLock(id);
	if (f == NULL)
		critical_error(CRITERR_MEM|9);
	f->bm.bits = (uchar *)(f+1);
	gr_scale_bitmap(&f->bm, x, y, (f->bm.w >> 1), (f->bm.h >> 1));
	RefUnlock(id);
}


errtype draw_raw_res_bm_extract(Ref id, int x, int y)
{
   grs_bitmap bm;
   extract_temp_res_bitmap(&bm,id);
   ss_bitmap(&bm,x,y);
   return OK;
}


errtype draw_raw_resource_bm(Ref id, int x, int y)
{
   FrameDesc *f;
   
   f = (FrameDesc *)RefLock(id);
   if (f == NULL)
      critical_error(CRITERR_MEM|9);
   f->bm.bits = (uchar *)(f+1);
   ss_bitmap(&f->bm, x, y);
   RefUnlock(id);
   return (OK);
}  

errtype draw_res_bm_core(Ref id, int x, int y, bool scale)
{
   FrameDesc *f;
   LGRect mouse_rect;

   f = (FrameDesc *)RefLock(id);
   if (f == NULL)
      critical_error(CRITERR_MEM|9);
   mouse_rect.ul.x = x;
   mouse_rect.ul.y = y;
   mouse_rect.lr.x = x + f->bm.w;
   mouse_rect.lr.y = y + f->bm.h;

   // Set the palette right, if one is provided....
   f->bm.bits = (uchar *)(f+1);
   if (is_onscreen()) uiHideMouse(&mouse_rect);
   if (scale)
      ss_bitmap(&f->bm, x, y);
   else
      ss_noscale_bitmap(&f->bm, x, y);
   if (is_onscreen()) uiShowMouse(&mouse_rect);
   RefUnlock(id);
   return (OK);
}  

errtype draw_res_bm(Ref id, int x, int y)
{
   return(draw_res_bm_core(id,x,y,TRUE));
}

// Note, does no mouse code!
errtype draw_full_res_bm(Ref id, int x, int y, bool fade_in)
{
   FrameDesc *f;
   short *temp_pall;
   byte  pal_id;
   extern void finish_pal_effect(byte id);
   extern byte palfx_start_fade_up(uchar *new_pal);

   f = (FrameDesc *)RefLock(id);
   if (f == NULL)
      critical_error(CRITERR_MEM|9);

   // Set the palette right, if one is provided....
   if (f->pallOff)
   {
      temp_pall = (short *) (((uchar *) ResGet(REFID(id))) + f->pallOff);
      gr_set_pal(*temp_pall,*(temp_pall + 1),(uchar *)(temp_pall + 2));
   }

   if (fade_in)
      pal_id=palfx_start_fade_up((uchar *) (temp_pall+2));

   f->bm.bits = (uchar *)(f+1);
   ss_scale_bitmap(&f->bm, x, y, 640, 400);		// KLC  ss_bitmap(&f->bm, x, y);
   RefUnlock(id);	
   ResDrop(REFID(id));
   if (fade_in)
      finish_pal_effect(pal_id);
   return (OK);
}  

int res_bm_width(Ref id)
{
   FrameDesc *f;
   int n;

   f = (FrameDesc *)RefLock(id);
   if (f == NULL)
      critical_error(CRITERR_MEM|9);
   n = f->bm.w;
   RefUnlock(id);
   return (n);
}  

int res_bm_height(Ref id)
{
   FrameDesc *f;
   int n;

   f = (FrameDesc *)RefLock(id);
   if (f == NULL)
      critical_error(CRITERR_MEM|9);
   n = f->bm.h;
   RefUnlock(id);
   return (n);
}

errtype res_draw_text_shadowed(Id id, char *text, int x, int y, bool shadow)
{
   gr_set_font((grs_font *)ResLock(id));
   draw_shadowed_string(text, x, y, shadow);
   ResUnlock(id);
   return(OK);
}


errtype res_draw_string(Id font, int strid, int x, int y)
{
   return res_draw_text(font,get_temp_string(strid),x,y);
}

// have some god damn parameters
// xc,yc is position, usually text center
// w.h. is rectangle wid+hgt, note <0 means that the x|yc is the upper left, not the center
// shad is how much to shade down color (-1 is no shadow)
// col is color of rectangle and text
void text_button(char *text, int xc, int yc, int col, int shad, int w, int h)
{
   int ux, uy;
   short tw, th;

   gr_string_size(text,&tw,&th);

   // do wacked out conversionitis
   if (w<0) { ux=xc; w=-w; xc=ux+(w>>1); } else ux=xc-(w>>1);
   if (h<0) { uy=yc; h=-h; yc=uy+(h>>1); } else uy=yc-(h>>1);

   // two rectangles...
   gr_set_fcolor(col);
   ss_rect(ux,uy,ux+w,uy+h);
   if (shad>=0)
   {
      gr_set_fcolor(col+shad);
      ss_rect(ux+1,uy+1,ux+w-1,uy+h-1);
      gr_set_fcolor(col);
   }
   // some text, eh?
   ss_string(text,xc-(tw>>1),yc-(th>>1));
}

// ok, the easy case...
// centered at xc,yc, color base, auto-shadowed, size out setting
void simple_text_button(char *text, int xc, int yc, int col)
{
   short w,h;
   gr_string_size(text,&w,&h);
   text_button(text,xc,yc,col,4,w+12,h+8);
}

void Rect_gr_rect(LGRect *r)
{
   ss_rect(r->ul.x,r->ul.y,r->lr.x,r->lr.y);
}

void Rect_gr_box(LGRect *r)
{
   ss_box(r->ul.x,r->ul.y,r->lr.x,r->lr.y);
}

char *itoa_2_10(char *s, int val)
{
   s[0]='0'+(val/10);
   s[1]='0'+(val%10);
   s[2]='\0';
   return s;
}

// max 99 hours... 
void second_format(int sec_remain, char *s)
{
   int c_l;
   if (sec_remain >= 3600)
	 { itoa_2_10(s,sec_remain/3600); sec_remain%=3600; c_l=3; s[2]=':'; }
   else c_l=0;
   itoa_2_10(s+c_l,sec_remain/60);
   s[c_l+2]=':';
   sec_remain%=60;
   itoa_2_10(s+c_l+3,sec_remain);
   if (s[0]=='0') s[0]=' ';
}

#ifdef NOT_YET //��� later, dude

#define BIG_BUF

#pragma disable_message(202)
bool gifdump_func(short keycode, ulong context, void* data)
{
   unsigned char *temp_buf;
   int giffp;
   char harold[45];

   strcpy(harold,"SHOCK000.GIF");
   giffp = open_gen(harold, O_CREAT|O_BINARY|O_WRONLY|O_TRUNC,S_IWRITE);
   if (giffp == -1)
   {
      message_info("GIF dump failed!");
      return(ERR_NOEFFECT);
   }
   {
      temp_buf=big_buffer;
	   gd_dump_screen(giffp, temp_buf);
      strcat(harold," saved");
      message_info(harold);
   }
   return(TRUE);
}
#pragma enable_message(202)

#endif //��� NOT_YET


#define FULLSCREEN_MESSAGE_X  125
#define FULLSCREEN_MESSAGE_Y  8

#define MESSAGE_BUFSZ 128

#ifdef SVGA_SUPPORT_HATE_HATE
void mouse_unconstrain(void)
{
   // Note we are not calling the UI here since we are looking
   // at actual screen size
   mouse_constrain_xy(0,0,grd_cap->w-1,grd_cap->h-1);
}
#endif


errtype string_message_info(int strnum)
{
   char buf[MESSAGE_BUFSZ];
   get_string(strnum,buf,MESSAGE_BUFSZ);
   return message_info(buf);
}


char last_message[128];
ulong message_clear_time;

#define MESSAGE_INTERVAL   1200
#define CHAR_SOFTCR     0x01            // soft carriage return (wrapped text)
#define CHAR_SOFTSP     0x02            // soft space (wrapped text)
#define MESSAGE_LEN 80

LGRect msg_rect[2]=
{
   {GAME_MESSAGE_X,GAME_MESSAGE_Y,GAME_MESSAGE_X+GAME_MESSAGE_W,GAME_MESSAGE_Y+GAME_MESSAGE_H},
   {FULLSCREEN_MESSAGE_X,FULLSCREEN_MESSAGE_Y,FULLSCREEN_MESSAGE_X+GAME_MESSAGE_W,FULLSCREEN_MESSAGE_Y+GAME_MESSAGE_H}
};

bool message_resend = FALSE;
extern bool game_paused;
extern bool view360_message_obscured;
void strip_newlines(char* buf);


// Use the string wrapper's secret characters to delete newlines and double spaces.
void strip_newlines(char* buf)
{
   char* s;
   for(s = buf; *s != '\0'; s++)
   {
      if (*s == '\n')
	      *s = CHAR_SOFTSP;
	   if (isspace(*s) && isspace(*(s+1)))
	      *s = CHAR_SOFTSP;
   }
}

errtype message_info(char *info_text)
{
   extern errtype inventory_draw_new_page(int pgnum);
   int x,y;
   char buf[MESSAGE_LEN];

   if (info_text != NULL)
   {
      strncpy(buf,info_text,MESSAGE_LEN);
      strip_newlines(buf);
   }
   else buf[0] = '\0';
   if (_current_loop <= FULLSCREEN_LOOP)
   {
      short a,b,c,d;
      LGRect *r=&msg_rect[(full_game_3d && !game_paused)?1:0];


      x = r->ul.x;
      y = r->ul.y;
      if (is_onscreen()) uiHideMouse(r);
      gr_push_canvas(grd_screen_canvas);
      STORE_CLIP(a,b,c,d);

      ss_safe_set_cliprect(r->ul.x,r->ul.y,r->lr.x,r->lr.y);
      if (!full_game_3d)
      {
	     if (!view360_message_obscured ||  game_paused)
         {
//KLC - chg for new art            draw_raw_resource_bm(REF_IMG_bmBlankMessageLine,x,y);
            draw_hires_resource_bm(REF_IMG_bmBlankMessageLine,
            										 SCONV_X(x), SCONV_Y(y));
         }
         x += 2;
         y += 1;
      }
      else if (game_paused)
      {
         extern grs_canvas inv_view360_canvas;
         ss_noscale_bitmap(&inv_view360_canvas.bm,x,y);
         x += 2;
         y += 1;
      }
      if (!message_resend && info_text != last_message && strcmp(last_message,info_text) == 0)
      {
         message_resend = TRUE;
         message_clear_time = *tmd_ticks + CIT_CYCLE/10;
         hud_unset(HUD_MSGLINE);
      }
      else
      {
         message_resend = FALSE;
         if (!full_game_3d  && !view360_message_obscured 
            || game_paused)
         {
            gr_set_fcolor(WHITE);
            res_draw_text_shadowed(RES_tinyTechFont,buf,x,y,full_game_3d);
            hud_unset(HUD_MSGLINE);
         }
         else if (full_game_3d || view360_message_obscured)            
         {
            if (buf[0] != '\0')
            {
               hud_set_time(HUD_MSGLINE,5 << APPROX_CIT_CYCLE_SHFT);
            }
         }
      }
      RESTORE_CLIP(a,b,c,d);
      gr_pop_canvas();
      if (is_onscreen()) uiShowMouse(r);
   }
   if (!message_resend && info_text != last_message)
   {
      message_clear_time = *tmd_ticks + MESSAGE_INTERVAL;
      strcpy(last_message,info_text);
    }
   return(OK);
}


bool message_clear_on = TRUE;

errtype message_clear_check()
{
// much as I like spews that print every frame.......
//   Spew(DSRC_GAMESYS_Messages, ("%d >? %d\n",player_struct.game_time,message_clear_time));
   if (*tmd_ticks < message_clear_time) return OK;
   if (message_resend)
   {
      char buf[sizeof(last_message)];
      strcpy(buf,last_message);
      return message_info(buf);
   }
   if (message_clear_on && !full_game_3d && !view360_message_obscured)
   {
      errtype retval = message_info("");
      return retval;
   }
   return(OK);
}

errtype message_box(char *box_text)
{
   message_info(box_text);
   return(OK);
}

#ifdef NOT_YET //��� later, dude

#pragma disable_message(202)
bool confirm_box(char *confirm_text)
{
   return(TRUE);
}
#pragma enable_message(202)

FILE *fopen_gen(char *fname,char *t)
{
   Datapath gen_path;
   FILE *retval;
   char temp[64];

   gen_path.numDatapaths = 0;
   gen_path.noCurrent = 1;
   DatapathAddDir(&gen_path, "gen");
   DatapathAddEnv(&gen_path, "GEN_DIR");
   strcpy(temp, getenv("CITHOME"));
   strcat(temp, "\\gen");
   DatapathAddDir(&gen_path, temp);
   DatapathNoCurrent(&gen_path);
   next_number_dpath_fname(&gen_path,fname);
   retval =  DatapathOpen(&gen_path,fname,t);
   DatapathFree(&gen_path);
   return retval;
}

int open_gen(char *fname,int access1, int access2)
{
   Datapath gen_path;
   int retval;

   gen_path.numDatapaths = 0;
   gen_path.noCurrent = 1;
   DatapathAddDir(&gen_path, "gen");
   DatapathAddEnv(&gen_path, "GEN_DIR");
   DatapathNoCurrent(&gen_path);
   next_number_dpath_fname(&gen_path,fname);
   retval= DatapathFDOpen(&gen_path,fname,access1,access2);
   DatapathFree(&gen_path);
   return retval;
}

char *next_number_dpath_fname(Datapath *dpath, char *fname)
{
   char *subname=strrchr(fname,'0');
   int fhnd, numlen=1, i, num=0;

   if (subname != NULL)
   {
      while ((strlen(subname) != strlen(fname)) && (subname[0] == subname[-1]))
      {
         subname--;
         numlen++;
      }
      // try them, lets go, rock and roll, so on
      while ( (fhnd = DatapathFDOpen(dpath, fname,O_BINARY|O_RDONLY)) != -1)
      {   /* Check next slot */
         close(fhnd);                       /* good idea to, like, close the opened file */
         ++num;
         for (i=0; i<numlen; i++)
            subname[numlen - (i+1)] = '0' + ((num>>(3*i))&7);
      }
      close(fhnd);
   }
   return fname;
}

char *next_number_fname(char *fname)
{
   char *subname = strrchr(fname,'0');
   int fhnd, numlen=1, i, num=0;

   while ((strlen(subname) != strlen(fname)) && (subname[0] == subname[-1]))
   {
      subname--;
      numlen++;
   }
   /* Look for files like uwpic000.gif */
   while ( (fhnd = open(fname,O_BINARY|O_RDONLY)) != -1)
   {  /* Check next slot */
      close(fhnd);                       /* good idea to, like, close the opened file */
      ++num;
      for (i=0; i<numlen; i++)
         subname[numlen - (i+1)] = '0' + ((num>>(3*i))&7);
   }
   close(fhnd);
   return fname;
}

#endif //NOT_YET���

errtype tight_loop(bool check_input)
{
   if (music_on)
      mlimbs_do_ai();

//KLC - does nothing!
//   if (music_on || sfx_on)
//      synchronous_update();

   if (check_input)
   {   
      uiPoll();
      kb_flush_bios();
   }
   return(OK);
}


// --------------------------------------------------
// STRING WRAPPER



int wrap_text(char *ps, short width)
{
   return gr_font_string_wrap(gr_get_font(),ps,width);
}

#define HYPHEN '-'

int hyphenated_wrap_text(char *ps, char* out, short width)
{
	char *psbase;
	char *p;
	char *pmark;
	short numLines;
	short currWidth;

//      Set up to do wrapping

	psbase = ps;                                    // psbase = string beginning
	numLines = 0;                                   // ps = base of current line

//      Do wrapping for each line till hit end

	while (*ps)
		{
		pmark = NULL;                           // no SOFTCR insert LGPoint yet
		currWidth = 0;                          // and zero width so far
		p = ps;

//      Loop thru each word

		while (*p)
			{

//      Skip through to next CR or space or '\0', keeping track of width

			while ((*p != 0) && (*p != '\n') && (*p != ' ') && (*p != CHAR_SOFTSP))
				{
				currWidth += gr_char_width(*p);
				p++;
				}

//      If bypassed width, break out of word loop

			if (currWidth > width || (*p == CHAR_SOFTSP) && currWidth + gr_char_width(HYPHEN) > width)
				{
   				if ((pmark == NULL) && (*p != 0) && (*p != '\n'))
	   				pmark = p;
		   		break;
				}

//      Else set new mark LGPoint (unless eol or eos, then bust out)

			else
		      {
				if ((*p == 0) || (*p == '\n'))  // hit end of line, wipe marker
					{
					pmark = NULL;
					break;
					}
				pmark = p;                            // else advance marker
   		   currWidth += gr_char_width(*p);       // and account for space
				p++;
				}
			}

//      Now insert soft cr if marked one

		if (pmark)
			{
            strncpy(out,ps,pmark-ps);
            out += pmark-ps;
            if (*pmark == CHAR_SOFTSP)
               *out++ = HYPHEN;
            *out++ = CHAR_SOFTCR;
   			ps = pmark + 1;
			if (*ps == ' ')                 // if wrapped and following space,
				ps++;                        // turn into (ignored) soft space
			}

//      Otherwise, bump past cr
		else
			{
            strncpy(out,ps,p-ps+1);
            out += p-ps+1;

			   if (*p)
				   ++p;
			   ps = p;
			}

//      Bump line counter in any case

		++numLines;
		}

//      When hit end of string, return # lines encountered

	return(numLines);
}

void unwrap_text(char *s)
{
	int c;

	while ((c = *s) != 0)
		{
		if ((c == CHAR_SOFTCR) || (c == CHAR_SOFTSP))
			*s = ' ';
		s++;
		}
}


// --------------------------------------------------------------------
// WAIT CURSOR

char wait_count = 0;

errtype begin_wait()
{
   extern LGCursor wait_cursor;
   errtype retval;
   if (wait_count == 0)
   {
      uiHideMouse(NULL);
      retval = uiPushGlobalCursor(&wait_cursor);
      uiShowMouse(NULL);
   }
   wait_count++;
   return(retval);
}

#ifdef NOT_YET //���
errtype spoof_mouse_event(void)
{
   int i;
   uiMouseEvent ev;

   uiMakeMotionEvent(&ev);
   if (ev.buttons == 0) return OK;
   for (i = 0; i < NUM_MOUSE_BTNS; i++)
   {
      if (ev.buttons & (1 << i))
         ev.action |= MOUSE_BTN2DOWN(i);
   }
   ev.type = UI_EVENT_MOUSE;
   return uiQueueEvent((uiEvent*)&ev);
}
#endif //NOT_YET���

errtype end_wait()
{
   errtype retval;
   wait_count--;
   if (wait_count <=0)
   {
      uiHideMouse(NULL);
      retval = uiPopGlobalCursor();
      uiShowMouse(NULL);
      wait_count = 0;
      uiFlush();
//      spoof_mouse_event();
   }
   return(retval);
}


// --------------------------------------------------------------------
// ZOOM BOXES 

#define NUM_BOXES 8
#define TICKS_PER_BOX 10

#define INTERP(s,f,i) (((f)*(i)+(s)*(NUM_BOXES-(i)-1))/(NUM_BOXES-1))

void zoom_rect(LGRect* start, LGRect* end)
{
   extern ulong last_real_time;
   long fillt = gr_get_fill_type();
   ulong start_time = *tmd_ticks;
   ulong last_time  = *tmd_ticks;
   int i;
   uiHideMouse(NULL);
   gr_set_fill_type(FILL_XOR);
   gr_set_fcolor(WHITE);
   for (i = 0; i < NUM_BOXES; i++)
   {
      short ulx = INTERP(start->ul.x,end->ul.x,i);
      short uly = INTERP(start->ul.y,end->ul.y,i);
      short lrx = INTERP(start->lr.x,end->lr.x,i);
      short lry = INTERP(start->lr.y,end->lr.y,i);
      ss_box(ulx,uly,lrx,lry);
      ss_box(ulx-1,uly-1,lrx+1,lry+1);
      while(*tmd_ticks - last_time < TICKS_PER_BOX);
      ss_box(ulx,uly,lrx,lry);
      ss_box(ulx-1,uly-1,lrx+1,lry+1);
      last_time = *tmd_ticks;
//KLC      synchronous_update();
   }
   uiShowMouse(NULL);
   gr_set_fill_type(fillt);
   if (time_passes)
      last_real_time += last_time - start_time;
}



// Returns the angle of difference between look_facing and the true direction
// that looker would have to be facing in order to see target, and puts that
// true direction into real_dir.

// Wow, I really ought to someday make this not use icky trig
// but instead write some fast simple version
fixang point_in_view_arc(fix target_x, fix target_y, fix looker_x, fix looker_y, fixang look_facing, fixang *real_dir)
{
   fix x_diff, y_diff;
   fixang retval;

   x_diff = target_x - looker_x;
   y_diff = target_y - looker_y;
   *real_dir = fix_atan2(y_diff, x_diff);

   // Compensate for difference between our coordinate system and fixpoint's   

   // Hmmm, how do fixangs deal with negatives? 
   // Better normalize to absolute difference, just to be sure
   // After all, they do have *real_dir to figure it out themselves
   // if they want to.
   if (*real_dir > look_facing)
      retval = (*real_dir - look_facing);
   else
      retval = (look_facing - *real_dir);
   if (retval > 0x8000)
      retval = 0x10000 - retval;
   return(retval);
}

// convert occurances of the character "from" to the character "to"
// in the string "s"
void string_replace_char(char* s, char from, char to)
{
   for(;*s;s++) {
      if(*s==from) *s=to;
   };
}

void gamma_dealfunc(ushort gamma_qvar)
{
	fix gamma;
	
	if (gamma_qvar > 99)
		gamma_qvar = 99;
	gamma_qvar = (ushort)(((int)gamma_qvar*FIX_UNIT)/100);
	gamma=FIX_UNIT-fix_make(0,gamma_qvar);
	gamma=fix_mul(gamma,gamma)+(FIX_UNIT/2);
//	gamma=QVAR_TO_GAMMA(gamma_qvar);
	gr_set_gamma_pal(0,256,gamma);
}
