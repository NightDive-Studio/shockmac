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
 * $Source: r:/prj/cit/src/RCS/amap.c $
 * $Revision: 1.41 $
 * $Author: dc $
 * $Date: 1994/11/28 06:38:12 $
 *
 * routines for creation and modification of the automap on a level
 * uses the map's internal state for showing stuff
 * as well as a list of objects (map notes) for highlights and such
 * this is the core canvas rendering/wall stuff
 * it is called by the mfd based system, or in full screen mode
 */

// color set....
// player/focused object - red
// elevators             - dark brown
// walls                 - bright green, mid green, dim green
// security              - cycling red
// bio                   - yellow
// radiation             - flat red
// mutant                - purple
// robot                 - metalblue
// cyborg                - brightbrownx
// doors                 - maize?

#include <string.h>
#include <ctype.h>

#define __AMAP_SRC

#include "map.h"
#include "mapflags.h"
#include "tilename.h"
#include "frquad.h"
#include "colors.h"
#include "rcolors.h"
#include "objprop.h"
#include "objuse.h"
#include "objgame.h"
#include "objbit.h"
#include "otrip.h"
#include "player.h"
#include "refstuf.h"
#include "textmaps.h"

// and now, for objects
#include "lvldata.h"
#include "tools.h"
#include "cybstrng.h"
#include "gamestrn.h"
#include "gamescr.h"
#include "cit2d.h"
#include "musicai.h"

#include "gr2ss.h"

#define FMK_INT_XX  (FMK_INT_NW||FMK_INT_SW||FMK_INT_EW||FMK_INT_WW)
#define FMK_INT_INT (1<<8)


// actual dimensions of a typical monitor, 
// for calculating pixel ratio.
#define STD_SCR_WID 11
#define STD_SCR_HGT 8

#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3

#define TERR_OBJ_PASS 0
#define REAL_OBJ_PASS 1
#define NOTE_OBJ_PASS 2
#define NUM_OBJ_PASSES 3

#define FIN_SWEEP 0x5800
#define FIN_NOSE  fix_make(0,0xB000)
#define FIN_TAIL  fix_make(0,0x4000)

#define DRAW_MASK_SEEN 0x1
#define DRAW_MASK_RAD  0x2
#define DRAW_MASK_FULL 0x4
#define DRAW_MASK_TERR 0x8
#define DRAW_MASK_SENS 0x10

//#define AMAP_SENS_TILEBOUND
#define AMAP_SENS_CIRCLE

#define CORRECT_PIXEL_RATIO
#define FIX_PIXRATIO

// beware the shifting version.  If pixratio_shf is negative,
// C does not define what happens.  This does not happen to
// occur in any of the screen modes we plan to support, and
// might work right anyway.
//
#ifdef CORRECT_PIXEL_RATIO
#ifdef FIX_PIXRATIO
fix pixratio_yx=FIX_UNIT;
fix pixratio_xy=FIX_UNIT;
//#define coor_to_pix(y) fast_fix_mul(pixratio_yx,(y))	 KLC - Changed this
//#define pix_to_coor(y) fast_fix_mul(pixratio_xy,(y))
#define coor_to_pix(y) fix_mul(pixratio_yx,(y))
#define pix_to_coor(y) fix_mul(pixratio_xy,(y))
#else
int pixratio_shf=0;
#define coor_to_pix(y) ((y)<<pixratio_shf)
#define pix_to_coor(y) ((y)>>pixratio_shf)
#endif

// fix times int can use regular multiply
#define am_vline(x,y0,y1) gr_vline(x,coor_to_pix(y0),coor_to_pix(y1))
#define am_hline(x0,y,x1) gr_hline(x0,coor_to_pix(y),x1)
#define am_rect(x0,y0,x1,y1) gr_rect(x0,coor_to_pix(y0),x1,coor_to_pix(y1))
#define am_int_line(x0,y0,x1,y1) gr_int_line(x0,coor_to_pix(y0),x1,coor_to_pix(y1))
#define am_fix_line(x0,y0,x1,y1) gr_fix_line(x0,coor_to_pix(y0),x1,coor_to_pix(y1))
#define am_int_circle(xc,yc,r) gr_int_circle(xc,coor_to_pix(yc),r)
#else
#define am_vline gr_vline
#define am_hline gr_hline
#define am_rect gr_rect
#define am_int_line gr_int_line
#define am_fix_line gr_fix_line
#define am_int_circle gr_int_circle
#endif

//-------------------
// Prototypes
//-------------------
void amap_version_set(int id, int new_ver);
void obj_draw(int xm, int ym, Obj *cobj, int tsize, int so, int color);
void line_draw(int xm, int ym, Obj *cobj, int tsize, int full, int color);
void obj_mess(curAMap *amptr, MapElem *curmp, int drw, int xm, int ym, int tsize, int pass);
bool wall_seen_p(int wallcode, int csbits, MapElem *cur);
void tile_draw(int xm, int ym, int tiletype, int size, int offs, int color);
void wall_draw(int xm, int ym, int wallcode, int size, MapElem *cur);
void draw_radius_obj(curAMap *amptr, short OtoF, int col, int zeroscrx,int zeroscry,int rad);
void draw_full_obj(curAMap *amptr, short OtoF, int col, int zeroscrx,int zeroscry);
void amap_pixratio_set(fix ratio);
void *amap_loc_to_sq(curAMap *amptr, int *x, int *y);
void *amap_loc_get_note(void *map_sq);
void amap_fixup_existing(int tolera, int delta);
grs_bitmap *screen_automap_bitmap(char which_amap);


