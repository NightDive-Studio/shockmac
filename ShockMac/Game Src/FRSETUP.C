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
 * FrSetup.c
 *
 * $Source: r:/prj/cit/src/RCS/frsetup.c $
 * $Revision: 1.34 $
 * $Author: dc $
 * $Date: 1994/11/25 16:57:55 $
 *
 * Citadel Renderer
 *  setup and modification calls for the view
 *  
 * $Log: frsetup.c $
 * Revision 1.34  1994/11/25  16:57:55  dc
 * force remake due to 2d.h change
 * 
 * Revision 1.33  1994/11/22  16:52:37  dc
 * setvmode 23 hack camera taking over stereo views
 * 
 * Revision 1.32  1994/11/21  09:02:30  dc
 * new olh stuff
 * 
 * Revision 1.31  1994/11/04  13:08:34  xemu
 * aspect ratio fix for VFX
 * 
 * Revision 1.30  1994/10/27  04:58:16  xemu
 * stero
 * 
 * Revision 1.29  1994/09/06  18:40:01  xemu
 * moved fr_closedown
 * 
 * Revision 1.28  1994/09/06  12:04:55  mahk
 * game closedown for renderer.
 * 
 * Revision 1.27  1994/09/06  05:03:27  jaemz
 * Added star support
 * 
 * Revision 1.26  1994/08/28  16:54:35  kevin
 * got rid of old no longer functional mapper switching routines.
 * replaced with new 3d mapper switching routines.
 * 
 * Revision 1.25  1994/08/27  02:13:45  kevin
 * Min detail level uses clut lighting always.
 * 
 * Revision 1.24  1994/08/16  07:21:37  dc
 * new terrain function
 * 
 * Revision 1.23  1994/08/09  20:09:32  tjs
 * Oops.
 * 
 * Revision 1.22  1994/08/09  02:45:20  tjs
 * End of view radius no longer same as tmap zero.
 * 
 * Revision 1.21  1994/08/04  23:49:18  dc
 * tweak some parameters in view structure
 * 
 * Revision 1.20  1994/07/29  18:11:27  roadkill
 * *** empty log message ***
 * 
 * Revision 1.19  1994/07/20  23:05:41  dc
 * odrop...
 * 
 * Revision 1.18  1994/07/14  01:59:56  xemu
 * shaking only half as extreme
 * 
 * Revision 1.17  1994/06/02  23:34:27  kevin
 * Changed min detail level to use 1d walls when applicable.
 * 
 * Revision 1.16  1994/05/25  21:36:14  xemu
 * default to max detail
 * 
 * Revision 1.15  1994/05/19  14:51:12  kevin
 * frsetup now initializes terrian rendering function pointers to
 * take advantage of wall and floor special cases when possible.
 * 
 * Revision 1.14  1994/05/11  21:19:21  xemu
 * got rid of random setting
 * 
 * Revision 1.13  1994/05/09  06:00:51  dc
 * bits for cnvs in place_view, fix global flags stuff...
 * 
 * Revision 1.12  1994/04/23  10:19:47  dc
 * ship NO_FAKE_TMAPS fix...
 * 
 * Revision 1.11  1994/04/23  08:12:13  dc
 * hack for vcolors...
 * 
 * Revision 1.10  1994/04/14  20:16:45  kevin
 * use zoom hack only with doubling.
 * 
 * Revision 1.9  1994/04/14  15:01:28  kevin
 * New detail stuff.
 * 
 * Revision 1.8  1994/03/20  21:14:40  xemu
 * hack cameras
 * 
 * Revision 1.7  1994/03/13  17:18:56  dc
 * obj_block, /#def for blender...
 * 
 * Revision 1.6  1994/03/08  03:27:29  dc
 * free the fake tmaps if they exist....
 * 
 * Revision 1.5  1994/03/03  12:18:11  dc
 * pass in canvas to place view
 * 
 */

#include <stdlib.h>  // I HATE THIS



#define __FRSETUP_SRC
#include "frcamera.h"
#include "fr3d.h"
#include "frtypes.h"
#include "frintern.h"
#include "frshipm.h"
#include "frflags.h"
#include "frparams.h"
#include "player.h"
#include "FrUtils.h"
#include "fullscrn.h"
#include "star.h"

#ifdef STEREO_SUPPORT
#include <inp6d.h>
#include <i6dvideo.h>
#endif

