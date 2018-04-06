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
// Source Code for the LGRegion library

#include "dbg.h"
//#include <_ui.h>
#include "region.h"
#include "slist.h"

#include "gadgets.h"

typedef struct {
   struct _slist *psnext;
   LGRegion *reg;
   LGRect exp_rect;
} Region_Sequence_Element;


/* Prototypes */

LGRegion *trav_get_first(LGRegion *reg, int order);
LGRegion *trav_get_next(LGRegion *curp, int order);
errtype region_place(LGRegion *reg);
errtype region_remove(LGRegion *reg, bool draw);
void region_moverect(LGRegion *reg, int delta_x, int delta_y, int move_rel);
void region_propagate_callback(LGRegion *reg,  ulong callback_code, LGRect *arg_rect);
bool reg_exp_CB(LGRegion *reg, LGRect *rc, void *data);
errtype region_expose_absolute(LGRegion *reg, LGRect *newr);
errtype region_manage_place(LGRegion *reg);
errtype region_manage_remove(LGRegion *reg);
int region_convert_tochild(LGRegion *from_reg, LGRect *orig, LGRect *conv);
errtype region_abs_rect(LGRegion *reg, LGRect *orig_rect, LGRect *conv);
errtype region_set_moving(LGRegion *reg, int val);
int region_convert_tochild(LGRegion *from_reg, LGRect *orig, LGRect *conv);
int region_convert_toparent(LGRegion *from_reg, LGRect *orig, LGRect *conv);
bool is_child(LGRegion *poss_parent, LGRegion *child);
bool region_obscured_callback(LGRegion *reg, LGRect *r, void *data);
Region_Sequence_Element *get_rse_from_pool();
errtype return_rse_to_pool(Region_Sequence_Element *rse);
errtype region_add_sequence_expose(LGRegion *reg, LGRect exp_rect);

/* Globals */

int region_in_sequence = 0;
bool region_system_init = FALSE, region_found;
slist_head sequence_header;
LGRegion *obsc_region, *current_expose_region;

/* API FUNCTIONS */

errtype init_rse_pool();

errtype region_init()
{
   region_system_init = TRUE;
   slist_init(&sequence_header);
   init_rse_pool();
   return(OK);
}

errtype region_create(LGRegion *parent, LGRegion *ret, LGRect *r, int z, int event_order,
   ulong status, RectCallback expose,
   RectCallback save_under, RectCallback replace, void *user_data)
{
   LGRegion *curp, *lastp;

   if (!region_system_init)
      region_init();

   /* Plug in parameters */
   ret->real_rect = *r;
   ret->r = &(ret->real_rect);
   ret->z = z;
   ret->event_order = event_order;                              
   ret->status_flags = status;
   ret->expose = expose;
   ret->save_under = save_under;
   ret->replace = replace;
   ret->user_data = user_data;
   ret->parent = parent;
   
   /* Compute initial values for other things */
   ret->handler = NULL;
   ret->cursors = NULL;
   ret->moving = 0;
   ret->sub_region = NULL;
   ret->next_region = NULL;
   ret->device_type = -1;
   ret->abs_x = r->ul.x;  // If in root region, rel = abs
   ret->abs_y = r->ul.y;  // otherwise, is base for real abs
   if (ret->parent != NULL)
   {
      /* Inherit device_type value */
      ret->device_type = (ret->parent)->device_type;
      ret->abs_x += (ret->parent)->abs_x;
      ret->abs_y += (ret->parent)->abs_y;
      curp = (ret->parent)->sub_region;
      // Spew(DSRC_UI_Initialization, ("parent loop: reg = %s(%d) ",GD_NAME(ret), ret->z));
      if (curp == NULL)
      {
         // If we are first child of parent...
         (ret->parent)->sub_region = ret;
      } else {
         // Otherwise put ourselves in proper place in z-sorted chain
         lastp = NULL;
         while ((curp != NULL) && (curp->z > ret->z)) {
            lastp = curp;
            curp = curp->next_region;
         }
 /*
         if (lastp != NULL)
             Spew(DSRC_UI_Initialization, ("lastp = %s(%d)  ",GD_NAME(lastp),lastp->z));
         if (curp != NULL)
             Spew(DSRC_UI_Initialization, ("curp  = %s(%d)  ",GD_NAME(curp),curp->z));
         Spew(DSRC_UI_Initialization, ("!\n"));
 */
         ret->next_region = curp;
         if (lastp != NULL)
            lastp->next_region = ret;
         else
            (ret->parent)->sub_region = ret;
      }
   }

   if (ret->status_flags & DISPLAY_ON_CREATION)
   {
      // Spew(DSRC_UI_Initialization, ("right before region_place: abs_x = %d, abs_y = %d\n",ret->abs_x, ret->abs_y));
      region_place(ret);
   }

   return(OK);
}

