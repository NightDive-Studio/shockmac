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
#ifndef __CURSORS_H
#define __CURSORS_H

/*
 * $Source: r:/prj/lib/src/ui/RCS/cursors.h $
 * $Revision: 1.10 $
 * $Author: mahk $
 * $Date: 1994/12/02 07:13:30 $
 *
 * $Log: cursors.h $
 * Revision 1.10  1994/12/02  07:13:30  mahk
 * c++ compatibility
 * 
 * Revision 1.9  1994/11/18  12:13:30  mahk
 * uiUpdateScreenSize or whatever.
 * 
 * Revision 1.8  1994/08/24  08:55:55  mahk
 * Cursor stacks and invisible regions.
 * 
 * Revision 1.7  1994/04/05  00:19:32  mahk
 * Added hflipped cursors.  Wacky. 
 * 
 * Revision 1.6  1993/10/11  20:27:12  dc
 * Angle is fun, fun fun fun
 * 
 * Revision 1.5  1993/05/26  16:33:39  mahk
 * Added REAL mouse tolerance.
 * 
 * Revision 1.4  1993/04/28  14:40:14  mahk
 * Preparing for second exodus
 * 
 * Revision 1.3  1993/04/27  16:36:44  xemu
 * rip out base
 * 
 * Revision 1.2  1993/04/05  23:42:58  mahk
 * Added mouse hide/show and slab support.
 * 
 * Revision 1.1  1993/03/31  23:22:11  mahk
 * Initial revision
 * 
 *
 */

// Includes
#include "lg.h"  // every file should have this
#include "2d.h"
//#include <base.h> 
#include "rect.h"
#include "region.h" 
#include "mouse.h" 
#include "error.h"

// Defines


typedef void (*CursorDrawFunc)(int cmd, LGRegion* r, struct _cursor* c, LGPoint pos);
//  A cursor drawfunc executes the command specified by cmd to draw and undraw cursor
//  c at point pos.  R is the region in which c was found. 

//  The commands are as follows:
#define CURSOR_DRAW        0
#define CURSOR_UNDRAW      1
#define CURSOR_DRAW_HFLIP  2 // draw horizontally flipped.  Go figure. 

typedef struct _cursor
{
  CursorDrawFunc func;
  void* state;
  LGPoint hotspot;
  short w,h;
} LGCursor;

// Every region has a cursor stack.  
typedef struct _cursorstack
{
   int size;
   int fullness;
   LGCursor** stack;
} cursor_stack;


typedef cursor_stack uiCursorStack; 



// Prototypes
errtype uiMakeBitmapCursor(LGCursor* c, grs_bitmap* bm, LGPoint hotspot);
// Initializez *c to a bitmap cursor whose bitmap is bm, with the specified hotspot

errtype uiSetRegionDefaultCursor(LGRegion* r, LGCursor* c);
// Sets the default cursor to be used when the cursor is in region r and no cursor has been
// pushed to r's cursor stack

errtype uiPushRegionCursor(LGRegion* r, LGCursor* c);
// Pushes c to r's regional cursor stack.  When the mouse is in region r,
// the top of r's cursor stack will be displayed.  

errtype uiPopRegionCursor(LGRegion* r);
// Pops the top cursor off of r's cursor stack

errtype uiGetRegionCursor(LGRegion* r, LGCursor** c);
// Gets the current cursor for region r.  *c will be NULL if 
// there is no default cursor for r, and no cursors on the 
// r's cursor stack.

errtype uiShutdownRegionCursors(LGRegion* r);
// Deletes the cursor stack and default cursor for region r.

errtype uiSetGlobalDefaultCursor(LGCursor* c);
// Sets the default cursor for the currently active slab.

errtype uiPushGlobalCursor(LGCursor* c);
// Pushes a cursor to the active slab's global cursor stack.  

errtype uiPopGlobalCursor(void);
// Pops the top cursor off of the active slab's global cursor stack. 

errtype uiGetGlobalCursor(LGCursor** c);
// Gets the cursor on top of the active slab's global cursor stack,
// or the global default cursor if the stack is empty.

