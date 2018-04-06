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
#include "tngapp.h"
#include "tngslidr.h"
//#include <_ui.h>

// Initializes the TNG slider
errtype tng_slider_init(void *ui_data, TNG *ptng, TNGStyle *sty, int alignment, int min, int max, int value, int increm, LGPoint size)
{
   return(tng_slider_full_init(ui_data, ptng, sty, alignment, min, max, value, increm, size,
      NULL, NULL, NULL, NULL, NULL));
}

// Initializes the TNG slider
errtype tng_slider_full_init(void *ui_data, TNG *ptng, TNGStyle *sty, int alignment, int min, int max, int value, int increm, LGPoint size,
   Ref left_id, Ref right_id, Ref up_id, Ref down_id, Ref slider_id)
{
   TNG_slider *psltng;
   psltng = (TNG_slider *)GUI_MALLOC(ptng->ui_data, sizeof(TNG_slider));

   //Spew(DSRC_UI_Slider, ("Starting slider init...\n"));
   TNGInit(ptng,sty,ui_data);
   ptng->flags = TNG_BEVEL;
   ptng->type_data = psltng;
   ptng->draw_func = &tng_slider_2d_draw;
   ptng->mousebutt = &tng_slider_mousebutt;
   ptng->keycooked = &tng_slider_keycooked;
   ptng->mousemove = &tng_slider_apply_click;
   ptng->signal    = &tng_slider_signal;

   psltng->tng_data = ptng;
   psltng->alignment = alignment;
   psltng->min = min;
   psltng->max = max;
   psltng->value = value;
   psltng->increm = increm;
   psltng->size = size;
   psltng->dragging = FALSE;
   
   psltng->left_id = left_id;
   psltng->right_id = right_id;
   psltng->up_id = up_id;
   psltng->down_id = down_id;
   psltng->slider_id = slider_id;
   return(OK);
}

// Deallocate all memory used by the TNG slider
errtype tng_slider_destroy(TNG *ptng)
{
   GUI_DEALLOC(ptng->ui_data, ptng->type_data);
   return(OK);
}

