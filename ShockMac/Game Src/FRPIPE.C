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
 * FrPipe.c
 *
 * $Source: r:/prj/cit/src/RCS/frpipe.c $
 * $Revision: 1.8 $
 * $Author: dc $
 * $Date: 1994/09/06 03:15:09 $
 *
 * Citadel Renderer
 *   pipeline controller
 *  
 * $Log: frpipe.c $
 * Revision 1.8  1994/09/06  03:15:09  dc
 * well, why not try and sort a little better, eh
 * really, though, it just doesnt work
 * 
 * Revision 1.7  1994/09/05  06:43:47  dc
 * diamond pipe, second wrong pipe, various fixes
 * 
 * Revision 1.6  1994/08/19  05:03:54  dc
 * diagonals with the correct height... how odd
 * 
 * Revision 1.5  1994/08/04  23:48:56  dc
 * no seen bits through hack cameras...
 * 
 * Revision 1.4  1994/04/23  09:06:49  dc
 * seen bit stuff
 * 
 * Revision 1.3  1994/04/06  07:19:14  dc
 * seen bits, clear dealt bits... soon we will use dealt bits in frobj and life will be good
 * 
 * Revision 1.2  1994/02/13  05:46:22  dc
 * sort
 * 
 * Revision 1.1  1994/01/02  17:12:05  dc
 * Initial revision
 * 
 */

#include <stdlib.h>
#include <string.h>

#define __FRPIPE_SRC

#include "map.h"
#include "mapflags.h"
#include "frsubclp.h"

#include "frintern.h"
#include "frspans.h"
#include "frtables.h"
#include "frquad.h"
#include "frparams.h"
#include "frflags.h"
#include "fr3d.h"    // for coor

#include "refstuf.h"

//#include <mprintf.h>

// tell me tell me what you're after
// cause i just want to get there faster

// externed for others in frintern
MapElem *fr_map_base;
int fr_map_x,fr_map_y,fr_map_z;
uchar *x_span_lists;
uchar *cone_span_list;

int _fr_x_cen, _fr_y_cen;                   /* center tile for eye */

// static bool cyber_on;

/*
static uchar hack_off;
*/

#ifdef _FR_TILEMAP
static int tile_x, tile_y;             /* tilemap x,y */
#endif

#ifdef PIPE_POINTUP
// set_point_parms
int (*set_point_parms)(g3s_phandle phd, int trans_off, int flrciel, int hgt);
// actual point parm thingies
int set_cspace_color(g3s_phandle phd, int trans_off, int flrciel, int hgt);
int set_texture_i(g3s_phandle phd, int trans_off, int flrciel, int hgt);
int set_null_vrtx(g3s_phandle phd, int trans_off, int flrciel, int hgt);
#endif

// see header file for defines/layout graph
uchar quad_code_to_mask_2[]=
{
   FMK_NW|FMK_EW|FMK_WW,
   FMK_NW|FMK_WW,
   FMK_NW|FMK_EW,
   FMK_EW|FMK_NW|FMK_SW,
   FMK_EW|FMK_NW,
   FMK_EW|FMK_SW,   
   FMK_SW|FMK_EW|FMK_WW,
   FMK_SW|FMK_EW,
   FMK_SW|FMK_WW,
   FMK_WW|FMK_NW|FMK_SW,
   FMK_WW|FMK_SW,   
   FMK_WW|FMK_NW,   
   FMK_EW|FMK_WW|FMK_SW|FMK_NW
};

static char  diag_moves[4][2]={{1,1},{1,-1},{-1,-1},{-1,1}};
static short diag_map_moves[4]={0xdead,0xbeef,0xdead,0xbeef};
static char  diag_stupid[4][2]={{0,1},{1,0},{0,-1},{-1,0}};
static char  diag_dirsets[4][2]={{2,1},{3,2},{0,3},{1,0}};

// Protoypes
void _fr_init_slopes(int zshf);
void do_seen_pass(void);
void draw_dir_dir(int dir, int st, int len);
int fr_pipe_go_3(void);


