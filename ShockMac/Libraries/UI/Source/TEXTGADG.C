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
// Source code for the Text Gadget

// THINGS STILL NEEDED:
// Cursors!!

// Gadgety stuff
#include <string.h>

#include "gadgets.h"
#include "textgadg.h"
#include "tngtextg.h"

// Other usefuls
//#include <_ui.h>

// fonts
#include "fakefont.h"

Gadget *gad_text_create(Gadget *parent, LGRect *dim, int z, ulong options, TNGStyle *sty, char *name)
{
   Gadget *retgad;
   TNG *temp_tng;
   LGPoint dsize, dloc;

   temp_tng = (TNG *)NewPtr(sizeof(TNG));

   dsize.x = RectWidth(dim);
   dsize.y = RectHeight(dim);
   dloc.x = parent->rep->abs_x + dim->ul.x;
   dloc.y = parent->rep->abs_y + dim->ul.y;
   tng_textgadget_init(NULL, temp_tng, sty, options, dsize, dloc); 

   gadget_create_setup(&retgad, parent, CLASS_TEXT, dim, z, name);
   DisposePtr((Ptr)retgad->tng_data);
   temp_tng->ui_data = retgad;
   retgad->tng_data = temp_tng;

   // Let the TNG get another shot...
   tng_textgadget_init2(temp_tng);

   return (retgad);
}

Gadget *gad_textgadget_create_from_tng(void *ui_data, LGPoint loc, TNG **pptng, TNGStyle *sty, ulong options, LGPoint size)
{
   LGRect newrect;
   Gadget *rgad, *g;
   char new_name[128];

   g = (Gadget *)ui_data;   
   newrect.ul = loc;
   newrect.lr.x = newrect.ul.x + size.x;
   newrect.lr.y = newrect.ul.y + size.y;
   strcpy(new_name, "textgadg-sub-");
   strcat(new_name, GD_NAME(g->rep));
   rgad = gad_text_create(g, &newrect, 0, options, sty, new_name);
   *pptng = rgad->tng_data;
   return(rgad);
}