errtype uiSetSlabDefaultCursor(struct _ui_slab* slab, LGCursor* c);
// Sets the default cursor for the specified slab.

errtype uiPushSlabCursor(struct _ui_slab* slab, LGCursor* c);
// Pushes a cursor to the specified slab's global cursor stack.  

errtype uiPopSlabCursor(struct _ui_slab* slab);
// Pops the top cursor off of the specified slab's global cursor stack. 

errtype uiGetSlabCursor(struct _ui_slab* slab, LGCursor** c);
// Gets the cursor on top of the specified slab's global cursor stack,
// or the global default cursor if the stack is empty.

errtype uiHideMouse(LGRect* r);
// Hides the mouse if it intersects r.

errtype uiShowMouse(LGRect* r);
// Shows the mouse if it intersects r.  

errtype uiSetCursor(void);
// Recomputes and redraws the current cursor based on the position of the
// mouse. 

// ------------------
// SCREEN_MODE_UPDATE
// ------------------

#define UI_DETECT_SCREEN_SIZE (MakePoint(-1,-1))


// resizes the ui coordinate space.
// if UI_DETECT_SCREEN_SIZE, detect the screen size
extern errtype uiUpdateScreenSize(LGPoint size);


// Globals

extern int CursorMoveTolerance;
// Number of pixels of movement the interrupt handler will "tolerate" before
// redrawing the cursor. 





// ----------------------
// CURSOR-STACK-BASED API 
// 

/*
   Routines for manipulating cursor stacks.  This is kind of an
   afterthought, but it's a good one, and in the future, when we have
   light without heat and travel to the stars, all good ui clients will use it.
   
   The idea here is to *expose* the notion of cursor stack to the client, so it can do
   clever things like have regions/slabs share cursor stacks.  In addition to this, we
   implement the push-one and pop-every operations, which will only operate on cursor stacks
   so that the API does not explode with element of { slab, region, stack} x { once, always} etc.

*/

extern errtype uiMakeCursorStack(uiCursorStack* cs);
// initializes cs to an empty cursor stack  

extern errtype uiDestroyCursorStack(uiCursorStack* cs);
// destroys a cursor stack.

extern errtype uiGetRegionCursorStack(LGRegion* reg, uiCursorStack** cs);
// points *cs to reg's cursor stack.  If reg has no cursor stack, 
// creates one.  

extern errtype uiSetRegionCursorStack(LGRegion* reg, uiCursorStack* cs);
// Sets reg's cursor stack to cs. 

extern errtype uiGetSlabCursorStack(struct _ui_slab* slab, uiCursorStack** cs);
// points *cs to slab's cursor stack.  If slab has no cursor stack, sets *cs to NULL
// and returns ERR_NULL;

// note that there is not currently a uiSetSlabCursorStack

extern errtype uiSetDefaultCursor(uiCursorStack* cs, LGCursor* c);
// sets cs' default cursor to c.  if cs is the cursor stack of a slab, 
// c will become the default global cursor when that slab is the current slab.  If
// cs is the cursor stack of a region, c will become the default cursor for the region. 

extern errtype uiGetDefaultCursor(uiCursorStack* cs, LGCursor** c);
// sets *c to the default cursor for cs, or NULL if there is none.

extern errtype uiPushCursor(uiCursorStack* cs, LGCursor* c);
// pushes cursor c onto cursor stack cs

extern errtype uiPopCursor(uiCursorStack* cs);
// pops the top cursor off of cs. 

extern errtype uiGetTopCursor(uiCursorStack* cs, LGCursor** c);
// Points *c to the top cursor on cs; or NULL if there is no 
// top cursor.  will NOT set *c to the default cursor.

extern errtype uiPushCursorOnce(uiCursorStack* cs, LGCursor* c);
// pushes cursor c to the top of cs ONLY IF c is not already 
// on the top of cs.  

extern errtype uiPopCursorEvery(uiCursorStack* cs, LGCursor* c);
// deletes every instance of c from cs.

#endif // __CURSORS_H
                                                                                                                                               
