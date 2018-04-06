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

#include "cit2d.h"
#include "gr2ss.h"
#include "frtypes.h"
#include "frintern.h"
#include "frprotox.h"
#include "gamescr.h"
//��#include <inp6d.h>

#ifdef STEREO_SUPPORT
#include <i6dvideo.h>
extern bool inp6d_stereo_active;

#define S_DELTA 5
#endif

#ifdef SVGA_SUPPORT
uchar gr2ss_override = OVERRIDE_NONE;
char convert_type = 0;
char convert_use_mode = 0;

char mode_count[MAX_CONVERT_TYPES];
fix convert_x[MAX_CONVERT_TYPES][MAX_USE_MODES];
fix convert_y[MAX_CONVERT_TYPES][MAX_USE_MODES];
fix inv_convert_x[MAX_CONVERT_TYPES][MAX_USE_MODES];
fix inv_convert_y[MAX_CONVERT_TYPES][MAX_USE_MODES];

#define SVGA_CONV_NONE     0
#define SVGA_CONV_NORMAL   1
#define SVGA_CONV_SCREEN   2

// Internal prototypes
uchar perform_svga_conversion(uchar mask);
void ss_scale_string(char *s, short x, short y);
void mouse_unconstrain(void);


uchar perform_svga_conversion(uchar mask)
{
	extern bool full_game_3d;
	
	if (gr2ss_override & OVERRIDE_FAIL)
		return(SVGA_CONV_NONE);
	//��� for now -   if ((convert_use_mode == 0) && (!inp6d_stereo_active))
	if (convert_use_mode == 0)
		return(SVGA_CONV_NONE);
	if (_fr->draw_canvas.bm.bits == grd_canvas->bm.bits)
		return(SVGA_CONV_SCREEN);
	if (grd_canvas->bm.bits == grd_screen_canvas->bm.bits)
		return(SVGA_CONV_SCREEN);
	if (gr2ss_override & mask)
		return(SVGA_CONV_NORMAL);
	if (convert_use_mode == 3)			// KLC - the normal mode for Mac Shock.
		return(SVGA_CONV_SCREEN);
	return(SVGA_CONV_NONE);
}


// Conversion functions from "abstract" shock 2d functions
// to the actual 2d functions, with appropriate compensations
// for screen mode coordinates and aspect ratios.

// Note, x and y already converted here!
void ss_scale_string(char *s, short x, short y)
{
   // needs to scale still! 
   // know about different fonts instead?  That would be better...
   grs_font *ttfont = (grs_font *)ResLock(RES_tinyTechFont);
   grs_font *mlfont = (grs_font *)ResLock(RES_mediumLEDFont);
   grs_font *f = gr_get_font();
   int c = gr_get_fcolor();
   Id use_font = ID_NULL;
#ifdef STEREO_SUPPORT
   uchar rv = perform_svga_conversion(OVERRIDE_SCALE);
#endif
   if (convert_use_mode == 0)
   {
      gr_string(s,x,y);
      return;
   }
   if ((f == ttfont) || (f == mlfont))
   {
      switch(convert_use_mode)
      {
         case 1:
            if (f == ttfont)
               use_font = RES_tallTinyTechFont;
            break;
         case 2:
         case 3:
            if (f == ttfont)
               use_font = RES_doubleTinyTechFont;
            else if (f == mlfont)
               use_font = RES_doubleMediumLEDFont;
            break;
         case 4:
            if (f == ttfont)
               use_font = RES_megaTinyTechFont;
            else if (f == mlfont)
               use_font = RES_megaMediumLEDFont;
            break;
      }
#ifdef STEREO_SUPPORT
      if ((rv == SVGA_CONV_SCREEN) && inp6d_stereo_active)
      {
         if (use_font == ID_NULL)
         {
            short w,h;
            gr_string_size(s,(short *)&w,(short *)&h);
            gr_push_canvas(i6d_ss->cf_left);
            gr_set_font(f);
            gr_set_fcolor(c);
            gr_scale_string(s, x+S_DELTA, y,SCONV_X(w)+S_DELTA,SCONV_Y(h));
            gr_pop_canvas();
            gr_push_canvas(i6d_ss->cf_right);
            gr_set_font(f);
            gr_set_fcolor(c);
            gr_scale_string(s,x-S_DELTA,y,SCONV_X(w)-S_DELTA,SCONV_Y(h));
         }
         else
         {
            gr_push_canvas(i6d_ss->cf_left);
            gr_set_font((grs_font *)ResLock(use_font));
            gr_set_fcolor(c);
            gr_string(s,x+S_DELTA,y);
            gr_pop_canvas();
            gr_push_canvas(i6d_ss->cf_right);
            gr_set_font((grs_font *)ResLock(use_font));
            gr_set_fcolor(c);
            gr_string(s,x-S_DELTA,y);
         }
         gr_pop_canvas();
      }
      else
      {
#endif
         if (use_font == ID_NULL)
         {
            short w,h;
            gr_string_size(s,(short *)&w,(short *)&h);
            gr_scale_string(s, x, y,SCONV_X(w),SCONV_Y(h));
         }
         else
         {
            gr_set_font((grs_font *)ResLock(use_font));
            gr_string(s, x, y);
         }
#ifdef STEREO_SUPPORT
      }
#endif
      if (use_font != ID_NULL)
         ResUnlock(use_font);
      gr_set_font(ttfont);
      ResUnlock(RES_tinyTechFont);
      ResUnlock(RES_mediumLEDFont);
   }
   else
   {
      // Attempt to scale it
      short w,h;
      gr_string_size(s,(short *)&w,(short *)&h);
      gr_scale_string(s, x, y,SCONV_X(w),SCONV_Y(h));
   }
}