// Draw the specified parts (may be all) of the TNG slider at screen coordinates loc
// assumes all appropriate setup has already been done!
errtype tng_slider_2d_draw(TNG *ptng, ushort partmask, LGPoint loc)
{
   LGPoint size;
   Ref decid, incid;   
   LGPoint incsize, decsize,slidsize;
   LGPoint p1,p2,p3;
   TNG_slider *psltng;
   int xc,yc;
   float temp;

//   Spew(DSRC_UI_Slider, ("TNG Slider 2d Draw at (%d, %d) -- partmask = %x\n  value = %d\n",loc.x,loc.y,partmask,
//      TNG_SL(ptng)->value));
   TNG_IF_OBSCURED(ptng)
   {
      return(OK);
   }
   psltng = (TNG_slider *)ptng->type_data;
   tng_slider_size(ptng, &size);
   TNGDrawBase(ptng, loc, size);
   loc.x += 1; loc.y += 1;  // compensate for bevelling space

   if (psltng->alignment == TNG_SL_HORIZONTAL)
   {
      decid = psltng->left_id;
      incid = psltng->right_id;
   }
   else
   {
      incid = psltng->down_id;  // Okay, this *seems* backwards but to make the drawing
      decid = psltng->up_id;    // easier it is best to maintain this fiction.
   }
   if (decid == NULL)
   {
      decsize.x = ptng->style->frobsize.x;
      decsize.y = ptng->style->frobsize.y;
   }
   else
   {
      decsize.x = resource_bm_width(decid);
      decsize.y = resource_bm_height(decid);
   }
   if (incid == NULL)
   {
      incsize.x = ptng->style->frobsize.x;
      incsize.y = ptng->style->frobsize.y;
   }
   else
   {
      incsize.x = resource_bm_width(incid);
      incsize.y = resource_bm_height(incid);
   }
   if (psltng->slider_id == NULL)
   {
      slidsize.x = ptng->style->frobsize.x;
      slidsize.y = ptng->style->frobsize.y;
   }
   else
   {
      slidsize.x = resource_bm_width(psltng->slider_id);
      slidsize.y = resource_bm_height(psltng->slider_id);
   }

   // Draw incrementer
   if (partmask && TNG_SL_INCREMENTER)
   {
      if (psltng->alignment == TNG_SL_HORIZONTAL)
      {
         xc = loc.x + size.x - incsize.x - 2;
         yc = loc.y;
      }
      else
      {
         xc = loc.x;
         yc = loc.y + size.y - incsize.y - 2;
      }
      if (incid != NULL)
         draw_resource_bm(incid, xc,yc);
      else
      {
         if (psltng->alignment == TNG_SL_VERTICAL)
         {
            p1.x = xc;  p1.y = yc;
            p2.x = xc + incsize.x; p2.y = yc;
            p3.x = xc + (.5 * incsize.x) - 1;  p3.y = yc + incsize.y - 1;
         }
         else
         {
            p1.x = xc;  p1.y = yc;
            p2.x = xc;  p2.y = yc + incsize.y - 2;
            p3.x = xc + incsize.x;  p3.y = yc + (.5 * (incsize.y - 2));
         }
         gr_set_fcolor(ptng->style->textColor);
         gr_int_line(p1.x,p1.y, p2.x, p2.y);
         gr_int_line(p1.x,p1.y, p3.x, p3.y);
         gr_int_line(p2.x,p2.y, p3.x, p3.y);
      }
   }

   if (partmask && TNG_SL_DECREMENTER)
   {
      if (decid != NULL)
      {
         draw_resource_bm(decid, loc.x, loc.y);
      }
      else
      {
         if (psltng->alignment == TNG_SL_VERTICAL)
         {
            p1.x = loc.x;  p1.y = incsize.y + loc.y;
            p2.x = loc.x + decsize.x; p2.y = decsize.y + loc.y - 1;
            p3.x = loc.x + (.5 * decsize.x) - 1;  p3.y = loc.y;
         }
         else
         {
            p1.x = loc.x + decsize.x;  p1.y = loc.y;
            p2.x = loc.x + decsize.x;  p2.y = loc.y + decsize.y - 2;
            p3.x = loc.x;  p3.y = (.5 * (decsize.y - 2)) + loc.y;
         }
         gr_set_fcolor(ptng->style->textColor);
         gr_int_line(p1.x,p1.y, p2.x, p2.y);
         gr_int_line(p1.x,p1.y, p3.x, p3.y);
         gr_int_line(p2.x,p2.y, p3.x, p3.y);
      }
   }

   // Draw slider
   if (partmask && TNG_SL_SLIDER)
   {
      if (psltng->alignment == TNG_SL_HORIZONTAL)
      {
         temp = TNG_SL_VALFRAC(psltng) * (float)(size.x - incsize.x - decsize.x - slidsize.x);
         xc = loc.x + decsize.x + temp;
         yc = loc.y;
      }
      else
      {
         xc = loc.x;
         temp = (1 - TNG_SL_VALFRAC(psltng)) * (float)(size.y - incsize.y - 2 - decsize.y - slidsize.y);
         yc = loc.y + decsize.y + temp;
      }
      if (psltng->slider_id != NULL)
         draw_resource_bm(psltng->slider_id, xc, yc);
      else
      {
         gr_set_fcolor(ptng->style->textColor);
         gr_rect(xc,yc,xc+slidsize.x -1,yc+slidsize.y-1);
      }
   }
   return(OK);
}

// Fill in ppt with the size of the TNG slider
errtype tng_slider_size(TNG *ptng, LGPoint *ppt)
{
   TNG_slider *psltng;
   psltng = (TNG_slider *)ptng->type_data;
   ppt->x = psltng->size.x;
   ppt->y = psltng->size.y;
   return(OK);
}

