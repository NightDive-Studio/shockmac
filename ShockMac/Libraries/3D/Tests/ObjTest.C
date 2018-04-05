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


g3s_vector viewer_position;
g3s_angvec viewer_orientation;

#define coor(val) (fix_make((eye[val]>>MAP_SH),(eye[val]&MAP_MK)<<MAP_MS))
#define ang(val)  (eye[val])

#define build_fix_angle(ang) ((65536*(ang))/360)
#define num_bitmaps 5

char bigbuff[20000];

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
	Handle				palRes,bmres[num_bitmaps],shade1;
	FrameDesc			*fd;
	Handle				object;
	long					size;
	int						id = 2300;
		
	palRes = GetResource('pal ',1000);
	HLock(palRes);
	
	for (i=0; i<num_bitmaps; i++)
	 {
		bmres[i] = GetResource('sIMG',1000+i);
		HLock(bmres[i]);
		fd = (FrameDesc *) *bmres[i];
		fd->bm.bits = (uchar *)(fd+1);
		bm[i] = fd->bm;
	 }
	
	gr_init();
	gr_set_mode(GRM_640x480x8, TRUE);
	screen = gr_alloc_screen(640,480);
	gr_set_screen (screen);
	
	gr_set_pal(0, 256, (uchar *) *palRes);
 	gr_alloc_ipal();
	gr_init_blend(1);
	shade1 = GetResource('shad',1000);
	HLock(shade1);
	gr_set_light_tab((uchar *) *shade1);

	gr_set_fcolor(fr_clear_color);
	
	off_canvas = gr_alloc_canvas(BMT_FLAT8,640,480);
	gr_set_canvas(off_canvas);
	gr_clear(fr_clear_color);
	gr_set_canvas(grd_screen_canvas);
	
	g3_init(80,AXIS_RIGHT,AXIS_DOWN,AXIS_IN);
	
	viewer_position.gX = viewer_position.gY = 0;
	viewer_position.gZ = fix_make(-1,0);
	viewer_orientation.head = viewer_orientation.bank = viewer_orientation.head = 0;

	vec.gX = 0;
	vec.gY = 0;
	vec.gZ = fix_make(0,8000);

	object = GetResource('sO3D',id);
	HLock(object);
	size = GetHandleSize(object);
	BlockMove(*object,bigbuff,size);

	do
	 {	 	
		gr_set_canvas(off_canvas);
		gr_clear(fr_clear_color);     
		gr_clear(0);     

		g3_start_frame();
		g3_set_view_angles(&viewer_position,&viewer_orientation,ORDER_YXZ,g3_get_zoom('X',build_fix_angle(80),640,480));
		 		 
		dx+=2;
		dy+=2;
		 		 
		g3_start_object_angles_xyz(&vec,build_fix_angle(dy), build_fix_angle(dx), build_fix_angle(dz), ORDER_YXZ);
	
		BlockMove(bigbuff,*object,size);

#define PARM_MAX  (0x0f8)
#define PARM_MOD  (PARM_MAX<<1)
#define PARM_SHF  (10)
#define PARM_BASE (16)
 {
  int pos_parm=abs(PARM_MAX-(TickCount()&PARM_MOD));         // this is dumb
	
	g3_interpret_object((ubyte *) (*object)+0x0C,((PARM_MAX+PARM_BASE)-pos_parm)<<PARM_SHF,PARM_BASE<<PARM_SHF);
 }	
//		g3_check_and_draw_tmap_quad_tile(trans, &bm[0],1,1);
		g3_end_object();
		g3_end_frame();

		gr_set_canvas(grd_screen_canvas);
		gr_bitmap(&off_canvas->bm,0,0);	
		
		if (Button())
		 {
		 	ReleaseResource(object);
		 	
			id++;
		 	object = GetResource('sO3D',id);
			HLock(object);
			size = GetHandleSize(object);
			BlockMove(*object,bigbuff,size);
			Delay(60L,(long *) &c);
		 }
	 }
	while (id<2380);
     
	g3_shutdown();
  gr_close();
}