void ss_string(char *s, short x, short y)
{
   uchar rv;
   if (rv = perform_svga_conversion(OVERRIDE_SCALE))
   {
#ifdef STEREO_SUPPORT
      if ((rv == SVGA_CONV_SCREEN) && (inp6d_stereo_active))
      {
         gr_push_canvas(i6d_ss->cf_left);
         if (convert_use_mode)
            ss_scale_string(s,SCONV_X(x)+S_DELTA, SCONV_Y(y));
         else
            ss_scale_string(s,x+S_DELTA,y);
         gr_set_canvas(i6d_ss->cf_right);
         if (convert_use_mode)
            ss_scale_string(s, SCONV_X(x)-S_DELTA, SCONV_Y(y));
         else
            ss_scale_string(s,x-S_DELTA,y);
         gr_pop_canvas();
      }
      else
#endif
         ss_scale_string(s,SCONV_X(x),SCONV_Y(y));
   }
   else
      gr_string(s,x,y);
}

void ss_bitmap(grs_bitmap *bmp, short x, short y)
{
   uchar rv;
   if (rv = perform_svga_conversion(OVERRIDE_SCALE))
   {
#ifdef STEREO_SUPPORT
      if ((rv == SVGA_CONV_SCREEN) && (inp6d_stereo_active))
      {
         gr_push_canvas(i6d_ss->cf_left);
         if (convert_use_mode)
            gr_scale_bitmap(bmp,SCONV_X(x)+S_DELTA,SCONV_Y(y),SCONV_X(bmp->w)+S_DELTA,SCONV_Y(bmp->h));
         else
            gr_bitmap(bmp,x+S_DELTA,y);
         gr_set_canvas(i6d_ss->cf_right);
         if (convert_use_mode)
            gr_scale_bitmap(bmp,SCONV_X(x)-S_DELTA,SCONV_Y(y),SCONV_X(bmp->w)-S_DELTA,SCONV_Y(bmp->h));
         else
            gr_bitmap(bmp,x-S_DELTA,y);
         gr_pop_canvas();
      }
      else
#endif
         gr_scale_bitmap(bmp, SCONV_X(x), SCONV_Y(y), SCONV_X(bmp->w), SCONV_Y(bmp->h));
//      Warning(("scaling %d x %d to %d x %d\n",bmp->w,bmp->h,SCONV_X(bmp->w),SCONV_Y(bmp->h)));
   }
   else
      gr_bitmap(bmp, x, y);
}