// Returns the current "value" of the TNG slider
int tng_slider_getvalue(TNG *ptng)
{
   return(((TNG_slider *)(ptng->type_data))->value);
}

// React appropriately for receiving the specified cooked key
bool tng_slider_keycooked(TNG *ptng, ushort key)
{
   TNG_slider *psltng;
   int code = key & 0xff;
   bool retval = FALSE;

   psltng = (TNG_slider *)ptng->type_data;
   if (psltng->alignment == TNG_SL_VERTICAL)
   {
      if (code == TNG_SL_DOWN_KEY)
         IF_SET_RV(ptng->signal(ptng, TNG_SIGNAL_DECREMENT));
      if (code == TNG_SL_UP_KEY)
         IF_SET_RV(ptng->signal(ptng, TNG_SIGNAL_INCREMENT));
   }
   else
   {
      if (code == TNG_SL_LEFT_KEY)
         IF_SET_RV(ptng->signal(ptng, TNG_SIGNAL_DECREMENT));
      if (code == TNG_SL_RIGHT_KEY)
         IF_SET_RV(ptng->signal(ptng, TNG_SIGNAL_INCREMENT));
   }
   IF_SET_RV(tng_cb_keycooked(ptng, key));
   return(retval);
}

bool tng_slider_apply_click(TNG *ptng, LGPoint loc)
{
   int perc;
   TNG_slider *psltng;
   int right_edge, left_edge, top_edge, bottom_edge;
   bool inc_area, dec_area, retval = FALSE;
   float t1,t2;

   psltng = (TNG_slider *)ptng->type_data;
   if (!psltng->dragging)
      return(FALSE);
   if ((loc.x < 0) || (loc.y < 0) || (loc.x > psltng->size.x) || (loc.y > psltng->size.y))
   {
     //Spew(DSRC_UI_Slider, ("Slider Releasing Focus\n"));
     psltng->dragging = FALSE;
     TNG_RELEASE_FOCUS(ptng, TNG_EVENT_MOUSE|TNG_EVENT_MOUSE_MOVE);
     return(FALSE);
   }
   if (psltng->alignment == TNG_SL_HORIZONTAL)
   {
      if (psltng->right_id != NULL)
         right_edge = psltng->size.x - resource_bm_width(psltng->right_id) - 2;
      else
         right_edge = psltng->size.x - ptng->style->frobsize.x - 2;
      if (psltng->left_id != NULL)
         left_edge = resource_bm_width(psltng->left_id) + 2;
      else
         left_edge = ptng->style->frobsize.x + 2;
      perc = (int)(100 * ((float)(loc.x - left_edge) / ((float)(right_edge - left_edge))));
      //Spew(DSRC_UI_Slider, ("loc=%d  left=%d  right=%d perc=%d\n",loc.x,left_edge,right_edge,perc));
      inc_area = loc.x > right_edge;
      dec_area = loc.x < left_edge;
   }
   else
   {
      if (psltng->up_id != NULL)
         top_edge = resource_bm_height(psltng->up_id) + 2;
      else
         top_edge = ptng->style->frobsize.y + 2;
      if (psltng->down_id != NULL)
         bottom_edge = psltng->size.y - resource_bm_height(psltng->down_id) - 2;
      else
         bottom_edge = psltng->size.y - ptng->style->frobsize.y - 2;
      t1 = (float)(loc.y - top_edge);
      t2 = (float)(bottom_edge - top_edge);
      perc = (int)(100 *(1 - ((float)(loc.y - top_edge) / ((float)(bottom_edge - top_edge)))));
      //Spew(DSRC_UI_Slider, ("Figuring out where I am... loc.y = %d top_edge = %d  bottom_edge = %d\n",
      //  loc.y, top_edge, bottom_edge));
      //Spew(DSRC_UI_Slider, ("perc = %d  t1 = %f  t2 = %f\n",perc,t1,t2));
      inc_area = loc.y < top_edge;
      dec_area = loc.y > bottom_edge;
   }
   if (inc_area)
   {
      IF_SET_RV(ptng->signal(ptng, TNG_SIGNAL_INCREMENT));
   }
   else if (dec_area)
   {
      IF_SET_RV(ptng->signal(ptng, TNG_SIGNAL_DECREMENT));
   }
   else
   {
      IF_RV(tng_slider_set(psltng, perc));
   }
   return(retval);
}

