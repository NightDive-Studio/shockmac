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
#include <string.h>

#include "region.h"
#include "gadgets.h"
#include "butarray.h"
#include "tngbarry.h"
#include "event.h"
#include "mouse.h"
#include "kbcook.h"
#include "2dres.h"

Gadget *gad_buttonarray_create(Gadget *parent, LGPoint coord, int z, int msize_x, int msize_y, int window_x, int window_y,
   int bsize_x, int bsize_y, int num_sel, ushort options, TNGStyle *sty, char *name)
{
   Gadget *retgad;
   LGRect dim;
   TNG *temp_tng;
   LGPoint basize;
   LGPoint msize, bsize, wsize;

   temp_tng = (TNG *)NewPtr(sizeof(TNG));

   msize.x = msize_x;   msize.y = msize_y;
   wsize.x = window_x;  wsize.y = window_y;
   bsize.x = bsize_x;   bsize.y = bsize_y;
   tng_buttonarray_init(NULL, temp_tng, sty, options, msize, wsize, bsize, num_sel);

   dim.ul = coord;
   tng_buttonarray_size(temp_tng, &basize);
   dim.lr.x = dim.ul.x + basize.x;
   dim.lr.y = dim.ul.y + basize.y;

   gadget_create_setup(&retgad, parent, CLASS_BUTTONARRAY, &dim, z, name);
   DisposePtr((Ptr)retgad->tng_data);
   temp_tng->ui_data = retgad;
   retgad->tng_data = temp_tng;

   // Let the TNG get another crack at things....
   tng_buttonarray_init2(retgad->tng_data);

   return (retgad);
}

errtype gad_buttonarray_addbutton_at(Gadget *g, int type, void *disp_data, int coord_x, int coord_y)
{
   return(tng_buttonarray_addbutton_at(g->tng_data, type, disp_data, coord_x, coord_y));
}

errtype gad_buttonarray_addbutton(Gadget *g, int type, void *disp_data)
{
   return(tng_buttonarray_addbutton(g->tng_data, type, disp_data));
}

errtype gad_buttonarray_setoffset(Gadget *g, int offset_x, int offset_y)
{
   return(tng_buttonarray_setoffset(g->tng_data, offset_x, offset_y));
}
