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
 * 2d font goofiness
 */

// things to do
//  prefix everything with bounds checking, note if _fnt_count=-1

//#define Malloc malloc

#include <string.h>  // for fake load
#include <stdlib.h>
//#include <fcntl.h>
//#include <io.h>

//#include <malloc.h>

//#include <mprintf.h>

#include <texttool.h>
#include <fakefont.h>
#include <2d.h>

struct _fnt_data {
   uchar wids[FNT_MAX_CHARS];           /* width of each character */
   uchar height;                        /* height of the font set */
   uchar type;                          /* mono, color, spec, whatever */
   uchar f_color, b_color;              /* foreground/background color */
   char *font_buf;
};

static struct _fnt_data *_fnt_lists;    /* the current lists */
static int _fnt_current, _fnt_count=-1; /* current font, total number of fonts */

#define FntCurr (_fnt_lists[_fnt_current])    /* the structure of current font */
int fnt_cursor(int x, int y);

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
 { x+=y; return 0; }

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

// 
bool fnt_free(void)
{
   if (_fnt_count==-1) return FALSE;
   free(_fnt_lists);
   _fnt_count=-1;
   return TRUE;
}

/*
// return the handle of the loaded font or -1
int fnt_load(char *fnt_fname)
{
   int hnd=_fnt_count;                 // our handle is the current max
   int fd;
   long size;

   if ((_fnt_lists=(struct _fnt_data *)realloc(_fnt_lists,sizeof(struct _fnt_data)*++_fnt_count))==NULL)
      return -1;

   // Load the font data from disk
   
   fd = open(fnt_fname, O_RDONLY|O_BINARY);
   size = lseek(fd, 0, SEEK_END);
   lseek(fd, 0, SEEK_SET);
   _fnt_lists[hnd].font_buf = (char *)NewPtr((size + 1) * sizeof(char));
   read(fd, _fnt_lists[hnd].font_buf, size);
   close(fd);
   gr_set_font((grs_font *)_fnt_lists[hnd].font_buf);
   mprintf ("Made it to here (after font load), at least!\n");

   // for now, we do mostly zany things 
   {
      // Must get real info here from Jon
      int base_s=strlen(fnt_fname),i;
      if (base_s<2) base_s=2;

      for (i=0; i<FNT_MAX_CHARS; i++)
         _fnt_lists[hnd].wids[i]=gr_char_width(i);
      _fnt_lists[hnd].wids[' ']=gr_char_width(' ');

      _fnt_lists[hnd].height=base_s;

      // ?? Do something real here...
      _fnt_lists[hnd].f_color=56+(rand()&15);
      _fnt_lists[hnd].b_color=0x03;
      _fnt_lists[hnd].type=0;
   }
   return hnd;
}

// unload the specified font
bool fnt_unload(int fnt_handle)
{
   if (fnt_handle>=_fnt_count) return FALSE;
   if (fnt_handle!=_fnt_count-1)
      memcpy(&_fnt_lists[fnt_handle],&_fnt_lists[_fnt_count-1],sizeof(struct _fnt_data));
   if (fnt_handle==_fnt_current)       // should check here for not deleting default font
      _fnt_current=0;
   _fnt_count--;
   return TRUE;
}
*/

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
