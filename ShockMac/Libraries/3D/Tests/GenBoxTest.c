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
 * $Source: n:/project/lib/src/3d/test/RCS/bitmap.c $
 * $Revision: 1.1 $
 * $Author: kaboom $
 * $Date: 1993/07/28 21:03:55 $
 */

#define __FAUXREND_SRC

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "2d.h"
#include "2dRes.h"
#include "3d.h"

#define pitch        tx
#define bank         tz
#define head         ty

long fr_clear_color = 0xff;

// prototypes
void test_3d(void);
void setup_box_side(fix boxsize, int whichside, g3s_phandle *trans_p);


g3s_vector viewer_position;
g3s_angvec viewer_orientation;

#define coor(val) (fix_make((eye[val]>>MAP_SH),(eye[val]&MAP_MK)<<MAP_MS))
#define ang(val)  (eye[val])
#define setvec(vec,x,y,z) {vec.xyz[0] = x; vec.xyz[1] = y; vec.xyz[2] = z;}

#define box_front 0
#define box_back 1
#define box_left 2
#define box_right 3
#define box_top 4
#define box_bottom 5


// setup polygon for a box side (0-5, front, back, left, right, top, bottom)
void setup_box_side(fix boxsize, int whichside, g3s_phandle *trans_p)
 {
	g3s_vector cur_vec;
	
	switch (whichside)
	 {
	 	case box_front:		setvec(cur_vec, -boxsize,-boxsize,-boxsize); 
	 										trans_p[0]=g3_transform_point(&cur_vec);
	 										setvec(cur_vec, boxsize,-boxsize,-boxsize); 
	 										trans_p[1]=g3_transform_point(&cur_vec);
	 										setvec(cur_vec, boxsize,boxsize,-boxsize); 
	 										trans_p[2]=g3_transform_point(&cur_vec);
	 										setvec(cur_vec, -boxsize,boxsize,-boxsize); 
	 										trans_p[3]=g3_transform_point(&cur_vec);
	 										
											trans_p[0]->uv.u = 0; trans_p[0]->uv.v = 0; 
											trans_p[1]->uv.u = 256; trans_p[1]->uv.v = 0; 
											trans_p[2]->uv.u = 256; trans_p[2]->uv.v = 256; 
											trans_p[3]->uv.u = 0; trans_p[3]->uv.v = 256; 
	 										break;
	 	
	 	case box_back: 		setvec(cur_vec, -boxsize,-boxsize,boxsize); 
	 										trans_p[3]=g3_transform_point(&cur_vec);
	 										setvec(cur_vec, boxsize,-boxsize,boxsize); 
	 										trans_p[2]=g3_transform_point(&cur_vec);
	 										setvec(cur_vec, boxsize,boxsize,boxsize); 
	 										trans_p[1]=g3_transform_point(&cur_vec);
	 										setvec(cur_vec, -boxsize,boxsize,boxsize); 
	 										trans_p[0]=g3_transform_point(&cur_vec);
	 										
											trans_p[3]->uv.u = 0; trans_p[3]->uv.v = 0; 
											trans_p[2]->uv.u = 256; trans_p[2]->uv.v = 0; 
											trans_p[1]->uv.u = 256; trans_p[1]->uv.v = 256; 
											trans_p[0]->uv.u = 0; trans_p[0]->uv.v = 256; 
	 										break;

	 	case box_left:		setvec(cur_vec, -boxsize,-boxsize,-boxsize); 
	 										trans_p[3]=g3_transform_point(&cur_vec);
	 										setvec(cur_vec, -boxsize,-boxsize,boxsize); 
	 										trans_p[2]=g3_transform_point(&cur_vec);
	 										setvec(cur_vec, -boxsize,boxsize,boxsize); 
	 										trans_p[1]=g3_transform_point(&cur_vec);
	 										setvec(cur_vec, -boxsize,boxsize,-boxsize); 
	 										trans_p[0]=g3_transform_point(&cur_vec);
	 										
											trans_p[3]->uv.u = 0; trans_p[3]->uv.v = 0; 
											trans_p[2]->uv.u = 256; trans_p[2]->uv.v = 0; 
											trans_p[1]->uv.u = 256; trans_p[1]->uv.v = 256; 
											trans_p[0]->uv.u = 0; trans_p[0]->uv.v = 256; 
	 										break;
	 	case box_right:		setvec(cur_vec, boxsize,-boxsize,-boxsize); 
	 										trans_p[0]=g3_transform_point(&cur_vec);
	 										setvec(cur_vec, boxsize,-boxsize,boxsize); 
	 										trans_p[1]=g3_transform_point(&cur_vec);
	 										setvec(cur_vec, boxsize,boxsize,boxsize); 
	 										trans_p[2]=g3_transform_point(&cur_vec);
	 										setvec(cur_vec, boxsize,boxsize,-boxsize); 
	 										trans_p[3]=g3_transform_point(&cur_vec);
	 										
											trans_p[0]->uv.u = 0; trans_p[0]->uv.v = 0; 
											trans_p[1]->uv.u = 256; trans_p[1]->uv.v = 0; 
											trans_p[2]->uv.u = 256; trans_p[2]->uv.v = 256; 
											trans_p[3]->uv.u = 0; trans_p[3]->uv.v = 256; 
	 										break;
	 	case box_top:			setvec(cur_vec, -boxsize,-boxsize,-boxsize); 
	 										trans_p[0]=g3_transform_point(&cur_vec);
	 										setvec(cur_vec, -boxsize,-boxsize,boxsize); 
	 										trans_p[1]=g3_transform_point(&cur_vec);
	 										setvec(cur_vec, boxsize,-boxsize,boxsize); 
	 										trans_p[2]=g3_transform_point(&cur_vec);
	 										setvec(cur_vec, boxsize,-boxsize,-boxsize); 
	 										trans_p[3]=g3_transform_point(&cur_vec);
	 										
											trans_p[0]->uv.u = 0; trans_p[0]->uv.v = 0; 
											trans_p[1]->uv.u = 256; trans_p[1]->uv.v = 0; 
											trans_p[2]->uv.u = 256; trans_p[2]->uv.v = 256; 
											trans_p[3]->uv.u = 0; trans_p[3]->uv.v = 256; 
	 										break;
	 	case box_bottom:	setvec(cur_vec, -boxsize,boxsize,-boxsize); 
	 										trans_p[3]=g3_transform_point(&cur_vec);
	 										setvec(cur_vec, -boxsize,boxsize,boxsize); 
	 										trans_p[2]=g3_transform_point(&cur_vec);
	 										setvec(cur_vec, boxsize,boxsize,boxsize); 
	 										trans_p[1]=g3_transform_point(&cur_vec);
	 										setvec(cur_vec, boxsize,boxsize,-boxsize); 
	 										trans_p[0]=g3_transform_point(&cur_vec);
	 										
											trans_p[3]->uv.u = 0; trans_p[3]->uv.v = 0; 
											trans_p[2]->uv.u = 256; trans_p[2]->uv.v = 0; 
											trans_p[1]->uv.u = 256; trans_p[1]->uv.v = 256; 
											trans_p[0]->uv.u = 0; trans_p[0]->uv.v = 256; 
	 										break;
	 }
 }
 

