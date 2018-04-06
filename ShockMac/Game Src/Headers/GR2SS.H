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
#include "frprotox.h"

#ifdef SVGA_SUPPORT
extern void ss_string(char *s, short x, short y);
extern void ss_bitmap(grs_bitmap *bmp, short x, short y);
extern void ss_ubitmap(grs_bitmap *bmp, short x, short y);
extern void ss_scale_bitmap(grs_bitmap *bmp, short x, short y, short w, short h);
extern void ss_noscale_bitmap(grs_bitmap *bmp, short x, short y);
extern void ss_rect(short x1, short y1, short x2, short y2);
extern void ss_box(short x1, short y1, short x2, short y2);
extern void ss_int_line(short x1, short y1,short x2,short y2);
extern void ss_thick_int_line(short x1, short y1,short x2,short y2);
extern void ss_int_disk(short x1, short y1,short rad);
extern void ss_safe_set_cliprect(short x1,short y1,short x2,short y2);
extern void ss_cset_cliprect(grs_canvas *pcanv, short x1,short y1,short x2,short y2);
extern void ss_vline(short x1,short y1,short y2);
extern void ss_hline(short x1,short y1,short y2);
extern void ss_fix_line(fix x1, fix y1, fix x2, fix y2);
extern void ss_thick_fix_line(fix x1, fix y1, fix x2, fix y2);
extern void ss_get_bitmap(grs_bitmap *bmp, short x, short y);
extern void ss_set_pixel(long color, short x, short y);
extern void ss_set_thick_pixel(long color, short x, short y);
extern void ss_clut_ubitmap(grs_bitmap *bmp, short x, short y, uchar *cl);
extern void ss_recompute_zoom(frc *w, short oldm);
extern void ss_mouse_convert(short *px, short *py, bool down);
extern void ss_mouse_convert_round(short *px, short *py, bool down);
extern void ss_point_convert(short *px, short *py, bool down);

extern void gr2ss_register_init(char convert_type, short init_x, short init_y);
extern void gr2ss_register_mode(char conv_mode, short nx, short ny);

extern short ss_curr_mode_width(void);
extern short ss_curr_mode_height(void);
extern void ss_set_hack_mode(short new_m, short *tval);

#define MAX_CONVERT_TYPES  4
#define MAX_USE_MODES      8

extern fix convert_x[MAX_CONVERT_TYPES][MAX_USE_MODES];
extern fix convert_y[MAX_CONVERT_TYPES][MAX_USE_MODES];
extern fix inv_convert_x[MAX_CONVERT_TYPES][MAX_USE_MODES];
extern fix inv_convert_y[MAX_CONVERT_TYPES][MAX_USE_MODES];

extern char convert_type;
extern char convert_use_mode;

extern short MODE_SCONV_X(short cval, short m);
extern short MODE_SCONV_Y(short cval, short m);

#define SCONV_X(x)   fast_fix_mul_int(fix_make((x),0), convert_x[convert_type][convert_use_mode])
#define SCONV_Y(y)   fast_fix_mul_int(fix_make((y),0), convert_y[convert_type][convert_use_mode])
#define RSCONV_X(x)   fix_int(0x8000 + fast_fix_mul(fix_make((x),0), convert_x[convert_type][convert_use_mode]))
#define RSCONV_Y(y)   fix_int(0x8000 + fast_fix_mul(fix_make((y),0), convert_y[convert_type][convert_use_mode]))

#define INV_SCONV_X(x)   fast_fix_mul_int(fix_make((x),0), inv_convert_x[convert_type][convert_use_mode])
#define INV_SCONV_Y(y)   fast_fix_mul_int(fix_make((y),0), inv_convert_y[convert_type][convert_use_mode])

#define FIXCONV_X(x)   fast_fix_mul((x), convert_x[convert_type][convert_use_mode])
#define FIXCONV_Y(y)   fast_fix_mul((y), convert_y[convert_type][convert_use_mode])
#define INV_FIXCONV_X(x)   fast_fix_mul((x), inv_convert_x[convert_type][convert_use_mode])
#define INV_FIXCONV_Y(y)   fast_fix_mul((y), inv_convert_y[convert_type][convert_use_mode])

extern uchar gr2ss_override;

#define OVERRIDE_NONE   0x00
#define OVERRIDE_SCALE  0x01
#define OVERRIDE_FONT   0x02
#define OVERRIDE_CLIP   0x04
#define OVERRIDE_GET_BM 0x10   
#define OVERRIDE_ALL    0x7F
#define OVERRIDE_FAIL   0x80


#else

#define ss_string(s,x,y) gr_string(s,x,y)
#define ss_bitmap(bmp,x,y) gr_bitmap(bmp,x,y)
#define ss_ubitmap(bmp,x,y) gr_ubitmap(bmp,x,y)
#define ss_noscale_bitmap(bmp,x,y) gr_bitmap(bmp,x,y)
#define ss_scale_bitmap(bmp,x,y,w,h) gr_scale_bitmap(bmp,x,y,w,h)
#define ss_rect(x1,y1,x2,y2) gr_rect(x1,y1,x2,y2)
#define ss_box(x1,y1,x2,y2) gr_box(x1,y1,x2,y2)
#define ss_int_line(x1,y1,x2,y2) gr_int_line(x1,y1,x2,y2)
#define ss_thick_int_line(x1,y1,x2,y2) gr_int_line(x1,y1,x2,y2)
#define ss_int_disk(x1,y1,rad) gr_int_disk(x1,y1,rad)
#define ss_safe_set_cliprect(x1,y1,x2,y2) gr_safe_set_cliprect(x1,y1,x2,y2)
#define ss_cset_cliprect(pcanv,x1,y1,x2,y2) gr_cset_cliprect(pcanv, x1,y1,x2,y2)
#define ss_vline(x1,y1,y2) gr_vline(x1,y1,y2)
#define ss_hline(x1,y1,y2) gr_hline(x1,y1,y2)
#define ss_fix_line(x1,y1,x2,y2) gr_fix_line(x1,y1,x2,y2)
#define ss_thick_fix_line(x1,y1,x2,y2) gr_fix_line(x1,y1,x2,y2)
#define ss_get_bitmap(bmp,x,y) gr_get_bitmap(bmp,x,y)
#define ss_set_pixel(color,x,y) gr_set_pixel(color,x,y)
#define ss_set_thick_pixel(color,x,y) gr_set_pixel(color,x,y)
#define ss_clut_ubitmap(bmp,x,y,cl) gr_clut_ubitmap(bmp,x,y,cl)
#define ss_recompute_zoom(w,oldm) 

#define gr2ss_register_init(convert_type,init_x,init_y)
#define gr2ss_register_mode(conv_mode,nx,ny)

extern void ss_mouse_convert(short *px, short *py, bool down);
extern void ss_mouse_convert_round(short *px, short *py, bool down);

#define SCONV_X(x)   x
#define SCONV_Y(y)   y
#define INV_SCONV_X(x)   x
#define INV_SCONV_Y(y)   y

#define FIXCONV_X(x)   x
#define FIXCONV_Y(y)   y
#define INV_FIXCONV_X(x)   x
#define INV_FIXCONV_Y(y)   y

#define MODE_SCONV_X(x,m)  x
#define MODE_SCONV_Y(y,m)  y
#endif

