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
 * $Source: r:/prj/lib/src/ui/RCS/gadgets.c $
 * $Revision: 1.44 $
 * $Author: kaboom $
 * $Date: 1994/08/10 18:43:13 $
 */

// Source code to implement all these goofy gadgets
// using the region system.

#include <string.h>
#include <stdio.h>

#include "lg.h"
#include "gadgets.h"
#include "event.h"
#include "mouse.h"
#include "2d.h"
#include "dbg.h"
//#include <_ui.h>
#include "rect.h"
#include "error.h"
#include "kbcook.h"
#include "2dres.h"
//#include <lgsprntf.h>


//---------------------------------------------------------------
// Defines and Globals
//---------------------------------------------------------------
#define NUM_RESOURCE_FILES 1

bool gadget_initialization  = FALSE;
bool initialize_2d = FALSE;


//---------------------------------------------------------------
// Internal Prototypes
//---------------------------------------------------------------
errtype draw_resource_bm(Ref id, int x, int y);
errtype gad_Mac_init(Gadget *g, LGPoint extent);
bool gad_Mac_blank_expose(LGRegion *reg, LGRect *r);
errtype gadget_initialize_system(void);
bool gadget_frob_canvas(LGRegion *reg, void *data);
bool gadget_tng_mouse_move_handler(uiEvent *e, LGRegion *r, void *state);


//---------------------------------------------------------------
// Device-Specific Stuff
//---------------------------------------------------------------
// For Mac version: Set up graphics for Mac screen.

/*
errtype gad_vga_init(Gadget *g, LGPoint extent)
{
//   grs_screen *screen;

   // Spew(DSRC_UI_Gadget, ("GAD_VGA_INIT!! %s, (%d, %d)\n",GD_NAME(g->rep), extent.x, extent.y));
   if (!initialize_2d)
   {
#ifdef WOW_WHAT_A_TOTAL_HACK
      gr_init();
      gr_set_mode(GRM_320X200X8, TRUE);
      screen = gr_alloc_screen(extent.x,extent.y);
      gr_set_screen(screen);
#endif
      initialize_2d = TRUE;
   }
   g->device_data = (grs_canvas *)Malloc(sizeof(grs_canvas));
   gr_init_sub_canvas(dr_scr_canv, g->device_data, 0, 0, extent.x, extent.y);
   return (OK);
}

errtype gad_modex_init(Gadget *g, LGPoint extent)
{
   grs_screen *screen;

   // Spew(DSRC_UI_Gadget, ("GAD_MODEX_INIT!! %s, (%d, %d)\n",GD_NAME(g->rep), extent.x, extent.y));
   if (!initialize_2d)
   {
      gr_init();
      gr_set_mode(GRM_320X200X8, TRUE);
      screen = gr_alloc_screen(extent.x,extent.y);
      gr_set_screen(screen);
      initialize_2d = TRUE;
   }

   g->device_data = (grs_canvas *)Malloc(sizeof(grs_canvas));
   gr_init_sub_canvas(dr_scr_canv, g->device_data, 0, 0, extent.x, extent.y);
   return (OK);
}

errtype gad_svga_init(Gadget *g, LGPoint extent)
{
   grs_screen *screen;

   // Spew(DSRC_UI_Gadget, ("GAD_VGA_INIT!! %s, (%d, %d)\n",GD_NAME(g->rep), extent.x, extent.y));
   if (!initialize_2d)
   {
      gr_init();
      gr_set_mode(GRM_320X200X8, TRUE);
      screen = gr_alloc_screen(extent.x,extent.y);
      gr_set_screen(screen);
      initialize_2d = TRUE;
   }

   g->device_data = (grs_canvas *)Malloc(sizeof(grs_canvas));
   gr_init_sub_canvas(dr_scr_canv, g->device_data, 0, 0, extent.x, extent.y);
   return (OK);
}

errtype gad_mono_init(Gadget *g, LGPoint extent)
{
   Gadget *dummy;
   LGPoint dummy2;
   dummy = g; dummy2 = extent;

   return (OK);
}
*/

