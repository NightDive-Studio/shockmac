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
 * $Source: r:/prj/cit/src/RCS/gameobj.c $
 * $Revision: 1.107 $
 * $Author: dc $
 * $Date: 1994/11/25 16:58:15 $
 */

#include <stdlib.h>
#include <string.h> // for memset

#include "frcamera.h"
#include "gameobj.h"

#include "fr3d.h"
#include "frintern.h"
#include "frtables.h"

#include "faketime.h"
#include "hud.h"
#include "player.h"
#include "colors.h"
#include "tilemap.h"
#include "map.h"
#include "mapflags.h"
#include "citres.h"
#include "objsim.h"
#include "objprop.h"
#include "objclass.h"
#include "render.h"
#include "hudobj.h"
#include "otrip.h"
#include "init.h"  // for MAX_MODELS
#include "textmaps.h"
#include "gettmaps.h"
#include "objbit.h"

#include "frflags.h"

#include "ice.h"

#include "cybmem.h"
#include "rcolors.h"
#include "models.h"
#include "objload.h"

#include "ai.h"

#ifdef DOOM_EMULATION_MODE
#include "diffq.h"
#endif

#ifdef NEW_2d
#include "double.h"
#endif

#include "objmode.h"

#define VOXEL_PIX_DIST_BASE (fix_make(0,0x1000))
#define VOXEL_PIX_DIST_DELTA (fix_make(0,0x6000))
// this is obfuscated to put it mildly
#define VOXEL_PIX_SIZE_BASE (fix_make(0,0x100))
#define VOXEL_PIX_SIZE_DELTA (fix_make(0,0xE00))
#define VOXEL_DEPTH     16

#define MAX_VCOLORS  NUM_SLOW_VCOLORS

// should get this into headers
extern grs_bitmap *get_text_bitmap_obj(ObjID cobjid, char dest_type, char *scale);
extern Ref ref_from_critter_data(ObjID oid, int triple, ubyte posture, ubyte frame, ubyte view);

extern int     fr_n_3d_models;

extern MapElem *_fdt_mptr;

static int _o_rad;
static g3s_vector _fr_p;
static Obj *_fr_cobj;
static int _sq_lght;

char curr_clut_table = 0;

#define height_step fix_make(0,0x010000>>SLOPE_SHIFT)

// prototypes
int munge_val(int val, int range, int delta);
void _fr_draw_parm_cube(grs_bitmap *side_bm, grs_bitmap *oth_bm, int x, int y, int z);
void _fr_draw_poly_cube(int p_color, int x, int y, int z);
void _fr_draw_polyobj(void *model_ptr, bool use_lighting);
void gen_seed_vec(g3s_vector *gpt_vec, int seed, int scale, int deviant);
void do_xplodamatron(int frame, int severity, int seed, int col1, int col2);
void gen_tetra(g3s_phandle *xplo_pts, fix size, int deviant, int color);
void draw_ice(void);
void draw_ice_wall(void);
void _fr_draw_tmtile(grs_bitmap *draw_bm, int col_val, g3s_phandle *plst, bool dblface, bool use_lighting);
void _fr_draw_bitmap(grs_bitmap *draw_bm, int dist, int sc, int anch_x, int anch_y);
short compute_3drep(Obj *cobj, ObjID cobjid, int obj_type);


int munge_val(int val, int range, int delta)
{
   int base=range-delta;
   if (val<=delta) base=0; else if (val>=range-delta) base=range-2*delta;
   val=(val+base+(rand()%(2*delta+1)))&0xff;
   return val;
}

#define PARM_MAX  (0x0f8)
#define PARM_MOD  (PARM_MAX<<1)
#define PARM_SHF  (10)
//#define PARM_BASE (64)
#define PARM_BASE (16)

#define LIGHT_3D_OBJS

#define setup_face(a,b,c,d) \
 cface[0]=cube_pt[a]; cface[0]->uv.u=0;     cface[0]->uv.v=0;     \
 cface[1]=cube_pt[b]; cface[1]->uv.u=0x100; cface[1]->uv.v=0;     \
 cface[2]=cube_pt[c]; cface[2]->uv.u=0x100; cface[2]->uv.v=0x100; \
 cface[3]=cube_pt[d]; cface[3]->uv.u=0;     cface[3]->uv.v=0x100 

#define setup_rface(a,b,c,d) \
 cface[0]=cube_pt[a]; cface[0]->uv.u=0x100; cface[0]->uv.v=0;     \
 cface[1]=cube_pt[b]; cface[1]->uv.u=0x0;   cface[1]->uv.v=0x000; \
 cface[2]=cube_pt[c]; cface[2]->uv.u=0;     cface[2]->uv.v=0x100; \
 cface[3]=cube_pt[d]; cface[3]->uv.u=0x100; cface[3]->uv.v=0x100

//#pragma disable_message(202)
void _fr_draw_parm_cube(grs_bitmap *side_bm, grs_bitmap *oth_bm, int x, int y, int z)
{
   g3s_phandle cube_pt[8], cface[4];
   g3s_vector  cube_vec;
   int cur_ft;

   g3_start_object_angles_xyz(&_fr_p,_fr_cobj->loc.p<<8,_fr_cobj->loc.h<<8,_fr_cobj->loc.b<<8,ANGLE_ORDER);
   cube_vec.gX=-x; cube_vec.gY=0; cube_vec.gZ=-y;
#ifdef NO_ANTIGRAV_CRATES
   cube_vec.gY+=_o_rad;
#endif
   cube_pt[0]=g3_transform_point(&cube_vec);     cube_pt[0]->p3_flags|=PF_U|PF_V;
#ifdef NO_ANTIGRAV_CRATES
   cube_vec.gY-=_o_rad;
#endif
   _fdt_pbase=0; cube_pt[0]->i=_sq_lght;  // _fr_do_light(cube_pt[0],FRPTSZFLR_DN); 
   x+=x; y+=y; z=-(z+z);       // go from radius to real
   cube_pt[1]=g3_copy_add_delta_z(cube_pt[0],y); cube_pt[1]->p3_flags|=PF_U|PF_V;
   cube_pt[2]=g3_copy_add_delta_x(cube_pt[1],x); cube_pt[2]->p3_flags|=PF_U|PF_V;
   cube_pt[3]=g3_copy_add_delta_x(cube_pt[0],x); cube_pt[3]->p3_flags|=PF_U|PF_V;
   cube_pt[4]=g3_copy_add_delta_y(cube_pt[0],z); cube_pt[4]->p3_flags|=PF_U|PF_V;
   cube_pt[5]=g3_copy_add_delta_y(cube_pt[1],z); cube_pt[5]->p3_flags|=PF_U|PF_V;
   cube_pt[6]=g3_copy_add_delta_y(cube_pt[2],z); cube_pt[6]->p3_flags|=PF_U|PF_V;
   cube_pt[7]=g3_copy_add_delta_y(cube_pt[3],z); cube_pt[7]->p3_flags|=PF_U|PF_V;
#ifdef LIGHT_3D_OBJS
   cur_ft=gr_get_fill_type();
   if (cur_ft!=FILL_SOLID)
   {
      gr_set_fill_type(FILL_CLUT);
      gr_set_fill_parm(_fr_clut_list[curr_clut_table]+(cube_pt[0]->i&0xf00));
   }
#endif
   setup_face(0,3,2,1);  g3_draw_tmap(4,cface,oth_bm);
   setup_rface(7,4,5,6); g3_draw_tmap(4,cface,oth_bm);
   setup_face(4,7,3,0);  g3_draw_tmap(4,cface,side_bm);
   setup_face(7,6,2,3);  g3_draw_tmap(4,cface,side_bm);
   setup_face(6,5,1,2);  g3_draw_tmap(4,cface,side_bm);
   setup_face(5,4,0,1);  g3_draw_tmap(4,cface,side_bm);
   g3_end_object();
#ifdef LIGHT_3D_OBJS
   gr_set_fill_type(cur_ft);
#endif
   g3_free_list(8,cube_pt);
}