// for now, create a default automap thingy
void amap_version_set(int id, int new_ver)
{
   curAMap* amptr=oAMap(id);

   switch (new_ver)
   {
   case -1:
      amptr->flags=AMAP_TRACK_OBJ|AMAP_SHOW_SEC|AMAP_SHOW_CRIT;
      amptr->sensor_rad=0x800;
      break;
   case 0:
   case 1:
      amptr->flags=AMAP_TRACK_OBJ|AMAP_SHOW_SEC|AMAP_SHOW_MSG;
      amptr->sensor_rad=0x400;
      break;
   case 2:
      amptr->flags=AMAP_TRACK_OBJ|AMAP_SHOW_SEC|AMAP_SHOW_ROB|AMAP_SHOW_MSG;
      amptr->sensor_rad=0x600;
      break;
   case 3:
      amptr->flags=AMAP_TRACK_OBJ|AMAP_SHOW_SEC|AMAP_SHOW_CRIT|AMAP_SHOW_HAZ|AMAP_SHOW_MSG;
      amptr->sensor_rad=0x800;
      break;
   }
   amptr->avail_flags=amptr->flags|AMAP_AVAIL_ALWAYS;
   amptr->version_id=new_ver;
}

void automap_init(int version, int id)
{
   curAMap *amptr;
   amptr=oAMap(id);
   amptr->xf=fix_make((MAP_YSIZE>>1)-1,0x8000);
   amptr->yf=fix_make((MAP_YSIZE>>1)-1,0x8000);
   amptr->obj_to_follow=amptr->sensor_obj=PLAYER_OBJ;
   amptr->note_obj=0;
   amptr->zoom=2;
   amptr->lh=grd_bm.h; amptr->lw=grd_bm.w;
   amap_version_set(id,version);
   amptr->init=TRUE;
}

void amap_invalidate(int id)
{
   oAMap(id)->init=FALSE;
}

void amap_settings_copy(curAMap* from, curAMap* to)
{
   to->flags=from->flags;
   to->zoom=from->zoom;
}


#ifdef USE_COMPILED_WALLS
bool wall_seen_p(int wallcode, int csbits, MapElem *cur)
{
   if(textprops[me_tmap_flr(cur)].force_dir==1)
      return 0;

   if (wallcode<FMK_INT_INT) {
	   return csbits&wallcode;
   }
   else
   {
      int mo1, mo2, lb1, lb2, ck1, ck2;
      switch (me_tiletype(cur))
      {
      case TILE_SOLID_NW: mo1=-MAP_XSIZE; mo2= 1; ck1=FMK_NW; ck2=FMK_WW; break;
      case TILE_SOLID_NE: mo1=-MAP_XSIZE; mo2=-1; ck1=FMK_NW; ck2=FMK_EW; break;
      case TILE_SOLID_SE: mo1= MAP_XSIZE; mo2= 1; ck1=FMK_SW; ck2=FMK_WW; break;
      case TILE_SOLID_SW: mo1= MAP_XSIZE; mo2=-1; ck1=FMK_SW; ck2=FMK_EW; break;
      default: return 0;
      }
      lb1=me_clearsolid(cur+mo1);
      lb2=me_clearsolid(cur+mo2);
      return ((csbits&FMK_INT_XX)||(((lb1|lb2)!=0)&&(((lb1&ck1)!=0)||((lb2&ck2)!=0))));
   }
}
#else
#include "fredge.h"
bool wall_seen_p(int wallcode, int csbits, MapElem *cur)
{
   if(textprops[me_tmap_flr(cur)].force_dir)
      return 0;
   if (wallcode==FMK_INT_INT)
   {
      switch (me_tiletype(cur))
      {
      case TILE_SOLID_NW: return 1;
      case TILE_SOLID_NE: return 1;
      case TILE_SOLID_SE: return 1;
      case TILE_SOLID_SW: return 1;
      }
	}
   else {
      switch (get_edge_code(cur,csbits))
      {
      case MEDGE_NO_EGRESS:   return 1;
      case MEDGE_CLIFF_THING: return 2;
      case MEDGE_LARGE_STEP:  return 3;
      case MEDGE_SMALL_STEP:  return 4;
	   }
   }
   return 0;
}
#endif

#ifdef REAL_XIST_CHECK
bool wall_xist_p(int wallcode, int csbits, MapElem *cur)
{
   if (wallcode<FMK_INT_INT)
	   return ((csbits&(wallcode>>4))==0);       // wow, this is super wacky (tm)? punt diags and go with?
   else
   {
      switch (me_tiletype(cur))
      {
      case TILE_SOLID_NW: return 1;
      case TILE_SOLID_NE: return 1;
      case TILE_SOLID_SE: return 1;
      case TILE_SOLID_SW: return 1;
      }
      return 0;
   }
}
#else
#define wall_xist_p(wc,cs,cur) (wallcode<FMK_INT_INT)
#endif

void wall_draw(int xm, int ym, int wallcode, int size, MapElem *cur)
{
   switch (wallcode)
   {
   case FMK_INT_SW: am_hline(xm,ym,xm+size); break; 
   case FMK_INT_NW: am_hline(xm,ym-size,xm+size); break;
   case FMK_INT_WW: am_vline(xm,ym,ym-size); break;
   case FMK_INT_EW: am_vline(xm+size,ym,ym-size); break;
   case FMK_INT_INT:
      if ((me_tiletype(cur)==TILE_SOLID_NW)||(me_tiletype(cur)==TILE_SOLID_SE))
         am_int_line(xm,ym,xm+size,ym-size);
      else
         am_int_line(xm,ym-size,xm+size,ym);
      break;
   }
}