errtype gad_Mac_init(Gadget *g, LGPoint extent)
{
	grs_screen *screen;
	
	if (!initialize_2d)
	{
		gr_init();
		gr_set_mode(GRM_640x480x8, TRUE);
		screen = gr_alloc_screen(extent.x,extent.y);
		gr_set_screen(screen);
		initialize_2d = TRUE;
	}
	
	g->device_data = (grs_canvas *)NewPtr(sizeof(grs_canvas));
	gr_init_sub_canvas(dr_scr_canv, (grs_canvas *)g->device_data, 0, 0, extent.x, extent.y);
	return (OK);
}

/*
bool gad_vga_blank_expose(LGRegion *reg, LGRect *r)
{
   LGRect nrect;

   region_abs_rect(reg, r, &nrect);
   SCALE_RECT(&nrect, GD_GADG(reg)->conversion);
   // Spew(DSRC_UI_Gadget, ("GAD_VGA_BLANK_EXPOSE!! %s, (%d, %d)(%d, %d)\n",GD_NAME(reg), RECT_EXPAND_ARGS(&nrect)));
   if (GD_CANV(reg) == NULL)
      // Spew(DSRC_UI_Gadget, ("Perhaps we have a problem here...\n"));
   gr_set_canvas(GD_CANV(reg));
   gr_set_fcolor(0x00);
   gr_rect(RECT_EXPAND_ARGS(&nrect));
//   Spew(DSRC_UI_Gadget, ("after the 2d part of that...\n"));
   return(0);
}

bool gad_mono_blank_expose(LGRegion *reg, LGRect *r)
{
   LGRegion *dummy;
   LGRect *dummy2;
   dummy = reg; dummy2 = r;

   return (0);
}
*/

bool gad_Mac_blank_expose(LGRegion *reg, LGRect *r)
{
	LGRect nrect;
	
	region_abs_rect(reg, r, &nrect);
	SCALE_RECT(&nrect, GD_GADG(reg)->conversion);
	gr_set_canvas((grs_canvas *)GD_CANV(reg));
	gr_set_fcolor(0x00);
	gr_rect(RECT_EXPAND_ARGS(&nrect));
	return(0);
}


errtype draw_resource_bm(Ref id, int x, int y)
{
   FrameDesc *f;
   int a1,a2,a3,a4;
   short *ppall;

   STORE_CLIP(a1,a2,a3,a4);
//   Spew(DSRC_UI_Utilities, ("cliprect = (%d, %d)(%d, %d)\n",a1,a2,a3,a4));
   // Spew(DSRC_UI_Utilities, ("drawing bitmap to (%d, %d)\n",x,y));
   f = (FrameDesc *)RefLock(id);

   // Set the palette right
   /*еее Ignore for now
   if (f->pallOff)
   {
      ppall = (short *) (((uchar *) RefGet(id)) + f->pallOff);
      // Spew(DSRC_UI_Utilities, ("ppall = %d   *ppall + 1 = %d\n",*ppall, *(ppall + 1)));
      gr_set_pal(*ppall,*(ppall + 1),(uchar *)(ppall + 2));
   }
   */
   f->bm.bits = (uchar *)(f+1);
   gr_bitmap(&f->bm, x, y);
   RefUnlock(id);
   return (OK);
}  

int resource_bm_width(Ref id)
{
   FrameDesc *f;
   int n;

   f = (FrameDesc *)RefLock(id);
   n = f->bm.w;
   // Spew(DSRC_UI_Utilities, ("resource_bm_width = %d\n",n));
   RefUnlock(id);
   return (n);
}  

int resource_bm_height(Ref id)
{
   FrameDesc *f;
   int n;

   f = (FrameDesc *)RefLock(id);
   n = f->bm.h;
   // Spew(DSRC_UI_Utilities, ("resource_bm_height = %d\n",n));
   RefUnlock(id);
   return (n);
}  