errtype region_destroy(LGRegion *reg, bool draw)
{
   LGRegion *curp, *lastp, *nextp;
   extern errtype uiShutdownRegionHandlers(LGRegion* r);

   // Make us disappear
   region_remove(reg,draw);

   // First, we kill our children...all of them!!
   curp = reg->sub_region;
   while (curp != NULL)
   {
      nextp = curp->next_region;;
      region_destroy(curp,draw);
      curp = nextp;
   }

   // Then, we make our siblings / parent forget about us!
   if (reg->parent != NULL)
   {
      curp = (reg->parent)->sub_region;
      lastp = NULL;
     while (curp != reg)
      {
         lastp = curp;
         curp = curp->next_region;
      }
      if (lastp != NULL)
         lastp->next_region = curp->next_region;
      else
         (reg->parent)->sub_region = curp->next_region;
   }

   // Shutdown handlers
   uiShutdownRegionHandlers(reg);
   
   // Then, we kill OURSELVES!!!!!!!!!!!!
   if (AUTODESTROY_FLAG & reg->status_flags)
   {
      DisposePtr((Ptr)reg->r);
      DisposePtr((Ptr)reg);
   }
   return(OK);
}

errtype region_move(LGRegion *reg, int new_x, int new_y, int new_z)
{
   int delta_x, delta_y;
   LGRegion *lastp, *curp;

   /* trigger the "picking up" callbacks */
   region_remove(reg,TRUE);

   // Spew(DSRC_UI_Region_Manipulation, ("after region_remove call..."));
   /* Update the database numbers */
   delta_x = new_x - (reg->r)->ul.x;
   delta_y = new_y - (reg->r)->ul.y;
   region_moverect(reg, delta_x, delta_y, 1);
   // Spew(DSRC_UI_Region_Manipulation, ("after region_moverect..."));
   if (reg->z != new_z)
   {
      // disconnect us from our previous position
      if (reg->parent != NULL)
      {
         curp = (reg->parent)->sub_region;
         lastp = NULL;
         while (curp != reg)
         {
            lastp = curp;
            curp = curp->next_region;
         }
         if (lastp != NULL)
            lastp->next_region = curp->next_region;
         else
            (reg->parent)->sub_region = curp->next_region;

         // Spew(DSRC_UI_Region_Manipulation, ("After disconnection..."));

         // set new value
         reg->z = new_z;

         // reconnect us in correct position
         curp = (reg->parent)->sub_region;
        // Spew(DSRC_UI_Region_Manipulation, ("reg = %s(%d) ",GD_NAME(reg), reg->z));
        if (curp == NULL)
         {
            // If we are first child of parent...
            (reg->parent)->sub_region = reg;
         } else {
            // Otherwise put ourselves in proper place in z-sorted chain
            lastp = NULL;
            while ((curp != NULL) && (curp->z > reg->z)) {
               lastp = curp;
               curp = curp->next_region;
            }
  /*
            if (lastp != NULL)
                Spew(DSRC_UI_Utilities, ("lastp = %s(%d)  ",GD_NAME(lastp),lastp->z));
            if (curp != NULL)
                Spew(DSRC_UI_Utilities, ("curp  = %s(%d)  ",GD_NAME(curp),curp->z));
             Spew(DSRC_UI_Utilities, ("!\n"));
   */
            reg->next_region = curp;
            if (lastp != NULL)
               lastp->next_region = reg;
            else
               (reg->parent)->sub_region = reg;
         }
         // Spew(DSRC_UI_Region_Manipulation, ("After reconnection..."));
      }

   }

   /* trigger the "placing down" callbacks */
   region_place(reg);
   return(OK);
}