// Internal Prototypes
void fr_tfunc_grab_start(void);
void fr_set_default_ptrs(void);
void _fr_update_context(int det);
void _fr_change_detail(int det);

// Globals
void  (*fr_mouse_hide)(void), (*fr_mouse_show)(void);
int   (*fr_get_idx)(void);
bool  (*fr_obj_block)(void *vmptr, uchar *_sclip, int *loc);
void  (*fr_clip_start)(bool headnorth);
void  (*fr_rend_start)(void);
grs_bitmap *(*fr_get_tmap)(void);

// Set by machine type 
#if (defined(powerc) || defined(__powerc))	
Boolean DoubleSize = false;
Boolean SkipLines = false;
#else
Boolean DoubleSize = true;
Boolean SkipLines = false;
#endif

int   (*_fr_glob_draw_call)(void *dest_canvas, void *dest_bm, int x, int y, int flags)=NULL;
void  (*_fr_glob_horizon_call)(void *dest_bm, int flags)=NULL;
void  (*_fr_glob_render_call)(void *dest_bm, int flags)=NULL;

//#define DOUBLE_DEF_STUPID_BLEND
#ifdef DOUBLE_DEF_STUPID_BLEND
uchar det_sizing[4][2]={{0,0},{1,0},{0,0},{0,0}};     /* setup for detail modes */
#else
uchar det_sizing[4][2]={{0,0},{0,0},{0,0},{0,0}};     /* sizing for detail modes */
#endif

/* KLC - no stereo in Mac version
extern bool inp6d_headset;
extern bool inp6d_stereo_active;
extern int inp6d_stereo_div;
*/
fauxrend_context *_fr, *_sr;           /* current and default fauxrend contexts */
uint _fr_glob_flags;                   /* global flag settings */
uint _fr_curflags;                     /* settings for current rendering */
uchar *_fr_clut_list[4];               /* various lighting fills */
// the default setup/configuration for fr parameters, modifiable

fauxrend_parameters _frp=
{
 {1,1,1,1,0,0},
 {0,0,0},
 {LIGHT_BITS_TERR|LIGHT_BITS_ANY,2,{0,3},{7,0},-fix_make(2,0x1500),fix_make(7,0x0000)},
 {1,2,0,3,0xff,{1,4,9},{1,4,9},18,0,0},		// MLA - 0xff is clear color, used to be 0
 {0,0,0,0}
};
int _fr_last_detail=-1;
int _fr_default_detail=0;
int _fr_global_detail=3;
#define FR_USE_GLOBAL_DETAIL 4

//======== global initialization
// startup and closedown functions, misc initialization and setup

// first, for now, we make sure we have full texture context, and eat 8K to boot
#define NO_FAKE_TMAPS

void fr_closedown(void)
{
   fr_global_mod_flag(0,0xFFFFFFFF); // not totally sure this is right.
   _frp.lighting.global_mod = 0;
}

#ifndef NO_FAKE_TMAPS
#include "3dinterp.h"

grs_bitmap tmap_bm[FAKE_TMAPS];  // this is dumb, yea yea

void _fr_init_all_tmaps(void)
{
   uchar *dummy_tm;
   int i, x, y;

   for (i=0; i<FAKE_TMAPS; i++)
	{
      int v1=(rand()&0xff), v2=(rand()&0xff), v3=(rand()&0xff), v4=(rand()&0xff);
      dummy_tm=(uchar *)NewPtr(16*16);
      for (x=0; x<16; x++)
         for (y=0; y<16; y++)
            dummy_tm[(x*16)+y]=(((x>>1)+(y>>1))&1)?((2*abs(8-x))>y)?v1:v2:((2*abs(8-x))>y)?v3:v4;
      gr_init_bm(tmap_bm+i,dummy_tm,BMT_FLAT8,0,16,16);
 	   g3_set_vtext(i, tmap_bm+i);
   }
#ifdef RANDOMLY_SET_VCOLORS
   for (i=0; i<16; i++)    // hack hack hack
    { g3_set_vcolor(i, 0x33+(i<<2)); }
#endif
}

void _fr_free_all_tmaps(void)
{
   int i;
   for (i=0; i<FAKE_TMAPS; i++)
      DisposePtr((Ptr)tmap_bm[i].bits);
}
#else
#define _fr_init_all_tmaps()
#define _fr_free_all_tmaps()
#endif