void ss_ubitmap(grs_bitmap *bmp, short x, short y)
{
   if (perform_svga_conversion(OVERRIDE_SCALE))
      gr_scale_ubitmap(bmp, SCONV_X(x), SCONV_Y(y), SCONV_X(bmp->w), SCONV_Y(bmp->h));
   else
      gr_ubitmap(bmp, x, y);
}

void ss_noscale_bitmap(grs_bitmap *bmp, short x, short y)
{
   uchar rv;
   if (rv = perform_svga_conversion(OVERRIDE_SCALE))    // ?
#ifdef STEREO_SUPPORT
      if ((rv == SVGA_CONV_SCREEN) && (inp6d_stereo_active))
      {
         gr_push_canvas(i6d_ss->cf_left);
         if (convert_use_mode)
            gr_bitmap(bmp,SCONV_X(x)+S_DELTA,SCONV_Y(y));
         else
            gr_bitmap(bmp,x+S_DELTA,y);
         gr_set_canvas(i6d_ss->cf_right);
         if (convert_use_mode)
            gr_bitmap(bmp,SCONV_X(x)-S_DELTA,SCONV_Y(y));
         else
            gr_bitmap(bmp,x-S_DELTA,y);
         gr_pop_canvas();
      }
      else
#endif
         gr_bitmap(bmp, SCONV_X(x), SCONV_Y(y));
   else
      gr_bitmap(bmp, x, y);
}

void ss_scale_bitmap(grs_bitmap *bmp, short x, short y, short w, short h)
{
   if (perform_svga_conversion(OVERRIDE_SCALE))
      gr_scale_bitmap(bmp, SCONV_X(x), SCONV_Y(y), SCONV_X(w), SCONV_Y(h));
   else
      gr_scale_bitmap(bmp, x, y,w,h);
}

void ss_rect(short x1, short y1, short x2, short y2)
{
   uchar rv;
   if (rv = perform_svga_conversion(OVERRIDE_SCALE))
#ifdef STEREO_SUPPORT
      if ((rv == SVGA_CONV_SCREEN) && (inp6d_stereo_active))
      {
         int c = gr_get_fcolor();
         gr_push_canvas(i6d_ss->cf_left);
         gr_set_fcolor(c);
         if (convert_use_mode)
            gr_rect(SCONV_X(x1)+S_DELTA,SCONV_Y(y1),SCONV_X(x2)+S_DELTA,SCONV_Y(y2));
         else
            gr_rect(x1+S_DELTA,y1,x2+S_DELTA,y2);
         gr_set_canvas(i6d_ss->cf_right);
         gr_set_fcolor(c);
         if (convert_use_mode)
            gr_rect(SCONV_X(x1)-S_DELTA,SCONV_Y(y1),SCONV_X(x2)-S_DELTA,SCONV_Y(y2));
         else
            gr_rect(x1-S_DELTA,y1,x2-S_DELTA,y2);
         gr_pop_canvas();
      }
      else
#endif
         gr_rect(SCONV_X(x1),SCONV_Y(y1),SCONV_X(x2),SCONV_Y(y2));
   else
      gr_rect(x1,y1,x2,y2);
}

void ss_box(short x1, short y1, short x2, short y2)
{
   uchar rv;
   if (rv = perform_svga_conversion(OVERRIDE_SCALE))
#ifdef STEREO_SUPPORT
      if ((rv == SVGA_CONV_SCREEN) && (inp6d_stereo_active))
      {
         int c = gr_get_fcolor();
         gr_push_canvas(i6d_ss->cf_left);
         gr_set_fcolor(c);
         if (convert_use_mode)
            gr_box(SCONV_X(x1)+S_DELTA,SCONV_Y(y1),SCONV_X(x2)+S_DELTA,SCONV_Y(y2));
         else
            gr_box(x1+S_DELTA,y1,x2+S_DELTA,y2);
         gr_set_canvas(i6d_ss->cf_right);
         gr_set_fcolor(c);
         if (convert_use_mode)
            gr_box(SCONV_X(x1)-S_DELTA,SCONV_Y(y1),SCONV_X(x2)-S_DELTA,SCONV_Y(y2));
         else
            gr_box(x1-S_DELTA,y1,x2-S_DELTA,y2);
         gr_pop_canvas();
      }
      else
#endif
         gr_box(RSCONV_X(x1),RSCONV_Y(y1),RSCONV_X(x2),RSCONV_Y(y2));
   else
      gr_box(x1,y1,x2,y2);
}

