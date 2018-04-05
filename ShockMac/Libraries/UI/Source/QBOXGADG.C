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
// Source code for Quickbox Gadgets

// THINGS STILL NEEDED:
// Concept of alignment
// menu-oid slots

//#include <fcntl.h>
//#include <io.h>

#include <string.h>

#include "region.h"
#include "gadgets.h"
#include "qboxgadg.h"
#include "tngqbox.h"
#include "region.h"
#include "2d.h"
//#include <_ui.h>
#include "event.h"
#include "mouse.h"
#include "kbcook.h"


RectCallback disp_fn;
GadgetData *qb_gd;
int current_z;
Gadget *current_box;
LGRect box_dim;

LGPoint QboxDefaultSpacing = {2, 0}; 
LGPoint QboxDefaultBorder = {1, 1};

// Prototypes
int gad_qb_destroy(Gadget *g, void *user_data);
bool gad_qbox_close_func(void *vg, void *ud);


int gad_qb_destroy(Gadget *g, void *user_data)
{
#ifndef NO_DUMMIES
   void *dummy;
   dummy = (void *)g;
   dummy = user_data;
#endif

   if (TNG_QB(g->tng_data)->options & QB_GRABFOCUS)
      uiReleaseFocus(g->rep, UI_EVENT_KBD_COOKED);
   return(0);
}

Gadget *gad_qbox_start_full(Gadget *parent, LGPoint coord, int z, TNGStyle *sty, ushort options, char *name, LGPoint ss, LGPoint spacing,
   LGPoint border, Ref left_id, Ref right_id)
{
   Gadget *retgad;
   TNG *qb_tng;

   if (parent == NULL)
   {
      //Spew(DSRC_UI_Bounds, ("Attempted to create child of null gadget!\n"));
      return(NULL);
   }

   retgad = (Gadget *)NewPtr(sizeof(Gadget));
   qb_gd = (GadgetData *)NewPtr(sizeof(GadgetData));
   qb_tng = (TNG *)NewPtr(sizeof(TNG));

   switch (parent->rep->device_type)
   {
/*      case DISPLAY_VGA:
      case DISPLAY_SVGA:
      case DISPLAY_MODEX:
         disp_fn = &gadget_tng_vga_expose;
         break;
      case DISPLAY_MONO:
         disp_fn = &gadget_tng_mono_expose;
         break;  */
      case DISPLAY_MAC:
         disp_fn = &gadget_tng_Mac_expose;
         break;
   }

   current_z = z;
   current_box = retgad;
   box_dim.ul = coord;
   tng_quickbox_init(retgad, qb_tng, sty, options, ss, spacing, border, left_id, right_id);

   // Fill in gadget level data
   retgad->tng_data = qb_tng;
   retgad->gclass = CLASS_QUICKBOX;
   retgad->parent = parent;
   retgad->device_data = parent->device_data;
   retgad->conversion = parent->conversion;
   retgad->handler_id = -1;
//   retgad->destroy_func = NULL;
   retgad->destroy_func = &gad_qb_destroy;

   // Fill out gadget data
   qb_gd->name = (char *)NewPtr((strlen(name) + 1) * sizeof(char));
   strcpy(qb_gd->name, name);
   qb_gd->g = retgad;

   return(retgad);
}

// Begin a quick box.  Until the qbox is ended, all subsequent qbox calls will use the "current" qbox.
Gadget *gad_qbox_start(Gadget *parent, LGPoint coord, int z, TNGStyle *sty, ushort options, char *name, LGPoint ss)
{
   return(gad_qbox_start_full(parent, coord, z, sty, options, name, ss, QboxDefaultSpacing, QboxDefaultBorder,
      NULL, NULL));
}

// Add a line to a quickbox.  slot_type describes the type of slot, var is a pointer to the variable to be
// displaying, and slot_options describes any additional modifiers to the qbox.  Note that some bizarre-o 
// combinations of options and types might not be implemented.
errtype gad_qbox_add(char *label, int slot_type, void *var, ulong slot_options)
{
   return(tng_quickbox_add(label, slot_type, var, slot_options));
}

// Just like gad_qbox_add but allows two parameters to be set for the slot.  Certain slot options require
// this form of accessing.
errtype gad_qbox_add_parm(char *label, int slot_type, void *var, ulong slot_options, void *parm1, void *parm2)
{
   return(tng_quickbox_add_parm(label, slot_type, var, slot_options, parm1, parm2));
}

bool gad_qbox_close_func(void *vg, void *ud)
{
#ifndef NO_DUMMIES
   void *dummy;
   dummy = vg;
#endif

   gadget_destroy((Gadget **)ud);
   return(FALSE);
}

// This represents that the quickbox is done being created and is ready for display, input, etc.
errtype gad_qbox_end()
{
   return(gad_qbox_end_full(NULL));
}

errtype gad_qbox_end_full(Gadget **ptr)
{
   LGPoint sz;
   if ((TNG_QB(current_box->tng_data)->options & QB_ADDCLOSE) &&  (ptr != NULL))
         gad_qbox_add_parm("Close", QB_PUSHBUTTON_SLOT, gad_qbox_close_func, QB_NO_OPTION, (void *)(ptr), NULL);

   tng_quickbox_size(current_box->tng_data, &sz);
   box_dim.lr.x = box_dim.ul.x + sz.x;
   box_dim.lr.y = box_dim.ul.y + sz.y;
/* Spew(DSRC_UI_Quickbox, ("box_dim = (%d,%d)(%d,%d), current_z = %d\n",RECT_EXPAND_ARGS(&box_dim),current_z));
   if (current_box->parent == NULL)
      Spew(DSRC_UI_Quickbox, ("current_box->parent is NULL!\n"));
   else if (current_box->parent->rep == NULL)
      Spew(DSRC_UI_Quickbox, ("current_box->parent->rep is NULL!\n"));
   else
      Spew(DSRC_UI_Quickbox, ("Current_box seems okay...\n"));
*/
   current_box->rep = (LGRegion *)NewPtr(sizeof(LGRegion));
   region_create(current_box->parent->rep, current_box->rep, &box_dim, current_z, 1,
      REG_USER_CONTROLLED | AUTOMANAGE_FLAG | STENCIL_CLIPPING | OBSCURATION_CHECK,
      disp_fn, NULL, NULL, qb_gd);
   tng_quickbox_end();
   uiInstallRegionHandler(current_box->rep, UI_EVENT_MOUSE, &gadget_tng_mouse_handler, current_box, &(current_box->handler_id));
   uiInstallRegionHandler(current_box->rep, UI_EVENT_KBD_COOKED, &gadget_tng_keyboard_handler, current_box, &(current_box->handler_id));
   uiSetRegionOpacity(current_box->rep,UI_EVENT_KBD_POLL);
/*еее
   kb_set_state(QB_LEFT_KEY,KBA_REPEAT);
   kb_set_state(QB_RIGHT_KEY,KBA_REPEAT);
   kb_set_state(QB_DOWN_KEY,KBA_REPEAT);
   kb_set_state(QB_UP_KEY,KBA_REPEAT);
*/
   if (TNG_QB(current_box->tng_data)->options & QB_GRABFOCUS)
      uiGrabFocus(current_box->rep, UI_EVENT_KBD_COOKED);

   return(OK);
}

errtype gad_qbox_rename_slot(Gadget *g, int slot_num, char *new_name)
{
   return(tng_quickbox_rename_slot(g->tng_data, slot_num,new_name));
}