extern int _game_fr_tmap;
void        fr_default_mouse(void)           {}
int         fr_default_idx(void)             {return (64+(((8*_fdt_y)+_fdt_x)&0x3f));}
int         fr_pickup_idx(void)              {gr_set_fill_parm(_game_fr_tmap+1); return _game_fr_tmap+1;}
#ifndef NO_FAKE_TMAPS
grs_bitmap *fr_default_tmap(void)            {return &tmap_bm[fr_default_idx()%FAKE_TMAPS];}
#else
grs_bitmap *fr_default_tmap(void)            {return NULL;}
#endif
bool        fr_default_block(void *, uchar *, int *) {return FALSE;}
void        fr_default_clip_start(bool ) {}
void        fr_default_rend_start(void)      {}

void fr_set_default_ptrs(void)
{
   fr_mouse_hide=fr_mouse_show=fr_default_mouse;
   fr_get_idx=fr_default_idx;
   fr_get_tmap=fr_default_tmap;
   fr_obj_block=fr_default_block;
   fr_clip_start=fr_default_clip_start;
   fr_rend_start=fr_default_rend_start;
}

// actually init the 3d, as one might expect, also set up global statics for the renderer
void fr_startup(void)
{
   // should be dynamic and flippable....
#ifdef STEREO_SUPPORT
   g3_init_stereo(FR_PT_CNT,AXIS_ORDER);
   g3_set_eyesep(FIX_UNIT/35);
#else
   g3_init(FR_PT_CNT,AXIS_ORDER);
#endif
   _fr_init_all_tmaps();
   fr_tables_build();
   _fr_glob_flags=0; _fr=_sr=NULL;
   fr_set_default_ptrs();
   fr_tfunc_grab_start();
#ifdef _FR_PIXPROF
   pixprof_setup();
#endif
}

// lets hit the fucking road
void fr_shutdown(void)
{
   _fr_free_all_tmaps();
	g3_shutdown();
}

// you taught me everything about a poison apple
void fr_set_cluts(uchar *base, uchar *bwclut, uchar *greenclut, uchar *amberclut)
{
   _fr_clut_list[0]=base; _fr_clut_list[1]=bwclut; _fr_clut_list[2]=greenclut; _fr_clut_list[3]=amberclut;
}

//======== view control/setup
// variables and functions for construction and maintenance (care and feeding of) views

// its okay to kill in the name of democracy
g3s_vector viewer_position;
g3s_angvec viewer_orientation;

// set the default view for NULL argument passing to the system
int fr_set_view(frc *view)
{
   _chkNull(view);
   _sr=(fauxrend_context *)view;
   _fr_ret;
}

// free the memory (frame buffers+structures) associated with view view
int fr_free_view(frc *view)
{
   _fr_top(view);
   if (_fr!=NULL)
   {
      if (_fr==_sr) _sr=NULL;          /* no longer a default rendering context */
      if ((_fr->flags&FR_DOUBLEB_MASK)&&((_fr->flags&FR_OWNBITS_MASK)==0))
      {
//         DisposePtr((Ptr)_fr->main_canvas.bm.bits);  // deal w/any other canvii
		DebugStr("\pHey!  We shouldn't ever be doing this!");
		DisposePtr(_fr->realCanvasPtr);
      }
  	 DisposePtr((Ptr)_fr);
   }
   _fr_ret;
}

int fr_mod_cams(frc *fr, void *v_cam, int mod_fac)
{
	cams *cam=(cams *)v_cam;
	
	_fr_top(fr);
	_fr->viewer_zoom = fix_mul(_fr->viewer_zoom,mod_fac);
	if (_fr->viewer_zoom==0)
		_fr->viewer_zoom=1;
	if ((unsigned long) _fr->viewer_zoom > 0x7fffffff)
		_fr->viewer_zoom=0x7fffffff;
	if ((long)cam != -1)
		if (cam == NULL)
			_fr->camptr = fr_camera_getdef(); 
		else 
			_fr->camptr = cam;
	_fr_ret;
}
// we put 
// eachother 
// down
int fr_context_mod_flag (frc *fr, int pflags_on, int pflags_off)       // change flags
{
   _fr_top(fr);
   _fr->flags&=~pflags_off;
   _fr->flags|=pflags_on;
   _fr_ret;
}

#if _fr_defdbg(ALTCAM)
extern int _fr_altcamx, _fr_altcamy;
int fr_mod_xtracam(frc *fr, void *v_xtra_cam)
{
   cams *xtra_cam=(cams *)v_xtra_cam;
   _fr_top(fr);
   _fr->xtracam=xtra_cam;
   _fr_ret;
}
#endif