errtype region_resize(LGRegion *reg, int new_x_size, int new_y_size)
{
   int delta_x, delta_y;

   delta_x = new_x_size - RectWidth(reg->r);
   delta_y = new_y_size - RectHeight(reg->r);

   (reg->r)->lr.x += delta_x;
   (reg->r)->lr.y += delta_y;

   region_place(reg);
   return(OK);
}

int region_traverse_point(LGRegion *reg, LGPoint target, TravRectCallback fn, int order, void *data)
{
   LGRect inter, newtarget;
   LGRegion *curp;
   int retval = 0, iflag = 0;

//   Spew(DSRC_UI_Traversal, ("r_t_point -- target = (%d, %d) %s->r = (%d, %d) - (%d, %d)\n",
//      target.x, target.y, GD_NAME(reg), RECT_PRINT_ARGS(reg->r)));

   inter.ul = target;
   inter.lr = target;
   if ((reg->status_flags & INVISIBLE_FLAG) != 0)
      return FALSE;
   if (RectTestPt(reg->r, target))
      iflag = 1;
   if ((order == BOTTOM_TO_TOP) && iflag)
   {
//      Spew(DSRC_UI_Traversal, ("BOTTOM_TO_TOP, root case, target = (%d,%d)\n",target.x, target.y));
      retval = fn(reg, &inter, data);
   }
   curp = trav_get_first(reg, order);
//   Spew(DSRC_UI_Traversal, ("before while, curp =  "));
//   if (curp)
//      Spew(DSRC_UI_Traversal, ("%s\n",GD_NAME(curp)));
//   else
//      Spew(DSRC_UI_Traversal, ("NULL\n"));
   while (!retval && (curp != NULL))
   {
      region_convert_tochild(reg, &inter, &newtarget);
//      Spew(DSRC_UI_Traversal, ("while case, newtarget.ul = (%d,%d)\n",newtarget.ul.x, newtarget.ul.y));
      retval = region_traverse_point(curp, newtarget.ul, fn, order, data);
      curp = trav_get_next(curp, order);
   }                                                                                                                 
   if ((order == TOP_TO_BOTTOM) && (iflag) && (!retval))
   {
//      Spew(DSRC_UI_Traversal, ("TOP_TO_BOTTOM, root case\n"));
      retval = fn(reg, &inter, data);
   }
   return (retval);
}

int region_traverse_rect(LGRegion *reg, LGRect *target, TravRectCallback fn, int order,
   void *data)
{
   LGRect inter, newtarget;
   LGRegion *curp;
   int retval = 0, iflag = 0;

   // Spew(DSRC_UI_Traversal, ("r_t_r -- target = (%d, %d) - (%d, %d) %s->r = (%d, %d) - (%d, %d)\n",
   //  RECT_PRINT_ARGS(target), GD_NAME(reg), RECT_PRINT_ARGS(reg->r)));
   if ((reg->status_flags & INVISIBLE_FLAG) != 0)
      return FALSE;
   if (RectTestSect(reg->r, target)) {
      RectSect(reg->r, target, &inter);
      iflag = 1;
   }
   if ((order == BOTTOM_TO_TOP) && iflag)
   {
      // Spew(DSRC_UI_Traversal, ("BOTTOM_TO_TOP, root case, target = (%d,%d)(%d,%d)\n",RECT_EXPAND_ARGS(target)));
      retval = fn(reg, &inter, data);
   }
   curp = trav_get_first(reg, order);
/*
   if (curp)
   {
      Spew(DSRC_UI_Traversal,("before while, curp = %s\n",GD_NAME(curp)));
   }
   else
   {
      Spew(DSRC_UI_Traversal,("before while, curp = NULL\n"));
   }
*/
   while (!retval && curp)
   {
      region_convert_tochild(reg, target, &newtarget);
      // Spew(DSRC_UI_Traversal, ("while case, newtarget = (%d,%d)(%d,%d)\n",RECT_EXPAND_ARGS(&newtarget)));
      retval = region_traverse_rect(curp, &newtarget, fn, order, data);
      curp = trav_get_next(curp,order);
   }
   if ((order == TOP_TO_BOTTOM) && (iflag) && (!retval))
   {
      // Spew (DSRC_UI_Traversal, ("TOP_TO_BOTTOM, root case\n"));
      retval = fn(reg, &inter, data);
   }
   return (retval);
}

