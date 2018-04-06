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
#ifndef __FRPROTOX_H
#define __FRPROTOX_H
/*
 * $Source: r:/prj/cit/src/inc/RCS/frprotox.h $
 * $Revision: 1.11 $
 * $Author: xemu $
 * $Date: 1994/08/05 03:06:45 $
 *
 * Citadel Renderer
 *  global external prototypes for the renderer
 *
 * $Log: frprotox.h $
 * Revision 1.11  1994/08/05  03:06:45  xemu
 * look, fr_get_at with transparency
 * 
 * Revision 1.10  1994/04/23  09:56:24  xemu
 * new params1
 * 
 * Revision 1.9  1994/04/14  15:00:33  kevin
 * New detail stuff.
 * 
 * Revision 1.8  1994/03/13  17:18:33  dc
 * more fields for obj_block
 * 
 * Revision 1.7  1994/03/03  12:18:27  dc
 * place view takes a canvas now
 * 
 * Revision 1.6  1994/02/13  05:48:00  dc
 * rend_start
 * 
 * Revision 1.5  1994/01/02  17:16:34  dc
 * Initial revision
 * 
 */

#ifndef __FRTYPESX_H
typedef void frc;
typedef void fmp;
#endif

//======== Basic truths
#define FR_OK         (1)
#define FR_BAD_VIEW  (-1)
#define FR_NOMEM     (-2)
#define FR_NULL_PTR  (-3)
#define FR_NO_NEED   (-4)

//======== Random prettiness
#define FR_NOCAM     ((void *)(-1))
#define FR_DEFCAM    (NULL)
#define FR_DEFVIEW   (NULL)
#define FR_NEWVIEW   (NULL)

//======== From frsetup.c
// global initialization
void fr_startup (void);
void fr_shutdown(void);
void fr_closedown(void);

// view control/setup
frc    *fr_place_view (frc *view, void *cam, void *canvas, int pflags, char axis, int fov, int xc, int yc, int wid, int hgt);
void    fr_use_global_detail (frc *view);
int     fr_view_resize(frc *view, int wid, int hgt);
int     fr_view_full(frc *view, int wid, int hgt);
int     fr_mod_size (frc *view, int xc, int yc, int wid, int hgt);
int     fr_mod_cams (frc *view, void *cam, int mod_fac);
int     fr_context_mod_flag (frc *view, int pflags_on, int pflags_off);   // remember to set flags_off for things you turn on
int     fr_global_mod_flag  (int flags_on, int flags_off);
void   *fr_get_canvas (frc *view);                                // really returns a grs_canvas, but no want 2d.h
int     fr_set_view (frc *view);
int     fr_free_view (frc *view);
void    fr_set_cluts(uchar *base, uchar *bwclut, uchar *greenclut, uchar *amberclut);
int    fr_set_callbacks(frc *view, 
								int (*draw)(void *dstc, void *dstbm, int x, int y, int flg),
                     				void (*horizon)(void *dstbm, int flg), 
                     				void (*render)(void *dstbm, int flg));
int    fr_set_global_callbacks( int (*draw)(void *dstc, void *dstbm, int x, int y, int flg),
                     							void (*horizon)(void *dstbm, int flg), 
              								void (*render)(void *dstbm, int flg) );

//======== From frcompil.c
void    fr_compile_rect(fmp *fm, int llx, int lly, int ulx, int uly, bool seen_bits);
void    fr_compile_restart(fmp *fm);

//======== From frmain.c
int     fr_rend(frc *view);
ushort  fr_get_at(frc *view, int x, int y,bool transp);

//======== From frutil.c
char   *fr_get_frame_rate(void);


//======== Externals to provide, initialized to dumb things
#ifndef __FRSETUP_SRC
extern int   _fr_default_detail;
extern int   _fr_global_detail;
extern void   (*fr_mouse_hide)(void), (*fr_mouse_show)(void);
extern int   (*fr_get_idx)(void);
extern bool  (*fr_obj_block)(void *mptr, uchar *_sclip, int *loc);
extern void  (*fr_clip_start)(bool headnorth);
extern void  (*fr_rend_start)(void);
#ifdef __2D_H
extern grs_bitmap *(*fr_get_tmap)(void);
#endif
#endif

// default versions of above, defined in frsetup and set there
void fr_default_mouse(void);
int   fr_default_idx(void), fr_pickup_idx(void);
bool  fr_default_block(void *mptr, uchar *_sclip, int *loc);
void  fr_default_clip_start(bool headnorth);
void  fr_default_rend_start(void);
#ifdef __2D_H
grs_bitmap *fr_default_tmap(void);
#endif

#endif // __FRPROTOX_H
