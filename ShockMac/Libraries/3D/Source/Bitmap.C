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
//
// $Source: r:/prj/lib/src/3d/RCS/bitmap.asm $
// $Revision: 1.16 $
// $Author: kevin $
// $Date: 1994/09/08 21:52:16 $
//
// 3d bitmap routines.
//
// $Log: bitmap.asm $
// Revision 1.16  1994/09/08  21:52:16  kevin
// Check bitmap size before blending, don't blend translucent bitmaps.
// 
// Revision 1.15  1994/09/07  23:48:52  kevin
// Streamlined interface to 2d, added blending support.
// 
// Revision 1.14  1994/08/18  03:46:54  jaemz
// Changed stereo glob names to have underscore for c
// 
// Revision 1.13  1994/08/04  16:35:54  jaemz
// Added end statement to return these to being real programs
// 
// Revision 1.12  1994/07/19  13:48:32  jaemz
// Added support for stereo
// 
// Revision 1.11  1994/06/02  15:06:40  junochoe
// changed matrix_scale to _matrix_scale
// 
// Revision 1.10  1994/03/12  20:32:22  kevin
// Added overflow checks to avoid passing bogus points to the 2d.
// Also cleaned things up a bit in general.
// 
// Revision 1.9  1994/02/08  20:43:53  kaboom
// Added support for rest of bitmap types.
// 
// Revision 1.8  1993/12/21  17:51:08  kevin
// Hacked in rsd8 support.
// 
// Revision 1.7  1993/12/04  16:59:56  kaboom
// Added lighting support.
// 
// Revision 1.5  1993/10/30  18:37:41  kaboom
// Changed SCALE_BITMAP constant to SCALE_FLAT8_BITMAP.
// 
// Revision 1.4  1993/10/22  09:31:44  kaboom
// Updated call to linear mapper to use new calling convention.
// 
// Revision 1.3  1993/10/02  09:24:33  kaboom
// Rewrote rolling routine to use the new uv linear mapper.  Also changed names
// of scrw and scrh to _scrw and _scrh.
// 
// Revision 1.2  1993/09/07  19:37:45  kaboom
// Updated LIN_MAP constant to FLAT8_LMAP.
//
// Revision 1.1  1993/07/08  23:37:34  kaboom
// Initial revision
//

#include "lg.h"
#include "3d.h"
#include "GlobalV.h"

// need this from 2D lib
extern int h_map(grs_bitmap *bm, int n, grs_vertex **vpl, grs_tmap_info *ti);

fix 	_g3d_bitmap_x_scale = 0x010000;
fix 	_g3d_bitmap_y_scale = 0x010000;
fix 	_g3d_bitmap_x_iscale = 0x010000;
fix 	_g3d_bitmap_y_iscale = 0x010000;
long 	_g3d_bitmap_u_anchor = 0;
long 	_g3d_bitmap_v_anchor = 0;
fix 	_g3d_roll_matrix[6];   
uchar *_g3d_bitmap_clut;      
int 	_g3d_light_flag;     

grs_vertex 		**_g3d_bitmap_poly;     
grs_vertex		vlist[16];	            
grs_vertex		*vpl[] = {&vlist[0], &vlist[1], &vlist[2], &vlist[3]};

grs_tmap_info tmap_info;

char 	_g3d_enable_blend = 0;

// prototypes
#if (defined(powerc) || defined(__powerc))	
char SubLongWithOverflow(long *result, long src, long dest);
char AddLongWithOverflow(long *result, long src, long dest);
#else
asm char SubLongWithOverflow(long *result, long src, long dest);
asm char AddLongWithOverflow(long *result, long src, long dest);
#endif

grs_vertex **do_bitmap(grs_bitmap *bm, g3s_phandle p);
grs_vertex **g3_bitmap_common(grs_bitmap *bm, g3s_phandle p);

// arguments:
//   ebx: u scale factor
//   ecx: v scale factor
void g3_set_bitmap_scale (fix u_scale, fix v_scale)
 {
 	_g3d_bitmap_x_scale = fix_mul(fix_mul(_matrix_scale.gX,u_scale),_scrw);
	_g3d_bitmap_x_iscale = AsmWideDivide(1,0,_g3d_bitmap_x_scale);

 	_g3d_bitmap_y_scale = fix_mul(fix_mul(_matrix_scale.gY,v_scale),_scrh);
	_g3d_bitmap_y_iscale = AsmWideDivide(1,0,_g3d_bitmap_y_scale);
 }
 