#define setup_poly_face(a,b,c,d) \
 cface[0]=cube_pt[a]; cface[1]=cube_pt[b]; cface[2]=cube_pt[c]; cface[3]=cube_pt[d] 

#define setup_poly_rface(a,b,c,d) \
 cface[0]=cube_pt[a]; cface[1]=cube_pt[b]; cface[2]=cube_pt[c]; cface[3]=cube_pt[d];

#define fpoly_rend(pc,fc,fpts) \
  if (pc==0) pc=-0xff; \
  if (pc>=0) g3_check_and_draw_poly(pc,fc,fpts); \
  else if (pc<-0x80) g3_check_and_draw_tluc_poly(-pc,fc,fpts); \
  else g3_check_and_draw_tluc_spoly(fc,fpts)

uchar hakx,haky,hakz,haktype=0xFF;

// polygon/translucent cubes...
// needs to learn to set i correctly
void _fr_draw_poly_cube(int p_color, int x, int y, int z)
{
   g3s_phandle cube_pt[8], cface[4];
   g3s_vector  cube_vec;
//   int cur_ft;

   g3_start_object_angles_xyz(&_fr_p,_fr_cobj->loc.p<<8,_fr_cobj->loc.h<<8,_fr_cobj->loc.b<<8,ANGLE_ORDER);
   cube_vec.gX=-x; cube_vec.gY=0; cube_vec.gZ=-y;
   cube_pt[0]=g3_transform_point(&cube_vec);     cube_pt[0]->p3_flags|=PF_I; cube_pt[0]->i=0x0100;
   _fdt_pbase=0; // cube_pt[0]->i=_sq_lght;  // _fr_do_light(cube_pt[0],FRPTSZFLR_DN); 
   x+=x; y+=y; z=-(z+z);       // go from radius to real
   if (haktype==0)
      y=(y*(haky+1))>>8; 
   cube_pt[1]=g3_copy_add_delta_z(cube_pt[0],y); cube_pt[1]->p3_flags|=PF_I; cube_pt[1]->i=0x0200;
   cube_pt[2]=g3_copy_add_delta_x(cube_pt[1],x); cube_pt[2]->p3_flags|=PF_I; cube_pt[2]->i=0x0300;
   cube_pt[3]=g3_copy_add_delta_x(cube_pt[0],x); cube_pt[3]->p3_flags|=PF_I; cube_pt[3]->i=0x0400;
   cube_pt[4]=g3_copy_add_delta_y(cube_pt[0],z); cube_pt[4]->p3_flags|=PF_I; cube_pt[4]->i=0x0500;
   cube_pt[5]=g3_copy_add_delta_y(cube_pt[1],z); cube_pt[5]->p3_flags|=PF_I; cube_pt[5]->i=0x0600;
   cube_pt[6]=g3_copy_add_delta_y(cube_pt[2],z); cube_pt[6]->p3_flags|=PF_I; cube_pt[6]->i=0x0700;
   cube_pt[7]=g3_copy_add_delta_y(cube_pt[3],z); cube_pt[7]->p3_flags|=PF_I; cube_pt[7]->i=0x0800;
#ifdef vvLIGHT_3D_OBJS
   cur_ft=gr_get_fill_type();
   if (cur_ft!=FILL_SOLID)
   {
      gr_set_fill_type(FILL_CLUT);
      gr_set_fill_parm(_fr_clut_list[curr_clut_table]+(cube_pt[0]->i&0xf00));
   }
#endif
   setup_face(0,3,2,1);  fpoly_rend(p_color,4,cface);
   setup_rface(7,4,5,6); fpoly_rend(p_color,4,cface);
   setup_face(4,7,3,0);  fpoly_rend(p_color,4,cface);
   setup_face(7,6,2,3);  fpoly_rend(p_color,4,cface);
   setup_face(6,5,1,2);  fpoly_rend(p_color,4,cface);
   setup_face(5,4,0,1);  fpoly_rend(p_color,4,cface);
   g3_end_object();
#ifdef vvLIGHT_3D_OBJS
   gr_set_fill_type(cur_ft);
#endif
   g3_free_list(8,cube_pt);
}

// you stand surrounded by dreams brutally crushed
void _fr_draw_polyobj(void *model_ptr, bool use_lighting)
{
   int pos_parm=abs(PARM_MAX-((*tmd_ticks)&PARM_MOD));         // this is dumb
   int cur_ft;
   // set up clut for lighting in square and all
   // should decode 0 and FACE_ somehow... ick
#ifdef LIGHT_3D_OBJS
   if (use_lighting)
   {
      cur_ft=gr_get_fill_type();
      if (cur_ft!=FILL_SOLID)
      {
         gr_set_fill_type(FILL_CLUT);
         gr_set_fill_parm(_fr_clut_list[curr_clut_table]+(_sq_lght&0xf00));
      }
   }
#endif
   g3_start_object_angles_xyz(&_fr_p,_fr_cobj->loc.p<<8,_fr_cobj->loc.h<<8,_fr_cobj->loc.b<<8,ANGLE_ORDER);
   g3_interpret_object((ubyte *) model_ptr,((PARM_MAX+PARM_BASE)-pos_parm)<<PARM_SHF,PARM_BASE<<PARM_SHF);
   g3_end_object();
#ifdef LIGHT_3D_OBJS
   if (use_lighting)
      gr_set_fill_type(cur_ft);
#endif
}
//#pragma enable_message(202)

//#pragma disable_message(202)
void gen_seed_vec(g3s_vector *gpt_vec, int seed, int scale, int deviant)
{
   deviant=(1<<deviant)-1;
   if (seed!=0)
   {
	   gpt_vec->gX=((seed&0x1f)-0xf); seed>>=5;
	   gpt_vec->gY=((seed&0x1f)-0xf); seed>>=5;
	   gpt_vec->gZ=((seed&0x1f)-0xf);
   }
   gpt_vec->gX*=scale;
   gpt_vec->gY*=scale;
   gpt_vec->gZ*=scale;
   gpt_vec->gX+=rand()&deviant;
   gpt_vec->gY+=rand()&deviant;
   gpt_vec->gZ+=rand()&deviant;
}

