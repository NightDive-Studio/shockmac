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
 * $Source: r:/prj/cit/src/RCS/rendtool.c $
 * $Revision: 1.37 $
 * $Author: buzzard $
 * $Date: 1994/11/25 09:16:45 $
 *
 *
 *  support for render functions and tools, such as mouse and so on
 */

#define __RENDTOOL_SRC

#include "map.h"
#include "frintern.h"

#include "faketime.h"

#include "gamescr.h"
#include "fullscrn.h"

#include "mainloop.h"
#include "init.h"

#include "textmaps.h"
#include "gettmaps.h"

#include "frcamera.h"
#include "player.h"

#include "objects.h"
#include "objprop.h"
#include "objbit.h"
#include "objsim.h"

#include "modtext.h"
#include "citmat.h"

// should probably wimp out and take frintern
// or have a separate fritrfce or something
// rather than externing everything a couple of 30 lines down

#include "rendtool.h"
#include "frtypes.h"	// so we get _fr, so we can get size + canvas and all
#include "frparams.h"
#include "frflags.h"
#include "tilemap.h"
#include "fr3d.h"
#include "frquad.h"

#include "gamesort.h"
#include "citres.h"

#include "star.h"

//#include <mprintf.h>

LGRect *rendrect;
extern fauxrend_context *_fr;
ubyte mouselocked = 0;

#define NUM_STARS 500

sts_vec star_vec[NUM_STARS];
uchar star_col[NUM_STARS];


//------------------------
//  Internal Prototypes
//------------------------
void rend_mouse_hide(void);
void rend_mouse_show(void);
bool game_obj_block_home(void *vmptr, uchar *_sclip, int *loc);
bool game_obj_block(void *vmptr, uchar *_sclip, int *loc);
int game_fr_idx(void);
grs_bitmap *game_fr_tmap_128(void);
grs_bitmap *game_fr_tmap_64(void);
grs_bitmap *game_fr_tmap_full(void);
void game_rend_start(void);
void game_fr_clip_start(bool headnorth);

void game_redrop_rad(int rad_mod);
void change_detail_level(byte new_level);
void set_global_lighting(short l_lev);
void fauxrend_camera_setfunc(TileCamera* tc);
void rendedit_process_tilemap(FullMap* fmap, LGRect* r, bool newMap);
ushort fr_get_at_raw(frc *fr, int x, int y, bool again, bool transp);
void load_model_vtexts(char model_num);
void free_model_vtexts(char model_num);

// Note that I have fixed this so that the cursor does not flicker.
// It just works.  Note its simplistic beauty.	I love this job.
void rend_mouse_hide(void)
{
	extern Boolean		DoubleSize;
	extern grs_canvas	gDoubleSizeOffCanvas;
	extern Boolean		view360_is_rendering;
		
	MouseLock++;
	if (MouseLock==1 && CurrentCursor != NULL)
	{
		int			cmd = CURSOR_DRAW;
		LGPoint 		pos = LastCursorPos;
		grs_canvas	*old_canvas = CursorCanvas;
		
		pos.x -= _fr->xtop;
		pos.y -= _fr->ytop;
		
/*KLC - this is never true, now
		if (_fr_curflags & FR_DOHFLIP_MASK)
		{
			pos.x = _fr->xwid - pos.x - 1;
			cmd = CURSOR_DRAW_HFLIP;
		}
*/
		mouselocked = 1;
		MouseLock++; // keep mouse locked while we blit
	
		if (DoubleSize && !view360_is_rendering)					// If double-sizing, then draw the cursor in the
		{																				// temporary doubled canvas.
			CursorCanvas = &gDoubleSizeOffCanvas;
			if (!full_game_3d)													// In slot view, adjust the cursor position.
			{
				pos.x -= 28;
				pos.y -= 29;
			}
		}
		else
			CursorCanvas = &_fr->draw_canvas;
		if (LastCursor)
			LastCursor->func(cmd,CursorRegion,LastCursor,pos);
		CursorCanvas = old_canvas;
	}
	else
		mouselocked = 0;
   MouseLock--;      			// decrement mouselock after frame buffer blit.
}

