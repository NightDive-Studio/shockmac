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
// $Source: r:/prj/lib/src/3d/RCS/interp.asm $
// $Revision: 1.19 $
// $Author: jaemz $
// $Date: 1994/10/13 20:51:43 $
//
// 3d object interpreter
//
// $Log: interp.asm $
// Revision 1.19  1994/10/13  20:51:43  jaemz
// Fixed lighting bug
// 
// Revision 1.18  1994/09/20  13:34:36  jaemz
// Lighting
// 
// Revision 1.14  1994/08/18  03:45:30  jaemz
// Added stereo to objects for real, reevals bsp tree 
// 
// Revision 1.13  1994/07/15  14:13:34  jaemz
// Added _view_position with an underscore to make it c readable
// 
// Revision 1.12  1994/05/19  09:46:39  kevin
// g3_draw_tmap now uses watcom register parameter passing conventions.
// 
// Revision 1.11  1994/02/08  20:46:17  kaboom
// Updated usage of gour_flag.
// 
// Revision 1.10  1993/11/18  10:08:11  dc
// first set of debug setup for the interpreter
// 
// Revision 1.9  1993/10/25  16:24:46  kaboom
// Changed call to polygon routine to use new calling convention.
// 
// Revision 1.8  1993/10/02  09:27:49  kaboom
// Added vtext_tab.  Also updated texture map opcode to call the uv perspective
// mapper.
// 
// Revision 1.7  1993/09/15  04:01:23  dc
// tmap interface, well, except there isnt a tmapper
// 
// Revision 1.6  1993/08/10  22:54:13  dc
// add _3d.inc to includes
// 
// Revision 1.5  1993/08/04  00:47:10  dc
// support for new interpreter opcodes
// 
// Revision 1.4  1993/06/03  14:34:00  matt
// Removed int -> sfix conversion in defres_i & setshade
// 
// Revision 1.3  1993/06/02  16:57:01  matt
// Gouraud polys handled differently: gouraud base now added at poly draw
// time, not point definition time.
// 
// Revision 1.2  1993/05/27  18:11:46  matt
// Added getparms opcodes, and changed parameter passing scheme.
// 
// Revision 1.1  1993/05/04  17:39:50  matt
// Initial revision
// 
// 

#include "lg.h"
#include "3d.h"
#include "GlobalV.h"
#include <String.h>
//#include <_stdarg.h>
#include <stdarg.h>

// prototypes;
uchar *do_eof(uchar *);
uchar *do_jnorm(uchar *);
uchar *do_ldjnorm(uchar *);
uchar *do_ljnorm(uchar *);
uchar *do_lnres(uchar *);
uchar *do_multires(uchar *);
uchar *do_polyres(uchar *);
uchar *do_setcolor(uchar *);
uchar *do_sortnorm(uchar *);
uchar *do_debug(uchar *);
uchar *do_setshade(uchar *);
uchar *do_goursurf(uchar *);
uchar *do_x_rel(uchar *);
uchar *do_y_rel(uchar *);
uchar *do_z_rel(uchar *);
uchar *do_xy_rel(uchar *);
uchar *do_xz_rel(uchar *);
uchar *do_yz_rel(uchar *);
uchar *do_icall_p(uchar *);
uchar *do_icall_b(uchar *);
uchar *do_icall_h(uchar *);
uchar *do_sfcal(uchar *);
uchar *do_defres(uchar *);
uchar *do_defres_i(uchar *);
uchar *do_getparms(uchar *);
uchar *do_getparms_i(uchar *);
uchar *do_gour_p(uchar *);
uchar *do_gour_vc(uchar *);
uchar *do_getvcolor(uchar *);
uchar *do_getvscolor(uchar *);
uchar *do_rgbshades(uchar *);
uchar *do_draw_mode(uchar *);
uchar *do_getpcolor(uchar *);
uchar *do_getpscolor(uchar *);
uchar *do_scaleres(uchar *);
uchar *do_vpnt_p(uchar *);
uchar *do_vpnt_v(uchar *);
uchar *do_setuv(uchar *);
uchar *do_uvlist(uchar *);
uchar *do_tmap(uchar *);
uchar *do_dbg(uchar *);

extern int check_and_draw_common(long c,int n_verts,g3s_phandle *p);
extern int draw_poly_common(long c,int n_verts,g3s_phandle *p);
extern void g3_light_obj(g3s_phandle norm,g3s_phandle pos);

void interpreter_loop(uchar *object);

void FlipVector(short n, g3s_vector *vec);
void FlipLong(long *lng);
void FlipShort(short *sh);

