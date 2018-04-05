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

#include "gadgets.h"
#include "pushbutt.h"
#include "tngpushb.h"
//#include <_ui.h>
#include "dbg.h"

Gadget *gad_pushbutton_create(Gadget *parent, LGRect *dim, int z, int type, void *disp_data, TNGStyle *sty, char *name)
{
   Gadget *retgad;
   LGPoint size;

   gadget_create_setup(&retgad, parent, CLASS_PUSHBUTTON, dim, z, name);

   // Fill in class-specific data
   size.x = RectWidth(dim);  size.y = RectHeight(dim);
   tng_pushbutton_init(retgad, retgad->tng_data, sty, type, disp_data, size);

   return (retgad);
}

Gadget *gad_pushbutton_create_from_tng(void *ui_data, LGPoint loc, TNG **pptng, TNGStyle *sty, int button_type,
   void *display_data, LGPoint size)
{
   LGRect newrect;
   Gadget *rgad;
   char new_name[128];

   newrect.ul = loc;
   newrect.lr.x = newrect.ul.x + size.x;
   newrect.lr.y = newrect.ul.y + size.y;
   strcpy(new_name, "pb-sub-");
   strcat(new_name, GD_NAME(((Gadget *)ui_data)->rep));
   rgad = gad_pushbutton_create((Gadget *)ui_data, &newrect, 0, button_type, display_data, sty, new_name);
   *pptng = rgad->tng_data;
   return(rgad);
}