void rend_mouse_show(void)
{
   MouseLock -=mouselocked;
}

extern int _game_fr_tmap;  // current tmap
extern MapElem *_fdt_mptr;

extern grs_bitmap tmap_bm[32];	// this is dumb, yea yea

static MapElem *home_ptr;


bool game_obj_block_home(void *vmptr, uchar *_sclip, int *loc)
{
   MapElem *mptr=(MapElem *)vmptr;
   Obj *cobj;
   ObjID cobjid;
   ObjRefID curORef;

   curORef=mptr->objRef;
   while (curORef!=OBJ_REF_NULL)
   {
      cobjid=objRefs[curORef].obj;
      if (ObjProps[OPNUM(cobjid)].flags & RENDER_BLOCK)
      {  // if we are a block type object
	      cobj=&objs[cobjid];
	 if (((cobj->loc.p|cobj->loc.b)==0)&&
	     ((cobj->loc.h&0x3f)==0)&&
	      (cobj->info.current_frame==0))
	    if ((objRefs[curORef].state.bin.sq.x==(cobj->loc.x>>8))&&
		(objRefs[curORef].state.bin.sq.y==(cobj->loc.y>>8)))
	    {  // is it at correct heading
	       if ((cobj->loc.h+0x20)&0x40)
	       {
//		    mprintf("Thinking about X subclip from %x and %x, clip %x\n",cobj->loc.x,fr_camera_last[0],_sclip[1]);
		  if ((home_ptr==mptr)||(loc[0]>>16==_fr_x_cen))
		  {
			  if (_sclip[1]==FMK_INT_WW)
			  {
			     if (cobj->loc.x<(fr_camera_last[0]>>8))
			      { _me_subclip(mptr)|=_sclip[1]; return TRUE; }
			  }
			  else
			     if (cobj->loc.x>(fr_camera_last[0]>>8))
				   { _me_subclip(mptr)|=_sclip[1]; return TRUE; }
//		    mprintf("Guess not\n");
			  return FALSE;
		  }
		  else
		   { _me_subclip(mptr)|=_sclip[1]; /* mprintf("xsubclip standard..."); */ return TRUE; }
	       }
	       else
	       {
		  if ((home_ptr==mptr)||(loc[1]>>16==_fr_y_cen))
		  {
			  if (_sclip[0]==FMK_INT_SW)
			  {
			     if (cobj->loc.y<(fr_camera_last[1]>>8))
			      { _me_subclip(mptr)|=_sclip[0]; } // mprintf("ysubclip yep south.."); }
			  }
			  else
			     if (cobj->loc.y>(fr_camera_last[1]>>8))
			      { _me_subclip(mptr)|=_sclip[0]; } // mprintf("ysubclip yep north.."); }
//		    mprintf(" Yo: Y subclip from %x and %x, clip %x\n",cobj->loc.y,fr_camera_last[1],_sclip[0]);
		  }
		  else
		   { _me_subclip(mptr)|=_sclip[0]; } // mprintf("ysubclip standard results.."); }
		  return FALSE;     // y direction
	       }
	    }
      }
      curORef = objRefs[curORef].next;
   }
   return FALSE;     // for now, no blockage in home square
}