void ss_safe_set_cliprect(short x1,short y1,short x2,short y2)
{
   if (perform_svga_conversion(OVERRIDE_CLIP))
   {
//      Warning(("setting rect (%d, %d) (%d,%d)!\n",SCONV_X(x1),SCONV_Y(y1),SCONV_X(x2),SCONV_Y(y2)));
      safe_set_cliprect(SCONV_X(x1),SCONV_Y(y1),SCONV_X(x2),SCONV_Y(y2));
   }
   else
      safe_set_cliprect(x1,y1,x2,y2);
}

void ss_cset_cliprect(grs_canvas *pcanv, short x, short y, short w, short h)
{
   if (perform_svga_conversion(OVERRIDE_CLIP))
   {
//      Warning(("cset to %d,%d   %d, %d!\n",SCONV_X(x), SCONV_Y(y), SCONV_X(w), SCONV_Y(h)));
      gr_cset_cliprect(pcanv, SCONV_X(x), SCONV_Y(y), SCONV_X(w), SCONV_Y(h));
   }
   else
      gr_cset_cliprect(pcanv, x, y, w, h);
}

void ss_int_line(short x1,short y1,short x2,short y2)
{
   if (perform_svga_conversion(OVERRIDE_SCALE))
      gr_int_line(SCONV_X(x1),SCONV_Y(y1),SCONV_X(x2),SCONV_Y(y2));
   else
      gr_int_line(x1,y1,x2,y2);
}

void ss_thick_int_line(short x1,short y1,short x2,short y2)
{
   if (perform_svga_conversion(OVERRIDE_SCALE))
   {
      short min_y,max_y,use_y,min_x,max_x,use_x;
      min_y = SCONV_Y(y1);
      max_y = SCONV_Y(y1+1);
      min_x = SCONV_X(x1);
      max_x = SCONV_X(x1+1);
      for (use_y = min_y; use_y < max_y; use_y++)
         gr_int_line(SCONV_X(x1),use_y,SCONV_X(x2),SCONV_Y(y2) + use_y - min_y);
      for (use_x = min_x; use_x < max_x; use_x++)
         gr_int_line(use_x,SCONV_Y(y1),SCONV_X(x2) + use_x - min_x,SCONV_Y(y2));
   }
   else
      gr_int_line(x1,y1,x2,y2);
}

void ss_int_disk(short x1,short y1,short diam)
{
   if (perform_svga_conversion(OVERRIDE_SCALE))
      gr_int_disk(SCONV_X(x1),SCONV_Y(y1),SCONV_X(diam)>>1); 
        // Hm, should we convert rad?
        // Yes, but sadly it's hosed in 320x400 mode, where
        // we need to draw an ellipse.  This stuff really
        // needs to be in the 2D.
   else
      gr_int_disk(x1,y1,diam>>1);
}

void ss_vline(short x1,short y1,short y2)
{
   if (perform_svga_conversion(OVERRIDE_SCALE))
      gr_vline(SCONV_X(x1),SCONV_Y(y1),SCONV_Y(y2));
   else
      gr_vline(x1,y1,y2);
}

void ss_hline(short x1,short y1,short x2)
{
   if (perform_svga_conversion(OVERRIDE_SCALE))
      gr_hline(SCONV_X(x1),SCONV_Y(y1),SCONV_X(x2));
   else
      gr_hline(x1,y1,x2);
}

void ss_fix_line(fix x1, fix y1, fix x2, fix y2)
{
   if (perform_svga_conversion(OVERRIDE_SCALE))
      gr_fix_line(FIXCONV_X(x1),FIXCONV_Y(y1),FIXCONV_X(x2),FIXCONV_Y(y2));
   else
      gr_fix_line(x1,y1,x2,y2);
}