// globals
extern char gour_flag;	       // gour flag for actual polygon drawer

#ifdef   stereo_on
        temp_vector      g3s_vector     <>
        tmp_address      dd             ?
#endif

#define OP_EOF 0
#define OP_JNORM 1

#define n_ops 40
void *opcode_table[n_ops] = {do_eof,do_jnorm,do_lnres,do_multires,do_polyres,do_setcolor,
														do_sortnorm,do_debug,do_setshade,do_goursurf,do_x_rel,do_y_rel,
														do_z_rel,do_xy_rel,do_xz_rel,do_yz_rel,do_icall_p,do_icall_b,
														do_icall_h,0,do_sfcal,do_defres,do_defres_i,do_getparms,do_getparms_i,
														do_gour_p,do_gour_vc,do_getvcolor,do_getvscolor,do_rgbshades,
		        								do_draw_mode,do_getpcolor,do_getpscolor,do_scaleres,do_vpnt_p,
		        								do_vpnt_v,do_setuv,do_uvlist,do_tmap,do_dbg};
        								

#define N_RES_POINTS  1000
#define PARM_DATA_SIZE  4*100

#define N_VCOLOR_ENTRIES  32
#define N_VPOINT_ENTRIES  32
#define N_VTEXT_ENTRIES   64

// This determines when we no longer reevaluate
// the bsp tree.  It corresponds to the tan of
// 7.12 degrees, which empirically seems fine
// we might have to set it lower some day if
// you see polygons drop out in stereo 
#define STEREO_DIST_LIM = 0x2000

g3s_point *resbuf[N_RES_POINTS];
g3s_point *poly_buf[100];

uchar	_vcolor_tab[N_VCOLOR_ENTRIES] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
g3s_point *_vpoint_tab[N_VPOINT_ENTRIES] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
grs_bitmap *_vtext_tab[N_VTEXT_ENTRIES] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// ptr to stack parms
char *parm_ptr;

// space for parms to objects
char	parm_data[PARM_DATA_SIZE];

char	_itrp_gour_flg = 0;
char	_itrp_wire_flg = 0;
char	_itrp_check_flg = 0;


// space for temp copy of object
char  obj_space[8000];

// MLA not used, - uchar 	*struct_ptr;

/*
// process the next opcode
next	macro	opsize
	ifnb	<opsize>
	 add	ebp,opsize	// point at next opcode
	endif
	jmp	interpreter_loop
	endm

call_next	macro
	call	interpreter_loop
	endm
*/

// c callable context setting routines
// set virtual color eax to ebx
// inlined in 3d.h for now....
// g3_setvcolor:
//       add     eax, _vcolor_tab
//       mov     byte ptr [eax], bl