int region_traverse(LGRegion *reg, TravCallback fn, int order, void *data)
{
   LGRegion *curp;
   int retval = 0;

   // Spew(DSRC_UI_Traversal, ("r_traverse -- %s\n",GD_NAME(reg)));
   if ((reg->status_flags & INVISIBLE_FLAG) != 0)
      return FALSE;
   if (order == BOTTOM_TO_TOP)
   {
      // Spew (DSRC_UI_Traversal, ("BOTTOM_TO_TOP, root case\n"));
      retval = fn(reg, data);
   }
   curp = trav_get_first(reg, order);
/*
   if (curp)
   {
      Spew(DSRC_UI_Traversal,("before while, curp = %s\n",GD_NAME(curp)));
   }
   else
   {
      Spew(DSRC_UI_Traversal,("before while, curp = NULL\n"));
   }
*/
   while (!retval && curp)
   {
      retval = region_traverse(curp,  fn, order, data);
      curp = trav_get_next(curp,order);
   }
   if ((order == TOP_TO_BOTTOM) && (!retval))
   {
      // Spew (DSRC_UI_Traversal, ("TOP_TO_BOTTOM, root case\n"));
      retval = fn(reg, data);
   }
   return (retval);
}

LGRegion *trav_get_first(LGRegion *reg, int order)
{
   LGRegion *ptr, *retval;

   if (order == TOP_TO_BOTTOM)
      return(reg->sub_region);
   else
   {
      ptr = reg->sub_region;
      retval = NULL;
      while (ptr != NULL)
      {
         retval = ptr;
         ptr = ptr->next_region;
      }
      return(retval);
   }
}

LGRegion *trav_get_next(LGRegion *curp, int order)
{
   LGRegion *retval, *ptr;

   if (order == TOP_TO_BOTTOM)
   {
      retval = curp->next_region;
   }
   else
   {
      if (curp->parent != NULL)
         ptr = curp->parent->sub_region;
      else
         ptr = curp;
      retval = NULL;
 //     if (ptr != NULL)
 // 		Spew(DSRC_UI_Traversal, ("ptr = %s\n",GD_NAME(ptr)));
 //     else
 // 		Spew(DSRC_UI_Traversal, ("ptr = NULL!\n"));
      while (ptr != curp)
      {
         retval = ptr;
         ptr = ptr->next_region;
//      if (ptr != NULL)
//          Spew(DSRC_UI_Traversal, ("ptr = %s\n",GD_NAME(ptr)));
//      else
//          Spew(DSRC_UI_Traversal, ("ptr = NULL!\n"));
      }
   }
/*
   // Spew(DSRC_UI_Traversal, ("order = %d  ",order));
   if (!curp)
   {
      // Spew(DSRC_UI_Traversal, ("curp = NULL "));
   }
   else
   {
      // Spew(DSRC_UI_Traversal, ("curp = %s ",GD_NAME(curp)));
   }
   if (!retval)
   {
      // Spew(DSRC_UI_Traversal, ("next = NULL \n"));
   }
   else
   {
      // Spew(DSRC_UI_Traversal, ("next = %s \n",GD_NAME(retval)));
   } 
   // Spew(DSRC_UI_Traversal, ("!!\n"));
*/
   return(retval);
}
                                                                                                                     
/* INTERNAL FUNCTIONS */

/* Call appropriate callbacks and automanaging functions for
   slapping a region down onto momma region.  Assumes the DB
   contains the new location for the thing. */