void ss_thick_fix_line(fix x1,fix y1,fix x2,fix y2)
{
   if (perform_svga_conversion(OVERRIDE_SCALE))
   {
      fix min_y,max_y,use_y,min_x,max_x,use_x;
      min_y = FIXCONV_Y(y1);
      max_y = FIXCONV_Y(y1+FIX_UNIT);
      min_x = FIXCONV_X(x1);
      max_x = FIXCONV_X(x1+FIX_UNIT);
      for (use_y = min_y; use_y < max_y; use_y = use_y + FIX_UNIT)
         gr_fix_line(FIXCONV_X(x1),use_y,FIXCONV_X(x2),FIXCONV_Y(y2) + use_y - min_y);
      for (use_x = min_x; use_x < max_x; use_x = use_x + FIX_UNIT)
         gr_fix_line(use_x,FIXCONV_Y(y1),FIXCONV_X(x2) + use_x - min_x,FIXCONV_Y(y2));
   }
   else
      gr_fix_line(x1,y1,x2,y2);
}

void ss_get_bitmap(grs_bitmap *bmp, short x, short y)
{
   if (perform_svga_conversion(OVERRIDE_GET_BM))
      gr_get_bitmap(bmp, SCONV_X(x), SCONV_Y(y));
   else
      gr_get_bitmap(bmp, x, y);
}

void ss_set_pixel(long color, short x, short y)
{
   if (perform_svga_conversion(OVERRIDE_SCALE))
      gr_set_pixel(color, SCONV_X(x), SCONV_Y(y));
   else
      gr_set_pixel(color, x, y);
}

void ss_set_thick_pixel(long color, short x, short y)
{
   if (perform_svga_conversion(OVERRIDE_SCALE))
   {
//      gr_set_pixel(color, SCONV_X(x), SCONV_Y(y));
      gr_set_fcolor(color);
      gr_box(SCONV_X(x),SCONV_Y(y),SCONV_X(x+1)-1,SCONV_Y(y+1)-1);
   }
   else
      gr_set_pixel(color, x, y);
}

void ss_clut_ubitmap(grs_bitmap *bmp, short x, short y, uchar *cl)
{
   if (perform_svga_conversion(OVERRIDE_SCALE))
      gr_clut_scale_ubitmap(bmp, SCONV_X(x), SCONV_Y(y), SCONV_X(bmp->w), SCONV_Y(bmp->h), cl);
   else
      gr_clut_ubitmap(bmp, x, y, cl);
}

// Registration!

// Note that the zeroth element of the conversion arrays
// are used to store the base size, since perform_svga_conversion
// already filters out calls with use_mode of 0.
void gr2ss_register_init(char ctype, short init_x, short init_y)
{
   convert_x[ctype][0] = fix_make(init_x,0);
   convert_y[ctype][0] = fix_make(init_y,0);
}

#define WACKY_FIX_COMPENSATION
void gr2ss_register_mode(char conv_mode, short nx, short ny)
{
   char m;
   mode_count[conv_mode]++;
   m = mode_count[conv_mode];
   convert_x[conv_mode][m] = fix_div(fix_make(nx,0),convert_x[conv_mode][0]);
   convert_y[conv_mode][m] = fix_div(fix_make(ny,0),convert_y[conv_mode][0]);
   inv_convert_x[conv_mode][m] = fix_div(convert_x[conv_mode][0],fix_make(nx,0));
   inv_convert_y[conv_mode][m] = fix_div(convert_y[conv_mode][0],fix_make(ny,0));

#ifdef WACKY_FIX_COMPENSATION
   // wacky fix point compensation!
   if (convert_x[conv_mode][m] & 0xF)
      convert_x[conv_mode][m]++;
   if (convert_y[conv_mode][m] & 0xF)
      convert_y[conv_mode][m]++;
   if (inv_convert_x[conv_mode][m] & 0xF)
      inv_convert_x[conv_mode][m]++;
   if (inv_convert_y[conv_mode][m] & 0xF)
      inv_convert_y[conv_mode][m]++;
#endif
}

void ss_recompute_zoom(frc *which_frc, short oldm)
{
   fr_mod_cams(which_frc,FR_NOCAM,
      fix_div(convert_x[convert_type][convert_use_mode],convert_x[convert_type][oldm]));
}