// takes ptr to object in eax. trashes all but ebp
// this is bullshit, man, takes ptr to object on the freakin' stack!
void g3_interpret_object(ubyte *object_ptr,...)
 {
 	int		i,scale;
 	short	size;
 	
 	size = * (short *) (object_ptr-4);
 	size -= 10;	// skip the first 10 bytes
 	
 	BlockMove(object_ptr-2,obj_space,size);
 	
  // lighting stuff, params are on the stack
  // so don't sweat it
  // set fill type so 2d can light the thang
	if ((_g3d_light_type & (LT_SPEC | LT_DIFF))!=0)
	 {
	 	gr_set_fill_type(FILL_CLUT);
	 	if (_g3d_light_type == LT_DIFF)
	 	 	opcode_table[OP_JNORM] = &do_ldjnorm;
	 	else
	 	 	opcode_table[OP_JNORM] = &do_ljnorm;
	 }
	 
#ifdef stereo_on
        test    _g3d_stereo,1
        jz      g3_interpret_object_raw
        //       call normally if eyesep/distance is small enough (angular change is small)
        //       transform 0,0,0 to get the z distance
        mov     eax,_view_position.x
        fixmul  view_matrix.m3
        mov     ecx,eax

        mov     eax,_view_position.y
        fixmul  view_matrix.m6
        add     ecx,eax

        mov     eax,_view_position.z
        fixmul  view_matrix.m9
        add     ecx,eax
        neg     ecx

        mov     eax,_g3d_eyesep_raw
        fixmul  _matrix_scale.z
        fixdiv  ecx

        cmp     eax,STEREO_DIST_LIM
        jl      g3_interpret_object_raw

        mov     _g3d_stereo,0    // kill stereo
        pop     eax     // grab real return address
        mov     tmp_address,eax         // save it for later

        push    ret1    // fake out the poor thing so it jumps back here
        jmp  g3_interpret_object_raw
        
        // shift view_position
        ret1:
        // save the current position
        lea     edi,temp_vector
        lea     esi,_view_position
        movsd
        movsd
        movsd

        mov     eax,_g3d_eyesep_raw
        mov     ebx,_matrix_scale.x
        fixdiv  ebx     // make ebx the scaled down eyesep
        mov     ebx,eax

        // get x slewed over (top row of current vector and scale)
        mov     eax,view_matrix.m1
        fixmul  ebx
        add     _view_position.x,eax

        mov     eax,view_matrix.m4
        fixmul  ebx
        add     _view_position.y,eax

        mov     eax,view_matrix.m7
        fixmul  ebx
        add     _view_position.z,eax

        set_rt_canv             // install rt canvas
        // this time when you call it, its still all set
        call    g3_interpret_object_raw
        mov     _g3d_stereo,1    // restore stereo
        set_lt_canv             // restore left canvas

        // restore view position
        lea     esi,temp_vector
        lea     edi,_view_position
        movsd
        movsd
        movsd

        // weeee, pretend we were here all along, but I suppose we
        // could just jmp there
        push    tmp_address

        ret
g3_interpret_object_raw:
#endif

	va_start(parm_ptr, object_ptr);	// get addr of stack parms

// MLA- not used ever?
/*
	mov	eax,16[esp]	// get angle
	mov	struct_ptr,eax*/

// mark res points as free
	LG_memset(resbuf, 0, N_RES_POINTS*4);

// scale view vector for scale
	FlipShort((short *) (object_ptr-2));
	scale = * (short *) (object_ptr-2);		
	if (scale)	
	 {
	 	if (scale>0)
	 	 {
	 	 	_view_position.gX >>= scale;
	 	 	_view_position.gY >>= scale;
	 	 	_view_position.gZ >>= scale;
	 	 }
	 	else
	 	 {
	 	 	int		temp;
	 	 	
	 	 	scale = -scale;
	 	 	
	 	 	temp = (((ulong) _view_position.gX)>>16);	// get high 16 bits
	 	 	if (((temp<<scale) && 0xffff0000)!=0) goto Exit; // overflow
	 	 	temp = (((ulong) _view_position.gY)>>16);	// get high 16 bits
	 	 	if (((temp<<scale) && 0xffff0000)!=0) goto Exit; // overflow
	 	 	temp = (((ulong) _view_position.gZ)>>16);	// get high 16 bits
	 	 	if (((temp<<scale) && 0xffff0000)!=0) goto Exit; // overflow
	 	 	
	 	 	_view_position.gX <<= scale;
	 	 	_view_position.gY <<= scale;
	 	 	_view_position.gZ <<= scale;
	 	 }
	 }

	interpreter_loop(object_ptr);
	
	// free res points
	for (i=N_RES_POINTS-1; i--; i>=0)
	 	if (resbuf[i]) 
	 		freepnt(resbuf[i]);
	
  // set lighting back to how it was
	if ((_g3d_light_type & (LT_SPEC | LT_DIFF))!=0)
	 {
	 	gr_set_fill_type(FILL_NORM);
		opcode_table[OP_JNORM] = &do_jnorm;
   }
   
Exit:
 	BlockMove(obj_space,object_ptr-2,size);
 }

// interpret the object
void interpreter_loop(uchar *object)
 {
 	do
 	 {
 	 	FlipShort((short *) object);
 	 	object = ((uchar * (*)(uchar *))opcode_table[* (short *) object])(object);
 	 }
 	while (object);
 }
 

// opcodes.  [ebp] points at op on entry
uchar *do_debug(uchar *opcode)
 {
 	return 0;
 }
 

uchar *do_eof(uchar *opcode)	// and return extra level
 {
 	return 0;
 }

// jnorm lbl,px,py,pz,nx,ny,nz
// v=viewer coords-p
// if (n*v)<0 then branch to lbl
uchar *do_jnorm(uchar *opcode)
 {
 	FlipShort((short *) (opcode+2));
 	FlipVector(2,(g3s_vector *) (opcode+4));
 	
 	if (g3_check_normal_facing((g3s_vector *) (opcode+16), (g3s_vector *) (opcode+4)))
 		return opcode+28;	// surface is visible. continue
 	else
 		return opcode+(* (short *) (opcode+2));	// surface not visible
 }
  