static void tri_draw(int x1, int y1, int x2, int y2, int x3, int y3, int color)
{
   grs_vertex vert[3], *pervert[3];

   vert[0].x=fix_make(x1,0);
   vert[0].y=pix_to_coor(fix_make(y1,0));
   vert[1].x=fix_make(x2,0);
   vert[1].y=pix_to_coor(fix_make(y2,0));
   vert[2].x=fix_make(x3,0);
   vert[2].y=pix_to_coor(fix_make(y3,0));

   pervert[0]=&vert[0];
   pervert[1]=&vert[1];
   pervert[2]=&vert[2];

   gr_poly(color,3,pervert);
}

void tile_draw(int xm, int ym, int tiletype, int size, int offs, int color)
{
   while ((offs>0)&&(size<2*offs)) offs--;
   switch(tiletype) {
      case TILE_SOLID_NW:
         tri_draw(xm+size+1,ym-size+offs-1,xm+size+1,ym+1,xm+offs-1,ym+1,color);
         break;
      case TILE_SOLID_SE:
         tri_draw(xm+offs,ym-size+offs,xm+size,ym-size+offs,xm+offs,ym,color);
         break;
      case TILE_SOLID_SW:
         tri_draw(xm+offs,ym-size+offs,xm+size+1,ym-size+offs,xm+size+1,ym+1,color);
         break;
      case TILE_SOLID_NE:
         tri_draw(xm+offs,ym-size+offs,xm+size+1,ym+1,xm+offs,ym+1,color);
         break;
      default:
         gr_set_fcolor(color);
         am_rect(xm+offs,ym-size+offs,xm+size+1,ym+1);
   }
}

void obj_draw(int xm, int ym, Obj *cobj, int tsize, int so, int color)
{
   xm+=((cobj->loc.x&0xff)*tsize)>>8;
   ym-=((cobj->loc.y&0xff)*tsize)>>8;
   gr_set_fcolor(color);
   if (so<=1) so=1;
   am_rect(xm-so,ym-so,xm+so,ym+so);
}

void line_draw(int xm, int ym, Obj *cobj, int tsize, int full, int color)
{
   if (cobj->loc.h&~0xc0) return;
   gr_set_fcolor(color);
   if (cobj->loc.h&0x40)   // ns
   {
      if (cobj->loc.x==0xff) xm+=tsize+1;
      else                   xm+=((cobj->loc.x&0xff)*tsize)>>8;
      if (full)
	      am_vline(xm,ym,ym-tsize);
      else
         am_vline(xm,ym-(tsize>>2),ym-3*(tsize>>2));
   }
   else
   {
	   if (cobj->loc.y==0xff) ym-=tsize+1;
      else                   ym-=((cobj->loc.y&0xff)*tsize)>>8;
      if (full)
	      am_hline(xm,ym,xm+tsize);
      else
	      am_hline(xm+(tsize>>2),ym,xm+3*(tsize>>2));
   }
}

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)<(b)?(b):(a))

