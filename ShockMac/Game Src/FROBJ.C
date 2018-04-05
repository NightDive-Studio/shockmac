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
 * FrObj.c
 *
 * $Source: r:/prj/cit/src/RCS/frobj.c $
 * $Revision: 1.7 $
 * $Author: dc $
 * $Date: 1994/08/30 05:50:47 $
 *
 * Citadel Renderer
 *  object draw/setup code
 */
#define __FROBJ_SRC

#include <stdlib.h>

//#include <3d.h>
#include "map.h"
#include "objects.h"
#include "refstuf.h"

#include "frintern.h"
#include "frparams.h"
#include "frsubclp.h"
#include "frflags.h"

#include "tilename.h"

extern void render_sorted_objs(void);
extern void sort_show_obj(ObjID cobjid);
extern void facelet_obj(ObjID cobjid);

bool pick_best_ref(ObjRefID cRef);


bool pick_best_ref(ObjRefID cRef)
{
   int bdist, cdist, ldist;
   ObjRefID curLRef, BRef;

   // really, need to sort so SCOOC should do it once we are done with clip + pipe
   if ((me_subclip(_fdt_mptr)!=SUBCLIP_OUT_OF_CONE)&&(me_tiletype(_fdt_mptr)!=TILE_SOLID))
   {
	   bdist=cdist=_fdt_dist;
		BRef=cRef;    // find correct version of object, initially, first guess is best
      _fr_sdbg(OBJ_TALK,mprintf("First version ok %d at %d %d\n",bdist,_fdt_x,_fdt_y));
   }
   else bdist=cdist=0xffff;
   curLRef=objRefs[cRef].nextref;                  // init the examine others loop
   while (curLRef!=cRef)
   {  // this should know to check the map for not actually seen
      int x,y;
      MapElem *mp;

      x=objRefs[curLRef].state.bin.sq.x;
      y=objRefs[curLRef].state.bin.sq.y;

      mp=MAP_GET_XY(x,y);

      _fr_sdbg(OBJ_TALK,mprintf("check %d %d..sc %x\n",x,y,me_subclip(mp)));

      if ((me_subclip(mp)!=SUBCLIP_OUT_OF_CONE)&&(me_tiletype(mp)!=TILE_SOLID))
      {
	      // this is super gross, but what to do
	      ldist=abs(x-_fr_x_cen)+abs(y-_fr_y_cen);
	      if (ldist<bdist)
	       { bdist=ldist; BRef=curLRef; }
         _fr_sdbg(OBJ_TALK,mprintf("tried it got %d bdist %d\n",ldist,bdist));
      }
	   curLRef=objRefs[curLRef].nextref;                 /* we are us */
   }
   if (bdist!=0xffff)
   {
	   CitrefSetDealt(BRef);
	   return (BRef==cRef);
   }
   else
      return FALSE;
}

// this is a total mess
// should use seen bit and objRefdone and a bit in the objRefs
// but for now, we just have to get all objs in the square sorting
// we can deal with speeding this up later
void render_parse_obj(void)
{
#ifndef __RENDTEST__
   ObjRefID curORef;
   ObjID cobjid;

   curORef=_fdt_mptr->objRef;
   while (curORef!=OBJ_REF_NULL)
   {
      bool show_here;
      cobjid=objRefs[curORef].obj;
      if (!ObjCheckDealt(cobjid))
      {
         show_here=pick_best_ref(curORef);
         ObjSetDealt(cobjid);
      }
      else
         show_here=CitrefCheckDealt(curORef);

      if (show_here)
      {
         _fr_sdbg(OBJ_TALK,mprintf("Rendering %d at %d %d\n",curORef,_fdt_x,_fdt_y));
         sort_show_obj(cobjid);
      }
//      else
//         mprintf("not rend %d @ %d %d\n",curORef,_fdt_x,_fdt_y);
      curORef = objRefs[curORef].next;
   }
   render_sorted_objs();
#else
   ushort curORef;
   curORef=_fdt_mptr->objRef;
   _fr_sdbg(OBJ_TALK,mprintf("Rendering %d at %d %d\n",curORef,_fdt_x,_fdt_y));
   // perhaps draw a box or something
#endif
}

void facelet_parse_obj(void)
{
#ifndef __RENDTEST__
   ObjRefID curORef;
   ObjID cobjid;

   curORef=_fdt_mptr->objRef;
   while (curORef!=OBJ_REF_NULL)
   {
      cobjid=objRefs[curORef].obj;
      if (!ObjCheckDealt(cobjid))
      {
         facelet_obj(cobjid);
         ObjSetDealt(cobjid);
      }
      curORef = objRefs[curORef].next;
   }
#else
   ushort curORef;
   curORef=_fdt_mptr->objRef;
   _fr_sdbg(OBJ_TALK,mprintf("Rendering %d at %d %d\n",curORef,_fdt_x,_fdt_y));
   // perhaps draw a box or something
#endif
}