void ss_point_convert(short *px, short *py, bool down)
{
#ifdef SVGA_SUPPORT
   if (convert_use_mode != 0)
   {
      short ox, oy;
      ox = *px;
      oy = *py;
      if (down)
      {
         *px = INV_SCONV_X(*px);
         *py = INV_SCONV_Y(*py);
      }
      else
      {
         *px = SCONV_X(*px);
         *py = SCONV_Y(*py);
      }
//      Warning(("%d >> %d %d --> %d %d\n",down,ox,oy,*px,*py));
   }
#endif
}

short ss_curr_mode_width(void)
{
   return(SCONV_X(convert_x[convert_type][0]));
}

short ss_curr_mode_height(void)
{
   return(SCONV_Y(convert_y[convert_type][0]));
}

// Basically, if you are in the secret hack mode 5
// then MODE_SCONV_X will act as if you are in mode M
// otherwise it behaves like SCONV_{X,Y}
short MODE_SCONV_X(short cval, short m)
{
   if ((!m) || (convert_use_mode != 5))
      return(SCONV_X(cval));
   return(fast_fix_mul_int(fix_make(cval,0), convert_x[convert_type][m]));
}

short MODE_SCONV_Y(short cval, short m)
{
   if ((!m) || (convert_use_mode != 5))
      return(SCONV_Y(cval));
   return(fast_fix_mul_int(fix_make(cval,0), convert_y[convert_type][m]));
}

// This allows us to override the real mode with some
// fake pretender mode, but only if we are in the magic hack mode 5
short hack_mode_on;
void ss_set_hack_mode(short new_m, short *tval)
{
   if ((convert_use_mode != 5) && (!hack_mode_on))
      return;
   if (new_m)
   {
      *tval = convert_use_mode;
      convert_use_mode = new_m;
      hack_mode_on++;
   }
   else
   {
      convert_use_mode = *tval;
      hack_mode_on--;
   }
}

#endif

void ss_mouse_convert(short *px, short *py, bool down)
{
   if (convert_use_mode != 0)
   {
#ifdef STEREO_SUPPORT
      if (convert_use_mode == 5)
      {
         switch (i6d_device)
         {
            case I6D_CTM:
               return;
               break;
            case I6D_VFX1:
               if (down)
                  *py = fix_int(fix_mul_div(fix_make(*py,0),fix_make(200,0),fix_make(480,0)));
               else
                  *py = fix_int(fix_mul_div(fix_make(*py,0),fix_make(480,0),fix_make(200,0)));
               return;
         }
      }
#endif
      if (down)
      {
         *px = INV_SCONV_X(*px);
         *py = INV_SCONV_Y(*py);
      }
      else
      {
         *px = SCONV_X(*px);
         *py = SCONV_Y(*py);
      }
   }
}

void ss_mouse_convert_round(short *px, short *py, bool down)
{
   short ox,oy;

   if (convert_use_mode != 0)
   {
#ifdef STEREO_SUPPORT
      if (convert_use_mode == 5)
      {
         ss_mouse_convert(px,py,down);
         return;
      }
#endif
      ox = *px;
      oy = *py;
      if (down)
      {
         *px = fix_int(INV_FIXCONV_X(fix_make(*px,0x8000)));
         *py = fix_int(INV_FIXCONV_Y(fix_make(*py,0x8000)));
      }
      else
      {
         *px = fix_int(FIXCONV_X(fix_make(*px,0x8000)));
         *py = fix_int(FIXCONV_Y(fix_make(*py,0x8000)));
      }
   }
}

void mouse_unconstrain(void)
{
/* ��� for now
#ifdef SVGA_SUPPORT
   if (convert_use_mode == 5)
   {
      switch (i6d_device)
      {
         case I6D_CTM:
            mouse_constrain_xy(0,0,grd_cap->w-1,grd_cap->h-1);
            break;
         case I6D_VFX1:
            mouse_constrain_xy(0,0,i6d_ss->scr_w >> 1, i6d_ss->scr_h);
            break;
      }
   }
   else
#endif  */
      mouse_constrain_xy(0,0,grd_cap->w-1,grd_cap->h-1);
}