// Initialize the overall gadget system.  Should be called once, when the first gadget_init is
// triggered
errtype gadget_initialize_system(void)
{
#ifdef INIT_RESOURCES_HERE
   ResInit();
   ResAddPath(RESPATH);
   for (i = 0; i < NUM_RESOURCE_FILES; i++)
      ResOpenFile(resource_files[i]);
#endif
   return(OK);
}

// Initialize a gadget system for a particular display type.  Returns the gadget that is the "root" gadget
// for that display.
Gadget *gadget_init(int display_type, LGPoint extent)
{
   Gadget *retgad;
   GadgetData *gd;
   RectCallback fn;
   LGRect r;
   
//	if ((extent.x <= 0) || (extent.y <= 0))
//		Spew(DSRC_UI_Bounds, ("Nonpositive extent in gadget_init!\n"));
   if (!gadget_initialization)
   {
      gadget_initialization = TRUE;
      gadget_initialize_system();
   }

   // Make a rectangle out of the extent
   r.ul.x = 0;  r.ul.y = 0;
   r.lr = extent;

   // Make the basic gadget
   retgad = (Gadget *)NewPtr(sizeof(Gadget));
   retgad->gclass = CLASS_ROOT;
   retgad->tng_data = NULL;
   retgad->draw_parts = TNG_ALLPARTS;
   retgad->handler_id = -1;
   retgad->destroy_func = NULL;
   retgad->conversion.x = extent.x / BASELINE_X;
   retgad->conversion.y = extent.y / BASELINE_Y;

   // Fill out the user data structure
   gd = (GadgetData *)NewPtr(sizeof(GadgetData));
   gd->g = retgad;
   gd->name  = (char *)NewPtr(8 * sizeof(char));  
//   lg_sprintf(gd->name, "root%d\0", display_type);
   sprintf(gd->name, "root%d\0", display_type);

   // Set the right kind o' callback
   switch (display_type)
   {
/*    case DISPLAY_VGA:
      case DISPLAY_SVGA:
      case DISPLAY_MODEX:
         fn = &gad_vga_blank_expose;
         break;
      case DISPLAY_MONO:
         fn = &gad_mono_blank_expose;
         break; */
	  case DISPLAY_MAC:
	  	 fn = &gad_Mac_blank_expose;
	  	 break;
   }

   // Create that durned rep

   retgad->rep = (LGRegion *)NewPtr(sizeof(LGRegion));
   region_create(NULL, retgad->rep, &r, 0, 0, REG_AUTOMATIC, NULL, NULL, NULL, gd);
   retgad->rep->expose = fn;
   retgad->rep->device_type = display_type;

   switch (display_type)
   {
/*    case DISPLAY_VGA:
         gad_vga_init(retgad, extent);
         break;
      case DISPLAY_MODEX:
         gad_modex_init(retgad, extent);
         break;
      case DISPLAY_SVGA:
         gad_svga_init(retgad, extent);
         break;
      case DISPLAY_MONO:
         gad_mono_init(retgad, extent);
         break; */
	  case DISPLAY_MAC:
	  	 gad_Mac_init(retgad, extent);
	  	 break;
   }

   return(retgad);
}

// Forces a gadget to display itself
errtype gadget_display(Gadget *g, LGRect *r)
{
   return(gadget_display_part(g,r,TNG_ALLPARTS));
}

errtype gadget_display_part(Gadget *g, LGRect *r, ushort partmask)
{
   g->draw_parts = partmask;
   if (r == NULL)
      region_expose(g->rep, g->rep->r);
   else
      region_expose(g->rep, r);
   return(OK);
}

errtype gadget_shutdown()
{
   gr_close();
   return(OK);
}