// the triangle is parm*n, 1<<z*n, 1, so we solve for n for each parm 0-31 and then get x,y
// ie. n^2=1/(((1<<z)^2)+(parm^2))
void _fr_init_slopes(int zshf)
{
   int loop;
   fix parm, lvl, n, tmp;
   extern fix outer_wall[3][4][2];

   _fr_fhgt_step=fix_make(0,0x10000>>zshf);
   lvl=fix_make((1<<zshf),0);
   for (loop=0; loop<HGT_STEPS; loop++)
   {
      parm=fix_make(loop,0);
      tmp=fix_mul(parm,parm)+fix_mul(lvl,lvl);
      n=fix_sqrt(tmp);
      slope_norm[loop][0]=fix_div(parm,n);
      slope_norm[loop][1]=fix_div(lvl,n);
      slope_norm[loop][2]=-slope_norm[loop][0];              // negative horiz for speed
      _fr_fhgt_list[loop]=_fr_fhgt_step*loop;
      _fr_sfuv_list[loop]=(_fr_fhgt_step*(HGT_STEPS-loop))>>8;
      _fr_sfuv_list[HGT_STEPS+loop]=(_fr_fhgt_step*(-loop))>>8;
   }
   _fr_fhgt_list[loop]=_fr_fhgt_step*loop;                   // make sure we get parm 32 for ceil
   _fr_sfuv_list[HGT_STEPS+loop]=(_fr_fhgt_step*(-HGT_STEPS))>>8;
   outer_wall[0][0][0]=outer_wall[0][3][0]=0;
   outer_wall[0][1][0]=outer_wall[0][2][0]=fix_make(1,0);
   outer_wall[0][0][1]=outer_wall[0][1][1]=_fr_fhgt_list[loop];
}

/* called by fr_compile_restart when size changes
 * this knows size of world and stuff like that
 */
int fr_pipe_resize(int x, int y, int z, void *mptr)
{
   int i;
   extern fix tf_diag_walls[4][2];
   fr_map_y=y;
   csp_trans_add[1]=wall_adds[0]=fr_map_x=x;
   wall_adds[2]=-wall_adds[0];
   csp_trans_add[2]=wall_adds[0]+wall_adds[1];
   _fr_init_slopes(fr_map_z=z);
   fr_pts_resize(x,y);
   fr_clip_resize(x,y);
   tf_diag_walls[0][1]=tf_diag_walls[1][1]=fix_make(HGT_STEPS,0)>>z;
   fr_map_base=(MapElem *)mptr;
   for (i=0; i<4; i++)
      diag_map_moves[i]=diag_moves[i][0]+(fr_map_x*diag_moves[i][1]);
   _fr_ret;
}

// currently all pipe memory is static, so this is easy
int fr_pipe_freemem(void)
{
   _fr_ret;
}

/* called at the beginning of every frame, sets up 3d variables for the world
 * also sets up the globals used in the clippers
 */
//#pragma disable_message(202)
int fr_pipe_start(int /*rad*/)
{
   _fr_x_cen=coor(EYE_X)>>(8+MAP_SH);
   _fr_y_cen=coor(EYE_Y)>>(8+MAP_SH);
#ifdef _FR_TILEMAP
	{
      LGPoint p;
      TileMapGetCursor(NULL,&p);
      tile_x=p.x; tile_y=p.y;
      if (fr_highlights)
       { TileMapClearHighlights(NULL); TileMapRedrawSquares(NULL,NULL); }
   }
#endif // _FR_TILEMAP
#ifdef NOT_IMPLEMENTED
// hack_off=fr_detail_master; if (hack_off==3) hack_off=2;
#endif
   fr_terr_frame_start();
   fr_clip_frame_start();
   fr_pts_frame_start();
   ObjsClearDealt();
   CitrefsClearDealt();

   _fr_ret;
}
//#pragma enable_message(202)

/* cleanup any memory or variable space used by the pipeline */
int fr_pipe_end(void)
{
   fr_terr_frame_end();
   fr_clip_frame_end();
   _fr_ret;
}

