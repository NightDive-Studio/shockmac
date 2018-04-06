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
#ifndef __GADGETS_H
#define __GADGETS_H

// Includes
// C Library Includes

// System Library Includes
#include "region.h"
#include "event.h"
#include "array.h"

// Master Game Includes

// Game Library Includes

// Game Object Includes

typedef enum {
   CLASS_ROOT,
   CLASS_PUSHBUTTON,
   CLASS_SLIDER,
   CLASS_BUTTONARRAY,
   CLASS_TEXT,
   CLASS_QUICKBOX,
   CLASS_PLAIN,
   CLASS_MENU,
   NUM_GAD_CLASSES
} GadgetClass;

// Your basic gadget.  Most of the real work info is in whatever
// class structure is pointed to be the class_data pointer.

typedef struct _TNG TNG;

typedef struct _Gadget{
   LGRegion *rep;
   struct _Gadget *parent;
   GadgetClass gclass;
   TNG *tng_data;
   void *device_data;
   LGPoint conversion;
   int handler_id;
   int draw_parts;
   int (*destroy_func)(struct _Gadget *g, void *user_data);
} Gadget;

#include "tng.h"

// A GadgetData is installed as the userdata for a region to get
// a pointer back to the gadget, as well as other useful info
typedef struct {
   char *name;
   Gadget *g;
} GadgetData;

typedef struct {
   LGPoint delta;
   Gadget *g;
} MoveStruct;

// Prototypes

// Initialize a gadget system for a particular display type.  Returns the gadget that is the "root" gadget
// for that display.
Gadget *gadget_init(int display_type, LGPoint extent);

// Forces a gadget to display itself across a rectangle.
// If r is NULL, entire gadget is displayed. 
// Coordinates for r are in g's coordinates.
errtype gadget_display(Gadget *g, LGRect *r);
errtype gadget_display_part(Gadget *g, LGRect *r, ushort partmask);

// Destroys the gadget and any children gadgets
errtype gadget_destroy(Gadget **pvic);

// Moves the gadget to a new x, y, z coordinate (relative)
errtype gadget_move(Gadget *g, LGPoint coord, int z);

// Resizes a gadget 
errtype gadget_resize(Gadget* g, int xsize, int ysize);

// Installs a callback onto a Gadget, triggered by a particular kind of pushbutton input.
// The callback will be called with the gadget and the user_data as arguments, assuming the input
// is allowed by the input_mask.
errtype gad_callback_install(Gadget *g, ushort event_type, ushort condition, TNGCallback tngcb, void *user_data, int *id);

// Removes the callback installed on gadgets g identified by identifier id
errtype gad_callback_uninstall(Gadget *g, int id);

// Shut down the gadget system
errtype gadget_shutdown(void);

// Draw the bitmap resource named by id, at location (x,y)
errtype draw_resource_bm(Ref id, int x, int y);

int resource_bm_width(Ref id);
int resource_bm_height(Ref id);

bool gadget_tng_vga_expose(LGRegion *reg, LGRect *r);
bool gadget_tng_mono_expose(LGRegion *reg, LGRect *r);
bool gadget_tng_Mac_expose(LGRegion *reg, LGRect *r);
bool gadget_tng_mouse_handler(uiEvent *e, LGRegion *r, void *state);
bool gadget_tng_keyboard_handler(uiEvent *e, LGRegion *r, void *state);
errtype gadget_tng_vga_expose_part(LGRegion *reg, LGRect *r, ushort partmask);
errtype gadget_create_setup(Gadget **pg, Gadget *parent, GadgetClass cl, LGRect *dim, int z, char *name);
errtype gadget_change_flags(Gadget *g, ulong flags, bool on, bool children);

// Defines
#define NULL_TYPE       -1
#define RESOURCE_TYPE    0
#define TEXT_TYPE        1
#define TRANSPARENT_TYPE 2
#define COLORED_TYPE     3
#define CALLBACK_TYPE    4

#define DISPLAY_VGA     0
#define DISPLAY_MONO    1   
#define DISPLAY_MODEX   2
#define DISPLAY_SVGA    3
#define DISPLAY_MAC 	4

#define BASELINE_X      640
#define BASELINE_Y      480

// These macros all take in a region and derive from it information about gadgets
#define GD_CANV(x) ((GadgetData *)((x)->user_data))->g->device_data
#define GD_NAME(x) ((GadgetData *)((x)->user_data))->name
#define GD_GADG(x) ((GadgetData *)((x)->user_data))->g

#define STORE_CLIP(a,b,c,d) a = gr_get_clip_l(); \
   b = gr_get_clip_t();  c = gr_get_clip_r(); d = gr_get_clip_b()

#define RESTORE_CLIP(a,b,c,d) gr_set_cliprect(a,b,c,d)

// Globals

#endif // __GADGETS_H