bool game_obj_block(void *vmptr, uchar *_sclip, int *loc)
{
   MapElem *mptr=(MapElem *)vmptr;
   Obj *cobj;
   ObjID cobjid;
   ObjRefID curORef;

   if ( (home_ptr==mptr) || (((loc[0]>>16)==_fr_x_cen)||((loc[1]>>16)==_fr_y_cen)) )
      return game_obj_block_home(vmptr,_sclip,loc);

   curORef=mptr->objRef;
   while (curORef!=OBJ_REF_NULL)
   {
      cobjid=objRefs[curORef].obj;
      if (ObjProps[OPNUM(cobjid)].flags & RENDER_BLOCK)
      {  // if we are a block type object
	      cobj=&objs[cobjid];
	 if (((cobj->loc.p|cobj->loc.b)==0)&&
	     ((cobj->loc.h&0x3f)==0)&&
	      (cobj->info.current_frame==0))
	    if ((objRefs[curORef].state.bin.sq.x==(cobj->loc.x>>8))&&
		(objRefs[curORef].state.bin.sq.y==(cobj->loc.y>>8)))
	    {  // is it at correct heading
	       if ((cobj->loc.h+0x20)&0x40)
	       {
		       _me_subclip(mptr)|=_sclip[1];
		       return TRUE;	 // x direction
	       }
	       else
	       {
		  _me_subclip(mptr)|=_sclip[0];
//		    mprintf("Set %d %d to %x from sclip %x\n",
//			     objRefs[curORef].state.bin.sq.x,objRefs[curORef].state.bin.sq.y,me_subclip(mptr),_sclip[0]);
		  return FALSE;     // y direction
	       }
	    }
      }
      curORef = objRefs[curORef].next;
   }
   return FALSE;
}


int game_fr_idx(void)
{
   return _game_fr_tmap;
}

#define TIM_WERE_AWAKE
#ifdef TIM_WERE_AWAKE
#define IsTpropStars() (textprops[_game_fr_tmap].force_dir>0)
#define IsTpStarDraw() (textprops[_game_fr_tmap].force_dir==2)
#else
#define IsTpropStars() (_game_fr_tmap<4)
#define IsTpStarDraw() (_game_fr_tmap<2)
#endif

extern g3s_phandle _fdt_tmppts[8];   /* these are used for all temporary point sets */

bool draw_tmap_p(int ptcnt);

// should i draw this texture/map/so on
bool draw_tmap_p(int ptcnt)
{
   // JAEMZ JAEMZ JAEMZ JAEMZ
   // notify yourself here, i would guess....
   if (IsTpropStars()) 
    {
      if (IsTpStarDraw())
       {
		 // texture map, don't draw, just eval
		 star_empty(ptcnt,_fdt_tmppts);
		 return TRUE;
       } 
      else 
       {
		 star_poly(ptcnt,_fdt_tmppts);
		 return FALSE;
		 //g3_draw_poly(152,ptcnt,_fdt_tmppts);
      }
   }

   return TRUE;
}


// SPEED THIS UP
// major changes left to do:
//  -- rewrite in assembler and stop being a wuss, self modify in drop vals and full screen adjust
//  -- currently doesnt have support for full screen adjust.. should be easy assembler though, just need the structure offsets
grs_bitmap *game_fr_tmap_128(void)
{
   grs_bitmap *draw_me;
   register int loop=TEXTURE_128_INDEX;
   register int cur_drop=_frp.view.drop_rad[0] + textprops[_game_fr_tmap].distance_mod;

   if (cur_drop<_fdt_dist)
   {
      cur_drop+=_frp.view.drop_rad[1]; loop++;
      if (cur_drop<_fdt_dist)
      {
	 loop++;
	 if (cur_drop+_frp.view.drop_rad[2]<_fdt_dist)
	    loop++;
      }
   }

   draw_me= get_texture_map(_game_fr_tmap + ANIMTEXT_FRAME(_game_fr_tmap),loop);
   return draw_me;
}

grs_bitmap *game_fr_tmap_64(void)
{
   grs_bitmap *draw_me;
   register int loop=TEXTURE_64_INDEX;
   register int cur_drop=_frp.view.drop_rad[1] + textprops[_game_fr_tmap].distance_mod;

   if (cur_drop<_fdt_dist)
   {
      loop++;	  // now 32
      if (cur_drop+_frp.view.drop_rad[2]<_fdt_dist)
	 loop++;  // now 16
   }

   draw_me= get_texture_map(_game_fr_tmap + ANIMTEXT_FRAME(_game_fr_tmap),loop);
   return draw_me;
}