// Destroys the gadget and any children gadgets
bool in_destroy = FALSE;
errtype gadget_destroy(Gadget **pvic)
{
   LGRegion *curp, *nextp;
   Gadget *victim;
   bool inner_destroy;

   victim = *pvic;

   inner_destroy = in_destroy;
   if (!inner_destroy)
      in_destroy = TRUE;
/*
   if (victim->rep != NULL)
   {
       Spew(DSRC_UI_Gadget, ("Destroying %s!\n",GD_NAME(victim->rep)));
      if (victim->rep->user_data == NULL)
         Warning(("Attempting to destroy gadget with no user_data!\n"));
   }
   else
       Spew(DSRC_UI_Gadget, ("rep is NULL!\n"));
*/
   if (victim == NULL)
   {
      // Spew(DSRC_UI_Bounds, ("Attempted to destroy null gadget!\n"));
      return(ERR_NULL);
   }

   if (victim->rep != NULL)
   {
      curp = victim->rep->sub_region;
      while (curp != NULL)
      {
         nextp = curp->next_region;

         // Is this thing a gadget, or a region hung off of us?
         if (curp->user_data != NULL)
            gadget_destroy(&(GD_GADG(curp)));
         else
            region_destroy(curp, FALSE);
         curp = nextp;
      }
      region_destroy(victim->rep, !inner_destroy);
   }

   if (victim->destroy_func != NULL)
      victim->destroy_func(victim, NULL);
   DisposePtr((Ptr)*pvic);
   *pvic = NULL;

   if (!inner_destroy)
   {
      in_destroy = FALSE;
   }
   return (OK);
}

// Moves the gadget to a new x, y, z coordinate (relative)
bool gadget_frob_canvas(LGRegion *reg, void *data)
{
   LGPoint delta;
   Gadget *g;
   // Spew(DSRC_UI_Gadget, ("Tippie-top of frob!\n"));
   delta = *((LGPoint *)(data));
   g = GD_GADG(reg);
   // Spew(DSRC_UI_Gadget, ("Frobbing canvas on %s:  Delta = (%d,%d)\n",GD_NAME(g->rep),delta.x,delta.y));
   gr_init_sub_canvas((grs_canvas *)g->parent->device_data, (grs_canvas *)g->device_data,
      g->rep->r->ul.x + delta.x, g->rep->r->ul.y + delta.y,
      RectWidth(g->rep->r), RectHeight(g->rep->r));
   ((LGPoint *)(data))->x = 0;
   ((LGPoint *)(data))->y = 0;
   return(FALSE);
}

errtype gadget_move(Gadget *g, LGPoint coord, int z)
{
   LGPoint delta;

   if (g == NULL)
   {
      // Spew(DSRC_UI_Bounds, ("Attempted to move null gadget!\n"));
      return(ERR_NULL);
   }
   if ((coord.x < 0) || (coord.y < 0))
   {
      // Spew(DSRC_UI_Bounds, ("Attempted to move gadget to negative point!\n"));
      return(ERR_RANGE);
   }
   delta.x = coord.x - g->rep->r->ul.x;
   delta.y = coord.y - g->rep->r->ul.y;
//   region_traverse(g->rep, &gadget_frob_canvas, BOTTOM_TO_TOP, &delta);
   region_move(g->rep, coord.x, coord.y, z);
   return (OK);
}

errtype gadget_resize(Gadget* g, int xsize, int ysize)
{
   if (g == NULL)
   {
      // Spew(DSRC_UI_Bounds, ("Attempted to resize null gadget!\n"));
      return(ERR_NULL);
   }
   if ((xsize < 0) || (ysize < 0))
   {
      // Spew(DSRC_UI_Bounds, ("Attempted to give region negative size\n"));
      return(ERR_RANGE);
   }
   region_resize(g->rep, xsize, ysize);
   return (OK);
}


errtype gad_callback_install(Gadget *g, ushort event_type, ushort condition, TNGCallback tngcb, void *user_data, int *id)
{
   return(tng_install_callback(g->tng_data, event_type, condition, tngcb, user_data, id));
}