// claiming i stepped out of line
// which forced you to leave me 
// as if that idea was mine
//    oh you stupid thing, speaking of course as your dear departed
//    oh you stupid thing, it wasnt me that you outsmarted
//    oh you stupid thing, stopping it all before it even started
extern void dumb_hack_for_now(int x, int y);
#define SEEN_SIZE 1
void do_seen_pass(void)
{
   int lx,ly,tx,ty,x;
   MapElem *cur_mpt, *base_mpt;
   lx=_fr_x_cen-SEEN_SIZE; tx=lx+(SEEN_SIZE<<1); if (lx<0) lx=0; if (tx>=MAP_XSIZE) tx=MAP_XSIZE-1;
   ly=_fr_y_cen-SEEN_SIZE; ty=ly+(SEEN_SIZE<<1); if (ly<0) ly=0; if (ty>=MAP_YSIZE) ty=MAP_YSIZE-1;
   base_mpt=MAP_GET_XY(lx,ly);
   for (;ly<=ty;ly++,base_mpt+=MAP_XSIZE)
      for (x=lx,cur_mpt=base_mpt;x<=tx;x++,cur_mpt++)
         if (me_subclip(cur_mpt)!=SUBCLIP_OUT_OF_CONE)
            me_bits_seen_set(cur_mpt);
}

/* Hey
 * new diamond scan pipeline, eh?
 * direction codes are 1 NE, 2 SE, 3 SW, 4 NW
 */
void draw_dir_dir(int dir, int st, int len)
{
   short dmm=diag_map_moves[dir], dmx=diag_moves[dir][0], dmy=diag_moves[dir][1];
   if (st!=0)
   {
      _fdt_mptr+=dmm*st;
      _fdt_x+=dmx*st;
      _fdt_y+=dmy*st;
   }
   while (len-->0)
   {
      if ((_fdt_x>=0)&&(_fdt_x<fr_map_x)&&(_fdt_y>=0)&&(_fdt_y<fr_map_y))
	      if (me_subclip(_fdt_mptr)!=SUBCLIP_OUT_OF_CONE)
		   {
		      dumb_hack_for_now(_fdt_x,_fdt_y);
		      fr_draw_tile();
	      }
      _fdt_mptr+=dmm;
      _fdt_x+=dmx;
      _fdt_y+=dmy;
   }
}

extern ushort frpipe_dist; // furtherest walking distance away

#define Q_N 0
#define Q_E 1
#define Q_S 2
#define Q_W 3
#define QoL(a,b,c,d) {Q_##a##,Q_##b##,Q_##c##,Q_##d##}

uchar quad_order_lists[8][4]=
{
   QoL(S,W,E,N),QoL(W,S,N,E),QoL(W,N,S,E),QoL(N,W,E,S),
   QoL(N,E,W,S),QoL(E,N,S,W),QoL(E,S,N,W),QoL(S,E,W,N)
};