// if objs, look for creatures, security systems, messages
// need to add bridges... perhaps something for energy, switches and levers
// works in three sorting passes: terrain type stuff (e.g., doors),
// other "real" objects, and map notes and stuff.
void obj_mess(curAMap *amptr, MapElem *curmp, int drw, int xm, int ym, int tsize, int pass)
{
   Obj *cobj;
   ObjID cobjid;
   ObjRefID curORef;
   int col, px, md;
   short w, h;
   char buf[50];
   grs_font* fon;

   curORef=curmp->objRef;
   while (curORef!=OBJ_REF_NULL)
   {
      cobjid=objRefs[curORef].obj;
      if ((amptr->flags&AMAP_TRACK_OBJ)&&(cobjid==amptr->obj_to_follow))
       { curORef = objRefs[curORef].next; continue; }
      if (CitrefCheckHomeSq(curORef))
      {  // this is the home square... so do stuff
         cobj=&objs[cobjid];
         px=ObjProps[OPNUM(cobjid)].physics_xr;
         px=(px<<amptr->zoom)>>6;
         switch (cobj->obclass)
         {     // critters, doors, elevators, or do we do that with music bits, yes!!! special
         case CLASS_CRITTER:
            if(pass!=REAL_OBJ_PASS) break;
            if ((drw&(DRAW_MASK_FULL|DRAW_MASK_RAD))&&(amptr->flags&(AMAP_SHOW_CRIT|AMAP_SHOW_ROB)))
            {
               static uchar base_col[3]={PURPLE_8_BASE+7,METALBLUE_8_BASE+7,BRIGHTBROWN_8_BASE+7};
               uchar col;

               if ((cobj->subclass!=CRITTER_SUBCLASS_ROBOT)&&((amptr->flags&AMAP_SHOW_CRIT)==0))
                  break;
               if (cobj->subclass<3) col=base_col[cobj->subclass]; else col=GRAY_8_BASE;
                  // pick color and intensity based on creature type and all
               if (cobj->info.type<5) col-=cobj->info.type; else col-=5;
               // okay, this is a hack, but hey, I felt sorry for the
               // poor guys.
               if(ID2TRIP(cobjid)!=ASSASSIN_TRIPLE)
                  obj_draw(xm,ym,cobj,tsize,px,col);
            }
            break;
         case CLASS_DOOR:
            if(pass!=TERR_OBJ_PASS) break;
            // perhaps only draw physics doors???
            if ((ObjProps[OPNUM(cobjid)].flags & TERRAIN_OBJECT)!=0)
               if (drw&DRAW_MASK_SEEN)    // used to do FULL too
                  if ((cobj->loc.p|cobj->loc.b)==0)      // if pitched or banked, hit the road
                  {
   	               int col=MAIZE_8_BASE;
                     int trip=ID2TRIP(cobjid);
   	               if ((trip>=SECRET_DOOR1_TRIPLE)&&(trip<=SECRET_DOOR3_TRIPLE)) col=GREEN_BASE+2;
                     if (USE_MODE(cobjid)==NULL_USE_MODE) col=GREEN_BASE+2;
//                     if (trip==INVISO_DOOR_TRIPLE) col=AQUA_8_BASE+2;
                     line_draw(xm,ym,cobj,tsize,cobj->info.current_frame>0?0:1,col);   // if frame, then open, else closed
                  }
            break;
         default: // special
            switch(ID2TRIP(cobjid))
            {  // security cameras, automap notes
            case LARGCPU_TRIPLE:
            case CAMERA_TRIPLE:
               if(pass!=REAL_OBJ_PASS) break;
               if ((drw&DRAW_MASK_SEEN)&&(amptr->flags&AMAP_SHOW_SEC))
               {
                  px = tsize >> 3;
                  obj_draw(xm,ym,cobj,tsize,px,RED_8_BASE + 5);
               }
               break;
            case MAPNOTE_TRIPLE:
               if(pass!=NOTE_OBJ_PASS) break;
               if (amptr->flags&AMAP_SHOW_MSG)
               {
                  if (amptr->note_obj==cobjid)  { px=tsize>>1; col=2; }
                  else                          { px=tsize>>2; col=6; }
                  obj_draw(xm,ym,cobj,tsize,px,AQUA_8_BASE+col);

                  if(amptr->flags&AMAP_FULL_MSG) {
                     strncpy(buf,amap_note_string(cobjid),49);
                     buf[49]=0;
                     fon=gr_get_font();
                     gr_set_font((grs_font *)ResLock(RES_tinyTechFont));
                     md=tsize-2;
                     wrap_text(buf,md);
                     gr_string_size(buf,&w,&h);
                     if(h>md) {
                        // square up text iteratively
                        md=(w+h)/2;
                        unwrap_text(buf);
                        wrap_text(buf,md);
                        gr_string_size(buf,&w,&h);
                        if(abs(w-h)>4) {
                           md=(w+h)/2;
                           unwrap_text(buf);
                           wrap_text(buf,md);
                        }
                     }
                     gr_set_fcolor(BLACK+1);
#ifdef SVGA_SUPPORT
                     {
                        extern bool shadow_scale;
                        shadow_scale = FALSE;
#endif
#ifdef CORRECT_PIXEL_RATIO
                        draw_shadowed_string(buf,xm+1,coor_to_pix(ym-tsize+1),AQUA_8_BASE+col);
#else
                        draw_shadowed_string(buf,xm+1,ym-tsize+1,AQUA_8_BASE+col);
#endif
#ifdef SVGA_SUPPORT
                        shadow_scale = TRUE;
                     }
#endif
                     ResUnlock(RES_tinyTechFont);
                     gr_set_font(fon);
                  }
               }
               break;
            case ENRG_CHARGE_TRIPLE:
            case CYB_TERM_TRIPLE:
            default:
               break;
            }
            break;
         }
      }  // end of in home square
      curORef = objRefs[curORef].next;
   }  // end of ORef loop
}  // end of objs loop


void draw_radius_obj(curAMap *amptr, short OtoF, int col, int zeroscrx,int zeroscry,int rad)
{
   int xm, ym;

   xm=zeroscrx+((objs[OtoF].loc.x<<amptr->zoom)>>8);
   ym=zeroscry-((objs[OtoF].loc.y<<amptr->zoom)>>8);

   gr_set_fcolor(col);
   am_int_circle(xm,ym,(rad<<amptr->zoom)>>8);
}

void draw_full_obj(curAMap *amptr, short OtoF, int col, int zeroscrx,int zeroscry)
{
   int hd=objs[amptr->obj_to_follow].loc.h;     // should add pitch and bank, really
   int xm, ym;
   fix tx,ty,lx,ly,rx,ry;
   fix csin,ccos;

   xm=zeroscrx+((objs[OtoF].loc.x<<amptr->zoom)>>8);
   xm=fix_make(xm,0);
   ym=zeroscry-((objs[OtoF].loc.y<<amptr->zoom)>>8);
   ym=fix_make(ym,0);

   hd=(((hd-64)+256)&0xff)<<8;

   fix_sincos(hd,&csin,&ccos);
   ty=ym+fast_fix_mul(csin,FIN_NOSE<<amptr->zoom);
   tx=xm+fast_fix_mul(ccos,FIN_NOSE<<amptr->zoom);

   fix_sincos((hd+(FIN_SWEEP))&0xffff,&csin,&ccos);
   ly=ym+fast_fix_mul(csin,FIN_TAIL<<amptr->zoom);
   lx=xm+fast_fix_mul(ccos,FIN_TAIL<<amptr->zoom);

   fix_sincos((hd+(0x10000-FIN_SWEEP))&0xffff,&csin,&ccos);
   ry=ym+fast_fix_mul(csin,FIN_TAIL<<amptr->zoom);
   rx=xm+fast_fix_mul(ccos,FIN_TAIL<<amptr->zoom);

   gr_set_fcolor(col);

   am_fix_line(tx,ty,lx,ly);
   am_fix_line(lx,ly,xm,ym);
   am_fix_line(xm,ym,rx,ry);
   am_fix_line(rx,ry,tx,ty);
}