void do_xplodamatron(int frame, int severity, int seed, int col1, int )
{
   g3s_phandle xplo_pts[3];
   g3s_vector  xplo_vec;
   int seed_add=severity*4183;

   g3_start_object_angles_xyz(&_fr_p,_fr_cobj->loc.p<<8,_fr_cobj->loc.h<<8,_fr_cobj->loc.b<<8,ANGLE_ORDER);
   xplo_vec.gX=xplo_vec.gY=xplo_vec.gZ=0;

   for (;severity>=0;severity--,seed+=seed_add)
   {
      gen_seed_vec(&xplo_vec,seed,4+(frame<<2),4+(frame<<3));
      xplo_pts[0]=g3_transform_point(&xplo_vec);
      xplo_vec.gX+=0x4000+(frame<<11); xplo_vec.gY+=0x6000+(frame<<12); xplo_vec.gZ-=0x3000+(frame<<11);
      xplo_pts[1]=g3_transform_point(&xplo_vec);
      xplo_vec.gX-=0x7000+(frame<<13); xplo_vec.gY+=0x5000+(frame<<10); xplo_vec.gZ-=0x2000+(frame<<12);
      xplo_pts[2]=g3_transform_point(&xplo_vec);
	   g3_check_and_draw_poly(col1+(rand()&0xf),3,xplo_pts);
	}

   g3_end_object();
}

// should be deviant some day
void gen_tetra(g3s_phandle *xplo_pts, fix size, int , int color)
{
   int i;
   g3s_vector  xplo_vec;
   fix annoying_val=fix_mul(size,fix_make(1,71*65536/100)); // 1/2 * 6/root3, in dougs head, = 1.71???

   xplo_vec.gX=xplo_vec.gZ=0; xplo_vec.gY=-2*size;
   xplo_pts[0]=g3_transform_point(&xplo_vec);                              // top
   xplo_pts[1]=g3_copy_add_delta_yz(xplo_pts[0],3*size,2*size);            // north
   xplo_pts[2]=g3_copy_add_delta_xz(xplo_pts[1],-annoying_val,-3*size);    // left
   xplo_pts[3]=g3_copy_add_delta_x (xplo_pts[2],2*annoying_val);           // right

   for (i=0; i<4; i++)
   	if (color>0)
	    { xplo_pts[i]->rgb=grd_bpal[color];             xplo_pts[i]->p3_flags|=PF_RGB; }
	   else
       { xplo_pts[i]->rgb=grd_bpal[-color+(rand()&3)]; xplo_pts[i]->p3_flags|=PF_RGB; }
}

// ok, size should be about 0x1000 for a toggle like thing, perhaps up to 3000 for big big stuff
// deviant should be 5-8, really
// color should come from ice_level, but oh well....

void draw_ice(void)
{
   g3s_phandle xplo_pts[8];
   int size, deviant=(obj_ICE_AGIT(_fr_cobj)>>6)+4;
   int p, h, b;

   // this is a total hack, should be made real when hp scale is known
   size=(_fr_cobj->info.current_hp)*6;    // just scaled us up to try and work better
   size+=(obj_ICE_AGIT(_fr_cobj))<<2;     // yea, sure, a why not
   size<<=obj_ICE_LEVEL(_fr_cobj);
   size+=0x400;
   if (size>0x3000) size=0x3000;

   p=_fr_cobj->loc.p<<8; p+=(rand()&(1<<(deviant+1)))-(1<<deviant);
   h=_fr_cobj->loc.h<<8; h+=(rand()&(1<<(deviant+1)))-(1<<deviant);
   b=_fr_cobj->loc.b<<8; b+=(rand()&(1<<(deviant+1)))-(1<<deviant);

   g3_start_object_angles_xyz(&_fr_p,p,h,b,ANGLE_ORDER);

   // should do color based on ice, do separate for outer and inner
   gen_tetra(xplo_pts,size,deviant,-BLUE_8_BASE);
   gen_tetra(xplo_pts+4,-(size<<2),deviant,-BLUE_8_BASE-4);
         
#ifndef NOT_REAL
   g3_draw_cline(xplo_pts[0],xplo_pts[6]);
   g3_draw_cline(xplo_pts[1],xplo_pts[6]);
   g3_draw_cline(xplo_pts[3],xplo_pts[6]);

   g3_draw_cline(xplo_pts[0],xplo_pts[7]);
   g3_draw_cline(xplo_pts[1],xplo_pts[7]);
   g3_draw_cline(xplo_pts[2],xplo_pts[7]);

   g3_draw_cline(xplo_pts[0],xplo_pts[5]);
   g3_draw_cline(xplo_pts[2],xplo_pts[5]);
   g3_draw_cline(xplo_pts[3],xplo_pts[5]);

   g3_draw_cline(xplo_pts[1],xplo_pts[4]);
   g3_draw_cline(xplo_pts[2],xplo_pts[4]);
   g3_draw_cline(xplo_pts[3],xplo_pts[4]);
#endif

#ifdef FAKE
   g3_draw_cline(xplo_pts[0],xplo_pts[3]);
   g3_draw_cline(xplo_pts[1],xplo_pts[3]);
   g3_draw_cline(xplo_pts[2],xplo_pts[3]);
   g3_draw_cline(xplo_pts[0],xplo_pts[1]);
   g3_draw_cline(xplo_pts[1],xplo_pts[2]);
   g3_draw_cline(xplo_pts[2],xplo_pts[0]);

   g3_draw_cline(xplo_pts[4],xplo_pts[5]);
   g3_draw_cline(xplo_pts[4],xplo_pts[6]);
   g3_draw_cline(xplo_pts[4],xplo_pts[7]);
   g3_draw_cline(xplo_pts[5],xplo_pts[6]);
   g3_draw_cline(xplo_pts[6],xplo_pts[7]);
   g3_draw_cline(xplo_pts[7],xplo_pts[5]);
#endif

   g3_free_list(8,xplo_pts);
   g3_end_object();
}
//#pragma enable_message(202)

//#pragma disable_message(202)
void draw_ice_wall(void)
{
   int size_x=0x8000, size_y=0x8000;




}
//#pragma enable_message(202)