errtype gad_callback_uninstall(Gadget *g, int id)
{
   return(tng_uninstall_callback(g->tng_data,id));
}

/*
bool gadget_tng_mono_expose(LGRegion *reg, LGRect *r)
{
   LGRegion *dummy;
   LGRect *dummy2;
   dummy = reg;
   dummy2 = r;

   return (FALSE);
}

bool gadget_tng_vga_expose(LGRegion *reg, LGRect *r)
{
   ushort partmask;
   LGRect nrect;
   int c1,c2,c3,c4;
   LGPoint loc;
   Gadget *g;

   g = GD_GADG(reg);
   partmask = g->draw_parts;
   if (GD_CANV(reg) != NULL)
   {
      gr_set_canvas(GD_CANV(reg));
      region_abs_rect(reg, r, &nrect);
      // Spew(DSRC_UI_Gadget, ("About to draw gadget %s\n",GD_NAME(reg)));
      // Spew(DSRC_UI_Gadget, ("r = (%d,%d)(%d,%d)\n",RECT_EXPAND_ARGS(r)));
      // Spew(DSRC_UI_Gadget, ("nrect = (%d,%d)(%d,%d)\n",RECT_EXPAND_ARGS(&nrect)));
      STORE_CLIP(c1,c2,c3,c4);
      gr_set_cliprect(nrect.ul.x, nrect.ul.y, nrect.lr.x , nrect.lr.y);
      loc.x = g->rep->abs_x;  loc.y = g->rep->abs_y;
      // Spew(DSRC_UI_Gadget, ("abs_x = %d  abs_y = %d\n",loc.x, loc.y));
      g->tng_data->draw_func(g->tng_data, partmask, loc);
      RESTORE_CLIP(c1,c2,c3,c4);
   }
   else
      // Spew(DSRC_UI_Gadget, ("Got a NULL canvas here, buddy...region %s\n",GD_NAME(reg)));
   return(OK);
}
*/
bool gadget_tng_Mac_expose(LGRegion *reg, LGRect *r)
{
	ushort	partmask;
	LGRect	nrect;
	int 		c1,c2,c3,c4;
	LGPoint	loc;
	Gadget	*g;
	
	g = GD_GADG(reg);
	partmask = g->draw_parts;
	if (GD_CANV(reg) != NULL)
	{
		gr_set_canvas((grs_canvas *)GD_CANV(reg));
		region_abs_rect(reg, r, &nrect);
		STORE_CLIP(c1,c2,c3,c4);
		gr_set_cliprect(nrect.ul.x, nrect.ul.y, nrect.lr.x , nrect.lr.y);
		loc.x = g->rep->abs_x;  loc.y = g->rep->abs_y;
		g->tng_data->draw_func(g->tng_data, partmask, loc);
		RESTORE_CLIP(c1,c2,c3,c4);
	}
	return(OK);
}

bool gadget_tng_mouse_handler(uiEvent *e, LGRegion *r, void *state)
{
   Gadget *g;
   uiMouseEvent *mickey;
   LGPoint rel;
   void *dummy;
   dummy = state;

   mickey = (uiMouseEvent *)e;
   g = GD_GADG(r);
   rel.x = mickey->pos.x - r->abs_x;
   rel.y = mickey->pos.y - r->abs_y;
   return(g->tng_data->mousebutt(g->tng_data, mickey->action, rel));
}

bool gadget_tng_mouse_move_handler(uiEvent *e, LGRegion *r, void *state)
{
   Gadget *g;
   uiMouseEvent *mickey;
   LGPoint rel;
   void *dummy;
   dummy = state;

   mickey = (uiMouseEvent *)e;
   g = GD_GADG(r);
   rel.x = mickey->pos.x - r->abs_x;
   rel.y = mickey->pos.y - r->abs_y;
   return(g->tng_data->mousemove(g->tng_data, rel));
}