#ifdef AMAP_SENS_TILEBOUND
static char facecheck[] = { (1<<NORTH)|(1<<WEST), (1<<NORTH)|(1<<EAST),
                            (1<<SOUTH)|(1<<EAST), (1<<SOUTH)|(1<<WEST) };
#endif

#ifdef CORRECT_PIXEL_RATIO
void amap_pixratio_set(fix ratio)
{
   if(ratio==0)
      ratio=fix_make(grd_screen_canvas->bm.h,0)*STD_SCR_WID/(grd_screen_canvas->bm.w*STD_SCR_HGT);

#ifdef FIX_PIXRATIO
   pixratio_yx=ratio;
   pixratio_xy=fix_div(FIX_UNIT,ratio);
#else
   // note once again that this only works if pixratio_shf is only
   // ever intended to be positive.
   pixratio_shf=0;
   while(ratio>FIX_UNIT) {
      ratio=ratio>>1;
      pixratio_shf++;
   }
   if(ratio<((FIX_UNIT*707)/1000)) {  // root(2)/2, or half a shift
      pixratio_shf--;
   }
#endif
}
#endif

void amap_draw(curAMap *amptr, int expose)
{
   int xc,yc,xm,ym,drw,static_drw,cv;  // loop control, so on
   int zeroscrx,zeroscry,init_yc;      // x and y screen coordinate for 0,0 of map
   int tsize=1<<amptr->zoom;
#ifdef AMAP_SENS_TILEBOUND
   int facemask;
#endif
   int mt, pass;
   ushort sensor_x,sensor_y;
   MapElem *curmp=MAP_GET_XY(0,0);
   fix amrh, amrw;
   int max_xc,max_yc,xbase,crnr_x,crnr_y;             // am w and h radius
   MapElem *mapybase, *init_mapybase;

   if (amptr->flags&AMAP_TRACK_OBJ)
   {
      amptr->xf=objs[amptr->obj_to_follow].loc.x<<8;
      amptr->yf=fix_make(MAP_YSIZE,0)-1-(objs[amptr->obj_to_follow].loc.y<<8);
   }

   amptr->lw=grd_bm.w; amptr->lh=grd_bm.h;

   zeroscrx=(grd_bm.w>>1)-fix_int(amptr->xf<<amptr->zoom);
   zeroscry=pix_to_coor(grd_bm.h>>1)-fix_int(amptr->yf<<amptr->zoom);
   zeroscry=zeroscry+(MAP_YSIZE<<amptr->zoom);
   if (amptr->sensor_obj!=OBJ_NULL)
	 { sensor_x=objs[amptr->sensor_obj].loc.x; sensor_y=objs[amptr->sensor_obj].loc.y; }
   else
      sensor_x=sensor_y=0;

   amrw=(grd_bm.w<<15)>>amptr->zoom;      // w radius of amap, in fix point tiles
   amrh=pix_to_coor((grd_bm.h<<15)>>amptr->zoom);  // h radius of amap, in fix point tiles
   xc=fix_int(amptr->xf-amrw);
   max_xc=1+fix_int(amptr->xf+amrw);
   yc=MAP_YSIZE-(1+fix_int(amptr->yf+amrh));
   max_yc=MAP_YSIZE-fix_int(amptr->yf-amrh);
   if (xc<0) xc=0; if (max_xc>=MAP_XSIZE) max_xc=MAP_XSIZE-1;
   if (yc<0) yc=0; if (max_yc>=MAP_XSIZE) max_yc=MAP_XSIZE-1;
   xbase=xc;
   crnr_x=zeroscrx+(xc<<amptr->zoom);
   crnr_y=zeroscry-(yc<<amptr->zoom);
   mapybase=curmp+(yc*MAP_XSIZE)+xc;

//   mprintf("rect %d %d and %d %d, crnr %d %d from rw and rh %.2q %.2q cnt %.2q %.2q....\n",xc,yc,max_xc,max_yc,crnr_x,crnr_y,amrw,amrh,amptr->xf,amptr->yf);

//   mprintf("Zero at %d %d...\n",zeroscrx,zeroscry);

   if (expose)
      gr_clear(0xFF);
   static_drw=0;
   if (amptr->flags&AMAP_SHOW_ALL)
      static_drw|=DRAW_MASK_FULL|DRAW_MASK_TERR;
   else if (amptr->flags&AMAP_SHOW_FLR)
      static_drw|=DRAW_MASK_TERR;
   if(amptr->flags&AMAP_SHOW_SENS)
      static_drw|=DRAW_MASK_SENS;
   gr_set_fcolor(GREEN_BASE+2);
   init_mapybase=mapybase; init_yc=yc;

   // draw hazard/elevator floor colors BEFORE drawing walls.
   for (ym=crnr_y;yc<max_yc;yc++,ym-=tsize,mapybase+=MAP_XSIZE)
      for (curmp=mapybase,xc=xbase,xm=crnr_x;xc<max_xc;xc++,xm+=tsize,curmp++) {
         drw=static_drw;
         if (me_bits_seen(curmp))
            drw|=DRAW_MASK_SEEN;
         if (fix_fast_pyth_dist((xc<<8)+0x80-sensor_x,(yc<<8)+0x80-sensor_y)<amptr->sensor_rad)
            drw|=DRAW_MASK_RAD;

         if (((mt=me_tiletype(curmp))!=TILE_SOLID)&&(drw!=0))
         {
            if (drw&(DRAW_MASK_SEEN|DRAW_MASK_RAD))
            {
               // if hazard or elevator, floor draw
               // check music bits for elevators!
               if (me_bits_music(curmp)==ELEVATOR_ZONE)
                 tile_draw(xm,ym,mt,tsize,1,BROWN_8_BASE+4);
               if (amptr->flags&AMAP_SHOW_HAZ)
               {              // bio, rad, both
                  static uchar col_map[]={YELLOW_8_BASE+6,RED_8_BASE+6,ORANGE_8_BASE+5};
                  int hv=0;
                  if (level_gamedata.hazard.zerogbio==0)
	                  if (me_hazard_bio_x(curmp)) hv=1;
                  if (me_hazard_rad_x(curmp)) hv|=2;
                  if (hv)
                     tile_draw(xm,ym,(drw&DRAW_MASK_SEEN)?mt:TILE_OPEN,tsize,1,col_map[hv-1]);
               }
	         }
         }
      }
   mapybase=init_mapybase; yc=init_yc;
   // now draw walls and such
   for (ym=crnr_y;yc<max_yc;yc++,ym-=tsize,mapybase+=MAP_XSIZE)
      for (curmp=mapybase,xc=xbase,xm=crnr_x;xc<max_xc;xc++,xm+=tsize,curmp++)
      {
         drw=static_drw;
         if (me_bits_seen(curmp))
            drw|=DRAW_MASK_SEEN;
         if (fix_fast_pyth_dist((xc<<8)+0x80-sensor_x,(yc<<8)+0x80-sensor_y)<amptr->sensor_rad)
            drw|=DRAW_MASK_RAD;

         if ((me_tiletype(curmp)!=TILE_SOLID)&&(drw!=0))
         {
            int csbits=me_clearsolid(curmp), loop;

#ifndef REAL_XIST_CHECK
            if (drw&DRAW_MASK_TERR)
            {
               gr_set_fcolor(GREEN_BASE+9);
               wall_draw(xm,ym,FMK_INT_NW,tsize,curmp);
               wall_draw(xm,ym,FMK_INT_EW,tsize,curmp);
               wall_draw(xm,ym,FMK_INT_SW,tsize,curmp);
               wall_draw(xm,ym,FMK_INT_WW,tsize,curmp);
               gr_set_fcolor(GREEN_BASE+2);
            }
#endif

            gr_set_fcolor(GREEN_BASE+2);

#ifdef USE_COMPILED_WALLS
            for (loop=(1<<4); loop<=FMK_INT_INT; loop<<=1)
               if (wall_seen_p(loop,csbits,curmp))     put back in later.
                  wall_draw(xm,ym,loop,tsize,curmp);
#else
            for (loop=(1<<4), csbits=0; loop<=FMK_INT_INT; csbits++, loop<<=1)
               if ((drw&DRAW_MASK_SEEN)&&(cv=wall_seen_p(loop,csbits,curmp)))
               {  // colors are gb+2,5,8 for wall,cliff,bigstep
                  gr_set_fcolor(GREEN_BASE+2+(2*(cv-1)));
                  wall_draw(xm,ym,loop,tsize,curmp);
               }
#endif

#ifdef REAL_XIST_CHECK
               else if (amptr->flags&AMAP_SHOW_ALL)
                  if (wall_xist_p(loop,csbits,curmp))
                  {
                     gr_set_fcolor(GREEN_BASE+9);
                     wall_draw(xm,ym,loop,tsize,curmp);
                     gr_set_fcolor(GREEN_BASE+2);
                  }
#endif

         }  // if !tile_solid
#ifdef AMAP_SENS_TILEBOUND
         if((drw & DRAW_MASK_RAD) && (drw & DRAW_MASK_SENS)) {
            if (fix_fast_pyth_dist((xc<<8)+0x80-sensor_x,(yc<<8)+0x80-sensor_y)+(1<<8)>=amptr->sensor_rad) { 
               facemask=0;
   
               if(fix_fast_pyth_dist((xc<<8)+0x80-sensor_x,((yc+1)<<8)+0x80-sensor_y)>=amptr->sensor_rad)
                  facemask|=(1<<NORTH);
               if(fix_fast_pyth_dist((xc<<8)+0x80-sensor_x,((yc-1)<<8)+0x80-sensor_y)>=amptr->sensor_rad)
                  facemask|=(1<<SOUTH);
               if(fix_fast_pyth_dist((xc+1<<8)+0x80-sensor_x,(yc<<8)+0x80-sensor_y)>=amptr->sensor_rad)
                  facemask|=(1<<EAST);
               if(fix_fast_pyth_dist((xc-1<<8)+0x80-sensor_x,(yc<<8)+0x80-sensor_y)>=amptr->sensor_rad)
                  facemask|=(1<<WEST);

               if(facemask) {
                  gr_set_fcolor(GRAY_8_BASE+3);
                  if(drw&DRAW_MASK_SEEN) {
                     mt=me_tiletype(curmp);
                     if(mt>=TILE_SOLID_NW && mt<=TILE_SOLID_SW) {
                        mt-=TILE_SOLID_NW;
                        if((facemask & facecheck[mt])==facecheck[mt]) {
                           wall_draw(xm,ym,FMK_INT_INT,tsize,curmp);
                           facemask^=facecheck[mt];
                        }
                     }
                  }

                  if(facemask & (1<<NORTH))
                     wall_draw(xm,ym,FMK_INT_NW,tsize,curmp);
                  if(facemask & (1<<SOUTH))
                     wall_draw(xm,ym,FMK_INT_SW,tsize,curmp);
                  if(facemask & (1<<EAST))
                     wall_draw(xm,ym,FMK_INT_EW,tsize,curmp);
                  if(facemask & (1<<WEST))
                     wall_draw(xm,ym,FMK_INT_WW,tsize,curmp);
               }
            }
         }
#endif
      }  // for y loop
   
   for(pass=0;pass<NUM_OBJ_PASSES;pass++) {
      mapybase=init_mapybase; yc=init_yc;
      for (ym=crnr_y;yc<max_yc;yc++,ym-=tsize,mapybase+=MAP_XSIZE)
         for (curmp=mapybase,xc=xbase,xm=crnr_x;xc<max_xc;xc++,xm+=tsize,curmp++)
         {
            drw=static_drw;
            if (me_bits_seen(curmp))
               drw|=DRAW_MASK_SEEN;
            if (fix_fast_pyth_dist((xc<<8)+0x80-sensor_x,(yc<<8)+0x80-sensor_y)<amptr->sensor_rad)
               drw|=DRAW_MASK_RAD;

	         obj_mess(amptr,curmp,drw,xm,ym,tsize,pass);
         }
   }

   // really should be in a "track_player" mode, not always...
   draw_full_obj(amptr,PLAYER_OBJ,RED_BASE+3,zeroscrx,zeroscry);
   if ((amptr->flags&AMAP_TRACK_OBJ)&&(amptr->obj_to_follow!=PLAYER_OBJ))
      draw_full_obj(amptr,amptr->obj_to_follow,PURPLE_8_BASE+3,zeroscrx,zeroscry);
#ifdef AMAP_SENS_CIRCLE
   if(drw & DRAW_MASK_SENS) {
      int r=amptr->sensor_rad;
      // accound for needing to see the center of square and
      // the fact that we're an octagon, not a circle.
      r = (r*89)/100 - (1<<8)*707/1000; 
      draw_radius_obj(amptr,PLAYER_OBJ,GRAY_8_BASE,zeroscrx,zeroscry,r);
      r=amptr->sensor_rad + (1<<8)*707/1000;
      draw_radius_obj(amptr,PLAYER_OBJ,GRAY_8_BASE+4,zeroscrx,zeroscry,r);
   }
#endif
}