// lnres pnt0,pnt1
uchar *do_lnres(uchar *opcode)
 {
 	FlipShort((short *) (opcode+2));
 	FlipShort((short *) (opcode+4));
 	
 	g3_draw_line(resbuf[* (unsigned short *) (opcode+2)], resbuf[* (unsigned short *) (opcode+4)]);
 	return opcode+6;
 }
 
uchar *do_multires(uchar *opcode)
 {
	short	 count;
	
 	FlipShort((short *) (opcode+2));
 	FlipShort((short *) (opcode+4));
	
	count = * (short *) (opcode+2);
 	FlipVector(count,(g3s_vector *) (opcode+6));

 	g3_transform_list(count, (g3s_phandle *) (resbuf+(* (short *) (opcode+4))), (g3s_vector *) (opcode+6));
 	return opcode+6+(count*12);	// fixup: ebp = esi + ecx*12
 }
 
// this should do some cute matrix transform trick, not this ugly hack

// that kid from the wrong side came over my house again, decapitated all my dolls
//  and if you bore me, you lose your soul to me       - "Gepetto", Belly, _Star_
uchar *do_scaleres(uchar *opcode)
 {
 	// MLA - this routine appears to be buggy and can't possibly work, so I'm not doing it yet.
 	DebugStr("\pCall Mark!");
 	
/* 	int					count,scale;
	long				temp_pnt[3];
 	g3s_phandle	temp_hand;
 	
 	count = * (unsigned short *) (opcode+2);
 	scale = * (unsigned short *) (opcode+4);
 	temp_hand = (g3s_phandle) (parm_data+(* (unsigned short *) (opcode+6)));
 	
 	opcode += 8;
 	do
 	 {
 	 }
 	while (--count>0);
 	
 	return opcode;
 	 */
 /*	
        movzx	ecx,w 2[ebp]	// get count
	movzx	eax,w 4[ebp]	// get scale factor
	movzx	ebx,w 6[ebp]	// get dest start num
        mov     eax,d parm_data [eax]
        add     ebp,8
//	lea	esi,[ebp]	// get vector array start
do_sr_loop:
        push    eax
        push    ecx
        push    ebx
        mov     ecx, eax
        mov     esi, OFFSET temp_pnt
// do better scaling here.....
        imul    d [ebp]
        shrd    eax,edx,16
        mov     [esi],eax
        mov     eax, ecx
        imul    d 4[ebp]
        shrd    eax,edx,16
        mov     4[esi],eax
        mov     eax, ecx
        imul    d 8[ebp]
        shrd    eax,edx,16
        mov     8[esi],eax
        call    g3_transform_point
        pop     ebx
        mov     resbuf[ebx*4],edi
        inc     ebx
        add     ebp,12
        lea     esi,[ebp]
        pop     ecx    
        pop     eax
        dec     ecx
        jnz     do_sr_loop
	next
 */
 	return 0;
 }
 
// these put the address of an old point in the interpreter respnt array
// note they will get freed when the interpreter punts
uchar *do_vpnt_p(uchar *opcode)
 {
 	FlipShort((short *) (opcode+2));
 	FlipShort((short *) (opcode+4));
 	
 	resbuf[* (short *) (opcode+4)] = (g3s_point *) (* (long *) (parm_data + (* (unsigned short *) (opcode+2))));
 	return opcode+6;
 }
 
uchar *do_vpnt_v(uchar *opcode)
 {
 	FlipShort((short *) (opcode+2));
 	FlipShort((short *) (opcode+4));

 	resbuf[* (short *) (opcode+4)] = _vpoint_tab[(* (unsigned short *) (opcode+2))>>2];
 	return opcode+6;
 }
 

uchar *do_defres(uchar *opcode)
 {
 	FlipShort((short *) (opcode+2));
 	FlipVector(1,(g3s_vector *) (opcode+4));

 	resbuf[* (unsigned short *) (opcode+2)] = g3_transform_point((g3s_vector *) (opcode+4));
 	return opcode+16;
 }

uchar *do_defres_i(uchar *opcode)
 {
 	g3s_phandle temphand;
 	
 	FlipShort((short *) (opcode+2));
 	FlipShort((short *) (opcode+16));
 	FlipVector(1,(g3s_vector *) (opcode+4));
 	
 	temphand = g3_transform_point((g3s_vector *) (opcode+4));
 	resbuf[* (unsigned short *) (opcode+2)] = temphand;
 	
 	temphand->i = * (short *) (opcode+16);
 	temphand->p3_flags |= PF_I;
 	
 	return opcode+18;
 }