errtype region_place(LGRegion *reg)
{
   // NOTE:  Assumes that the DB already has the correct values about you...


#ifdef UI_LINKED
   /* If appropriate, do automanaging things */
   if (reg->status_flags & AUTOMANAGE_FLAG)
   {
      region_manage_place(reg);
   }
#endif // UI_LINKED

   /* Dispatch appropriate callbacks */
   if (reg->parent != NULL)
      region_propagate_callback(reg, SAVEUNDER_CB, reg->r);

   region_expose(reg, reg->r);
   return(OK);
}

errtype region_remove(LGRegion *reg, bool draw)
{
   // This is currently a very stupid algorithm with lots of flicker and wasted effort
   // Needs to be made better!

#ifdef UI_LINKED
   /* If appropriate, do automanaging things */
   if (reg->status_flags & AUTOMANAGE_FLAG)
   {
      region_manage_remove(reg);
   }
#endif // UI_LINKED
   
   if (reg->parent != NULL)
      region_propagate_callback(reg, REPLACE_CB, reg->r);
   region_set_moving(reg,1);
   // Spew(DSRC_UI_Callbacks, ("Removing %s\n",GD_NAME(reg)));
   if (draw && (reg->parent != NULL))
   {
      // First off, if we have a parent, expose that area of parent to fill
      // in gap we leave behind.
      // Spew(DSRC_UI_Callbacks, ("parent exposure of %s, (%d,%d)(%d,%d)!\n",GD_NAME(reg),RECT_EXPAND_ARGS(reg->r)));
      region_expose(reg, reg->r);
   }
   region_set_moving(reg,0);
   return(OK);
}

void region_moverect(LGRegion *reg, int delta_x, int delta_y, int move_rel)
{
   LGRegion *curp;

   // Spew(DSRC_UI_Region_Manipulation, ("starting region moverect for (%d, %d)(%d, %d)\n",RECT_EXPAND_ARGS(reg->r)));
   /* Move us */
   if (move_rel)
   {
      (reg->r)->ul.x += delta_x;
      (reg->r)->ul.y += delta_y;
      (reg->r)->lr.x += delta_x;
      (reg->r)->lr.y += delta_y;
   }
   reg->abs_x += delta_x;
   reg->abs_y += delta_y;
/*   if (reg->parent != NULL)
   {
      reg->abs_x += (reg->parent)->abs_x;
      reg->abs_y += (reg->parent)->abs_y;
   } */

   /* Since our kids are conveniently in OUR frame of reference, 
      we only need to update their absolute coords */

   /* For our kids */
   curp = reg->sub_region;
   while (curp != NULL)
   {
      curp->abs_x += delta_x;
      curp->abs_y += delta_y;
      curp = curp->next_region;
   } 

   // Spew(DSRC_UI_Region_Manipulation, ("ending region moverect for (%d, %d) - (%d, %d)\n",
   //   RECT_PRINT_ARGS(reg->r)));
}

void region_propagate_callback(LGRegion *reg,  ulong callback_code, LGRect *arg_rect)
{
   LGRegion *curp;
   RectCallback fn;
   LGRect new_rect,abs_rect;
   extern errtype uiHideMouse(LGRect* r), uiShowMouse(LGRect* r);

   switch(callback_code)
   {
      case SAVEUNDER_CB:
         fn = reg->save_under;
         break;
      case REPLACE_CB:
         fn = reg->replace;
         break;
   }
   if ((reg->status_flags & callback_code) && (fn != NULL) && !(reg->moving))
   {
      // Spew(DSRC_UI_Callbacks, ("CB %d sent , arg (%d,%d)-(%d,%d)\n",callback_code,
      //    RECT_PRINT_ARGS(arg_rect)));
      abs_rect.ul.x = reg->abs_x;
      abs_rect.ul.y = reg->abs_y;
      abs_rect.lr.x = reg->abs_x + RectWidth(reg->r);
      abs_rect.lr.y = reg->abs_y + RectHeight(reg->r);
      uiHideMouse(&abs_rect);
      if (!fn(reg, arg_rect))
      {
         region_convert_tochild(reg, arg_rect, &new_rect);
         curp = reg->sub_region;
         while (curp != NULL)
         {
            region_propagate_callback(curp, callback_code, &new_rect);
            curp = curp->next_region;
         }
      }
      uiShowMouse(&abs_rect);
   }
}