// x and y are window relative, return the map square?
// returns NULL for not in map, else a mapelem *
// this can be checked for mapnotes, or one can be added
// functions to do these things exist as well
void *amap_loc_to_sq(curAMap *amptr, int *x, int *y)
{
   int offsx, offsy;
   MapElem *curmp=MAP_MAP;
   fix tmpy;
   
   if (amptr->flags&AMAP_TRACK_OBJ)
   {
      amptr->xf=objs[amptr->obj_to_follow].loc.x<<8;
      amptr->yf=fix_make(MAP_YSIZE,0)-1-(objs[amptr->obj_to_follow].loc.y<<8);
   }
   tmpy=fix_make(MAP_YSIZE,0)-1-amptr->yf;

   offsx= (*x)-(amptr->lw>>1)+fix_int(amptr->xf<<amptr->zoom);
   offsy=-*y+pix_to_coor(amptr->lh>>1)+fix_int(tmpy<<amptr->zoom);

   offsx>>=amptr->zoom;
   offsy>>=amptr->zoom;
   *x=offsx; *y=offsy;     // sneaky sneaky
   if ((offsx&(~((1<<MAP_XSHF)-1)))|(offsy&(~((1<<MAP_YSHF)-1))))
      return NULL;
   else
	   return (void *)(curmp+offsx+(offsy<<MAP_XSHF));
}