// React appropriately for receiving the specified mouse button event
bool tng_slider_mousebutt(TNG *ptng, uchar type, LGPoint loc)
{
   TNG_slider *psltng;
   bool retval = FALSE;

   psltng = (TNG_slider *)ptng->type_data;
   //Spew(DSRC_UI_Slider, ("type == %x  MOUSE_LDOWN = %x  MOUSE_LUP = %x\n",type, TNG_MOUSE_LDOWN,TNG_MOUSE_LUP));
   if (type & TNG_MOUSE_LUP)
   {
     //Spew(DSRC_UI_Slider, ("Slider Releasing Focus\n"));
     psltng->dragging = FALSE;
     TNG_RELEASE_FOCUS(ptng, TNG_EVENT_MOUSE|TNG_EVENT_MOUSE_MOVE);
     IF_SET_RV(ptng->signal(ptng, TNG_SIGNAL_CHANGED));
   }
   else if (type & TNG_MOUSE_LDOWN)
   {
      //Spew(DSRC_UI_Slider, ("Slider Grabbing Focus\n"));
      psltng->dragging = TRUE;
      TNG_GRAB_FOCUS(ptng, TNG_EVENT_MOUSE|TNG_EVENT_MOUSE_MOVE);
      IF_SET_RV(tng_slider_apply_click(ptng, loc));
   }
   IF_SET_RV(tng_cb_mousebutt(ptng, type,loc));
   retval = TRUE;
   return(retval);
}

// Handle incoming signals
bool tng_slider_signal(TNG *ptng, ushort signal)
{
   bool retval = FALSE;
   //Spew(DSRC_UI_Slider, ("Slider Received signal: %x\n",signal));
   if (signal & TNG_SIGNAL_INCREMENT)
      IF_SET_RV(tng_slider_increm((TNG_slider *)ptng->type_data));
   if (signal & TNG_SIGNAL_DECREMENT)
      IF_SET_RV(tng_slider_decrem((TNG_slider *)ptng->type_data));
   if (signal & TNG_SIGNAL_CHANGED)
      IF_RV(TNG_DRAWPART(ptng, TNG_SL_SLIDER));
   IF_SET_RV(tng_cb_signal(ptng,signal));
   return(retval);
}

bool tng_slider_increm(TNG_slider *psltng)
{
   psltng->value += psltng->increm;
   if (psltng->value > psltng->max)
      psltng->value = psltng->max;
   psltng->tng_data->signal(psltng->tng_data, TNG_SIGNAL_CHANGED);
   return(TRUE);
}

bool tng_slider_decrem(TNG_slider *psltng)
{
   psltng->value -= psltng->increm;
   if (psltng->value < psltng->min)
      psltng->value = psltng->min;
   psltng->tng_data->signal(psltng->tng_data, TNG_SIGNAL_CHANGED);
   return(TRUE);
}

errtype tng_slider_set(TNG_slider *psltng, int perc)
{
   int temp, temp1;
   float ftemp;
   //Spew(DSRC_UI_Slider, ("perc = %d\n",perc));
   ftemp = (float)perc / 100;
   temp1 = psltng->max - psltng->min;
   temp = (int)((ftemp * temp1) + 0.5);
//   temp = (int)(((perc / 100) * (psltng->max - psltng->min)) + 0.5);
   psltng->value = psltng->min + temp;
   psltng->tng_data->signal(psltng->tng_data, TNG_SIGNAL_CHANGED);
   return(OK);
}