bool reg_exp_CB(LGRegion *reg, LGRect *rc, void *data)
{
   bool *dbp;
   dbp = (bool *)data;
   if ((!(*dbp)) && !(current_expose_region->moving))
   {
      if (reg == current_expose_region)
         *dbp = TRUE;
      else
      {
         // Spew(DSRC_UI_Callbacks, ("Did not display region %s (vs. %s) (%d,%d)(%d,%d)\n",GD_NAME(reg),GD_NAME(current_expose_region),
         //   RECT_EXPAND_ARGS(rc)));
         return(FALSE);
      }
   }

   if ((reg->status_flags & EXPOSE_CB) && (reg->expose != NULL) && (!reg->moving))
   {
      // Spew(DSRC_UI_Callbacks, ("Expose CB sent! (%d,%d)(%d,%d)\n", RECT_EXPAND_ARGS(rc)));
      reg->expose(reg, rc);
   }
   return (FALSE);
}

errtype region_expose_absolute(LGRegion *reg, LGRect *newr)
{
   LGRect absr, dummy_rect;
   bool draw_beneath = TRUE;
   LGRegion *par;

   absr.ul.x = reg->abs_x; absr.ul.y = reg->abs_y;
   absr.lr.x = absr.ul.x + RectWidth(reg->r);
   absr.lr.y = absr.ul.y + RectHeight(reg->r);
   region_convert_to_root(reg, &par, newr, &dummy_rect);

   // Spew(DSRC_UI_Callbacks, ("Expose of %s (%d,%d)(%d,%d) -- newr = (%d,%d)(%d,%d)\n",GD_NAME(reg),
   //    RECT_EXPAND_ARGS(&absr), RECT_EXPAND_ARGS(newr)));
   if (RECT_ENCLOSES(&absr, newr))
      draw_beneath = FALSE;
   current_expose_region = reg;
   uiHideMouse(&absr);
   region_traverse_rect(par, newr, &reg_exp_CB, BOTTOM_TO_TOP, &draw_beneath);
   uiShowMouse(&absr);

   return (OK);
}

errtype region_expose(LGRegion *reg, LGRect *exp_rect)
{
   LGRegion *par;
   LGRect newr;
   region_convert_to_root(reg, &par, exp_rect, &newr);
   if (region_in_sequence)
   {
      region_add_sequence_expose(reg, newr);
      return(OK);
   }
   return(region_expose_absolute(reg, &newr));
}

errtype region_set_moving(LGRegion *reg, int val)
{
   LGRegion *curp;

   reg->moving = val;
   curp = reg->sub_region;
   while (curp != NULL)
   {
      region_set_moving(curp, val);
      curp = curp->next_region;
   }
   return(OK);
}

// Converts a rectangle from from_reg's frame of reference to that of one of it's children

int region_convert_tochild(LGRegion *from_reg, LGRect *orig, LGRect *conv)
{
   LGPoint delta_pt;
   int retval = 1;

   *conv = *orig;

   delta_pt.x = (from_reg->r)->ul.x * -1;
   delta_pt.y = (from_reg->r)->ul.y * -1;
   RectMove(conv,delta_pt);
   // Spew(DSRC_UI_Conversion, ("c_conv = (%d, %d) - (%d, %d)\n", RECT_PRINT_ARGS(conv)));
   return(retval);
}

// Converts a rectangle from from_reg's frame of reference to that of it's parent

int region_convert_toparent(LGRegion *from_reg, LGRect *orig, LGRect *conv)
{
   LGPoint delta_pt;
   int retval = 1;

   *conv = *orig;

   delta_pt.x = (from_reg->parent->r)->ul.x;
   delta_pt.y = (from_reg->parent->r)->ul.y;
   RectMove(conv,delta_pt);
   // Spew(DSRC_UI_Conversion, ("p_conv = (%d, %d) - (%d, %d)\n", RECT_PRINT_ARGS(conv)));
   return(retval);
}

