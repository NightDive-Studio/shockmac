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
//#include <fcntl.h>
//#include <io.h>

#include "tngpushb.h"
//#include <_ui.h>

// Initializes the TNG G
errtype tng_pushbutton_init(void *ui_data, TNG *ptng, TNGStyle *sty, int button_type, void *display_data, LGPoint size)
{
   TNG_pushbutton *ppbtng;

   ppbtng = (TNG_pushbutton *)GUI_MALLOC(ptng->ui_data, sizeof(TNG_pushbutton));
//   Spew(DSRC_UI_Pushbutton, ("Starting pushbutton init...\n"));

   TNGInit(ptng,sty,ui_data);
   ptng->flags = TNG_BEVEL;
   ptng->type_data = ppbtng;
   ptng->draw_func = &tng_pushbutton_2d_draw;
   ptng->mousebutt = &tng_pushbutton_mousebutt;
   ptng->keycooked = &tng_pushbutton_keycooked;
   ptng->signal = &tng_pushbutton_signal;

   ppbtng->tng_data = ptng;
   ppbtng->size = size;
   ppbtng->disp_data = display_data;
   ppbtng->type = button_type;
   ppbtng->pressed = FALSE;
   return(OK);
}

// Deallocate all memory used by the TNG 
errtype tng_pushbutton_destroy(TNG *ptng)
{
   GUI_DEALLOC(ptng->ui_data, ptng->type_data);
   return(OK);
}

// Draw the specified parts (may be all) of the TNG at screen coordinates loc
// assumes all appropriate setup has already been done!
errtype tng_pushbutton_2d_draw(TNG *ptng, ushort partmask, LGPoint loc)
{
   TNG_pushbutton *ppbtng;
   LGRect r;
   Ref id;
   ushort dummy;
   dummy = partmask;

//   Spew(DSRC_UI_Pushbutton, ("TNG Pushbutton 2d Draw at (%d, %d) -- partmask = %x\n",loc.x,loc.y,partmask));
   TNG_IF_OBSCURED(ptng)
   {
      return(OK);
   }
   ptng->signal(ptng, TNG_SIGNAL_EXPOSE);
   ppbtng = TNG_PB(ptng);
   TNGDrawBase(ptng, loc, ppbtng->size);
   r.ul.x = loc.x + 1;
   r.ul.y = loc.y + 1;  // +1 to compensate for bevelling space
   r.lr.x = r.ul.x + ppbtng->size.x;
   r.lr.y = r.ul.y + ppbtng->size.y;

   switch(ppbtng->type)
   {
      case TNG_COLORED_TYPE:
         gr_set_fcolor(*((int *)(ppbtng->disp_data)));
         gr_rect(r.ul.x, r.ul.y, r.lr.x, r.lr.y);
         break;
      case TNG_TEXT_TYPE:
         gr_set_fcolor(ptng->style->textColor);
         TNG_DRAW_TEXT(ptng, (char *)(ppbtng->disp_data), r.ul.x, r.ul.y);
//         gr_set_font(ppbtng->font_buf);
//         gr_string((char *)(ppbtng->disp_data), r.ul.x, r.ul.y);
         break;
      case TNG_RESOURCE_TYPE:
         id = *((Ref *)(ppbtng->disp_data));
         draw_resource_bm(id, r.ul.x, r.ul.y);
         break;
   }
   return(OK);
}

// Fill in ppt with the size...
errtype tng_pushbutton_size(TNG *ptng, LGPoint *ppt)
{
   *ppt = TNG_PB(ptng)->size;
   return(OK);
}

// Returns the current "value" of the TNG
int tng_pushbutton_getvalue(TNG *ptng)
{
   return(TNG_PB(ptng)->pressed);
}

// React appropriately for receiving the specified cooked key
bool tng_pushbutton_keycooked(TNG *ptng, ushort key)
{
   int code = key & 0xff;
   bool retval = FALSE;

   if (code == 0xd)
   {
      IF_RV(tng_pushbutton_pressed(TNG_PB(ptng)));
      IF_SET_RV(ptng->signal(ptng,TNG_SIGNAL_SELECT));
      IF_SET_RV(ptng->signal(ptng,TNG_SIGNAL_DESELECT));
   }
   IF_SET_RV(tng_cb_keycooked(ptng, key));
   return(retval);
}

// React appropriately for receiving the specified mouse button event
bool tng_pushbutton_mousebutt(TNG *ptng, uchar type, LGPoint loc)
{
   bool retval = FALSE;
//   Spew(DSRC_UI_Pushbutton, ("After doing cb_mousebutt, inside pushbutton_mousebutt\n"));
   if (type == TNG_MOUSE_LDOWN)
      IF_SET_RV(ptng->signal(ptng,TNG_SIGNAL_SELECT));
   if (type == TNG_MOUSE_LUP)
      IF_SET_RV(ptng->signal(ptng,TNG_SIGNAL_DESELECT));
   IF_SET_RV(tng_cb_mousebutt(ptng,type,loc));
   retval = TRUE;
   return(retval);
}

// Handle incoming signals
bool tng_pushbutton_signal(TNG *ptng, ushort signal)
{
   bool retval = FALSE;
//   Spew(DSRC_UI_Pushbutton, ("After doing cb_signal, inside pushbutton_signal\n"));
   if (signal & TNG_SIGNAL_SELECT)
      IF_SET_RV(tng_pushbutton_pressed(TNG_PB(ptng)));
   if (signal & TNG_SIGNAL_DESELECT)
      IF_SET_RV(tng_pushbutton_released(TNG_PB(ptng)));
   IF_SET_RV(tng_cb_signal(ptng,signal));
   retval = TRUE;
   return(retval);
}

errtype tng_pushbutton_pressed(TNG_pushbutton *ppbtng)
{
   ppbtng->tng_data->flags |= TNG_INVERTBEVEL;
   TNG_DRAW(ppbtng->tng_data);
 
   return(OK);
}

errtype tng_pushbutton_released(TNG_pushbutton *ppbtng)
{
   ppbtng->tng_data->flags &= ~TNG_INVERTBEVEL;
   TNG_DRAW(ppbtng->tng_data);
 
   return(OK);
}
