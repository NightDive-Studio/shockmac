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
 * $Source: r:/prj/cit/src/RCS/popups.c $
 * $Revision: 1.9 $
 * $Author: xemu $
 * $Date: 1994/10/31 06:31:46 $
 *
 */

#include <string.h>
#include <TextUtils.h>

#include "popups.h"
#include "citres.h"
#include "criterr.h"
#include "gamestrn.h"

#include "gamescr.h"
#include "cybstrng.h"
#include "colors.h"
#include "cit2d.h"
#include "gr2ss.h"

#define BUF_SIZ 80
#define EMAIL_CURS_WID 40
#define EMAIL_CURS_MARG 5
#define EMAIL_CURS_FONT RES_tinyTechFont

bool 				popup_cursors = TRUE;
grs_bitmap	popup_bitmaps[NUM_POPUPS];
LGRect			popup_rects[NUM_POPUPS];
LGPoint			popup_hotspots[NUM_POPUPS] = 	// in 0-16 dimension-independent units
{
   { 0,8},
   { 16,8},
   { 8,16},   
   { 0,8},
   { 16,8},
};

#define CURSOR_TEXT_COLOR 0x36

void init_popups(void)
{
	for (int i = 0; i < NUM_POPUPS; i++)
	{
		Ref id = MKREF(RES_popups,i);
		FrameDesc* f = (FrameDesc *)RefGet(id);
		popup_rects[i] = f->anchorArea;
		if (load_res_bitmap(&popup_bitmaps[i],id,TRUE) != OK)
			critical_error(CRITERR_RES|0xA);
	}
	ResUnlock(RES_popups);
	ResDrop(RES_popups);
}


void make_popup_cursor(LGCursor* c, grs_bitmap* bm, char* s, uint tmplt,bool allocate, LGPoint offset)
{
	LGRect* r = &popup_rects[tmplt];
	grs_bitmap* pbm = &popup_bitmaps[tmplt];
	grs_canvas gc;
	short x,y,w,h;
	LGPoint p,ph;
	uchar old_over = gr2ss_override;
	uchar *bptr;
	uchar* bits = bm->bits;
	extern void ss_scale_string(char *s, short x, short y);
	
	MouseLock++;
	*bm = *pbm;
		
	if (allocate)
	{
		bptr = (uchar *)NewPtr(bm->w*bm->h);
		if (bptr == NULL)
			critical_error(CRITERR_MEM|4);
	}
	else
	{
		bptr = bits;
	}
	gr_init_bm(bm,bptr,BMT_FLAT8,BMF_TRANS,bm->w,bm->h);
	gr_make_canvas(bm,&gc);
	gr_push_canvas(&gc);
	gr_clear(0);
	gr_bitmap(pbm,0,0);
	gr_set_font((grs_font*)ResLock(RES_tinyTechFont));
	gr_string_size(s,&w,&h);
	ss_point_convert(&w, &h, FALSE);
	x = (r->ul.x + r->lr.x - w)/2 + offset.x;
	y = (r->ul.y + r->lr.y - h)/2 + offset.y;
	ss_point_convert(&x, &y, TRUE);
	gr_set_fcolor(CURSOR_TEXT_COLOR);
//	ss_string(s,x,y+1);
	ss_scale_string(s, SCONV_X(x), SCONV_Y(y)+1);
	ResUnlock(RES_tinyTechFont);
	gr_pop_canvas();
	ph = popup_hotspots[tmplt];
	p.x = (bm->w*ph.x)>>4;
	p.y = (bm->h*ph.y)>>4;
	uiMakeBitmapCursor(c,bm,p);
		
	MouseLock--;
}

void load_string_array(Ref first, char* arry[], char buf[], int bufsz, int n)
{
   int off = 0;
   int i;
   for (i = 0; i < n; i++)
   {
      short sz = bufsz - off;
      get_string(first+i,buf+off,sz);
      arry[i] = buf + off;
      off += strlen(buf+off) +1;
   }

}

#ifdef SVGA_SUPPORT
static char cursor_buf[4096];
#else
static char cursor_buf[512];
#endif

void make_email_cursor(LGCursor* c, grs_bitmap* bm, uchar page, bool init)
{
   grs_canvas gc;
   short x,y,w,h;
   int len;
   LGPoint p;
   char s[BUF_SIZ];
#ifdef SVGA_SUPPORT
   short temp;
   uchar old_over = gr2ss_override;
   gr2ss_override = OVERRIDE_ALL;
   ss_set_hack_mode(2,&temp);
#endif

   gr_font_char_size((grs_font*)ResGet(EMAIL_CURS_FONT),'X',&w,&h);
   h+=2;
   w=EMAIL_CURS_WID;
#ifdef SVGA_SUPPORT
   ss_point_convert(&w,&h,FALSE);
#endif
   if(init)
      gr_init_bm(bm,NULL,BMT_FLAT8,BMF_TRANS,w,h);
   get_string(REF_STR_WordPage,s,BUF_SIZ);
   len=strlen(s);
   s[len]=' ';
//   itoa(page,s+len+1,10);
   numtostring(page, s+len+1);
   MouseLock++;
   if (sizeof(cursor_buf) < w*h)
      critical_error(CRITERR_MEM|7);
   bm->bits = (uchar *)cursor_buf;
   gr_make_canvas(bm,&gc);
   gr_push_canvas(&gc);
   gr_clear(0);
   gr_set_font((grs_font*)ResLock(EMAIL_CURS_FONT));
   gr_string_size("Page goof",&w,&h);
   x = EMAIL_CURS_MARG;
   y = 1;
   gr_set_fcolor(CURSOR_TEXT_COLOR);
   draw_shadowed_string(s,x,y,TRUE);
   ResUnlock(RES_tinyTechFont);
   gr_set_fcolor(BLACK+1);
   ss_vline(1,h/2-1,h/2+1);
   ss_hline(0,h/2,2);
   ss_set_pixel(CURSOR_TEXT_COLOR,1,h/2);
   gr_pop_canvas();
   p.x = 1;
   p.y = h/2;
   uiMakeBitmapCursor(c,bm,p);
   MouseLock--;
#ifdef SVGA_SUPPORT
   ss_set_hack_mode(0,&temp);
   gr2ss_override = old_over;
#endif
}

    