grs_vertex **g3_full_light_bitmap(grs_bitmap *bm, grs_vertex **p)
 {
 	_g3d_light_flag = 1;
 	_g3d_bitmap_poly = p;
 	return(do_bitmap(bm,(g3s_phandle) p));
 }
 
grs_vertex **g3_full_light_anchor_bitmap(grs_bitmap *bm, grs_vertex **p, short u_anchor, short v_anchor)
 {
 	_g3d_light_flag = 1;
 	_g3d_bitmap_u_anchor = u_anchor;
 	_g3d_bitmap_v_anchor = v_anchor;
 	_g3d_bitmap_poly = p;
 	return(g3_bitmap_common(bm,(g3s_phandle) p));
 }

grs_vertex **g3_light_anchor_bitmap(grs_bitmap *bm, g3s_phandle p, short u_anchor, short v_anchor)
 {
 	_g3d_light_flag = 2;
 	_g3d_bitmap_u_anchor = u_anchor;
 	_g3d_bitmap_v_anchor = v_anchor;
 	_g3d_bitmap_clut = (p->i & 0x00ff00)+grd_screen->ltab;
 	_g3d_bitmap_poly = vpl;
 	return(g3_bitmap_common(bm,p));
 }

grs_vertex **g3_light_bitmap(grs_bitmap *bm, g3s_phandle p)
 {
 	_g3d_light_flag = 2;
 	_g3d_bitmap_clut = (p->i & 0x00ff00)+grd_screen->ltab;
 	_g3d_bitmap_poly = vpl;
 	return(do_bitmap(bm,p));
 }
 
grs_vertex **g3_anchor_bitmap(grs_bitmap *bm, g3s_phandle p, short u_anchor, short v_anchor)
 {	
 	_g3d_light_flag = 0;
 	_g3d_bitmap_u_anchor = u_anchor;
 	_g3d_bitmap_v_anchor = v_anchor;
 	_g3d_bitmap_poly = vpl;
 	return(g3_bitmap_common(bm,p));
 } 
 

grs_vertex **g3_bitmap (grs_bitmap *bm, g3s_phandle p)
 {
 	_g3d_light_flag = 0;
 	_g3d_bitmap_poly = vpl;
 	return(do_bitmap(bm,p));
 }
 
grs_vertex **do_bitmap(grs_bitmap *bm, g3s_phandle p)
 {
 	_g3d_bitmap_u_anchor = bm->w>>1;
 	_g3d_bitmap_v_anchor = bm->h-1;
 	return(g3_bitmap_common(bm,p));
 }
 