// Converts a rectangle within a region to the absolute coords for that region, not relative
errtype region_abs_rect(LGRegion *reg, LGRect *orig_rect, LGRect *conv)
{
   conv->ul.x = orig_rect->ul.x;
   conv->ul.y = orig_rect->ul.y;
   conv->lr.x = orig_rect->lr.x;
   conv->lr.y = orig_rect->lr.y;
   if (reg->parent != NULL)
   {
      conv->ul.x += reg->parent->abs_x;
      conv->ul.y += reg->parent->abs_y;
      conv->lr.x += reg->parent->abs_x;
      conv->lr.y += reg->parent->abs_y;
   }
   return((errtype)OK);
}

bool is_child(LGRegion *poss_parent, LGRegion *child)
{
   bool retval = FALSE;
   LGRegion *curp;

   if (child == poss_parent)
   {
      return(TRUE);
   }
   curp = poss_parent->sub_region;
   while (curp != NULL)
   {
      retval = is_child(curp, child);
      if (retval)
      {
         return(TRUE);
      }
      curp = curp->next_region;
   }
   return(FALSE);
}

bool ignore_children;

bool region_obscured_callback(LGRegion *reg, LGRect *r, void *data)
{
   int *ival;
   LGRect ar1, ar2;
   if (!region_found)
   {
      if (reg == obsc_region)
         region_found = TRUE;
      return(FALSE);
   }
   if (ignore_children)
   {
      if (is_child(obsc_region,reg))
      {
         return(FALSE);
      }
   }
   if (reg->moving)
      return(FALSE);
   ival = (int *)data;
   region_abs_rect(reg, r, &ar1);
   region_abs_rect(obsc_region, obsc_region->r, &ar2);
   // Spew(DSRC_UI_Utilities, ("Obscured callback on %s obsc_region->r = (%d,%d)(%d,%d) r = (%d,%d)(%d,%d)\n",GD_NAME(reg),
   //   RECT_EXPAND_ARGS(&ar2), RECT_EXPAND_ARGS(&ar1)));
   if (*ival != COMPLETELY_OBSCURED)
   {
      if (RECT_ENCLOSES(&ar1, &ar2))
         *ival = COMPLETELY_OBSCURED;
      else
         *ival = PARTIALLY_OBSCURED;
   }
   // Spew(DSRC_UI_Utilities, ("*ival = %d\n",*ival));
   return(FALSE);
}

int region_obscured(LGRegion *reg, LGRect *obs_rect)
{
   int retval = UNOBSCURED;
   LGRect newr;
   LGRegion *rr;

   obsc_region = reg;
   region_found = FALSE;
   region_convert_to_root(reg, &rr, obs_rect, &newr);
   ignore_children = FALSE;
   if (reg != NULL)
      region_traverse_rect(rr, &newr, &region_obscured_callback, BOTTOM_TO_TOP, &retval);
   return(retval);
}

int foreign_region_obscured(LGRegion *reg, LGRect *obs_rect)
{
   int retval = UNOBSCURED;
   LGRect newr;
   LGRegion *rr;
     
   obsc_region = reg;
   region_found = FALSE;
   region_convert_to_root(reg, &rr, obs_rect, &newr);
   ignore_children = TRUE;
   if (reg != NULL)
      region_traverse_rect(rr, &newr, &region_obscured_callback, BOTTOM_TO_TOP, &retval);
   return(retval);
}

errtype region_begin_sequence()
{
   region_in_sequence += 1;
   // Spew(DSRC_UI_Utilities, ("Beginning sequence...\n"));
   return(OK);
}

#define RSE_POOL_SIZE   40
Region_Sequence_Element rse_pool[RSE_POOL_SIZE];

Region_Sequence_Element *get_rse_from_pool()
{
   int i = 0;
   while ((i < RSE_POOL_SIZE) && (rse_pool[i].reg != NULL))
      i++;
   if (i < RSE_POOL_SIZE)
      return(&rse_pool[i]);
   return(NULL);
}