void _fr_draw_tmtile(grs_bitmap *draw_bm, int col_val, g3s_phandle *plst, bool dblface, bool use_lighting)
{
   int t_off_l, t_off_r;
//   int face_c, face_f, hgt_f, hgt_c;
   int y1=((_fr_cobj->loc.y)&0xff)<<8, x1=((_fr_cobj->loc.x)&0xff)<<8;

   switch (((_fr_cobj->loc.h+0x20)&0xff)>>6)
   {
   case 0: if (y1>0x8000) {t_off_l=1; t_off_r=2; } else {t_off_l=0; t_off_r=3; } break;
   case 2: if (y1>0x8000) {t_off_l=2; t_off_r=1; } else {t_off_l=3; t_off_r=0; } break;
   case 1: if (x1>0x8000) {t_off_l=2; t_off_r=3; } else {t_off_l=1; t_off_r=0; } break;
   case 3: if (x1>0x8000) {t_off_l=3; t_off_r=2; } else {t_off_l=0; t_off_r=1; } break;
   }
//   hgt_c=-p.gY+(fix_yoff<<1); hgt_f=-p.gY;
   _fdt_pbase=t_off_l;
   _fr_do_light(plst[0],FRPTSZCEIL_DN);   plst[0]->uv.u=0; plst[0]->uv.v=0;            plst[0]->p3_flags=PF_I|PF_V|PF_U;
   _fr_do_light(plst[3],FRPTSZFLR_DN);    plst[3]->uv.u=0; plst[3]->uv.v=0x100;        plst[3]->p3_flags=PF_I|PF_V|PF_U;
   _fdt_pbase=t_off_r;
   _fr_do_light(plst[1],FRPTSZCEIL_DN);   plst[1]->uv.u=0x100; plst[1]->uv.v=0;        plst[1]->p3_flags=PF_I|PF_V|PF_U;
   _fr_do_light(plst[2],FRPTSZFLR_DN);    plst[2]->uv.u=0x100; plst[2]->uv.v=0x100;    plst[2]->p3_flags=PF_I|PF_V|PF_U;
   if (!use_lighting)
    { plst[0]->i = 0; plst[3]->i = 0; plst[1]->i = 0; plst[2]->i = 0; }
//mprintf("Note light %x %x %x %x\n",plst[0]->i,plst[1]->i,plst[2]->i,plst[3]->i);
   if (_fr_curflags&FR_PICKUPM_MASK)
      if ((draw_bm==NULL)||(_fr_curflags&FR_NOTRANS_MASK)||((draw_bm->flags&BMF_TRANS)==0))
	      g3_check_and_draw_poly(gr_get_fill_parm(),4,plst);
      else
         g3_draw_tmap(4,plst,draw_bm);
   else
      if (col_val!=0xFF)
      { 
         int cur_ft = gr_get_fill_type();
         gr_set_fill_type(FILL_CLUT);
         gr_set_fill_parm(_fr_clut_list[curr_clut_table]+(_sq_lght&0xf00));
         fpoly_rend(col_val,4,plst); 
         gr_set_fill_type(cur_ft);
      }
      else
      {
 		   g3_light_tmap(4,plst,draw_bm);
      }
   if (dblface)  // draw the bleeding backside, nudge nudge
   {
      g3s_phandle tmp;
      tmp=plst[0]; plst[0]=plst[1]; plst[1]=tmp; 
      tmp=plst[2]; plst[2]=plst[3]; plst[3]=tmp;   // and if you thought that was obscure
	   if (_fr_curflags&FR_PICKUPM_MASK)
         if ((draw_bm==NULL)||(_fr_curflags&FR_NOTRANS_MASK)||((draw_bm->flags&BMF_TRANS)==0))
		      g3_check_and_draw_poly(gr_get_fill_parm(),4,plst);
	      else
	         g3_draw_tmap(4,plst,draw_bm);
	   else
      {
         if (col_val!=0xFF)
   	   { 
            int cur_ft = gr_get_fill_type();
            gr_set_fill_type(FILL_CLUT);
            gr_set_fill_parm(_fr_clut_list[curr_clut_table]+(_sq_lght&0xf00));
            fpoly_rend(col_val,4,plst); 
            gr_set_fill_type(cur_ft);
         }
         else
         {
 		      g3_light_tmap(4,plst,draw_bm);
         }
      }
   }
}

// all the qsc code is in the backup of fauxobjd
// so we dont need to carry it around till it works

// note this always has show_obj's p for p and _fdt_dist for dist.. perhaps shouldnt pass them
//#pragma disable_message(202)
void _fr_draw_bitmap(grs_bitmap *draw_bm, int /*dist*/, int sc, int anch_x, int anch_y)
{
#ifdef SMOOTH_BITMAPS
   grs_canvas tmp_can;
   grs_bitmap tmp_bm;
   uchar *tmp_ptr;
   bool do_qsc=(dist<fr_qscale_obj);
#endif
   g3s_phandle anchor;
   grs_vertex **bitmap_verts;

   _fr_p.gY+=_o_rad;
   anchor=g3_transform_point(&_fr_p);
   _fr_p.gY-=_o_rad;

   _fdt_pbase=0;
   anchor->i=_sq_lght;  // _fr_do_light(anchor,FRPTSZFLR_DN);

   if (sc) g3_set_bitmap_scale(fix_make(0,(int)(4096/3)),fix_make(0,(int)(4096/3)));
   if ((anch_x<=0)&&(anch_y<=0))
	   bitmap_verts=g3_light_bitmap(draw_bm,anchor);
   else
      bitmap_verts=g3_light_anchor_bitmap(draw_bm,anchor,anch_x,anch_y);
   if (sc) g3_set_bitmap_scale(fix_make(0,(int)(2048/3)),fix_make(0,(int)(2048/3)));

   if ((bitmap_verts!=NULL)&&IS_HUDOBJ(_fr_cobj-objs))
   {
#ifdef SVGA_SUPPORT
      fix lx=fix_make(1024,0),ly=fix_make(768,0),rx=fix_make(0,0),ry=fix_make(0,0);
#else
      fix lx=fix_make(320,0),ly=fix_make(200,0),rx=fix_make(0,0),ry=fix_make(0,0);
#endif
      int i;
      for (i=0; i<4; i++)
      {
         if (bitmap_verts[i]->x<lx) lx=bitmap_verts[i]->x;
         if (bitmap_verts[i]->x>rx) rx=bitmap_verts[i]->x;
         if (bitmap_verts[i]->y<ly) ly=bitmap_verts[i]->y;
         if (bitmap_verts[i]->y>ry) ry=bitmap_verts[i]->y;
      }
      SET_HUDOBJ_RECT(_fr_cobj-objs,fix_int(lx),fix_int(ly),fix_int(rx),fix_int(ry));
   }
   g3_free_point(anchor);
}
//#pragma enable_message(202)

#define FAUBJ_BULLET_HACK (NUM_OBJ_RENDER_TYPES)
#define ADD_IT            (0)

// this really shouldnt be here in this way
// Some special bitfields that determine how the data fields
// are parsed for slaving animations
#define INDIRECTED_STUFF_INDICATOR_MASK 0x1000
#define INDIRECTED_STUFF_DATA_MASK 0xFFF

#define SECRET_FURNITURE_DEFAULT_O3DREP 0x80

extern char extract_object_special_color(ObjID id);

