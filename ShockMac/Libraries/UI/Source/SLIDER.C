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
#include "slider.h"
#include "event.h"
//#include <_ui.h>
#include "dbg.h"
#include "tngslidr.h"

Gadget *gad_slider_create(Gadget *parent, LGRect *dim, int z, int alignment, int increment,
   int min, int max, TNGStyle *sty, char *name)
{
   return(gad_slider_create_full(parent, dim, z, alignment, increment, min, max, sty, name,
      NULL, NULL, NULL, NULL, NULL));
}

Gadget *gad_slider_create_full(Gadget *parent, LGRect *dim, int z, int alignment, int increment,
   int min, int max, TNGStyle *sty, char *name, Ref res_left, Ref res_right, Ref res_up,
   Ref res_down, Ref res_slider)
{
   Gadget *retgad;
//   TNG *sl_tng;
   LGPoint size;

   gadget_create_setup(&retgad, parent, CLASS_SLIDER, dim, z, name);

   size.x = RectWidth(dim);
   size.y = RectHeight(dim);
   tng_slider_full_init(retgad, retgad->tng_data, sty, alignment, min, max,
      min, increment, size, res_left, res_right, res_up, res_down, res_slider);

   return (retgad);
}

char new_name[80];

Gadget *gad_slider_create_from_tng(void *ui_data, LGPoint loc, TNG **pptng, TNGStyle *sty, int alignment, int min, int max,
   int value, int increment, LGPoint size)
{
   LGRect newrect;
   Gadget *rgad;

   newrect.ul = loc;
   newrect.lr.x = newrect.ul.x + size.x;
   newrect.lr.y = newrect.ul.y + size.y;
   strcpy(new_name,"slider-sub-");
   strcat(new_name,GD_NAME(((Gadget *)ui_data)->rep));
   rgad = gad_slider_create((Gadget *)ui_data, &newrect, 0, alignment, increment, min, max, sty, new_name);
//      GD_NAME(((Gadget *)ui_data)->rep));
   TNG_SL(rgad->tng_data)->value = value;
   *pptng = rgad->tng_data;
   return(rgad);
}