grs_vertex **g3_bitmap_common(grs_bitmap *bm, g3s_phandle p)
 {
 	fix					tempF,tempF2;
 	long				tempL,tempL2;
 	short				tempS,tempS2;
  fix					sintemp,costemp;
	grs_vertex	*tempG1,*tempG2;
	fix					tempResult;
	long				bm_w,bm_h;
	fix			 	  dx,dy;
	
// MLA- these were globals, I made them locals for PPC speed, they aren't referenced externally
	long	rm0;
	long	rm1;
	long	rm2;
	long	rm3;
  
#ifdef stereo_on
	if (_g3d_stereo & 1)
	 {
	 
        ; edi is point handle
        pushm   edi,esi
        call g3_bitmap_common_raw
        set_rt_canv

        popm    edi,esi
        add     edi,_g3d_stereo_base
        call g3_bitmap_common_raw
        set_lt_canv

        ret

g3_bitmap_common_raw:
	 }
#endif

	if ((p->p3_flags & PF_PROJECTED)==0)
	 	if (g3_project_point(p)==0) p->codes |= CC_CLIP_OVERFLOW;

	if ((p->codes & CC_CLIP_OVERFLOW)!=0)
		return(0L);

	// copy a few things into locals
	bm_w = bm->w;
	bm_h = bm->h;
	
	tempL = p->gZ;			//     mov     eax,[edi].z
	tempL <<= 8;						//     sal     eax,8                   ;should be 16-log(max(bitmap_width, bitmap_height))
  tempF = view_bank;			//     mov     ebx,view_bank

	if ((p->gZ & 0xff000000)==0)	// ;skip checks if z large enough  
	 {
	 	if (tempL<_g3d_bitmap_x_scale) return(0L);
	 	if (tempL<_g3d_bitmap_y_scale) return(0L);
	 }
	 
// compute polygon for bitmap
// args: ebx=bank, esi=bitmap, edi=anchor pt.
//gb_compute_poly:

	fix_sincos((fixang) tempF,&sintemp,&costemp); 	//    call    fix_sincos
   
  tempL = p->gZ;
      											  												
	//  rm0 = cos(bank)*_g3d_bitmap_x_scale/z
	rm0 = fix_mul_div(costemp,_g3d_bitmap_x_scale,tempL);

  // rm1 = sin(bank)*_g3d_bitmap_x_scale/z
	rm1 = fix_mul_div(sintemp,_g3d_bitmap_x_scale,tempL);
   
	// rm2 = -sin(bank)*_g3d_bitmap_y_scale/z
	rm2 = -fix_mul_div(sintemp,_g3d_bitmap_y_scale,tempL);

	// rm3 = cos(bank)*_g3d_bitmap_y_scale/z
	rm3 = fix_mul_div(costemp,_g3d_bitmap_y_scale,tempL);

// 0 x, 1 y, 2 u, 3 v, 4 w, 5 i
	tempG1 = _g3d_bitmap_poly[0];
	
// vpl[0][0] = x-u_anchor*rm0-v_anchor*rm1
	tempL2 = p->sx;
	tempL = _g3d_bitmap_u_anchor*rm0;
	if (SubLongWithOverflow(&tempL2, tempL2, tempL)) return 0;		// tempL2 -= tempL

	tempL = _g3d_bitmap_v_anchor*rm1;
	if (SubLongWithOverflow(&tempL2, tempL2, tempL)) return 0;		// tempL2 -= tempL
	tempG1->x = tempL2;
	        
// vpl[0][1] = y-u_anchor*rm2-v_anchor*rm3
	tempL2 = p->sy;
	tempL = _g3d_bitmap_u_anchor*rm2;
	if (SubLongWithOverflow(&tempL2, tempL2, tempL)) return 0;		// tempL2 -= tempL

	tempL = _g3d_bitmap_v_anchor*rm3;
	if (SubLongWithOverflow(&tempL2, tempL2, tempL)) return 0;		// tempL2 -= tempL
	tempG1->y = tempL2;
                
//  vpl[0][2] = 0
	tempG1->u = 0;
                
//  vpl[0][3] = 0
	tempG1->v = 0;
                
//  vpl[0][4] undefined
                
//  vpl[0][5] provided

	tempG2 = tempG1;
	tempG1 = _g3d_bitmap_poly[1];
                
//  vpl[1][0] = vpl[0][0]+rm0*w
	tempL2 = tempG2->x;
	tempL =  rm0 * bm_w;
	if (AddLongWithOverflow(&tempL2, tempL2, tempL)) return 0;	// tempL2 += tempL
	tempG1->x = tempL2;
	                
//  vpl[1][1] = vpl[0][1]+rm2*w
	tempL2 = tempG2->y;
	tempL =  rm2 * bm_w;
	if (AddLongWithOverflow(&tempL2, tempL2, tempL)) return 0;	// tempL2 += tempL
	tempG1->y = tempL2;
                
//  vpl[1][2] = bitmap.w-1
	tempG1->u = (bm_w-1)<<16;
                
//  vpl[1][3] = 0
  tempG1->v = 0;
                
//  vpl[1][4] undefined
                
//  vpl[1][5] provided

	tempG2 = tempG1;
	tempG1 = _g3d_bitmap_poly[2];
                
//  vpl[2][0] = vpl[1][0]+rm1*h
	tempL2 = tempG2->x;
	tempL =  rm1 * bm_h;
	if (AddLongWithOverflow(&tempL2, tempL2, tempL)) return 0;	// tempL2 += tempL
	tempG1->x = tempL2;
                
//  vpl[2][1] = vpl[1][1]+rm3*h
	tempL2 = tempG2->y;
	tempL =  rm3 * bm_h;
	if (AddLongWithOverflow(&tempL2, tempL2, tempL)) return 0;	// tempL2 += tempL
	tempG1->y = tempL2;
                
//  vpl[2][2] = bitmap.w-1
	tempG1->u = (bm_w-1)<<16;
                
//  vpl[2][3] = bitmap.h-1
	tempG1->v = (bm_h-1)<<16;
                
// vpl[2][4] undefined
                
// vpl[2][5] provided

	tempG2 = _g3d_bitmap_poly[0];
	tempG1 = _g3d_bitmap_poly[3];
                
//  vpl[3][0] = vpl[0][0]+rm1*h
	tempL2 = tempG2->x;
	tempL =  rm1 * bm_h;
	if (AddLongWithOverflow(&tempL2, tempL2, tempL)) return 0;	// tempL2 += tempL
	tempG1->x = tempL2;
                
//  vpl[3][1] = vpl[0][1]+rm3*h
	tempL2 = tempG2->y;
	tempL =  rm3 * bm_h;
	if (AddLongWithOverflow(&tempL2, tempL2, tempL)) return 0;	// tempL2 += tempL
	tempG1->y = tempL2;
                
// vpl[3][2] = 0
  tempG1->u = 0;
                
// vpl[3][3] = bitmap.h
	tempG1->v = bm_h<<16;
                
// vpl[3][4] undefined
                
// vpl[3][5] provided
	tmap_info.flags = 0;
	if (_g3d_light_flag!=2)	goto NoBlend; // do blending?

	tmap_info.clut = _g3d_bitmap_clut;
	tmap_info.flags = TMF_CLUT;
	
	if (!_g3d_enable_blend) goto NoBlend; 
	
// don't blend if translucent or compressed translucent bitmap
	if ((bm->type==BMT_TLUC8) || (bm->flags==BMF_TLUC8)) goto NoBlend;  

// check for bitmap width<polygon width, if so, blend

// MLA - have no idea how this code could ever work, so I'm trying my own code
	dx = _g3d_bitmap_poly[0]->x - _g3d_bitmap_poly[2]->x;
	if (dx<0) dx = -dx;
	dy = _g3d_bitmap_poly[0]->y - _g3d_bitmap_poly[2]->y;
	if (dy<0) dy = -dy;
		
	if (dy > dx) dx = dy;	// get max value in dx
	
	dx >>= 0x11;					// shift down because bm_w isn't fixed, and we only double when twice the size
	if (bm_w > dx) goto NoBlend;
		
// make sure doubled bitmap won't overflow unpack buffer (that's roughly 1/5 of 64k)
	if (bm_w * bm_h > 12000) goto NoBlend;
		
// first fix all u's and v's
	for (tempL=0; tempL<4; tempL++)
	 {
	 	_g3d_bitmap_poly[tempL]->u <<= 1;
	 	_g3d_bitmap_poly[tempL]->v <<= 1;
	 }

	tmap_info.tmap_type = GRC_POLY;
	h_map(bm, 4, _g3d_bitmap_poly, &tmap_info);
	return(_g3d_bitmap_poly);

// nasty C code!! its a goto!!
NoBlend:
	tmap_info.tmap_type = (_g3d_light_flag<<1) + GRC_BILIN;
	h_map(bm, 4, _g3d_bitmap_poly, &tmap_info);
		
 	return(_g3d_bitmap_poly);
 }
 