short compute_3drep(Obj *cobj, ObjID cobjid, int obj_type)
{
   short o3drep = -1;
   extern bool anim_data_from_id(ObjID, bool*, bool*);
   extern bool obj_is_display(int triple);

   // fix for screens with data2 of zero wanting to animate,
   // and other bigstuffs presumbably wanting to use it as a default.
   // only indirect through data2 if it is nonzero OR if we're
   // animating.
   if ((cobj->obclass == CLASS_BIGSTUFF) && (obj_is_display(ID2TRIP(cobjid))) && 
      ((objBigstuffs[cobj->specID].data2 != 0) || anim_data_from_id(cobjid,NULL,NULL)))
   {
      int d2 = objBigstuffs[cobj->specID].data2;
      if (d2 & INDIRECTED_STUFF_INDICATOR_MASK)
      {
         ObjID newid = d2 & INDIRECTED_STUFF_DATA_MASK;
         o3drep=objBigstuffs[objs[newid].specID].data2 + objs[newid].info.current_frame;
      }
      else
   	   o3drep=objBigstuffs[cobj->specID].data2 + cobj->info.current_frame;
   }
   else 
   {
      switch(obj_type)
      {
         case FAUBJ_TPOLY:
         case FAUBJ_TEXTPOLY:
            o3drep = 0;
            break;
         default:
   	      o3drep=BMAP_NUM_3D(ObjProps[OPNUM(cobjid)].bitmap_3d);
   	      if ((obj_type != FAUBJ_VOX) && (cobj->obclass != CLASS_DOOR) && (cobj->info.current_frame != 255))
            {
#ifdef PLAYTEST
               if ((cobj->obclass == CLASS_CONTAINER) && (cobj->info.current_frame > FRAME_NUM_3D(ObjProps[OPNUM(cobjid)].bitmap_3d)))
               {
                  Warning(("hey, obj id %x has frame %d, but max is %d!\n",cobjid,cobj->info.current_frame,
                     FRAME_NUM_3D(ObjProps[OPNUM(cobjid)].bitmap_3d)));
               }
               else
#endif
      	         o3drep += cobj->info.current_frame;
            }
            break;
      }   
   }
   if ((cobj->obclass == CLASS_BIGSTUFF) && (cobj->subclass == BIGSTUFF_SUBCLASS_FURNISHING) && 
      (objBigstuffs[cobj->specID].data2 == 0))
         o3drep = SECRET_FURNITURE_DEFAULT_O3DREP;
   return(o3drep);
}

#define TRANSLUCENT_INVISOS
//#define TLUC_IN_2D

extern void load_model_vtexts(char model_num);
extern void free_model_vtexts(char model_num);

// in effect.c also
#define MAX_TELEPORT_FRAME    10
#define TELEPORT_COLOR        0x1C
// in objsim.c also
#define DIEGO_DEATH_BATTLE_LEVEL 8

#define DESTROYED_SCREEN_ANIM_BASE 0x1B