// polyres cnt,pnt0,pnt1,...
uchar *do_polyres(uchar *opcode)
 {
 	int		count,count2;
 	
 	FlipShort((short *) (opcode+2));

 	count2 = count = * (unsigned short *) (opcode+2);
 	opcode += 4;
 	while (--count>=0)
 	 {
 		FlipShort((short *) (opcode+(count<<1)));

 	 	poly_buf[count] = resbuf[* (unsigned short *) (opcode+(count<<1))];
 	 }
 	
 	opcode += count2<<1; 
 	 
 	gour_flag = _itrp_gour_flg;
 	if ((_itrp_check_flg & 1)==0)
 	 	draw_poly_common(gr_get_fcolor(), count2, poly_buf);
 	else
 	 	check_and_draw_common(gr_get_fcolor(), count2, poly_buf);
 	 
 	return opcode; 
 }

uchar *do_sortnorm(uchar *opcode)
 { 	
 	FlipVector(2,(g3s_vector *) (opcode+2));
 	FlipShort((short *) (opcode+26));
 	FlipShort((short *) (opcode+28));
 	
 	if (g3_check_normal_facing((g3s_vector *) (opcode+14),(g3s_vector *) (opcode+2)))
 	 { 	 	
		interpreter_loop(opcode + (* (short *) (opcode+26)));
		interpreter_loop(opcode + (* (short *) (opcode+28)));
 	 } 
 	else
 	 {
		interpreter_loop(opcode + (* (short *) (opcode+28)));
		interpreter_loop(opcode + (* (short *) (opcode+26)));
 	 }

	return opcode+30;
 }

uchar *do_goursurf(uchar *opcode)
 {
 	FlipShort((short *) (opcode+2));

 	gouraud_base = (* (short *) (opcode+2)) << 8;
 	_itrp_gour_flg = 2;
 	return opcode+4;
 }

uchar *do_gour_p(uchar *opcode)
 {
 	FlipShort((short *) (opcode+2));

 	gouraud_base = parm_data[(* (short *) (opcode+2))] << 8;
 	_itrp_gour_flg = 2;
 	return opcode+4;
 }

uchar *do_gour_vc(uchar *opcode)
 {
 	int	 temp;

 	FlipShort((short *) (opcode+2));
 	
 	gouraud_base = ((long) _vcolor_tab[* (unsigned short *) (opcode+2)]) << 8;
 	_itrp_gour_flg = 2;
 	return opcode+4;
 }


uchar *do_draw_mode(uchar *opcode)
 {
 	short 	flags;

 	FlipShort((short *) (opcode+2));
 	
 	flags = * (short *) (opcode+2);
 	_itrp_wire_flg = flags>>8;
 	flags &= 0x00ff;
 	flags <<= 1;
 	_itrp_check_flg = flags>>8;
 	flags &= 0x00ff;
 	flags <<= 2;
 	_itrp_gour_flg = flags-1;
 	return opcode+4;
 }

uchar *do_setshade(uchar *opcode)
 {
 	int					i;
 	uchar				*new_opcode;
 	g3s_phandle temphand;
 
  FlipShort((short *) (opcode+2));
	
 	i = * (unsigned short *) (opcode+2);	// get number of shades
 	new_opcode = opcode+4+(i<<2);
 	
 	while (--i>=0)
 	 {
 	 	FlipShort((short *) (opcode+4+(i<<2)));
 	 	FlipShort((short *) (opcode+6+(i<<2)));

 	 	temphand = resbuf[* (unsigned short *) (opcode+4+(i<<2))];	// get point handle
 	 	temphand->i = * (short *) (opcode+6+(i<<2));
 	 	temphand->p3_flags |= PF_I;
 	 }
 	 
 	return new_opcode;
 }

uchar *do_rgbshades(uchar *opcode)
 {
 	uchar 			*new_opcode;
 	int					i;
 	g3s_phandle temphand;

  FlipShort((short *) (opcode+2));
	
 	i = * (unsigned short *) (opcode+2);	// get number of shades
 	new_opcode = opcode+4;
 	while (--i>=0)
 	 {
		FlipShort((short *) new_opcode);
		FlipLong((long *) (new_opcode+2));

 	 	temphand = resbuf[* (unsigned short *) new_opcode];	// get point handle
 	 	temphand->rgb = * (long *) (new_opcode+2);
 	 	temphand->p3_flags |= PF_RGB;
 	 	new_opcode += 10; 
 	 }
 	return new_opcode;
 }

