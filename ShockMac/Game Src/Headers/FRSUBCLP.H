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
 * $Source: n:/project/cit/src/inc/RCS/frsubclp.h $
 * $Revision: 1.1 $
 * $Author: dc $
 * $Date: 1994/01/02 17:16:45 $
 *
 * Citadel renderer
 *   definition and accessors for subclip data
 */

// each vec is hi 14 bits of intersection (low word of a fix & ~3) and low 2 bit face code
// so to test if exists & the longword with 0x00030003 and if = 0x00030003 then it means nil

// subclip cache size
#define FR_SC_FULL  0xff
#define NUM_SUBCLIP (FR_SC_FULL+1)
#define FR_SC_NONE  0
#define FR_SC_BASE  1

// subclip region id's
#define SC_LEFT_BASE  0
#define SC_LEFT_END   1
#define SC_RIGHT_BASE 2
#define SC_RIGHT_END  3
#define SC_VEC_COUNT  4

#ifndef __FRCLIP_SRC
extern ushort  sc_reg[NUM_SUBCLIP][SC_VEC_COUNT];
extern ushort *cur_sc_ptr;
extern uint    cur_sc_reg;
#endif

#define SC_FACE_MASK 0x0003
#define SC_INTR_MASK (~SC_FACE_MASK)           //#define SC_INTR_MASK 0xFFFC
#define SC_NILL_VECT ((SC_FACE_MASK<<16)|SC_FACE_MASK)

#define FIRST_SC_REG        (1)
#define SUBCLIP_OUT_OF_CONE (0xFF)
#define SUBCLIP_FULL_TILE   (0x00)

// if the compiler doesnt shift here (or even better, scalar) i will kill it
#define sc_get_fullv(sc,vn)     (sc_reg[sc][vn])
#define sc_get_facev(sc,vn)     (sc_reg[sc][vn]&SC_INTR_MASK)
#define sc_get_intrv(sc,vn)     (sc_reg[sc][vn]&SC_FACE_MASK)
#define sc_set_partv(sc,vn,v,f) (sc_reg[sc][vn]=(v)|(f))
#define sc_set_fullv(sc,vn,v)   (sc_reg[sc][vn]=v)
#define sc_set_facev(sc,vn,f)   (sc_reg[sc][vn]=(sc_reg[sc][vn]&SC_INTR_MASK)|(f))
#define sc_set_intrv(sc,vn,i)   (sc_reg[sc][vn]=(sc_reg[sc][vn]&SC_FACE_MASK)|(i&SC_INTR_MASK))
// wacky read it as a long, vn is the base, ie. 0 or 2
#define sc_set_novec(sc,vn)     ((*((long *)&sc_reg[sc][vn]))=SC_NILL_VECT)
#define sc_chk_novec(sc,vn)     ((*((long *)&sc_reg[sc][vn]))==SC_NILL_VECT)

#define sc_reset()              { cur_sc_ptr=&sc_reg[FIRST_SC_REG][0]; cur_sc_reg=FIRST_SC_REG; }
#define sc_nxtvec()             { cur_sc_ptr+=SC_VEC_COUNT; cur_sc_reg++; }
#define sc_region()             cur_sc_reg