// we never want the truth.... to be found
int fr_global_mod_flag(int flags_on, int flags_off)
{
   _fr_glob_flags&=~flags_off;
   _fr_glob_flags|=flags_on;
   _fr_ret;
}

// we are all bigots
// so filled with hatred
// we release our poisons
int fr_mod_size (frc *view, int xc, int yc, int wid, int hgt)    // move us around
{
   int detail;
   _fr_top(view);
   // should leard to deal with built zoom and such, so on
   detail=_fr->detail;
   fr_place_view(_fr,_fr->camptr,NULL,_fr->flags,_fr->axis,_fr->fov,xc,yc,wid,hgt);
   _fr->detail=detail;
   _fr_ret;
}

// like styrofoam
int fr_set_callbacks(frc *view, 
								int (*draw)(void *dstc, void *dstbm, int x, int y, int flg),
                     				void (*horizon)(void *dstbm, int flg), 
                     				void (*render)(void *dstbm, int flg))
{  // build local context setup for render
   _fr_top(view);
   _fr->draw_call = draw;
   _fr->horizon_call = horizon;
   _fr->render_call = render;
   _fr_ret;
}

// like styrofoam
int fr_set_global_callbacks( int (*draw)(void *dstc, void *dstbm, int x, int y, int flg),
                     							void (*horizon)(void *dstbm, int flg), 
              								void (*render)(void *dstbm, int flg) )
{  // build local context setup for render
   _fr_glob_draw_call = draw;
   _fr_glob_horizon_call = horizon;
   _fr_glob_render_call = render;
   _fr_ret;
}

//---------------------------------------------------------------------------------
//  The big dude that sets up a rendering context.  Changed in Mac version to use our already allocated main
//  offscreen buffer.
//---------------------------------------------------------------------------------
frc *fr_place_view (frc *view, void *v_cam, void *cnvs, int pflags, char axis, int fov, int xc, int yc, int wid, int hgt)
{
	cams *cam=(cams *)v_cam;
	fauxrend_context *fr;

	if (view==NULL)
		fr = (fauxrend_context *)NewPtr(sizeof(fauxrend_context));
/* KLC - this never actually happens
	else        // are their other canvii to free here...
	{
		fr = (fauxrend_context *)view;
		if (fr->flags & FR_DOUBLEB_MASK)
//			DisposePtr((Ptr)fr->main_canvas.bm.bits);
			DisposePtr(fr->realCanvasPtr);
	}
*/	
	if (pflags&FR_DOUBLEB_MASK)
	{
		if (cnvs==NULL)
		{
DebugStr("\pHey! I though we always supplied a canvas");
/*
			uchar *p;
			int	rowbytes = (wid + 31) & 0xFFFFFFE0;
			
			// if the number of cache lines is even, then add a cache line width to improve
			// even/odd line usage for vertical drawing.
			if (!((rowbytes / 64) & 1))
				rowbytes += 64;
			p = (uchar *)NewPtr(rowbytes*hgt + 32);
			if (p == NULL)
				return NULL;
			fr->realCanvasPtr = (Ptr)p;
			gr_init_canvas(&fr->main_canvas, (uchar *)((ulong)(p+31) & 0xFFFFFFE0), BMT_FLAT8, wid, hgt);
			fr->main_canvas.bm.row = rowbytes;
*/
		}
		else
		{
			// lets pretend we are getting a bitmap instead, eh?
			//         fr->main_canvas=*((grs_canvas *)cnvs);
			gr_init_canvas(&fr->main_canvas,(uchar *)cnvs,BMT_FLAT8,wid,hgt);
			pflags|=FR_OWNBITS_MASK;
		}
	}
	else
		gr_init_sub_canvas(grd_screen_canvas,&fr->main_canvas,xc,yc,wid,hgt);
	
	gr_init_sub_canvas(grd_screen_canvas,&fr->hack_canvas,xc,yc,wid,hgt);
	// set everything and its brothers brother, first inherit global callbacks, then set up axis and window and all
	fr->draw_call=_fr_glob_draw_call; fr->horizon_call=_fr_glob_horizon_call; fr->render_call=_fr_glob_render_call;
	fr->draw_canvas=fr->main_canvas; fr->axis=axis; fr->fov=fov;
	fr->xtop=xc; fr->ytop=yc; fr->xwid=wid; fr->ywid=hgt; fr->flags=pflags; fr->camptr=cam;
	if (fov==0) fov=FR_DEF_FOV; if (axis==0) axis=FR_DEF_AXIS;
	fr->viewer_zoom=g3_get_zoom(axis,build_fix_angle(fov),wid,hgt);
	fr->detail=_fr_default_detail; /* default to lowest detail level. */
	fr->last_detail=-1; /* always need to init detail. */
	return (frc*)fr;
}

