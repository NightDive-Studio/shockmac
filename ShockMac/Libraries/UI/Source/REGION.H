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
// Header file for the Region library

#ifndef __REGION_H
#define __REGION_H

#include "lg.h"
#include "rect.h"
#include "error.h"

#define INVISIBLE_FLAG 0x80000000


#define EVENT_CB     0x0001UL
#define EXPOSE_CB    0x0002UL
#define SAVEUNDER_CB 0x0004UL
#define REPLACE_CB   0x0008UL

#define AUTOMANAGE_FLAG        0x0100
#define DISPLAY_ON_CREATION    0x0200
#define STENCIL_CLIPPING       0x0400
#define OBSCURATION_CHECK      0x0800
#define AUTODESTROY_FLAG       0x1000

#define UI_LINKED    1

typedef struct _Region
{                  
	int     abs_x, abs_y;			// upper left in absolute coords
	LGRect  *r;						// rectangle covered by this region, in coord frame of parent.
	int     z;						// z-coordinate to determine stacking
	int 	moving;
	bool 	(*expose)(struct _Region *reg, LGRect *r);		// function to draw a given rectangle
	bool 	(*save_under)(struct _Region *reg, LGRect *r);	// function to save under a given rectangle
	bool 	(*replace)(struct _Region *reg, LGRect *r);
	ulong  	status_flags;
	int    	device_type;
	void 	*handler;
	void 	*cursors; 
	void 	*user_data;				// user-provided region information for callback use
	int    	event_order;
	struct 	_Region *sub_region;   	// Head of children regions
	struct 	_Region *next_region;	// next region at same level
	struct 	_Region *parent; 		// parent of this region
    LGRect real_rect;
} LGRegion;

typedef bool (*RectCallback)(LGRegion *reg, LGRect *r);  // all in relative coords
typedef bool (*TravRectCallback)(LGRegion *reg, LGRect *r, void *data);
typedef bool (*TravCallback)(LGRegion *reg, void *data);


// If the callback returns non-zero, then it indicates that the callback triggering
// should not propagate further downwards.  Otherwise, the callback will go
// further, with parents getting callbacks before their children.

// Initialize the region system.  Note that this gets called automatically
// the first time you try to create a region, if you haven't done so already.
errtype region_init();

// Register a region with the UI manager, geometry described by r, and as a 
// subregion of the parent region.  When the passed
// events are not filtered out by emask, and callbacks are allowed by
// cb mask, callback will be called.  cbmask can be used to prevent
// any of expose, save_under, or event callbacks from happening.
// Returns a pointer to the newly-defined region.  Depending
// on the value of the z coordinate, the new region will be 
// "above" or "below" other overlapping regions.  Any covered region
// will be given a saveunder callback if it's mask allows.  Whether or
// not the parent handles the events before it's children is dependant
// on the event_order parameter.  The user_data parameter is simply
// stored and can be used by callbacks to determine information about
// what is contained in the region.  The status parameter determines the degree of
// control that the region system has over the newly created region, as well as
// defining the callback mask.

// Note that when creating the root region, you must set the device_type by hand -- this
// will then be inherited by each subregion as they are created.

errtype region_create(LGRegion *parent, LGRegion *ret, LGRect *r, int z, int event_order,
   ulong status, RectCallback expose,
   RectCallback save_under, RectCallback replace, void *user_data);

#define REG_USER_CONTROLLED   EVENT_CB | EXPOSE_CB | SAVEUNDER_CB | REPLACE_CB 
#define REG_NORMAL            EVENT_CB | EXPOSE_CB | SAVEUNDER_CB | REPLACE_CB | AUTOMANAGE_FLAG | DISPLAY_ON_CREATION
#define REG_AUTOMATIC         AUTOMANAGE_FLAG | EXPOSE_CB | EVENT_CB | DISPLAY_ON_CREATION
#define REG_TRANSPARENT       EVENT_CB  // Dunno whether this will actually work...
#define REG_NONEXISTANT       0x0000

// Removes a region from the system, along with all it's children.  
// Returns whether or not the operation was successful.  Any newly exposed
// areas will recieve expose callbacks if their masks allow.