grs_bitmap *game_fr_tmap_full(void)
{
   grs_bitmap *draw_me;
   int loop=TEXTURE_128_INDEX, lmask;
   register cur_drop=_frp.view.drop_rad[0] + textprops[_game_fr_tmap].distance_mod;

   if (cur_drop<_fdt_dist)
   {
      cur_drop+=_frp.view.drop_rad[1]; loop++;
      if (cur_drop<_fdt_dist)
      {
	 loop++;
	 if (cur_drop+_frp.view.drop_rad[2]<_fdt_dist)
	  { loop++; goto draw_it; }
      }
   }
   lmask=(1<<loop);
#ifdef CAN_MISS
   if (((texture_array[_game_fr_tmap].sizes_loaded)&lmask)==0)
   {
      do {
	      loop++; lmask<<=1;
	   } while ((loop<TEXTURE_16_INDEX)&&(((texture_array[_game_fr_tmap].sizes_loaded)&lmask)==0));
   }
#endif

draw_it:
   draw_me = get_texture_map(_game_fr_tmap + ANIMTEXT_FRAME(_game_fr_tmap), loop);
   return draw_me;
}

void game_rend_start(void)
{
   extern ObjID no_render_obj;
   extern uchar cam_mode;
   cams *cur_cam;
   // hey, gots to do this somewhere
   // remove self from object list
   if (cam_mode==OBJ_PLAYER_CAMERA)
      no_render_obj=PLAYER_OBJ;
   else
   {
      cur_cam=fr_camera_getdef();
      if (cur_cam->type&CAMBIT_OBJ)
	 no_render_obj=cur_cam->obj_id;
      else
	 no_render_obj=-1;
   }

  render_sort_start();
}

void game_fr_clip_start(bool headnorth)
{
   if (headnorth)
   {
	   home_ptr=MAP_GET_XY(_fr_x_cen,_fr_y_cen);
	   fr_obj_block=game_obj_block;
   }
}

extern int gamesys_draw_func(void *dest_canvas, void *dest_bm, int x, int y, int flags);
extern void gamesys_render_func(void *dest_bitmap, int flags);

/*KLC - no longer used here
// new regieme, has gruesome hacks for memory saving....
#define FRAME_BUFFER_SIZE (320*200)
static uchar *frameBufferFreePtr=NULL;
extern uchar  frameBuffer[];
*/

uchar model_base_nums[MAX_VTEXT_OBJS];

void game_fr_startup(void)
{
   short curr, index;
   extern int std_alias_size;

// we know that the main screen we support is 320x200, so.....
//KLC   frameBufferFreePtr=frameBuffer;
// has to fixed, clearly
   fr_set_global_callbacks(gamesys_draw_func, NULL, gamesys_render_func);
   fr_mouse_hide=rend_mouse_hide;
   fr_mouse_show=rend_mouse_show;
   fr_get_idx=game_fr_idx;
   fr_get_tmap=game_fr_tmap_full;
   fr_clip_start=game_fr_clip_start;
   fr_rend_start=game_rend_start;
// this has to be fixed as well, should be real_ship or something
   curr = 1; index = 0;
   model_base_nums[0] = 0;
   while (model_vtext_data[index] != -1)  // check for a -1 in what was supposedly a nice place
   {
      while (model_vtext_data[index] != -1)
	  index++;   // eat up numbers until index points at -1
      model_base_nums[curr++] = index + 1;  // point at nice one beyond our -1 delimter
      index++; // increment past the -1
   }

//   for (i=0; i<num_materials; i++)
//	 { g3_set_vtext(i, material_maps+i); }
// _fr_dbgflg_tog(SHOW_PICKUP);
// mprintf("SizeofRef %d\n",sizeof(ObjRef));

   // initialize stars randomly for now, eventually use some resource or another
   // oops, using malloc.  Oh well
   star_set(NUM_STARS,star_vec,star_col);
   srand(0);
   star_rand(211,10);
   std_alias_size = 400;
}