// this void star is really an Obj
void *amap_loc_get_note(void *map_sq)
{
   ObjID cobjid;
   ObjRefID curORef;
   MapElem *curmp=(MapElem *)map_sq;

   curORef=curmp->objRef;
   while (curORef!=OBJ_REF_NULL)
   {
      cobjid=objRefs[curORef].obj;
      if (CitrefCheckHomeSq(curORef))
         if (ID2TRIP(cobjid)==MAPNOTE_TRIPLE)
            return (void *)cobjid;
      curORef = objRefs[curORef].next;
   }  // end of ORef loop
   return NULL;
}

#define MAP_LOOK_AROUND
// sets to_do to AMAP_OFF_MAP, AMAP_HAVE_NOTE, AMAP_NO_NOTE
// returns MapElem of the square if NO_NOTE
// returns NULL if OFF_MAP, note this line was shorter than the others
// returns Obj of the map_note if HAVE_NOTE
void *amap_loc_note_check(curAMap *amptr, int *x, int *y, int *to_do)
{
   void *map_note;
   MapElem *curmp= (MapElem *)amap_loc_to_sq(amptr,x,y);
   if (curmp==NULL)
   {
      *to_do=AMAP_OFF_MAP;
      return NULL;
   }
#ifdef MAP_LOOK_AROUND
   map_note=amap_loc_get_note(curmp);
   if (map_note==NULL)
   {  // check around, in the traditional way - big zoom = small map
   	int extloop, inloop, clen, dvec[2]={0,1}, rad=2*(3-amptr->zoom), mx=*x, my=*y;
//      mprintf("Looking around %d from %x %x dv %d %d\n",rad,mx,my,dvec[0],dvec[1]);
      for (clen=1; clen<rad; clen++) // for each radius
			for (extloop=0; extloop<2; extloop++) // two of each
			{
				for (inloop=0; inloop<clen; inloop++)
				{
					mx+=dvec[0]; my+=dvec[1];
					if (((mx>=0)&&(mx<MAP_XSIZE))&&((my>=0)&&(my<MAP_YSIZE)))
               {
                  map_note=amap_loc_get_note(MAP_GET_XY(mx,my));
						if (map_note!=NULL)
                   { *x=mx; *y=my; goto hack_breakout; }
					}
				}
				if (dvec[0]!=0) { dvec[1]= -dvec[0]; dvec[0]=0; }
				else 				 { dvec[0]= dvec[1] ; dvec[1]=0; }
			}
   }
hack_breakout:
#else
   map_note=amap_loc_get_note(curmp);
#endif
   if (map_note!=NULL)
   {     // a note is there...
      *to_do=AMAP_HAVE_NOTE;
      return map_note; 
   }
   else
   {
	   *to_do=AMAP_NO_NOTE;
	   return (void *)curmp;
   }
}