void fr_use_global_detail(frc *view)
{
   if (view!=NULL) ((fauxrend_context *)view)->detail=FR_USE_GLOBAL_DETAIL;
}

int fr_view_resize(frc *view, int wid, int hgt)
{
   int nw, nh, nxt, nyt;
   int detail;
   _fr_top(view);
   nw=_fr->xwid; nh=_fr->ywid; nxt=_fr->xtop; nyt=_fr->ytop;           /* get base new coors */
   if ((nw+nxt<=wid)&&(nh+nyt<=hgt))
      ;  /* all ok... */
   else
   {
	   if (nw<wid) nxt=(wid-nw)/2; else { nw=wid; nxt=0; }               /* either center old size, or fill new */
	   if (nh<hgt) nyt=(hgt-nh)/2; else { nh=hgt; nyt=0; }               /* either center old size, or fill new */
   }
   detail=_fr->detail;
   fr_place_view(_fr,_fr->camptr,NULL,_fr->flags,_fr->axis,_fr->fov,nxt,nyt,nw,nh);
   _fr->detail=detail;
   _fr_ret;
}

int fr_view_full(frc *view, int wid, int hgt)
{
   int detail;
   _fr_top(view);
   detail=_fr->detail;
   fr_place_view(_fr,_fr->camptr,NULL,_fr->flags,_fr->axis,_fr->fov,0,0,wid,hgt);
   _fr->detail=detail;
   _fr_ret;
}

void *fr_get_canvas(frc *view)
{
   _fr_top_cast(view,(void *));
   return &_fr->draw_canvas;
}

// they're all talking bout, beatles songs
// written a hundred years before they were born
// yea they're all talking bout, the round and round
// but whose got the real, anti-parent culture sound

// run when context detail has changed.
void _fr_update_context(int det)
{
   if (_fr->flags&FR_DOUBLEB_MASK)
	   gr_init_canvas(&_fr->draw_canvas,_fr->main_canvas.bm.bits,BMT_FLAT8,_fr->xwid>>det_sizing[det][0],_fr->ywid>>det_sizing[det][1]);
   else                                         // 0,0 was xtop,ytop
      gr_init_sub_canvas(&_fr->main_canvas,&_fr->draw_canvas,0,0,_fr->xwid>>det_sizing[det][0],_fr->ywid>>det_sizing[det][1]);
   _fr->last_detail=det;
}

// run when current context detail is different from the last rendered context detail.
#ifndef NEW_2D
extern void gr_set_lin_always();
extern void gr_reset_per();
#endif

void _fr_change_detail(int det)
{
   // note: pixel_ratio 5 data types before scrw, if order is preserved
   int tmpz, fov;
#ifdef DOUBLE_DEF_STUPID_BLEND
   if ((det==1)&&(_fr_last_detail!=1))
    { /*_fr->viewer_zoom<<=1; */ *(fix *)((&scrw)-5)>>=1; }
   if ((det!=1)&&(_fr_last_detail==1))
    { /*_fr->viewer_zoom>>=1; */ *(fix *)((&scrw)-5)<<=1; }
#endif
   switch (det) {
      case 0:
         g3_set_tmaps_linear();
         gr_set_per_detail_level(GR_LOW_PER_DETAIL);
         break;
      case 1:
         g3_reset_tmaps();
         gr_set_per_detail_level(GR_LOW_PER_DETAIL);
         break;
      case 2:
         g3_reset_tmaps();
         gr_set_per_detail_level(GR_MEDIUM_PER_DETAIL);
         break;
      case 3:
         g3_reset_tmaps();
         gr_set_per_detail_level(GR_HIGH_PER_DETAIL);
   }
   if (_fr->fov==0) fov=FR_DEF_FOV; else fov=_fr->fov;
   tmpz=g3_get_zoom(FR_DEF_AXIS,fov,_fr->draw_canvas.bm.w,_fr->draw_canvas.bm.h);
// mprintf("Tmpz %x, vz %x (%d <- %d), ps. %x\n",tmpz,_fr->viewer_zoom,_fr->detail,_fr_last_detail,*(fix *)((&scrw)-5));
   _fr_last_detail=det;
}