uchar *do_setuv(uchar *opcode)
 {
 	g3s_phandle temphand;

  FlipShort((short *) (opcode+2));
	FlipLong((long *) (opcode+4));
	FlipLong((long *) (opcode+8));
 	
	temphand = resbuf[* (unsigned short *) (opcode+2)];	// get point handle
 	temphand->uv.u = (* (unsigned long *) (opcode+4)) >> 8;
 	temphand->uv.v = (* (unsigned long *) (opcode+8)) >> 8;
 	temphand->p3_flags |= PF_U | PF_V;
 	
 	return opcode+12;
 }

uchar *do_uvlist(uchar *opcode)
 {
 	int					i;
 	g3s_phandle temphand;

  FlipShort((short *) (opcode+2));

 	i = * (unsigned short *) (opcode+2);	// get number of shades
 	opcode += 4;
 	while (--i>=0)
 	 {
  	FlipShort((short *) opcode);
		FlipLong((long *) (opcode+2));
		FlipLong((long *) (opcode+6));

 	 	temphand = resbuf[* (unsigned short *) opcode];	// get point handle
		temphand->uv.u = (* (unsigned long *) (opcode+2)) >> 8;
		temphand->uv.v = (* (unsigned long *) (opcode+6)) >> 8;
 	 	temphand->p3_flags |= PF_U | PF_V;
 	 	opcode += 10; 
 	 }
 	 
 	return opcode;
 }


// should we be hacking _itrp_gour_flg?
uchar *do_setcolor(uchar *opcode)
 {
  FlipShort((short *) (opcode+2));

 	gr_set_fcolor(* (unsigned short *) (opcode+2));
 	_itrp_gour_flg = 0;
 	return opcode+4;
 }

uchar *do_getvcolor(uchar *opcode)
 {
  FlipShort((short *) (opcode+2));

 	gr_set_fcolor(_vcolor_tab[* (unsigned short *) (opcode+2)]);
 	_itrp_gour_flg = 0;
 	return opcode+4;
 }

uchar *do_getpcolor(uchar *opcode)
 {
  FlipShort((short *) (opcode+2));

 	gr_set_fcolor(* (unsigned short *) (parm_data + (* (unsigned short *) (opcode+2))));
 	_itrp_gour_flg = 0;
 	return opcode+4;
 }

uchar *do_getvscolor(uchar *opcode)
 {
 	short		temp;
 	
  FlipShort((short *) (opcode+2));
  FlipShort((short *) (opcode+4));

 	temp = (byte) _vcolor_tab[* (unsigned short *) (opcode+2)];
 	temp |= (* (short *) (opcode+4)) << 8;
 	gr_set_fcolor(gr_get_light_tab()[temp]);
 	return opcode+6;
 }

uchar *do_getpscolor(uchar *opcode)
 {
 	short		temp;
 	
  FlipShort((short *) (opcode+2));
  FlipShort((short *) (opcode+4));

 	temp = (unsigned short) parm_data[* (unsigned short *) (opcode+2)];
 	temp &= 0x00ff;
 	temp |= (* (short *) (opcode+4)) << 8;
 	gr_set_fcolor(gr_get_light_tab()[temp]);
 	return opcode+6;
 }


uchar *do_x_rel(uchar *opcode)
 {
  FlipShort((short *) (opcode+2));
  FlipShort((short *) (opcode+4));
  FlipLong((long *) (opcode+6));

 	resbuf[* (short *) (opcode+2)] = g3_copy_add_delta_x(resbuf[* (short *) (opcode+4)], * (fix *) (opcode+6));
 	return opcode+10;
 }


uchar *do_y_rel(uchar *opcode)
 {
  FlipShort((short *) (opcode+2));
  FlipShort((short *) (opcode+4));
  FlipLong((long *) (opcode+6));

 	resbuf[* (short *) (opcode+2)] = g3_copy_add_delta_y(resbuf[* (short *) (opcode+4)], * (fix *) (opcode+6));
 	return opcode+10;
 }


uchar *do_z_rel(uchar *opcode)
 {
  FlipShort((short *) (opcode+2));
  FlipShort((short *) (opcode+4));
  FlipLong((long *) (opcode+6));

 	resbuf[* (short *) (opcode+2)] = g3_copy_add_delta_z(resbuf[* (short *) (opcode+4)], * (fix *) (opcode+6));
 	return opcode+10;
 }


uchar *do_xy_rel(uchar *opcode)
 {
  FlipShort((short *) (opcode+2));
  FlipShort((short *) (opcode+4));
  FlipLong((long *) (opcode+6));
  FlipLong((long *) (opcode+10));

 	resbuf[* (short *) (opcode+2)] = g3_copy_add_delta_xy(resbuf[* (short *) (opcode+4)], 
 																												* (fix *) (opcode+6), * (fix *) (opcode+10));
 	return opcode+14;
 }