bool amap_flags(curAMap *amptr, int flags, int set)
{
   flags&=amptr->avail_flags;
   if (flags==0) return FALSE;
   switch (set)
   {
   case AMAP_SET: amptr->flags|=flags; break;
   case AMAP_UNSET: amptr->flags&=~flags; break;
   case AMAP_TOGGLE:
      if (amptr->flags&flags)
         amptr->flags&=~flags;
      else
         amptr->flags|=flags;
      break;
   }
   return TRUE;
}

bool amap_zoom(curAMap *amptr, bool set, int zoom_delta)
{
   if (set)
      amptr->zoom=zoom_delta;
   else
   {
      if (zoom_delta>0)
       { if (amptr->zoom+zoom_delta>=AMAP_MAX_ZOOM) return FALSE; }
      else
       { if (amptr->zoom+zoom_delta<AMAP_MIN_ZOOM) return FALSE; }
      amptr->zoom+=zoom_delta;
   }
   return TRUE;
}

void amap_pan(curAMap *amptr, int dir, int* dist)
{
   int d;
   d=*dist>>amptr->zoom;
   *dist-=d<<amptr->zoom;
   switch (dir)
   {
   case AMAP_PAN_E: amptr->xf+=d; break;
   case AMAP_PAN_W: amptr->xf-=d; break;
   case AMAP_PAN_N: amptr->yf-=d; break;
   case AMAP_PAN_S: amptr->yf+=d; break;
   }
   amptr->flags&=~AMAP_TRACK_OBJ;
}

void *amap_deal_with_map_click(curAMap *amptr, int *x, int *y)
{
   int todo;
   void *datum;
   // actually clicked on the map, we should deal...
   datum=amap_loc_note_check(amptr,x,y,&todo);
   switch (todo)
   {
   case AMAP_NO_NOTE:   amptr->note_obj=NULL; return datum;
   case AMAP_HAVE_NOTE: amptr->note_obj=(int)datum; return AMAP_NOTE_HACK_PTR;
	}
//   case AMAP_OFF_MAP:   return NULL;
   return NULL;
}


// string hacks
char amap_strings[AMAP_STRING_SIZE];
char *amap_str_ptr=&amap_strings[0];

char *amap_str_next(void)
{
   return amap_str_ptr;
}

void amap_str_grab(char *str)
{
   amap_str_ptr=str+strlen(str)+1;
}

int amap_str_deref(char *str)
{
   return str-&amap_strings[0];  // get offset into string cluster
}

char *amap_str_reref(int offs)
{
   return (&amap_strings[0])+offs;  // go offset into string cluster
}

void amap_str_startup(int magic_num)
{
   amap_str_ptr = &amap_strings[0] + magic_num;
}


void amap_fixup_existing(int tolera, int delta)
{
   ObjSpecID pmo;
   ObjID cur_obj;
   for (pmo = objTraps[OBJ_SPEC_NULL].id; pmo != OBJ_SPEC_NULL; pmo = objTraps[pmo].next)
   {
      cur_obj=objTraps[pmo].id;
      if (ID2TRIP(cur_obj)==MAPNOTE_TRIPLE)
         if (amap_note_value(cur_obj)>tolera)
            amap_note_value(cur_obj)-=delta;
   }
}

// simply recompact
void amap_str_delete(char *toast_str)
{  
   int del_len=strlen(toast_str)+1;        // how much to delete
   char *s=toast_str+del_len;              // beginning of real data
   int recompact_len=amap_str_ptr-s;       // how much to copy around
   if (s==amap_str_ptr)
      amap_str_ptr=toast_str;              // we are freeing the last created string
   else
   {
      LG_memmove(toast_str,s,recompact_len);  // move over the data
      amap_str_ptr-=del_len;               // move the next pointer back
      amap_fixup_existing(amap_str_deref(toast_str),del_len); // set current notes up right...
   }
}

bool amap_get_note(curAMap *amptr, char *buf)
{
   bool retval = TRUE;
// later, do this for real
// ie base on the string stuff
#ifdef USE_OBJ
   if (amptr->note_obj!=NULL)
   {
	   strcpy(buf,"map note 0000");
      buf[9] ='0'+(((int)amptr->note_obj)/1000)%10;
      buf[10]='0'+(((int)amptr->note_obj)/100)%10;
      buf[11]='0'+(((int)amptr->note_obj)/10)%10;
      buf[12]='0'+(((int)amptr->note_obj)%10);
   }
#else
   if (amptr->note_obj!=NULL)
      strcpy(buf,amap_note_string(amptr->note_obj));
#endif
   else
   {
      retval = FALSE;
      strcpy(buf,get_temp_string(REF_STR_NoMapMessage));
   }
   return retval;
}

grs_bitmap *screen_automap_bitmap(char)
{
   extern grs_bitmap *static_bitmap;
   return(static_bitmap);
}