/* sets global fr 
 * masks in the global rendering context
 * deals with any change in detail settings
 */
int fr_prepare_view(frc *view)
{
   int det;
   _fr_top(view);
   _fr_curflags=_fr_glob_flags|_fr->flags;       // for now, simply merge
   if (_fr->detail==FR_USE_GLOBAL_DETAIL)
      det = _fr_global_detail;
   else
      det = _fr->detail;
   if (_fr_last_detail!=det)
      _fr_change_detail(det);
   if (_fr->last_detail!=det)
      _fr_update_context(det);
   _fr_ret;
}

/* For cyberspace model which uses g3_start_object_matrix()...*/
extern fix EDMS_Dirac_basis[9];

#ifdef STEREO_SUPPORT
extern uchar hack_cameras_needed;
#endif

/* sets the 3d system up based upon the prepared context */
#define FIXANG_EPS (FIXANG_PI>>5)
#define FIXANG_MASK (2*FIXANG_PI-1)
extern int (*_fr_lit_floor_func)(int, g3s_phandle *, grs_bitmap *);
extern int (*_fr_floor_func)(int, g3s_phandle *, grs_bitmap *);
extern int (*_fr_lit_wall_func)(int, g3s_phandle *, grs_bitmap *);
extern int (*_fr_wall_func)(int, g3s_phandle *, grs_bitmap *);
extern int (*_fr_lit_per_func)(int, g3s_phandle *, grs_bitmap *);
extern int (*_fr_per_func)(int, g3s_phandle *, grs_bitmap *);
int fr_start_view(void)
{
   g3s_matrix system_matrix;
   int use_zoom;
   uchar old_cam_type;
   int detail;

   // check detail for canvas sizing
   gr_set_canvas(&_fr->draw_canvas);
   if (_fr_curflags&FR_PICKUPM_MASK)
    { gr_set_fill_type(FILL_SOLID); gr_set_fill_parm(0); }
   else
      gr_set_fill_type(FILL_NORM);
   if (_fr_curflags&FR_CURVIEW_MASK)
      old_cam_type=fr_camera_modtype(_fr->camptr,((_fr->flags&FR_CURVIEW_MASK)>>FR_CURVIEW_SHFT)<<CAMANG_S,CAMBIT_ANG);
   fr_camera_getpos(_fr->camptr);        /* loads into camera_last */
   // this is a total hack, goddamn....
   if ((_fr_curflags&FR_SFX_MASK)==FR_SFX_SHAKE)
    { fr_camera_last[4]+=(rand()&0x03ff)-0x200; fr_camera_last[5]+=(rand()&0x03ff)-0x200; }
   if (_fr_curflags&FR_CURVIEW_MASK)
      fr_camera_modtype(_fr->camptr,old_cam_type&CAMANG_S,CAMBIT_ANG);
   viewer_position.xaxis   = coor(EYE_X); viewer_position.yaxis   =-coor(EYE_Z); viewer_position.zaxis   = coor(EYE_Y);
   viewer_orientation.pitch=  ang(EYE_P); viewer_orientation.bank =  ang(EYE_B); viewer_orientation.head =  ang(EYE_H);

   if (_fr->detail==FR_USE_GLOBAL_DETAIL)
      detail = _fr_global_detail;
   else
      detail = _fr->detail;
   if (detail!=0) {
      /* check viewer orientation.  Use wall/floor/full perspective texture maps accordingly. */
      _fr_lit_per_func=g3_light_tmap;
      _fr_per_func=g3_draw_tmap;
      if (((viewer_orientation.bank+(FIXANG_EPS/2))&FIXANG_MASK)<FIXANG_EPS) {
         _fr_lit_floor_func=g3_light_floor_map;
         _fr_floor_func=g3_draw_floor_map;
         if (((viewer_orientation.pitch+(FIXANG_EPS/2))&FIXANG_MASK)<FIXANG_EPS) {
            _fr_lit_wall_func=g3_light_wall_map;
            _fr_wall_func=g3_draw_wall_map;
         } else {
            _fr_lit_wall_func=g3_light_tmap;
            _fr_wall_func=g3_draw_tmap;
         }
      } else {
         _fr_lit_floor_func=g3_light_tmap;
         _fr_floor_func=g3_draw_tmap;
         _fr_lit_wall_func=g3_light_tmap;
         _fr_wall_func=g3_draw_tmap;
      }
   } else {
      /* Use linear texture maps unless 1d wall applicable. */
      if ((((viewer_orientation.bank+(FIXANG_EPS/2))&FIXANG_MASK)<FIXANG_EPS)&&
            (((viewer_orientation.pitch+(FIXANG_EPS/2))&FIXANG_MASK)<FIXANG_EPS)) {
         _fr_lit_wall_func=g3_light_wall_map;
         _fr_wall_func=g3_draw_wall_map;
      } else {
         _fr_lit_wall_func=g3_light_lmap;
         _fr_wall_func=g3_draw_lmap;
      }
      _fr_lit_floor_func=g3_light_lmap;
      _fr_floor_func=g3_draw_lmap;
      _fr_lit_per_func=g3_light_lmap;
      _fr_per_func=g3_draw_lmap;
   }

#ifdef _FR_PIXPROF
   gr_start_frame();
#endif

#ifdef STEREO_SUPPORT
   if (((_fr_curflags&(FR_PICKUPM_MASK|FR_HACKCAM_MASK))==0)&&inp6d_stereo_active&&
        ((_fr_curflags&FR_CURVIEW_MASK)==FR_CURVIEW_STRT))
   {
      extern uchar g3d_stereo;
      i6_video(I6VID_FRM_START,NULL);  // lets go
      i6_video(I6VID_FRM_INFIN,NULL);  // begin infinite region
      gr_set_canvas(i6d_ss->cf_infin);
//      gr_clear(0x78);
      gr_clear(0);
      i6_video(I6VID_FRM_STEREO,NULL); // now, the stereo set
      gr_set_canvas(i6d_ss->cf_left);
      if (i6d_device == I6D_CTM)
         grd_cap->aspect<<=1;
      g3_set_eyesep(inp6d_stereo_div/96);    // stereo div is in fix inches...
      g3_start_stereo_frame(i6d_ss->cf_right);
//      g3d_stereo=0;
   }
   else
#endif
      g3_start_frame();

/*KLC - stereo
   if (inp6d_headset)
   {
      extern int inp6d_curr_fov;
	   use_zoom=g3_get_zoom(FR_DEF_AXIS,build_fix_angle(inp6d_curr_fov),320,200);
      std_size=2;
   }
   else*/
   {
      use_zoom=_fr->viewer_zoom;
      std_size=1;
   }

   if ( _frp.faces.cyber ) {

        system_matrix.m1 = EDMS_Dirac_basis[0];
        system_matrix.m2 = EDMS_Dirac_basis[1];
        system_matrix.m3 = EDMS_Dirac_basis[2];
        system_matrix.m4 = EDMS_Dirac_basis[3];
        system_matrix.m5 = EDMS_Dirac_basis[4];
        system_matrix.m6 = EDMS_Dirac_basis[5];
        system_matrix.m7 = EDMS_Dirac_basis[6];
        system_matrix.m8 = EDMS_Dirac_basis[7];
        system_matrix.m9 = EDMS_Dirac_basis[8];

      g3_set_view_matrix(&viewer_position,&system_matrix,_fr->viewer_zoom);
//    g3_set_view_angles(&viewer_position,&viewer_orientation,ANGLE_ORDER,use_zoom);
   } else
      g3_set_view_angles(&viewer_position,&viewer_orientation,ANGLE_ORDER,use_zoom);

   g3_set_bitmap_scale(fix_make(0,(int)(2048/3)),fix_make(0,(int)(2048/3)));
//	g3_get_FOV(&x_fov,&y_fov);
   if (!_fr->flags&FR_DOUBLEB_MASK)
      (*fr_mouse_hide)();
   else
      if (_fr->horizon_call)
         _fr->horizon_call(&_fr->draw_canvas.bm, _fr_curflags);
//KLC      else if (global_fullmap->cyber)
		gr_clear(_frp.view.clear_color);
		
   // now have everything set up for 3d view
   // if wacky secondary camera mode, set up
#if _fr_defdbg(ALTCAM)
   _fr_sdbg(ALTCAM,{fr_camera_getpos(_fr->xtracam);_fr_altcamx=coor(EYE_X)>>(16);_fr_altcamy=coor(EYE_Y)>>(16);})
#endif
   return TRUE;
}