#define build_fix_angle(ang) ((65536*(ang))/360)
#define num_bitmaps 5

int	face_value[6] = {0,1,2,3,4,1};

uchar pal_buf[768];
uchar bitmap_buf[70000];

void test_3d(void)
 {
	g3s_phandle 	trans[8];
	int 					i,j,c=0;
	grs_bitmap 		bm[num_bitmaps];
	EventRecord 	evt;
	grs_screen 		*screen;
	g3s_vector		vec;
	g3s_angvec		ang;
	int						dx = 0, dy = 0, dz = 0;
	grs_canvas 		*off_canvas;
	FrameDesc			*fd;
	fix						vx,vy,vz;
	int						ddx = 0, ddy = 0, ddz = 0;
	long					time,frames;
	Str255				str;
	FILE        	*fp;
	uchar					*temp_ptr;
	long				  top,left,bot,right;
	long					Ntop,Nleft,Nbot,Nright;
	long					Mtop,Mleft,Mbot,Mright;
	Rect					r;
	grs_bitmap		sub_bm;
/*	g3s_vector		v;
	
	v.gX = fix_make(100,10);
	v.gY = fix_make(200,200);
	v.gZ = fix_make(300,00);
	time=TickCount();
	for (j=0; j<600000; j++)
	  vx = g3_vec_mag(&v);
  time=TickCount()-time;
  NumToString(time,str);
  DebugStr(str); 
  NumToString(vx,str);
  DebugStr(str); 
  return;
	*/
	
	fp = fopen("pal.dat","rb");
	fread (pal_buf, 1, 768, fp);
	fclose (fp);

	fp = fopen("bit.dat","rb");
	fread (bitmap_buf, 1, 69772, fp);
	fclose (fp);
	
	temp_ptr = bitmap_buf;
	
	for (i=0; i<num_bitmaps; i++)
	 {
		fd = (FrameDesc *) temp_ptr;
		fd->bm.bits = (uchar *)(fd+1);
		bm[i] = fd->bm;
	 
	 	temp_ptr += 16412;
	 }
	
	gr_init();
	gr_set_mode(GRM_640x480x8, TRUE);
	screen = gr_alloc_screen(640,480);
	gr_set_screen (screen);
	
	gr_set_pal(0, 256, (uchar *) pal_buf);
	gr_set_fcolor(fr_clear_color);
	
	off_canvas = gr_alloc_canvas(BMT_FLAT8,640,480);
	gr_set_canvas(off_canvas);
	gr_clear(fr_clear_color);
	gr_set_canvas(grd_screen_canvas);
	
	g3_init(80,AXIS_RIGHT,AXIS_DOWN,AXIS_IN);
	
	viewer_position.gX = viewer_position.gY = 0;
	viewer_position.gZ = fix_make(-60,0);
	viewer_orientation.head = viewer_orientation.bank = viewer_orientation.head = 0;

	vec.gX = 0;
	vec.gY = 0;
	vec.gZ = fix_make(5,0);

	vx = 0xc000;
	vy = 0xc000;
	vz = -0xc000;
	ddx = 2;
	ddy = 2;
	
	frames = 0;
	time = TickCount();

	gr_set_canvas(off_canvas);
	top = left = 1000;
	bot = right = 0;

//for (j=0; j<200; j++)
	do
	 {
		gr_set_canvas(off_canvas);
		gr_set_fcolor(fr_clear_color);
//		gr_rect(left,top,right+1,bot+1);		// erase old image
		gr_clear(fr_clear_color);     

		g3_start_frame();
		g3_set_view_angles(&viewer_position,&viewer_orientation,ORDER_YXZ,g3_get_zoom('X',build_fix_angle(80),640,480));
		
		dx += ddx;
		dy += ddy;
		dz += ddz;
		
		vec.gX += vx;
		vec.gY += vy;
		vec.gZ += vz;
		
		if ((vec.gZ<fix_make(0,0)) || (vec.gZ>fix_make(130,0))) vz = -vz;
		if ((vec.gX<fix_make(-50,0)) || (vec.gX>fix_make(50,0))) {vx = -vx; ddx = -ddx;}
		if ((vec.gY<fix_make(-40 ,0)) || (vec.gY>fix_make(40,0))) {vy = -vy; ddy = -ddy;}
		 
		Ntop = Nleft = fix_make(1000,0); Nbot = Nright = -1;

		g3_start_object_angles_xyz(&vec,build_fix_angle(dy), build_fix_angle(dx), build_fix_angle(dz), ORDER_YXZ);
		for (i=box_front; i<=box_bottom; i++)
		 {
		 	extern grs_vertex	*p_vpl[];
		 	extern long				_n_verts;
		 	
			setup_box_side(fix_make(10,0), i, trans);
			g3_check_and_draw_tmap_quad_tile(trans, &bm[face_value[i]],1,1);
			
			for (j=0; j<_n_verts; j++)	// get max, min of each coord
			 {
			 	if (p_vpl[j]->x < Nleft) Nleft = p_vpl[j]->x;
			 	if (p_vpl[j]->x > Nright) Nright = p_vpl[j]->x;
			 	if (p_vpl[j]->y < Ntop) Ntop = p_vpl[j]->y;
			 	if (p_vpl[j]->y > Nbot) Nbot = p_vpl[j]->y;
			 }
		 } 
		g3_end_object();
		g3_end_frame();

		gr_set_canvas(grd_screen_canvas);
/*
		Nleft = fix_int(Nleft);
		Ntop = fix_int(Ntop);
		Nbot = fix_int(Nbot);
		Nright = fix_int(Nright);
		
		Mleft = (min(left,Nleft));
		Mtop = (min(top,Ntop));
		Mbot = (max(bot,Nbot));
		Mright = (max(right,Nright));
		gr_init_sub_bm (&off_canvas->bm, &sub_bm, Mleft, Mtop, (Mright-Mleft)+1, (Mbot-Mtop)+1);
		gr_bitmap(&sub_bm, Mleft, Mtop);
		left = Nleft; top = Ntop; bot = Nbot; right = Nright;
*/
		gr_bitmap(&off_canvas->bm,0,0);	

	 	frames++;
		if (TickCount()-time>=60L)
		 {
			NumToString(frames,str);
			SetRect(&r,0,-20,32,0);
			EraseRect(&r);
			MoveTo(2,-8);
			DrawString(str);
			
			frames = 0;
			time = TickCount();
		 }
	 }
	while (!Button());
   
/*  time = TickCount()-time;
  NumToString(time,str);
  DebugStr(str); */
   
	g3_shutdown();
  gr_close();
}
