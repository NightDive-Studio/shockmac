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
 * 2d font goofiness, using the Resource System and stylez....
 */

// things to do
//  prefix everything with bounds checking, note if _fnt_count=-1

#include <string.h>  // for fake load
#include <stdlib.h>
//#include <fcntl.h>
//#include <io.h>

//#include <malloc.h>

//#include <mprintf.h>

#include "texttool.h"
#include "fakefont.h"
#include "tngfont.h"
#include "2d.h"
#include "tng.h"

struct _fnt_data {
   uchar wids[FNT_MAX_CHARS];           /* width of each character */
   uchar height;                        /* height of the font set */
   uchar type;                          /* mono, color, spec, whatever */
   uchar f_color, b_color;              /* foreground/background color */
//   char *font_buf;
   long *font_buf;
};

static struct _fnt_data *_fnt_lists;    /* the current lists */
static int _fnt_current, _fnt_count=-1; /* current font, total number of fonts */

TTFontInfo TTTNGFontInfo = 
       {fnt_s_width, fnt_s_draw, fnt_c_width, fnt_c_draw, fnt_s_clr, fnt_l_clr, fnt_c_clr, fnt_cursor, 7, 7, TTFI_PROP|TTFI_SPACE };

#define FntCurr (_fnt_lists[_fnt_current])    /* the structure of current font */

/* rendering and sizing primitives */
// returns the width of the character w/o spacing
int fnt_c_draw(char c, int xpos, int ypos)
{
//   mprintf("C_draw %x at %d %d, wid %d\n",c,xpos,ypos,FntCurr.wids[c]);
   gr_set_font((grs_font *)_fnt_lists[_fnt_current].font_buf);
   gr_set_fcolor(FntCurr.f_color);
   gr_char(c,xpos,ypos);
//   return FntCurr.wids[c];
   return gr_char_width(c);
}

// return the width of the whole string including internal spacing
int fnt_s_draw(char *s, int xpos, int ypos)
{
   gr_set_fcolor(FntCurr.f_color);
   gr_set_font((grs_font *)_fnt_lists[_fnt_current].font_buf);
   gr_string(s,xpos,ypos);
//   return FntCurr.wids[c];
   return gr_string_width(s);
}

// return the c width
int fnt_c_width(char c)
{
   gr_set_font((grs_font *)_fnt_lists[_fnt_current].font_buf);
   return gr_char_width(c);
}

// return the s width
int fnt_s_width(char *s)
{
   gr_set_font((grs_font *)_fnt_lists[_fnt_current].font_buf);
   return gr_string_width(s);
}

// clear functions
int fnt_l_clr(int len, int xpos, int ypos)
{
   gr_set_fcolor(FntCurr.b_color);
   gr_rect(xpos,ypos,xpos+len,ypos+FntCurr.height);
   return len;
}
int fnt_s_clr(char *s, int xpos, int ypos)
 { fnt_l_clr(fnt_s_width(s),xpos,ypos); return (fnt_s_width(s)); }
int fnt_c_clr(char c, int xpos, int ypos)
 { fnt_l_clr(fnt_c_width(c),xpos,ypos); return (fnt_c_width(c)); }

// cursor bar
int fnt_cursor(int x, int y)
{
   int dummy;  dummy = x + y;
//   mprintf ("Cursor at (%d, %d)\n",x,y);
//   gr_vline(x, y, y + FntCurr.height);
   return(0);
}

/* load/system primitives */
// 
int fnt_init(char *def_fname)
{
   _fnt_lists=(struct _fnt_data *)malloc(1);
   _fnt_count=0;
   _fnt_current=0;
   if (def_fname!=NULL)
   {
      int hnd=fnt_load(def_fname);
      if (hnd!=-1)
       { fnt_select(hnd); return hnd; }
   }
   return 0;
}

int fnt_init_from_style(void *style_ptr)
{
   if (_fnt_count == -1)
   {
      _fnt_lists=(struct _fnt_data *)malloc(1);
      _fnt_count=0;
      _fnt_current=0;
   }
   if (style_ptr != NULL)
   {
      int hnd=fnt_load_from_style(style_ptr);
      if (hnd!=-1)
       { fnt_select(hnd); return hnd; }
   }
   return 0;
}

// 
bool fnt_free(void)
{
   if (_fnt_count==-1) return FALSE;
   free(_fnt_lists);
   _fnt_count=-1;
   return TRUE;
}

// return the handle of the loaded font or -1
int fnt_load(char * /*fnt_fname*/)
{
//   mprintf ("AGH!  NO!  USE STYLES WITH THIS FONT CODE (%s)!!\n",fnt_fname);
   return 0;
}

// return the handle of the loaded font or -1
int fnt_load_from_style(void *style_ptr)
{
   int hnd=_fnt_count;                 /* our handle is the current max */
   TNGStyle *psty;
   grs_font *f;
   int i;

   if ((_fnt_lists=(struct _fnt_data *)realloc(_fnt_lists,sizeof(struct _fnt_data)*++_fnt_count))==NULL)
      return -1;

   // Load the font data from disk
   psty = (TNGStyle *)style_ptr;
//   _fnt_lists[hnd].font_buf = (char *)Malloc((ResSize(psty->font) + 1) * sizeof(char));
   _fnt_lists[hnd].font_buf = (long *)ResLock(psty->font);
   gr_set_font((grs_font *)_fnt_lists[hnd].font_buf);
   f = (grs_font *)gr_get_font();

   // for now, we do mostly zany things 
   for (i=0; i<FNT_MAX_CHARS; i++)
      _fnt_lists[hnd].wids[i]=gr_char_width(i);
   _fnt_lists[hnd].wids[' ']=gr_char_width(' ');
   _fnt_lists[hnd].height=f->h;

   _fnt_lists[hnd].f_color = psty->textColor;
   _fnt_lists[hnd].b_color = psty->backColor;
   _fnt_lists[hnd].type=TTFI_PROP | TTFI_SPACE;

   return hnd;
}

// unload the specified font
bool fnt_unload(int fnt_handle)
{
   if (fnt_handle>=_fnt_count) return FALSE;
   if (fnt_handle!=_fnt_count-1)
      LG_memcpy(&_fnt_lists[fnt_handle],&_fnt_lists[_fnt_count-1],sizeof(struct _fnt_data));
   if (fnt_handle==_fnt_current)       /* should check here for not deleting default font */
      _fnt_current=0;
   _fnt_count--;
   return TRUE;
}

// set the current font
bool fnt_select(int fnt_handle)
{
   if (fnt_handle>=_fnt_count) return FALSE;
   _fnt_current=fnt_handle;
   return TRUE;
}

#ifdef nOAG
TTFontInfo *build_font_TTFI(TextToolFontInfo *rect_font)
{
   if (rect_font==NULL) rect_font=(TextToolFontInfo *)Malloc(sizeof(TextToolFontInfo));
   rect_font->s_draw=fnt_s_draw; rect_font->s_wid=fnt_s_width;
   rect_font->c_draw=fnt_c_draw; rect_font->c_wid=fnt_c_width;
   rect_font->height=FntCurr.height; rect_font->base_w=FntCurr.wids[' ']+1; rect_font->type=TTFI_PROP|TTFI_SPACE;
   return rect_font;
}
#endif

// do not pass go
// do not collect
// your dignity
// and your self respect
//   "I am your clock", Lard, _The Last Temptation of Reid_
