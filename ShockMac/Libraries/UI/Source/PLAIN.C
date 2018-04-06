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
#include "gadgets.h"
#include "plain.h"
#include "tngplain.h"
//#include <_ui.h>
#include "dbg.h"

Gadget *gad_plain_create(Gadget *parent, LGRect *dim, int z, char *name)
{
   Gadget *retgad;
   LGPoint size;

   gadget_create_setup(&retgad, parent, CLASS_PLAIN, dim, z, name);

   // Fill in class-specific data
   size.x = RectWidth(dim);  size.y = RectHeight(dim);
   tng_plain_init(retgad, retgad->tng_data, size);

   return (retgad);
}

Gadget *gad_plain_create_from_tng(void *ui_data, LGPoint loc, TNG **pptng, LGPoint size)
{
   LGRect newrect;
   Gadget *rgad;

   newrect.ul = loc;
   newrect.lr.x = newrect.ul.x + size.x;
   newrect.lr.y = newrect.ul.y + size.y;
   rgad = gad_plain_create((Gadget *)ui_data, &newrect, 0, GD_NAME(((Gadget *)ui_data)->rep));

   *pptng = rgad->tng_data;
   return(rgad);
}
