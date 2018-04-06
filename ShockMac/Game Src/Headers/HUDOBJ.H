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
#ifndef __HUDOBJ_H
#define __HUDOBJ_H

/*
 * $Source: n:/project/cit/src/inc/RCS/hudobj.h $
 * $Revision: 1.3 $
 * $Author: mahk $
 * $Date: 1994/06/29 00:47:25 $
 *
 * $Log: hudobj.h $
 * Revision 1.3  1994/06/29  00:47:25  mahk
 * Added hudobj_rect_capable.
 * 
 * Revision 1.2  1993/12/18  00:32:33  xemu
 * made flag definition in line with objbit.h
 * 
 * Revision 1.1  1993/10/13  21:47:24  mahk
 * Initial revision
 * 
 *
 */

// Includes

// ---------
// INTERNALS
// ---------

extern ushort hudobj_classes[];

#define HUDOBJ_INST_FLAG 0x01
#define NUM_HUDOBJS 16


extern struct _hudobj_data
{
   short id;
   short xl,yl,xh,yh;
}  hudobj_vec[NUM_HUDOBJS];

extern ubyte current_num_hudobjs;


// ------------
// RENDERER API
// ------------

// given an objid, determin whether this is an object the renderer
// should store in the u
#define IS_HUDOBJ(id) ((objs[id].info.inst_flags & HUDOBJ_INST_FLAG) \
                     || ((1 << objs[id].subclass) & hudobj_classes[objs[id].obclass]))

#define SET_HUDOBJ_RECT(oid,oxl,oyl,oxh,oyh)  if (current_num_hudobjs < NUM_HUDOBJS) \
                                          { \
                                            struct _hudobj_data *hd = &hudobj_vec[current_num_hudobjs++];\
                                            hd->id = (oid); hd->xl = (oxl); hd->xh = (oxh); hd->yl = (oyl); hd->yh = (oyh);\
                                          }
// ---------------
// GAME SYSTEM API 
// ---------------                                         



#define HUDOBJ_ALL_SUBCLASSES 0xFF
void hudobj_set_subclass(ubyte l_class, ubyte subclass, bool val);
// Sets the value of IS_HUDOBJ for all objects of a particular class & subclass. 
// if subclass is HUDOBJ_ALL_SUBCLASSES then all subclasses will be set accordingly.  

void hudobj_set_id(short id, bool val);
// Sets the value of IS_HUDOBJ for the specified object. 

#define hudobj_rect_capable(triple) (ObjProps[OPTRIP(triple)].render_type == FAUBJ_BITMAP)


// Globals

#endif // __HUDOBJ_H