errtype return_rse_to_pool(Region_Sequence_Element *rse)
{
   int i;
   for (i=0; i < RSE_POOL_SIZE; i++)
   {
      if (&rse_pool[i] == rse)
      {
         rse_pool[i].reg = NULL;
         return(OK);
      }
   }
   return(ERR_NOEFFECT);
}

errtype init_rse_pool()
{
   int i;
   for (i=0; i < RSE_POOL_SIZE; i++)
      rse_pool[i].reg = NULL;
   return(OK);
}

errtype region_end_sequence(bool replay)
{
   Region_Sequence_Element *pnode,*pnode_prior,*pnode_next;

   // Spew(DSRC_UI_Utilities, ("Ending sequence...\n"));

      region_in_sequence -= 1;
      if (!region_in_sequence)
      {
         pnode = (Region_Sequence_Element *)slist_head(&sequence_header);
         pnode_prior = (Region_Sequence_Element *)(&sequence_header);
         while (pnode != NULL)
       	{
         	pnode_next = (Region_Sequence_Element *)slist_next(pnode);
            if (replay)
               region_expose_absolute(pnode->reg, &(pnode->exp_rect));
            slist_remove(pnode, pnode_prior);
            return_rse_to_pool(pnode);
//         	pnode_prior = pnode;	// unless deleted, then keep same
         	pnode = pnode_next;
        	}
      }
   return(OK);
}

errtype region_add_sequence_expose(LGRegion *reg, LGRect exp_rect)
{
   Region_Sequence_Element *rse, *pnode;
   bool add_flag = TRUE;
   rse = get_rse_from_pool();
   if (rse == NULL)
   {
      Warning(("No available RSE's in pool!\n"));
      return(ERR_NOMEM);
   }
   rse->reg = reg;
   rse->exp_rect = exp_rect;
   forallinslist(Region_Sequence_Element, &sequence_header, pnode)
   {
//      if (add_flag && (pnode->reg == reg) && PointsEqual(pnode->exp_rect.ul,exp_rect.ul)
//         && PointsEqual(pnode->exp_rect.lr, exp_rect.lr))

        if (add_flag && (pnode->reg == reg) && RECT_ENCLOSES(&(exp_rect),&(pnode->exp_rect)))
            add_flag = FALSE;
   }
   if (add_flag)
   {
      // Spew(DSRC_UI_Utilities, ("Adding RSE for %s (%d,%d)\n",GD_NAME(reg),RECT_EXPAND_ARGS(&exp_rect)));
      slist_add_head(&sequence_header, rse);
   }
   else
   {
      return_rse_to_pool(rse);
      // Spew(DSRC_UI_Utilities, ("RSE not added -- duplicate!\n"));
   }
   return(OK);
}

errtype region_convert_to_root(LGRegion *reg, LGRegion **root_reg, LGRect *rect, LGRect *conv)
{
   LGRect oldr,newr;

   oldr = *rect;
   if (reg->parent != NULL)
      region_convert_toparent(reg, &oldr, &newr);
   else
      newr = oldr;
   *root_reg = reg;
   while ((*root_reg)->parent != NULL)
   {
      region_convert_toparent(*root_reg, &oldr, &newr);
      *root_reg = (*root_reg)->parent;
      oldr = newr;
   }
   *conv = newr;
   return(OK);
}

#ifdef UI_LINKED
errtype region_manage_place(LGRegion *reg)
{
   LGRegion *dummy;
   dummy = reg;

   return(OK);
}

errtype region_manage_remove(LGRegion *reg)
{
   LGRegion *dummy;
   dummy = reg;

   return(OK);
}

errtype region_set_invisible(LGRegion* reg, bool invis)
{
   if (invis)
      reg->status_flags |= INVISIBLE_FLAG;
   else
      reg->status_flags &= ~INVISIBLE_FLAG;
   return OK;
}

errtype region_get_invisible(LGRegion* reg, bool* invis)
{
   *invis = (reg->status_flags & INVISIBLE_FLAG) != 0;
   return OK;
}

#endif // UI_LINKED
                                                    
