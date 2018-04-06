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
 * $Source: r:/prj/cit/src/RCS/cybermfd.c $
 * $Revision: 1.5 $
 * $Author: xemu $
 * $Date: 1994/10/13 15:50:50 $
 *
 *
 */

#include "mfdint.h"
#include "mfdext.h"
#include "mfddims.h"
#include "colors.h"
#include "cit2d.h"
#include "fullscrn.h"

// Includes for example mfd.
#include <stdlib.h>

#include "gr2ss.h"


// ============================================================
//                   THE CYBERSPACE MFD
// ============================================================
void mfd_cspace_expose(MFD* mfd, ubyte control);


// -------
// DEFINES
// -------
#define GOOD_RED  (RED_BASE+5)


// ---------------
// EXPOSE FUNCTION
// ---------------

/* This gets called whenever the MFD needs to redraw or
   undraw.  
   The control value is a bitmask with the following bits:
   MFD_EXPOSE: Update the mfd, if MFD_EXPOSE_FULL is not set,
               update incrementally.  
   MFD_EXPOSE_FULL: Fully redraw the mfd, implies MFD_EXPOSE

   if no bits are set, the mfd is being "unexposed;" its display
   being pulled off the screen to make room for a different func.
*/

void mfd_cspace_expose(MFD* mfd, ubyte control)
{
   bool full = control & MFD_EXPOSE_FULL;
   if (control == 0)  // MFD is drawing stuff
   {
      // Do unexpose stuff here.  
   }
   if (control & MFD_EXPOSE) // Time to draw stuff
   {
      // clear update rects
      mfd_clear_rects();
      // set up canvas
      gr_push_canvas(pmfd_canvas);
      safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);

      // Clear the canvas by drawing the background bitmap
      if (!full_game_3d)
         ss_bitmap(&mfd_background,0,0);

      // INSERT GRAPHICS CODE HERE
      mfd_draw_string("CYBERSPACE",5,3,GOOD_RED,TRUE);
      mfd_draw_string("MFD",5,13,GOOD_RED,TRUE);
      // on a full expose, make sure to draw everything
 
      if (full)
         mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);

      // Pop the canvas
      gr_pop_canvas();
      // Now that we've popped the canvas, we can send the 
      // updated mfd to screen
      mfd_update_rects(mfd);

   }
  
}