uchar *do_xz_rel(uchar *opcode)
 {
  FlipShort((short *) (opcode+2));
  FlipShort((short *) (opcode+4));
  FlipLong((long *) (opcode+6));
  FlipLong((long *) (opcode+10));

 	resbuf[* (short *) (opcode+2)] = g3_copy_add_delta_xz(resbuf[* (short *) (opcode+4)], 
 																												* (fix *) (opcode+6), * (fix *) (opcode+10));
 	return opcode+14;
 }

uchar *do_yz_rel(uchar *opcode)
 {
  FlipShort((short *) (opcode+2));
  FlipShort((short *) (opcode+4));
  FlipLong((long *) (opcode+6));
  FlipLong((long *) (opcode+10));

 	resbuf[* (short *) (opcode+2)] = g3_copy_add_delta_yz(resbuf[* (short *) (opcode+4)], 
 																												* (fix *) (opcode+6), * (fix *) (opcode+10));
 	return opcode+14;
 }


uchar *do_icall_p(uchar *opcode)
 {
 	FlipVector(1,(g3s_vector *) (opcode+6));
  FlipShort((short *) (opcode+18));
  FlipLong((long *) (opcode+2));
 	
 	g3_start_object_angles_x((g3s_vector *) (opcode+6), * (fixang *) (parm_data+(* (unsigned short *) (opcode+18))));
	interpreter_loop((uchar *) (* (long *) (opcode+2)));
	g3_end_object();

	return opcode+20; 
 }


uchar *do_icall_h(uchar *opcode)
 {
 	FlipVector(1,(g3s_vector *) (opcode+6));
  FlipShort((short *) (opcode+18));
  FlipLong((long *) (opcode+2));

 	g3_start_object_angles_y((g3s_vector *) (opcode+6), * (fixang *) (parm_data+(* (unsigned short *) (opcode+18))));
	interpreter_loop((uchar *) (* (long *) (opcode+2)));
	g3_end_object();

	return opcode+20; 
 }

uchar *do_icall_b(uchar *opcode)
 {
 	FlipVector(1,(g3s_vector *) (opcode+6));
  FlipShort((short *) (opcode+18));
  FlipLong((long *) (opcode+2));

 	g3_start_object_angles_z((g3s_vector *) (opcode+6), * (fixang *) (parm_data+(* (unsigned short *) (opcode+18))));
	interpreter_loop((uchar *) (* (long *) (opcode+2)));
	g3_end_object();

	return opcode+20; 
 }

uchar *do_sfcal(uchar *opcode)
 {
  FlipShort((short *) (opcode+2));

	interpreter_loop(opcode + (* (unsigned short *) (opcode+2)));
 	return opcode+4;
 }

// copy parms of stack. takes offset,count
uchar *do_getparms(uchar *opcode)
 {
 	long	*src,*dest;
 	int		count;
 	
  FlipShort((short *) (opcode+2));
  FlipShort((short *) (opcode+4));
  FlipShort((short *) (opcode+6));

 	dest = (long *) (parm_data + (* (unsigned short *) (opcode+2)));
 	src = (long *) (parm_ptr + (* (unsigned short *) (opcode+4)));
	count = * (unsigned short *) (opcode+6);
	while (count-->0)
	 *(dest++) = *(src)++;

	return opcode+8;
 }

// copy parm block. ptr is on stack. takes dest_ofs,src_ptr_ofs,size
uchar *do_getparms_i(uchar *opcode)
 {
 	long	*src,*dest;
 	int		count;
 	
  FlipShort((short *) (opcode+2));
  FlipShort((short *) (opcode+4));
  FlipShort((short *) (opcode+6));

 	dest = * (long **) (parm_data + (* (unsigned short *) (opcode+2)));
 	src = (long *) (parm_ptr + (* (unsigned short *) (opcode+4)));
	count = * (unsigned short *) (opcode+6);
	while (count-->0)
	 *(dest++) = *(src)++;

	return opcode+8;
 }

uchar *do_dbg(uchar *opcode)
 {
#ifdef  _itrp_dbg
        mov     ax, w 4[ebp]    // code
        and     ax, _itrp_dbg_mask // mask in current debug mode
        jz      dbg_end         // none currently on
        cmp     ax, DBG_POLY_ID // itrp_pcode
        jnz     dbg_nxt1
        mov     ax, w 6[ebp]    // pgon_id
        cmp     ax,_pgon_id_low
        jl      pgon_skip
        cmp     ax,_pgon_id_high
        jle     dbg_end
pgon_skip:
	movsx	eax,w 2[ebp]    // skip whatever
	next	eax
dbg_nxt1:
        cmp     ax, DBG_POLY_MAX
        jnz     dbg_end
        mov     ax, w 6[ebp]
        mov     _pgon_max, ax
//        jmp     dbg_end
#endif

 	return opcode+8;
 }