#if (defined(powerc) || defined(__powerc))	
// subtract two longs, put the result in result, and return true if overflow
// result = src-dest;
char SubLongWithOverflow(long *result, long src, long dest)
 {
 	long	 tempres;
 	
 	*result = tempres = src - dest;
 	if ((dest>=0 && src<0 && tempres>=0) ||  
 	    (dest<0 && src>=0 && tempres<0))
 	  return true;
 	else
 		return false;
 }
 
// add two longs, put the result in result, and return true if overflow
// result = src+dest;
char AddLongWithOverflow(long *result, long src, long dest)
 {
 	long	 tempres;
 	
 	*result = tempres = src + dest;
 	if ((dest>=0 && src>=0 && tempres<0) ||  
 	    (dest<0 && src<0 && tempres>=0))
 	  return true;
 	else
 		return false;
 }
#else
asm char SubLongWithOverflow(long *result, long src, long dest)
 {
 	move.l	8(a7),d0
 	sub.l		12(a7),d0
 	bvs.s		@overflow
 	
 	move.l	4(a7),a0
 	move.l	d0,(a0)
 	moveq		#0,d0
 	rts
 	
@overflow:
 	moveq		#-1,d0
	rts
 }

asm char AddLongWithOverflow(long *result, long src, long dest)
 {
 	move.l	8(a7),d0
 	add.l		12(a7),d0
 	bvs.s		@overflow
 	
 	move.l	4(a7),a0
 	move.l	d0,(a0)
 	moveq		#0,d0
 	rts
 	
@overflow:
 	moveq		#-1,d0
	rts
 }
#endif