//#define JUST_SHOW_THE_THING

/* send the actual frame out a here.... */
// you're so kind when it serves you well
bool smooth_double=FALSE;
g3s_vector zvec = {0,0,0};

extern bool view360_is_rendering;

int fr_send_view (void)
{
   bool 	snd_frm=TRUE;
   bool	ok_to_double;

   // JAEMZ JAEMZ JAEMZ
   // render the stars, if there were
   // no stars in this scene it simply returns
   // spin it, spin it more when reactor blown
   // rotation every 20 minutes, every 1 minute after explosion

   g3_start_object_angles_y(&zvec, QUESTBIT_GET(0x14) ? player_struct.game_time * 3 : player_struct.game_time / 5 );

   star_render();
   g3_end_object();

   g3_end_frame();

      // stereo support - closedown ??
#ifdef STEREO_SUPPORT
   if (((_fr_curflags&(FR_PICKUPM_MASK|FR_HACKCAM_MASK))==0)&&inp6d_stereo_active&&
        ((_fr_curflags&FR_CURVIEW_MASK)==FR_CURVIEW_STRT))
   {
      gr_set_canvas(grd_screen_canvas);
      if (_fr->draw_call) snd_frm=_fr->draw_call(grd_screen_canvas, &_fr->draw_canvas.bm,_fr->xtop,_fr->ytop,_fr_curflags);
      gr_set_canvas(i6d_ss->cf_left);
	   (*fr_mouse_show)();
      gr_set_canvas(i6d_ss->cf_right);
	   (*fr_mouse_show)();
      i6_video(I6VID_FRM_DONE,NULL);
      i6_video(I6VID_FRM_COPY,NULL);   // send it's butt
      gr_set_canvas(i6d_ss->cf_left);
      if (i6d_device == I6D_CTM)
         grd_cap->aspect>>=1;
      _fr_ret;   
   }
#endif
	
	// If we're rendering just the quick mono bitmap (for clicking on items, on-line help, etc),
	// then return here.
	if (_fr_curflags&FR_PICKUPM_MASK)
	{
		gr_set_canvas(grd_screen_canvas);
		_fr_ret;
	}
	
	// Determine if it's okay to double (it's not okay when rendering the 360 view).
	ok_to_double = (DoubleSize && !view360_is_rendering);
	
	// If double sizing, double-size the rendered bitmap onto the intermediate buffer, 
	// before doing any overlays.  Don't do it if rendering the 360 view.
	if (ok_to_double)
	{
		gr_set_canvas(&gDoubleSizeOffCanvas);
		if (full_game_3d)
			FastFullscreenDouble2Canvas(&_fr->draw_canvas.bm, &gDoubleSizeOffCanvas, _fr->xwid, _fr->ywid);
		else
			FastSlotDouble2Canvas(&_fr->draw_canvas.bm, &gDoubleSizeOffCanvas, _fr->xwid, _fr->ywid);
	}
	
	// Draw the overlays
	if (_fr->draw_call)
		snd_frm = _fr->draw_call(grd_screen_canvas, 
												(ok_to_double) ? &gDoubleSizeOffCanvas.bm : &_fr->draw_canvas.bm,
												_fr->xtop,_fr->ytop,_fr_curflags);

	if (snd_frm)
	{
		(*fr_mouse_hide)();									// This actually draws the mouse into the rendered canvas.
		gr_set_canvas(grd_screen_canvas);				// Now set us to the screen canvas.
		
		if (_fr_curflags&FR_DOUBLEB_MASK)		// If we're double-buffered (which we always are)
		{
//			if (_fr_curflags&FR_DOHFLIP_MASK)		// Does this ever occur?
//				gr_hflip_ubitmap(&_fr->draw_canvas.bm,_fr->xtop,_fr->ytop);
			if (view360_is_rendering)
				gr_ubitmap(&_fr->draw_canvas.bm,_fr->xtop,_fr->ytop);
			else
			{
				if (ok_to_double)								// If double-sizing, just copy the already double-sized
				{														// scene from the temp offscreen canvas to the screen.
					if (full_game_3d)
						Fast_FullScreen_Copy(&gDoubleSizeOffCanvas.bm);
					else
						Fast_Slot_Copy(&gDoubleSizeOffCanvas.bm);
				}
				else													// For high-res, just copy from the draw canvas.
				{
					if (full_game_3d)
						Fast_FullScreen_Copy(&_fr->draw_canvas.bm);
					else
						Fast_Slot_Copy(&_fr->draw_canvas.bm);
				}
			}
		}
		(*fr_mouse_show)();
   }
   else
	 	gr_set_canvas(grd_screen_canvas);
   _fr_ret;   
}