errtype region_destroy(LGRegion *reg, bool draw);

// Move a region to a new set of coordinates.  Expose and saveunder
// callbacks are dished out for the original area and any newly covered
// area.  As usual, these coords are relative....

errtype region_move(LGRegion *reg, int new_x, int new_y, int new_z);

// Change the size of a region.  Appropriate callbacks are
// triggered if the regions masks allow.

errtype region_resize(LGRegion *reg, int new_x_size, int new_y_size);

errtype region_expose(LGRegion *reg, LGRect *exp_rect);

// Traverse the region stack by calling fn on every Region that is 
// intersected by the target rectangle.  The order parameter determines
// whether traversal is front to back or back to front.  If the callback
// function ever returns a non-zero value, the traversal stops.  Returns
// true if non-zero value was returned during traversal.

int region_traverse_rect(LGRegion *reg, LGRect *target, TravRectCallback fn, int order, void *data);
int region_traverse_point(LGRegion *reg, LGPoint target, TravRectCallback fn, int order, void *data);
int region_traverse(LGRegion *reg, TravCallback fn, int order, void *data);

// Converts a rectangle from a given region's coordinate system to the frame of a child of that
// coordinate system.
int region_convert_rect(LGRegion *from_reg, LGRect *conv);

// Converts a rectangle within a region to the absolute coords for that region, not relative
errtype region_abs_rect(LGRegion *reg, LGRect *orig_rect, LGRect *conv);

// Converts a rectangle within a region to it's root coordinates, as well as return a pointer
// to that root region.
errtype region_convert_to_root(LGRegion *reg, LGRegion **root_reg, LGRect *rect, LGRect *conv);

// Returns whether or not a particular rectangle within a region is obscured by anything or
// not.  The coordinates of the rectangle are local coords.  region_foreign_obscured is
// like region_obscured but ignores children for purposes of obscuration.
int region_obscured(LGRegion *reg, LGRect *obs_rect);
int foreign_region_obscured(LGRegion *reg, LGRect *obs_rect);

// These functions control whether or not the region library thinks the application is
// in the middle of a sequence which will generate multiple, probably duplicate, expose events.
// While a sequence is active, it captures all the expose events, and saves them until the
// sequence has ended, a which point it lets the exposes get through, after filtering out 
// all the duplicate exposes.
errtype region_begin_sequence();
errtype region_end_sequence(bool replay);


// An _invisible_ region is not detected through any kind of traversal, does not 
// receive mouse events and does not change the cursor.

errtype region_set_invisible(LGRegion* reg, bool invis);
// Sets whether or not a region is invisible

errtype region_get_invisible(LGRegion* reg, bool* invis);
// determines whether a region is currently invisible. 

#define UNOBSCURED            0
#define PARTIALLY_OBSCURED    1
#define COMPLETELY_OBSCURED   2

#define TOP_TO_BOTTOM   0
#define BOTTOM_TO_TOP   1

#define RECT_EXPAND_ARGS(pr) (pr)->ul.x,(pr)->ul.y,(pr)->lr.x,(pr)->lr.y
#define RECT_PRINT_ARGS(pr) RECT_EXPAND_ARGS(pr)
#define RECT_MULTIPLY(rc,factor) { (rc)->ul.x = (rc)->ul.x * (factor); \
   (rc)->ul.y = (rc)->ul.y * (factor); (rc)->lr.x = (rc)->lr.x * (factor); (rc)->lr.y = (rc)->lr.y * (factor); }
#define POINT_MULTIPLY(pt,factor) (pt).x = (pt).x * factor; (pt).y = (pt).y * (factor)
#define SCALE_RECT(rc, scale_pt) { (rc)->ul.x = (rc)->ul.x * (scale_pt).x; (rc)->lr.x = (rc)->lr.x * (scale_pt).x ;\
      (rc)->ul.y = (rc)->ul.y * (scale_pt).y; (rc)->lr.y = (rc)->lr.y * (scale_pt).y; }

#endif // __REGION_H