extern void (*g3_tmap_func)();
extern int temp_poly (long c, int n, grs_vertex **vpl);

uchar *do_tmap(uchar *opcode)
 {
 	int		count,count2;
 	short	temp;
 	
 	
  FlipShort((short *) (opcode+2));
  FlipShort((short *) (opcode+4));

 	count2 = count = * (unsigned short *) (opcode+4);
 	count--;
 	do
 	 {
 		FlipShort((short *) (opcode+6+(count<<1)));
 		temp = * (short *) (opcode+6+(count<<1));
 		
 	 	poly_buf[count] = resbuf[temp];
 	 }
 	while (--count>=0);
 	 
 	((int (*)(int,g3s_phandle *,grs_bitmap *)) *g3_tmap_func)(count2,poly_buf,_vtext_tab[* (unsigned short *) (opcode+2)]);
 	
 	return opcode + 6+ (count2*2);
 }


// routines to shade objects
// mostly replacements for jnorm
// ljnorm lbl,px,py,pz,nx,ny,nz
// v=viewer coords-p
// if (n*v)<0 then branch to lbl
// does lit version of jnorm, for flat lighting
uchar *do_ljnorm(uchar *opcode)
 {
  FlipShort((short *) (opcode+2));
 	FlipVector(2,(g3s_vector *) (opcode+4));
 	
 	if (g3_check_normal_facing((g3s_vector *) (opcode+16),(g3s_vector *) (opcode+4)))
 	 {
 	 	g3_light_obj((g3s_phandle) (opcode+4), (g3s_phandle) (opcode+16));
 	 	return opcode+28;
 	 }
 	else
 		return opcode+(* (short *) (opcode+2));	// surface not visible
 }

// light diff not near norm
uchar *do_ldjnorm(uchar *opcode)
 {
 	fix temp;
 	
  FlipShort((short *) (opcode+2));
 	FlipVector(2,(g3s_vector *) (opcode+4));

 	if (g3_check_normal_facing((g3s_vector *) (opcode+16),(g3s_vector *) (opcode+4)))
 	 {
 	 	temp = g3_vec_dotprod(&_g3d_light_vec, (g3s_vector *) (opcode+4));
 	 	temp <<=1;
 	 	if (temp<0) temp = 0;
 	 	temp += _g3d_amb_light;
 	 	temp >>= 4;
 	 	temp &= 0x0ffffff00;
 	 	temp += _g3d_light_tab;
 	 	gr_set_fill_parm(temp);
 	 	
 	 	return opcode+28;
 	 }
 	else
 		return opcode+(* (short *) (opcode+2));	// surface not visible
 }


// MLA - this routine doesn't appear to ever be called anywhere
/*
// check if a surface is facing the viewer and save the view vector and
// its dot product with everything.  Normalizes view vec and stuff
// takes esi=point on surface, edi=surface normal (can be unnormalized)
// trashes eax,ebx,ecx,edx. returns al=true & sign set, if facing
g3_light_check_normal_facing:
        call g3_eval_view
        mov     eax,_g3d_view_vec.x
        imul    [edi].x
        mov     ebx,eax
        mov     ecx,edx

        mov     eax,_g3d_view_vec.y
        imul    [edi].y
        add     ebx,eax
        adc     ecx,edx

        mov     eax,_g3d_view_vec.z
        imul    [edi].z
        add     eax,ebx
        adc     edx,ecx

        // now save this to ldotv

        sets    al      // al=true if facing

        ret
*/

void FlipShort(short *sh)
 {
 	uchar temp;
 	uchar *src = (uchar *) sh;
 	
 	temp = src[0];
 	src[0] = src[1];
 	src[1] = temp;
 }
 
void FlipLong(long *lng)
 {
 	short *src = (short *) lng;
 	short	temp;
 	
 	temp = src[0];
 	src[0] = src[1];
 	src[1] = temp;
 	
 	FlipShort(src);
 	FlipShort(src+1);
 }
 
void FlipVector(short n, g3s_vector *vec)
 {
 	int		i,j;
 	
 	for (i=0; i<n; i++, vec++)
 	 	for (j=0; j<3; j++)
 	 		FlipLong((long *) &vec->xyz[j]);
 }