bool gadget_tng_keyboard_handler(uiEvent *e, LGRegion *r, void *state)
{
   Gadget *g;
   uiCookedKeyEvent *cke;
   void *dummy;
   dummy = state;
   
   if ((e->type != UI_EVENT_KBD_COOKED) || !(e->subtype & KB_FLAG_DOWN))
      return FALSE;
   cke = (uiCookedKeyEvent *)e;
   g = GD_GADG(r);
   return(g->tng_data->keycooked(g->tng_data, cke->code));
}

errtype gadget_create_setup(Gadget **pg, Gadget *parent, GadgetClass cl, LGRect *dim, int z, char *name)
{
   Gadget *retgad;
   GadgetData *gd;
   TNG *pb_tng;
   RectCallback fn;

   if ((RectWidth(dim) < 1) || (RectHeight(dim) < 1))
   {
      // Spew(DSRC_UI_Bounds, ("Nonpositive dimension of created gadget!\n"));
      return(NULL);
   }

   // Do those yummy mallocs
   retgad = (Gadget *)NewPtr(sizeof(Gadget));
   gd = (GadgetData *)NewPtr(sizeof(GadgetData));
   pb_tng = (TNG *)NewPtr(sizeof(TNG));

   switch (parent->rep->device_type)
   {
/*  case DISPLAY_VGA:
      case DISPLAY_SVGA:
      case DISPLAY_MODEX:
         fn = &gadget_tng_vga_expose;
         break;
      case DISPLAY_MONO:
         fn = &gadget_tng_mono_expose;
         break; */
      case DISPLAY_MAC:
         fn = &gadget_tng_Mac_expose;
         break;
   }

   // Fill out basic gadgety info
   retgad->tng_data = pb_tng;
   retgad->draw_parts = TNG_ALLPARTS;
   retgad->gclass = cl;
   retgad->parent = parent;
//   retgad->device_data = gr_alloc_sub_canvas(parent->device_data, dim->ul.x, dim->ul.y, RectWidth(dim), RectHeight(dim));
   retgad->device_data = parent->device_data;
   retgad->conversion = parent->conversion;
   retgad->handler_id = -1;
   retgad->destroy_func = NULL;
  
   // Fill in the gadget data info
   gd->name = (char *)NewPtr((strlen(name) + 1) * sizeof(char));
   strcpy(gd->name, name);
   gd->g = retgad;

   // Create that durned rep
   retgad->rep = (LGRegion *)NewPtr(sizeof(LGRegion));
   region_create(parent->rep, retgad->rep, dim, z, 0,
      REG_USER_CONTROLLED | AUTOMANAGE_FLAG | STENCIL_CLIPPING | OBSCURATION_CHECK,
      fn, NULL, NULL, gd);

   // Install the general pushbutton handler
   uiInstallRegionHandler(retgad->rep, UI_EVENT_MOUSE_MOVE, &gadget_tng_mouse_move_handler, retgad, &(retgad->handler_id));
   uiInstallRegionHandler(retgad->rep, UI_EVENT_MOUSE, &gadget_tng_mouse_handler, retgad, &(retgad->handler_id));
   uiInstallRegionHandler(retgad->rep, UI_EVENT_KBD_COOKED, &gadget_tng_keyboard_handler, retgad, &(retgad->handler_id));

   *pg = retgad;
   return(OK);
}

errtype gadget_change_flags(Gadget *g, ulong flags, bool on, bool children)
{
   LGRegion *cur_child;
   if (on)
      g->rep->status_flags |= flags;
   else
      g->rep->status_flags &= ~flags;
   if (children)
   {
      cur_child = g->rep->sub_region;
      while (cur_child != NULL)
      {
         gadget_change_flags(GD_GADG(cur_child), flags, on, children);
         cur_child = cur_child->next_region;
      }
   }
   return(OK);
}