int fr_pipe_go_3(void)
{
   int i, j, p_dir;  // p_dir is how many per diagonal element
   uchar *loc_code_ptr, *quad_order;
   short clip_len[4]; // , clip_contrib[4];
   MapElem *endcaps[4], *center=MAP_GET_XY(_fr_x_cen,_fr_y_cen);

//   mprintf("\npipedist %d center %x %x\n",frpipe_dist,_fr_x_cen,_fr_y_cen);
   fr_rend_start();
   if ((_fr_curflags&FR_HACKCAM_MASK)==0)
	   do_seen_pass();

   // use clip len as temp for delta within the square
   clip_len[0]=(short)((fr_camera_last[0]&0xffff)-0x8000);
   clip_len[1]=(short)((fr_camera_last[1]&0xffff)-0x8000);

   if (abs(clip_len[1])>abs(clip_len[0]))
      if (clip_len[0]>0) if (clip_len[1]>0) p_dir=0; else p_dir=3;
      else               if (clip_len[1]>0) p_dir=7; else p_dir=4;
   else
      if (clip_len[1]>0) if (clip_len[0]>0) p_dir=2; else p_dir=6;
      else               if (clip_len[0]>0) p_dir=3; else p_dir=5;
   quad_order=quad_order_lists[p_dir];

   _fdt_dist=frpipe_dist;
   // initialize our wacked out setup
   endcaps[0]=center+(fr_map_x*frpipe_dist); endcaps[2]=center-(fr_map_x*frpipe_dist);
   endcaps[1]=center+frpipe_dist;            endcaps[3]=center-frpipe_dist;
   clip_len[0]=_fr_y_cen+_fdt_dist-fr_map_y; clip_len[2]=_fdt_dist-_fr_y_cen;
   clip_len[1]=_fr_x_cen+_fdt_dist-fr_map_x; clip_len[3]=_fdt_dist-_fr_x_cen;

   while (_fdt_dist>0) // move in from the maximal distance
   {
      p_dir=(_fdt_dist+1)>>1;    // how many per diagonal grouping, including

      for (i=0; i<4; i++)
      {
         j=quad_order[i];
	      loc_code_ptr=&quad_code_to_mask_2[j*QUAD2_DELTA];
         _fdt_x=_fr_x_cen+(diag_stupid[j][0]*_fdt_dist);
         _fdt_y=_fr_y_cen+(diag_stupid[j][1]*_fdt_dist);
         _fdt_mptr=endcaps[j];
         if (clip_len[j]<0)
         {
            if (me_subclip(_fdt_mptr)!=SUBCLIP_OUT_OF_CONE)
            {
	            _fdt_mask=(int)*loc_code_ptr;      // the endcap
	            dumb_hack_for_now(_fdt_x,_fdt_y);
	            fr_draw_tile();
            }
         }
         if (clip_len[j]<p_dir)
         {
	         loc_code_ptr++; // go to the right fork...
	         _fdt_mask=(int)*loc_code_ptr;
	         draw_dir_dir(diag_dirsets[j][0],1,p_dir-1);
	
            // for now just hard restore these three, get cooler later
            _fdt_x=_fr_x_cen+(diag_stupid[j][0]*_fdt_dist);
            _fdt_y=_fr_y_cen+(diag_stupid[j][1]*_fdt_dist);
            _fdt_mptr=endcaps[j];

	         loc_code_ptr++; // and then the left fork
	         _fdt_mask=(int)*loc_code_ptr;
	         draw_dir_dir(diag_dirsets[j][1],1,p_dir-1);
         }
         clip_len[j]--;
         endcaps[j]-=wall_adds[j];
      }
      if ((_fdt_dist&1)==0)    // do the perfect diagonal
      {
         int d_dist=_fdt_dist>>1;
//         mprintf("Gonna try diagonals, uh-huh dist %d dd %d\n",_fdt_dist,d_dist);
         for (i=0; i<4; i++)
         {                     // go through each quadrant, dealing
	         j=quad_order[i];
		      loc_code_ptr=&quad_code_to_mask_2[(j*QUAD2_DELTA)+QUAD2_LEFT_FORK];
            _fdt_x=_fr_x_cen+(diag_moves[j][0]*d_dist);
            _fdt_y=_fr_y_cen+(diag_moves[j][1]*d_dist);
		      if ((_fdt_x>=0)&&(_fdt_x<fr_map_x)&&(_fdt_y>=0)&&(_fdt_y<fr_map_y))
            {
               _fdt_mptr=center+diag_map_moves[j]*d_dist;
			      if (me_subclip(_fdt_mptr)!=SUBCLIP_OUT_OF_CONE)
				   {
                  _fdt_mask=quad_code_to_mask_2[(j*QUAD2_DELTA)+QUAD2_LEFT_FORK];
				      dumb_hack_for_now(_fdt_x,_fdt_y);
				      fr_draw_tile();
			      }
            }
	      }
      }
      _fdt_dist--;      
   }

   if (me_subclip(center)!=SUBCLIP_OUT_OF_CONE)
   {
	 	_fdt_mask=(int)quad_code_to_mask_2[QUAD2_CENTER];
      _fdt_x=_fr_x_cen; _fdt_y=_fr_y_cen; _fdt_mptr=center;
      dumb_hack_for_now(_fdt_x,_fdt_y);
   	fr_draw_tile();
   }
#ifndef CLEAR_AS_WE_GO
   for (j=0; j<64; j++)
      span_count(j)=0;
#endif

   _fr_ret;
}