void show_obj(ObjID cobjid)
{
   short objtrip;
   short o3drep;
   int model_num=0;
   extern uchar cam_mode;
   uchar *model_ptr;
   grs_bitmap *tpdata;
#ifdef TRANSLUCENT_INVISOS
#ifndef TLUC_IN_2D
   grs_bitmap tpdata_temp;
#endif
#endif
   char scale = 0;
   uchar type = 0xFF;
   bool use_cache = FALSE;
   Ref ref = 0;
   int obj_type, tluc_val=0xFF, index=0, loc_h;
   bool light_me = TRUE;
   extern cams objmode_cam;
   extern bool obj_too_smart(ObjID id);
   extern void check_up(int num);

//   check_up(0x220000|cobjid);
//   mprintf("cobjid = %x\n",cobjid);
#ifdef DOOM_EMULATION_MODE
   if (obj_too_smart(cobjid))
      return;
#endif
   objtrip=OPNUM(cobjid);
   obj_type=ObjProps[objtrip].render_type;
   _fr_cobj=&objs[cobjid];
   o3drep = compute_3drep(_fr_cobj,cobjid,obj_type);

   _fr_p.gX = _fr_cobj->loc.x<<8;
   _fr_p.gZ = _fr_cobj->loc.y<<8;
   _fr_p.gY =-_fr_cobj->loc.z*height_step>>3;    // down 3, neato magic number technology

   if (_fr_curflags&FR_PICKUPM_MASK)
   {
      gr_set_fill_parm(fr_cur_obj_col);
      fr_col_to_obj[(fr_cur_obj_col++)-FR_CUR_OBJ_BASE]=cobjid;
   }

   _o_rad=fix_make(ObjProps[objtrip].physics_xr,0)/96;      // for reanchoring

   // should look at renderer!!!!, not this

   if (global_fullmap->cyber)
   {
      if ((obj_type!=FAUBJ_CRIT) && (obj_type != FAUBJ_TEXTPOLY) && (obj_type != FAUBJ_VOX) && (obj_type != FAUBJ_SPECIAL))
      {
	      // for now, really want a function call here
//         uchar sftware_col[5]={0x3B,0x54,0x7D,0x62,0x27};
         uchar sftware_col[5]={0x3B,0x4F,0x76,0x5A,0x23};
         int col=0xC3, dm=0, move_me=1, ndm;
         extern bool time_passes;
         fix fx,fy,fz;
         int sc,v;
//         static long ltime=0;
         
         switch (_fr_cobj->obclass)
         {
         case CLASS_HARDWARE:
            col=0x48-(objHardwares[_fr_cobj->specID].version<<1); if (col<0x45) col=0x42; 
            break;
         case CLASS_BIGSTUFF:
            sc = objBigstuffs[_fr_cobj->specID].data1;
            v = objBigstuffs[_fr_cobj->specID].cosmetic_value;
            dm = 0;
            // Fall through, good
         case CLASS_SOFTWARE:  // ultra secret gnosis time
            if (_fr_cobj->obclass == CLASS_SOFTWARE)
            {
               sc = _fr_cobj->subclass;
               v = objSoftwares[_fr_cobj->specID].version;
               dm = objSoftwares[_fr_cobj->specID].data_munge;
            }
            col=sftware_col[sc] - v;
            break;
#ifdef OLD_BIGSTUFF_WAY
         case CLASS_BIGSTUFF:
            col=sftware_col[objBigstuffs[_fr_cobj->specID].data1]-(objBigstuffs[_fr_cobj->specID].cosmetic_value<<1);
            dm=0;
#endif
         case CLASS_SMALLSTUFF:
            if (_fr_cobj->subclass == SMALLSTUFF_SUBCLASS_CYBER)
		         col=objSmallstuffs[_fr_cobj->specID].cosmetic_value;
            break;
         case CLASS_ANIMATING:
            if (_fr_cobj->subclass == ANIMATING_SUBCLASS_EXPLOSION)  // should derive 3 and RED_8_BASE from somewhere
               do_xplodamatron(_fr_cobj->info.current_frame,3,0x8457,RED_8_BASE,GREEN_8_BASE);
            return;
         }
         if (dm==0)
	  	    { fx=0x4000; fy=0x3000; fz=0x2000; }
         else
         {  // first set the paramets
            fix bsc=0x3000, csc=(0x0500*((dm&0x7)+2));
            if (dm&0x10) fx=csc; else fx=bsc;
            if (dm&0x20) fy=csc; else fy=bsc;
            if (dm&0x40) fz=csc; else fz=bsc;
            if ((csc==0x4500)&&(dm&0x8))           ndm=0x7;
            else if ((csc==0x1000)&&((dm&0x8)==0)) ndm=0x8;
            else ndm=0;
            if (ndm) { dm=(dm&~0xf)+ndm; if (((dm&0x88)==0x88)&&((rand()&0x7)==0)) dm=(dm&~0x70)+(rand()&0x70); }
            if (dm&0x8) dm++; else dm--;
//	         switch (_fr_cobj->class)
//	          { case CLASS_SOFTWARE: objSoftwares[_fr_cobj->specID].data_munge=dm; break; }
         }
	      _fr_draw_poly_cube(col,fx,fy,fz);
         if (move_me && time_passes)
		   {
// I'm just ifdef-ing this out for now, I'll put it back in
// when I have a more coherent plan WRT it.
#ifdef RUBBER_BABY_BUGGY_BUMPERS
		      _fr_cobj->loc.gZ=munge_val(_fr_cobj->loc.gZ,256,4);
	   	   _fr_cobj->loc.gX=(_fr_cobj->loc.gX&~0xff)+munge_val(_fr_cobj->loc.gX&0xff,256,6);
		      _fr_cobj->loc.gY=(_fr_cobj->loc.gY&~0xff)+munge_val(_fr_cobj->loc.gY&0xff,256,6);
#endif
		      _fr_cobj->loc.p+=248+(((uint)cobjid)%17);
		      _fr_cobj->loc.b+=247+(((uint)cobjid)%19);
		      _fr_cobj->loc.h+=245+(((uint)cobjid)%23);
	      }
         if (obj_ICE_ICE_BABY(_fr_cobj))
            draw_ice();
         return;
      }
   }
   else

	   // this is horrible and must be fixed!!!
	   if ( ((ObjProps[objtrip].flags & LIGHT_TYPE) <= (1<<LIGHT_TYPE_SHF)) ||
	       (((ObjProps[objtrip].flags & LIGHT_TYPE) == (3<<LIGHT_TYPE_SHF))&&((_fr_cobj->info.inst_flags&UNLIT_FLAG)==0)) )
	   {
	      MapElem *tmp;
	      int _obj_do_light(int which, fix dist);
	      fix hack_dist_approx= fix_fast_pyth_dist(fix_fast_pyth_dist((_fr_p.gX-fr_camera_last[0]),(_fr_p.gZ-fr_camera_last[1])),(_fr_p.gY+fr_camera_last[2]));
	      tmp=_fdt_mptr; _fdt_pbase=0;
	      _fdt_mptr=MAP_MAP+((_fr_cobj->loc.x+0x80)>>8)+(((_fr_cobj->loc.y+0x80)>>2)&~0x3F);
	      _sq_lght=_obj_do_light(FRPTSZFLR_DN,hack_dist_approx);
	      _fdt_mptr=tmp;
	   }
	   else _sq_lght=0;

   switch (obj_type)
   {
   case FAUBJ_BITMAP:    
         if ((anchors_3d[o3drep].x > 0) || (anchors_3d[o3drep].y > 0))
            _o_rad = 0;
      _fr_draw_bitmap(bitmaps_3d[o3drep],_fdt_dist,ObjProps[objtrip].flags&MY_IM_LARGE,anchors_3d[o3drep].x,anchors_3d[o3drep].y);
      if (global_fullmap->cyber)
		   if (obj_ICE_ICE_BABY(_fr_cobj))
            draw_ice();
      break;
      
   case FAUBJ_SPECIAL:
      switch(ID2TRIP(cobjid))
      {
      case MAPNOTE_TRIPLE:
	      {
            extern bool map_notes_on;
            g3s_phandle note_pts[4];
            int h=player_struct.game_time&0x3fff;

            if (((_fr_curflags&FR_HACKCAM_MASK)==0)&&(map_notes_on))
            {
               int col=hud_colors[hud_color_bank][2];
	            _fr_p.gY-=0x4000;
	            g3_start_object_angles_xyz(&_fr_p,0,h<<2,0,ANGLE_ORDER);
	            gen_tetra(note_pts, 0x2000, 0, 0);
               gr_set_fcolor(col);
	            g3_draw_line(note_pts[1],note_pts[0]);
	            g3_draw_line(note_pts[2],note_pts[0]);
	            g3_draw_line(note_pts[3],note_pts[0]);
	            g3_draw_line(note_pts[1],note_pts[2]);
	            g3_draw_line(note_pts[2],note_pts[3]);
	            g3_draw_line(note_pts[3],note_pts[1]);
	            g3_free_list(4,note_pts);
	            g3_end_object();
	            _fr_p.gY+=0x6500;
            }
         }
         break;
	   case TRIPBEAM_TRIPLE:
         // need a 3d line here...
         break;
      case FORCE_BRIJ_TRIPLE:
      case FORCE_BRIJ2_TRIPLE:
         tluc_val=-(int)((uchar)extract_object_special_color(cobjid));
//         mprintf("tluc %d\n",tluc_val);
      case BARRICADE_TRIPLE:
         // We want to hack tluc_val so that we draw a poly_cube and not a parm_cube
          if (ID2TRIP(cobjid) == BARRICADE_TRIPLE)
          {
             tluc_val = objSmallstuffs[_fr_cobj->specID].data2;
             if (tluc_val == 0)
               tluc_val = me_cybcolor_flr(MAP_GET_XY(OBJ_LOC_BIN_X(_fr_cobj->loc), OBJ_LOC_BIN_Y(_fr_cobj->loc)));
          }
      case BRIDGE_TRIPLE:
      case PILLAR_TRIPLE:
      case CATWALK_TRIPLE:
         _o_rad = 0;
      case SML_CRT_TRIPLE:
      case LG_CRT_TRIPLE:
      case SECURE_CONTR_TRIPLE:
         {
            extern grs_bitmap tmap_bm[];
            extern grs_bitmap *obj_get_model_data(ObjID id, fix *x, fix *y, fix *z, grs_bitmap *bm2, Ref *ref1, Ref *ref2);
            grs_bitmap *b1, b2;
            Ref ref1 = 0, ref2 = 0;
            fix fx,fy,fz;
            b1 = obj_get_model_data(cobjid, &fx,&fy,&fz,&b2, &ref1, &ref2);

            // 255 is fully extended for hak x thru z
            obj_model_hack(cobjid, &hakx,&haky,&hakz,&haktype);
//            mprintf("Note for %x, got %d %d %d and %d\n",cobjid,hakx,haky,hakz,haktype);

            if (tluc_val!=0xFF)
            {
               // Note that we do all this here, instead of in draw_poly_cube, since
               // most poly cubes, namely cyberspace stuff, don't wanna be lit.  
               // Maybe this is the wrong way to do it, tho' -- X
               int cur_ft;
               cur_ft = gr_get_fill_type();
               if (cur_ft != FILL_SOLID)
               {
                  gr_set_fill_type(FILL_CLUT);
                  gr_set_fill_parm(_fr_clut_list[curr_clut_table]+(_sq_lght&0xf00));
               }
               _fr_draw_poly_cube(tluc_val,fx,fy,fz);
               gr_set_fill_type(cur_ft);
            }
            else if (b1 != NULL)
            {
               _fr_draw_parm_cube(&b2,b1,fx,fy,fz);
            }
            if (ref1 != 0)
               RefUnlock(ref1);
            if (ref2 != 0)
               RefUnlock(ref2);
         }
         break;
      }
      haktype=0xFF;
      break;

   case FAUBJ_MULTIVIEW:
   case FAUBJ_CRIT:
      {
	      int view;

	      static uchar _qtab[4]={3,0,2,1};
				int xd=_fr_cobj->loc.x-(coor(EYE_X)>>8);
	      int yd=_fr_cobj->loc.y-(coor(EYE_Y)>>8);
         int q, l;

         q=_qtab[((xd>0)?2:0)+((yd>0)?1:0)];
//   mprintf("Q %d.. v(%d,%d)",q,xd,yd);
         xd=abs(xd); yd=abs(yd);
         if (xd>=(yd<<1)) l=0;
         else if ((xd<<1)<=yd) l=2;
         else l=1;
         if (q&1) view=(q<<1)+2-l;
         else     view=(q<<1)+l;
//   mprintf(".. yields view %d w/l at %d .. ",view,l);
         view=((_fr_cobj->loc.h-(view<<5)+0x180)&0xff)>>5;
//   mprintf(".. w/crit at %d gives %d\n",_fr_cobj->loc.h,view);
	      if (obj_type == FAUBJ_CRIT)
	      {
            LGRect anch;
//            extern void ai_critter_seen(ObjID id);
//            extern char curr_hack_cam;
//            if (curr_hack_cam)
//              ai_critter_seen(cobjid);
            switch(ID2TRIP(cobjid))
            {
               case DIEGO_TRIPLE:
                  if ((get_crit_posture(_fr_cobj->specID) == DEATH_CRITTER_POSTURE) && 
                      (player_struct.level != DIEGO_DEATH_BATTLE_LEVEL))
                  {
                     grs_bitmap tele_bm;
                     uchar line, *srcp, *dstp, *trgp;
		               tpdata=get_critter_bitmap_fast(cobjid, ID2TRIP(cobjid),get_crit_posture(_fr_cobj->specID),0,(ubyte)view,&ref,&anch);
                     gr_rsd8_convert(tpdata, &tpdata_temp);
                     tpdata = &tpdata_temp;
                     LG_memcpy(&tele_bm, tpdata, sizeof(grs_bitmap));
                     tele_bm.bits = big_buffer + 32768;
                     LG_memset(tele_bm.bits, 0, tele_bm.w * tele_bm.h);
                     line = fix_int(fix_mul_div(fix_make(_fr_cobj->info.current_frame,0), fix_make(tele_bm.h,0),
                        fix_make(MAX_TELEPORT_FRAME,0)));
                     for (srcp=tpdata->bits,dstp=tele_bm.bits,trgp=tpdata->bits+line*tele_bm.w; srcp<trgp; srcp++,dstp++)
                        if (*srcp!=0) *dstp=TELEPORT_COLOR + (rand()&0x3);
                     trgp=tpdata->bits+tele_bm.h*tele_bm.w;
                     LG_memcpy(dstp,srcp,trgp-srcp);

                     _fr_draw_bitmap(&tele_bm,_fdt_dist,FALSE,anch.ul.x,anch.ul.y);
                     release_critter_bitmap_fast(ref);
                     return;
                  }
                  break;
            }
            // bitmask out the lighting stuff
		      tpdata=get_critter_bitmap_obj_fast(cobjid,view,&ref,&anch);
            switch (ID2TRIP(cobjid))
            {
               case INVISO_CRIT_TRIPLE:
#ifdef TRANSLUCENT_INVISOS
#ifdef TLUC_IN_2D
                  tpdata->flags |= BMF_TLUC8;
#else
                  gr_rsd8_convert(tpdata, &tpdata_temp);
                  tpdata = &tpdata_temp;
                  tpdata->type = BMT_TLUC8;   
#endif
#endif
                  break;
            }
		      _fr_draw_bitmap(tpdata,_fdt_dist,0,anch.ul.x,anch.ul.y);
		      release_critter_bitmap_fast(ref);
	      }
	      else
	      {
	         tpdata=bitmaps_3d[o3drep + view];
	         _fr_draw_bitmap(tpdata,_fdt_dist,0,-1,-1);
	      }
      }
      break;
      
   case FAUBJ_VOX:
      {           // wait, cant we just keep vvv around, and stuff the two bitmaps, w+h each time, save some here
         vxs_vox vvv;
         fix damage_factor = fix_div(fix_make(_fr_cobj->info.current_hp,0), fix_make(ObjProps[objtrip].hit_points,0));
         fix pdist = VOXEL_PIX_DIST_BASE;
         fix psize = VOXEL_PIX_SIZE_BASE + fix_mul(VOXEL_PIX_SIZE_DELTA, damage_factor);
            
         g3_start_object_angles_xyz(&_fr_p,_fr_cobj->loc.p<<8,_fr_cobj->loc.h<<8,_fr_cobj->loc.b<<8,ANGLE_ORDER);
         vx_init_vox(&vvv, pdist, psize, VOXEL_DEPTH, bitmaps_3d[o3drep], bitmaps_3d[o3drep+1]);
         vx_render(&vvv);
         g3_end_object();
      }
      break;

   case FAUBJ_TL_POLY:
      tluc_val=-(int)((uchar)extract_object_special_color(cobjid));
      tpdata=NULL;
   case FAUBJ_TEXBITMAP:
   case FAUBJ_TPOLY:
	   { 
         g3s_phandle corn[4];
         g3s_vector ul;
         fix fix_xoff, fix_yoff;

         scale = 0;
         switch(obj_type)
         {
         case FAUBJ_TPOLY:
               switch (ID2TRIP(cobjid))
               {
                  case TMAP_TRIPLE:
                     {
                        extern int all_textures;
                        tpdata = get_texture_map(objBigstuffs[_fr_cobj->specID].data2, (all_textures) ? TEXTURE_128_INDEX : TEXTURE_64_INDEX);
                        scale = -1;
                     }
                     break;
                  default:
         		      tpdata=bitmap_from_tpoly_data(o3drep, (ubyte *) &scale, &index, &type, &ref);
                     if (ref != 0)
                        use_cache = TRUE;
                     break;
               }
               switch (ID2TRIP(cobjid))
               {
                  case SUPERSCREEN_TRIPLE: scale=7-tpdata->wlog; break;
                  case TMAP_TRIPLE:
                  case BIGSCREEN_TRIPLE:   scale=6-tpdata->wlog; break;
                  case SCREEN_TRIPLE:      scale=5-tpdata->wlog; break;
               }
               switch (ID2TRIP(cobjid))
               {
                  case SCREEN_TRIPLE:
                  case SUPERSCREEN_TRIPLE:
                  case BIGSCREEN_TRIPLE:
                     if ((!curr_clut_table) && (objBigstuffs[_fr_cobj->specID].data2 != DESTROYED_SCREEN_ANIM_BASE + 3))
                        light_me = FALSE;
                     break;
               }
	         break;
	      case FAUBJ_TEXBITMAP:
            switch (_fr_cobj->obclass)
            {
            case CLASS_BIGSTUFF:
               switch (ID2TRIP(cobjid))
               {
                  case WORDS_TRIPLE:
            		   tpdata = get_text_bitmap_obj(cobjid, 0, &scale); // C is for magic cookie, it's good enough for me   
                     ref = 0xFFFFFFFF;
                     break;
                  default:
             		   tpdata = get_obj_cache_bitmap(cobjid, &ref);
                     break;
               }
               break;
            case CLASS_DOOR:
               tpdata = get_obj_cache_bitmap(cobjid, &ref);
               if ((_fr_cobj->info.current_frame == 0) && (ObjProps[objtrip].flags & RENDER_BLOCK))
                  tpdata->flags &= ~BMF_TRANS;
#ifdef HIGHRES_DOORS
#ifdef SVGA_SUPPORT
               scale -= 1;
#endif
#endif
               break;
            default:
       		   tpdata = get_obj_cache_bitmap(cobjid, &ref);
               break;
            }
		      if (ref == 0)
               tpdata = bitmaps_3d[o3drep];
            else if (ref != 0xFFFFFFFF)
               use_cache = TRUE;
            break;
   	   }
         // scale us
       	if (tpdata==NULL) // (type==TPOLY_TMAP)
	         fix_yoff=fix_xoff=fix_make(0,0x8000);
	      else        // here is where to scale up and all.....
	       { fix_xoff=tpdata->w<<9; fix_yoff=tpdata->h<<9; }
	      if (scale>0)      { fix_xoff<<=scale;  fix_yoff<<=scale;  } 
         else if (scale<0) { fix_xoff>>=-scale; fix_yoff>>=-scale; }
	      ul.gX=-fix_xoff; ul.gY=-fix_yoff;
         ul.gZ=0; // ul.gZ=fix_make(0,0x0080);

         // gruesome hack of destruction!!!
         if ((_fr_p.gX&0xffff)==0xff00) _fr_p.gX+=0x100;
         if ((_fr_p.gZ&0xffff)==0xff00) _fr_p.gZ+=0x100;
//         mprintf("Door at %x %x %x\n",_fr_p.gX,_fr_p.gY,_fr_p.gZ); 

         g3_start_object_angles_xyz(&_fr_p,_fr_cobj->loc.p<<8,_fr_cobj->loc.h<<8,_fr_cobj->loc.b<<8,ANGLE_ORDER);
	      corn[0]=g3_transform_point(&ul);                   
	      corn[1]=g3_copy_add_delta_x(corn[0],fix_xoff<<1);  
	      corn[2]=g3_copy_add_delta_y(corn[1],fix_yoff<<1);  
	      corn[3]=g3_copy_add_delta_y(corn[0],fix_yoff<<1);
         _fr_draw_tmtile(tpdata,tluc_val,corn,(_fr_cobj->obclass==CLASS_DOOR),light_me);
	      if (use_cache)
	         release_obj_cache_bitmap(ref);
         g3_end_object();
	      break;
      }

   default:
   case FAUBJ_TEXTPOLY:
      tpdata=bitmap_from_tpoly_data(o3drep, (ubyte *) &scale, &index, &type, &ref);
      g3_set_vtext(0,tpdata);
   case FAUBJ_ANIMPOLY:
      if ((_fr_cobj->obclass == CLASS_SMALLSTUFF) && (_fr_cobj->subclass == SMALLSTUFF_SUBCLASS_CYBER))
         g3_set_vcolor(0,objSmallstuffs[_fr_cobj->specID].cosmetic_value);
   case FAUBJ_FLATPOLY:
      if (global_fullmap->cyber)
      {
         int foog;
         switch(_fr_cobj->obclass)
         {
            case CLASS_CRITTER:
               if (_fr_cobj->subclass == CRITTER_SUBCLASS_CYBER)
               {
                  for (foog =0; foog < NUM_VCOLORS; foog++)
                  {
                     if ((objCritters[_fr_cobj->specID].mood == AI_MOOD_HOSTILE)|| 
                         (objCritters[_fr_cobj->specID].mood == AI_MOOD_ATTACKING))
                     {
                        uchar c = CyberCritterProps[SCNUM(cobjid)].alt_vcolors[foog], nc;
                        int hp_state;
                        hp_state=256*14-((256*14*_fr_cobj->info.current_hp/ObjProps[objtrip].hit_points)&(~0xff));
                        if (hp_state>0xf00) hp_state=0xf00; else if (hp_state<0) hp_state=0;
                        nc=*((_fr_clut_list[0]) + hp_state + c);
                        g3_set_vcolor(foog+1,nc);
                     }
                     else
                        g3_set_vcolor(foog+1,CyberCritterProps[SCNUM(cobjid)].vcolors[foog]);
                  }
               }
               break;
            case CLASS_SMALLSTUFF:
               // Yes, arbitrariness....
               for (foog = 0; foog < NUM_SMALLSTUFF_VCOLORS; foog++)
               {   
                  g3_set_vcolor(foog+1,CyberSmallstuffProps[SCNUM(cobjid)].vcolors[foog]);
//                  Warning(("setting vcolor %d to 0x%x!\n",foog+1,CyberSmallstuffProps[SCNUM(cobjid)].vcolors[foog]));
               }
//                  g3_set_vcolor(foog+1,0x33 + objSmallstuffs[_fr_cobj->specID].cosmetic_value + (foog << 2));
               break;
            case CLASS_PHYSICS:
               if (_fr_cobj->subclass == PHYSICS_SUBCLASS_SLOW)
               {
                  for (foog =0; foog < NUM_SLOW_VCOLORS; foog++)
                  {
                     g3_set_vcolor(foog+1,SlowPhysicsProps[SCNUM(cobjid)].vcolors[foog]);
                  }
               }
               break;
         }
      }
      model_num=ObjProps[objtrip].mfd_id;
      if (!model_valid(model_num))
	      model_num=0;
      load_model_vtexts(model_num);
      model_ptr=(uchar *) get_model_data(model_num);
      // go go go
      if (ID2TRIP(cobjid)==CAMERA_TRIPLE)
      {
         int mval;
         loc_h=_fr_cobj->loc.h;
         switch (objBigstuffs[_fr_cobj->specID].data1)
         {
         case 0:
            break;
         default:
	         loc_h-=32;
	         mval=((*tmd_ticks)>>5)&0x7f;
	         if (mval>64) mval=128-mval;
	         loc_h+=mval;
	         mval=_fr_cobj->loc.h;
	         _fr_cobj->loc.h=loc_h;
	         loc_h=mval;
         }
      }

      _fr_draw_polyobj(model_ptr, !global_fullmap->cyber);
      if (ID2TRIP(cobjid)==CAMERA_TRIPLE)
   	   _fr_cobj->loc.h=loc_h;  //hack hack hack
      if (ref != 0)
         release_obj_cache_bitmap(ref);
      release_model_data(model_num);
      free_model_vtexts(model_num);
      if (global_fullmap->cyber)
		   if (obj_ICE_ICE_BABY(_fr_cobj))
            draw_ice();
      break;
   }
}