void game_fr_shutdown(void)
{

}

//#pragma disable_message(202)
void game_fr_reparam(int is_128s, int , int )
{
   if (is_128s!=-1)
	   fr_get_tmap=is_128s?game_fr_tmap_128:game_fr_tmap_64;
   switch (_frp.faces.cyber=global_fullmap->cyber)
   {
   case 1: _fr_glob_flags|=FR_SHOWALL_MASK;  _frp.view.radius=13; break;
   case 0: _fr_glob_flags&=~FR_SHOWALL_MASK; _frp.view.radius=18; break;
	}
}

void game_redrop_rad(int rad_mod)
{
   _frp.view.drop_rad[0]=_frp.view.odrop_rad[0]+rad_mod;
   _frp.view.drop_rad[1]=_frp.view.odrop_rad[1]+rad_mod;
   _frp.view.drop_rad[2]=_frp.view.odrop_rad[2]+rad_mod;
}
//#pragma enable_message(202)

// errtype is icky
extern int _fr_global_detail;
void change_detail_level(byte new_level)
{
   _fr_global_detail=new_level;
}

void set_global_lighting(short l_lev)
{
   _frp.lighting.global_mod+=l_lev;
}


void rendedit_process_tilemap(FullMap* fmap, LGRect* r, bool newMap)
{
//   mprintf("RPT %d\n",new);
   if (fmap==NULL)		       /* support null pass in */
      fmap=global_fullmap;
   if (newMap)
      fr_compile_restart(fmap);
   fr_compile_rect(fmap,r->ul.x,r->ul.y,r->lr.x,r->lr.y,FALSE);
}

// lets move this to the tilemap, eh?
void fauxrend_camera_setfunc(TileCamera* tc)
{
   tc->x = last_coor(EYE_X);
   tc->y = last_coor(EYE_Y);
   tc->theta = last_ang(EYE_H)-FIXANG_PI/2;
   tc->show = TRUE;
}


// Like fr_get_at, but takes real screen coordinates.
ushort fr_get_at_raw(frc *fr, int x, int y, bool again, bool transp)
{
   extern fauxrend_context *_sr;
   extern ushort fr_get_again(frc *fr, int x, int y);
   _fr_top(fr);
   if (again)
      return fr_get_again(_fr, x - _fr->xtop, y - _fr->ytop);
   else
      return	fr_get_at(_fr, x - _fr->xtop, y - _fr->ytop, transp);
}

/*KLC - no longer used
// this is a hack for render canvas memory usage....
uchar *get_free_frame_buffer_bits(int size)
{
   if ((size==FRAME_BUFFER_SIZE)||(size<=0))
      return frameBuffer;
   else
   {
      uchar *tmp=frameBufferFreePtr;
      if (frameBufferFreePtr-frameBuffer+size>FRAME_BUFFER_SIZE)
      {
//	 Warning(("Mini Frame Buffers Too Big %d\n",size));
	 return NULL;
      }
      frameBufferFreePtr+=size;
      return tmp;
   }
}
*/

#define MATERIAL_BASE	RES_materialMaps

void load_model_vtexts(char model_num)
{
   short curr = model_base_nums[model_num];
   grs_bitmap *stupid;
   if (model_num >= MAX_VTEXT_OBJS)
      return;
   while (model_vtext_data[curr] != -1)
   {
      stupid = lock_bitmap_from_ref_anchor(MKREF(MATERIAL_BASE + model_vtext_data[curr],0),NULL);
      g3_set_vtext(model_vtext_data[curr], stupid);
      curr++;
   }
}

void free_model_vtexts(char model_num)
{
   short curr = model_base_nums[model_num];
   if (model_num >= MAX_VTEXT_OBJS)
      return;
   while (model_vtext_data[curr] != -1)
   {
      RefUnlock(MKREF(MATERIAL_BASE + model_vtext_data[curr], 0));
      curr++;
   }
}
