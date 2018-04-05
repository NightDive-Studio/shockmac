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
 * $Source: r:/prj/lib/src/palette/RCS/palette.c $
 * $Revision: 1.34 $
 * $Author: minman $
 * $Date: 1994/08/15 04:49:03 $
 *
 *
 */

#include "palette.h"

/*
 * GLOBAL VARIABLES
 *
 * Palette_Effects_Table holds entries for different types of
 *   palette effects.  Its size is user defined when the initializing
 *   routine is called.
 *
 * Shadow_Fixed_Cmap is a shadow colormap in fixed point, used to keep a
 *   tighter approximation in palette shifts.  (Integer divides would make
 *   successive steps jerky).  Delta_Cmap keeps fixed point track
 *   of the delta which should be added successively to shifting entries.
 *
 * Timestamps per step, also user-defined at initialization time,
 *   defines how how many timestamp increments should pass before a
 *   palette change is required.
 */

PAL_TABLE_ENTRY *Palette_Effects_Table = NULL;
short Palette_Effects_Table_Size = 0;

fix   Shadow_Fixed_Cmap[768];    // r,g,b for each entry in colormap 
fix   Delta_Cmap[768];           // r,g,b delta shifts for same
uchar local_smap[768];

short timestamps_per_step;       // How many ts units = 1 palette step
long  last_timestamp = 0L;
short num_active_effects = 0;

byte num_installed_shifts = 0;

#ifndef REAL_PAL_SWAP_SHAD
// should really do a warning here for so people to understand a problem, she has happened
#define palette_swap_shadow(s,n,d)
#endif

/*
 * ROUTINES
 */

/*
 * ADVANCE ROUTINES
 *
 * The first routine advances a palette effect by the appropriate
 *   approximate delta, or increments a delay field for the entry.
 *   This routine is responsible for figuring out whether change
 *   should occur, and if so, how much change.  It also executes
 *   that change.
 *
 * Note that for palette shifts, this routine will also notice if
 *   the effect is completed and will remove the effect from the
 *   effect table at that time.
 *
 * The second routine is the master routine, which should be called
 *   in every pass of the calling application's main loop.  It
 *   figures out which effects are active, if any, and advances
 *   them as neccessary.  The application programmer should never
 *   have to explicitly call the former advance routine: he will
 *   only screw up this library's internal bookkeeping.  To
 *   advance an effect, install it and let the master "Advance All"
 *   routine take care of the execution.
 */ 

void palette_advance_all_fx(long timestamp)
{
   static int	ts_remainder = 0;   
   int         	i, time_diff;
   short		c1, c2, t;
   int         	steps_to_do;
   div_t 		result;
   
   // Figure out how many steps of work need to be done
   //   while updating timestamp info

   if (timestamps_per_step <= 0) return; // dont do a div by 0
   
   time_diff = (int) (timestamp - last_timestamp);
   last_timestamp = timestamp;

   result = div(time_diff, timestamps_per_step);
   steps_to_do  = result.quot;
   ts_remainder += result.rem;

   if (ts_remainder > timestamps_per_step)
   {
      ts_remainder -= timestamps_per_step;
      steps_to_do++;
   }
   
   // Inform all active palette effects in the table how many
   //   steps of work they need to do.

   if (steps_to_do == 0) return;
   
  	gr_get_pal(0, 256, local_smap);
	c1 = 255;
	c2 = 0;
	for (i = 0; i < Palette_Effects_Table_Size; i++)
	{
		if (Palette_Effects_Table[i].status == ACTIVE)
		{
			t = Palette_Effects_Table[i].entry_1;
			if (t < c1) c1 = t;
			t = Palette_Effects_Table[i].entry_n;
			if (t > c2) c2 = t;

			palette_advance_effect(i, steps_to_do);
		}
	}
	if (Palette_Effects_Table[0].effect == CBANK)
		gr_set_pal((int)c1, (int)(c2 - c1 +1), &local_smap[c1*3]);
}

uchar c_off_stack[3];

void palette_advance_effect(byte id, int steps)
{
   short e1, en, er;
   short i, j;
   fix x;
   div_t dv;
   uchar *t, *v;
   short m, n;
//   uchar c[3];
   int add_to_color, add_to_delay, a;
   short s1, sn, sr;

   if (Palette_Effects_Table[id].mode == STEADY) steps = 1;
   
   e1 = Palette_Effects_Table[id].entry_1;
   en = Palette_Effects_Table[id].entry_n;
   er = Palette_Effects_Table[id].range;
   t  = Palette_Effects_Table[id].from_pal;  // "Colors" array for cycle
   v  = Palette_Effects_Table[id].to_pal;
  
   // We need to figure out how many color steps to advance,
   //   and how much delay to increment.  So, divide "steps to do"
   //   by delay + 1, which is a complete cycle of effect operation.
   // The quotient will tell us how many colors to increment,
   //   the modulus how many delay elements to add.
      
   dv = div(steps, (int) (Palette_Effects_Table[id].dsteps + 1));
    
   add_to_color = dv.quot;
   add_to_delay = dv.rem;

   // Then, if adding to delay puts us through to another color,
   //   update accordingly.
      
   Palette_Effects_Table[id].curr_dstep += add_to_delay;
   if (Palette_Effects_Table[id].curr_dstep >
       Palette_Effects_Table[id].dsteps)
   {
      Palette_Effects_Table[id].curr_dstep -=
         Palette_Effects_Table[id].dsteps;
   
      add_to_color++;
   }
   
   if (Palette_Effects_Table[id].effect != SHIFT) add_to_color %= er;
   else if ((Palette_Effects_Table[id].curr_stage + add_to_color) > 256)
      add_to_color %= 256;
      
   // If we've advanced exactly 0 or 1 times the entire range
   //   of colors, we don't really need to do anything.

   if (add_to_color == 0) return;
   
   switch(Palette_Effects_Table[id].effect) {

      case SHIFT:

         // If we're about to first start the shift, and the source
         // palette segment is not the exact same as the real colormap
         // segment corresponding, then go immediately to that
         // source colormap and load it into the real colormap.

         if ((Palette_Effects_Table[id].curr_stage == -1) &&
             (&grd_pal[e1*3] != &t[e1*3])) {
            gr_set_pal((int)e1, (int)er, &t[e1*3]);
            Palette_Effects_Table[id].curr_stage = 0;
         }
         
         // If adding the # of steps to do to our current number of
         // steps pushes us up to or past the total number of steps
         // to do, then jump to the final palette and remove the
         // entry.
         
         Palette_Effects_Table[id].curr_stage += add_to_color;

         if (Palette_Effects_Table[id].curr_stage >=
             Palette_Effects_Table[id].stages)
         {
            gr_set_pal((int) e1, (int) er, &v[e1*3]);
            palette_remove_effect(id);
         }

         // Otherwise, we jump our palette shift ahead by "steps"
         // steps, taking care to update both the real colormap
         // and our shadow fixed point map.

         else {

            x = fix_make(add_to_color, 0);

  		  gr_get_pal(0, 256, local_smap);

            for (i = e1; i <= en; i++)
            {
               j = i - e1;

               Shadow_Fixed_Cmap[3*i]   += fix_mul(x,Delta_Cmap[3*j]);
               Shadow_Fixed_Cmap[3*i+1] += fix_mul(x,Delta_Cmap[3*j+1]);
               Shadow_Fixed_Cmap[3*i+2] += fix_mul(x,Delta_Cmap[3*j+2]);

               local_smap[i*3] = (uchar) fix_rint(Shadow_Fixed_Cmap[3*i]);
               local_smap[i*3+1] = (uchar) fix_rint(Shadow_Fixed_Cmap[3*i+1]);
               local_smap[i*3+2] = (uchar) fix_rint(Shadow_Fixed_Cmap[3*i+2]);
            }
         }

         gr_set_pal(0, 256, local_smap);
         
         break;

      case CYCLE:

         // If the data segments here dont make sense, realize that
         //   for cycles, the "entry_n" portion is the current color,
         //   and "range" is the cycle size.
         // We're seeing if we've gone past the end of the colors array.

         Palette_Effects_Table[id].entry_n += add_to_color;
         if (Palette_Effects_Table[id].entry_n > er)
            Palette_Effects_Table[id].entry_n -= er;

         m  = Palette_Effects_Table[id].entry_n;   // "Curr_color" for cycle
         n  = e1;                                  // "Cmap_Index" for cycle

         // Update colormap and shadow fixed map 

         if (m < 0) break;            // Rare initial case w/fast frame rate
         
         gr_set_pal((int) n, 1, &t[3*m]);

         Shadow_Fixed_Cmap[3*n]   = fix_make(t[3*m],0);
         Shadow_Fixed_Cmap[3*n+1] = fix_make(t[3*m+1],0);
         Shadow_Fixed_Cmap[3*n+2] = fix_make(t[3*m+2],0);
         
         break;

      case CBANK:

         a = add_to_color;

         // Shift the colormap by using a uchar *shadowmap:
         //   Write from a->end of colormap to start of shadowmap,
         //   then finish shadowmap with 0->a.  Then copy the whole
         //   thing back.

         gr_get_pal((int) e1+a, (int) er-a, &local_smap[e1*3]);
         gr_get_pal((int) e1, (int) a, &local_smap[(e1+er-a)*3]);

// KLC         gr_set_pal((int) e1, (int) er, &local_smap[e1*3]);
// Now does it once after checking all CBANK effects.

         // NOT ONLY THE COLORMAP must be swapped: the delta and
         //   Shadow-fixed arrays must be swapped, 3 elements at
         //   a time.  But this is only if there is a SHIFT
         //   effect active.  If 1 SHIFT is active, we check for
         //   overlap.  If more are active, we just swap the whole
         //   cycling bank's palette segment worth.  (huh?)

         if (num_installed_shifts == 0) break;
         else if (num_installed_shifts != 1) palette_swap_shadow((int)e1, (int)er, (int)a);

         else {
            
            for (i = 0; i < Palette_Effects_Table_Size; i++)
               if (Palette_Effects_Table[i].effect == SHIFT) break;

            s1 = Palette_Effects_Table[i].entry_1;
            sn = Palette_Effects_Table[i].entry_n;
            sr = sn - s1 + 1;

            if ((s1 >= e1) && (sn <= en)) palette_swap_shadow((int)s1, (int)sr, (int)a);
            else if (s1 > e1) palette_swap_shadow((int)s1, (int)(en - s1 + 1), (int)a);
            else if (sn < en) palette_swap_shadow((int)e1, (int)(sn - e1 + 1), (int)a);

            else palette_swap_shadow((int)e1, (int)er, (int)a);
         }

         break;
   }

   return;
}

/*
 * INSTALL and REMOVE ROUTINES
 *
 * These either remove a palette change (easy) or install
 * a new one and return an id handle (slightly more difficult).
 *
 * Remove routine returns ERR_NOEFFECT if the entry was already
 * empty, OK otherwise.  Install routine returns -1 if there
 * were no available entries or if they could not install the
 * palette change, and return an id handle otherwise.
 *
 * Also, there's not very much sanity checking.  Install assumes
 * legitimate arguments passed to it (as do most of the other routines
 * in this library)
 */

byte palette_install_effect(PAL_TYPE type, PAL_MODE mode,
                            short b1, short b2, short b3, short b4,
                            uchar *ptr1, uchar *ptr2)
{
   byte i;

   // Find an available slot for installation?

   for (i = 0; i < Palette_Effects_Table_Size; i++)
      if (Palette_Effects_Table[i].status == EMPTY) break;

   if (i == Palette_Effects_Table_Size) return -1;   // didn't find anything

   // We're psyched.  Let's install the standard stuff 1st...

   Palette_Effects_Table[i].status	= ACTIVE;
   Palette_Effects_Table[i].effect	= type;
   Palette_Effects_Table[i].mode	= mode;
   
   // ...and then, the type-specific stuff

   switch(type) {
      
      case SHIFT:
                                          
         Palette_Effects_Table[i].entry_1		= b1;
         Palette_Effects_Table[i].entry_n		= b2;
         Palette_Effects_Table[i].range			= b2 - b1 + 1;
         Palette_Effects_Table[i].from_pal	= ptr1;
         Palette_Effects_Table[i].to_pal		= ptr2;
         Palette_Effects_Table[i].dsteps			= b3;
         Palette_Effects_Table[i].stages			= b4;
         Palette_Effects_Table[i].curr_dstep	= 0;
         Palette_Effects_Table[i].curr_stage	= -1;
         
         palette_init_smap(b1, b2, ptr1, ptr2, b4); // reserve smap + deltas
         
         num_installed_shifts++;
         
         break;

      case CYCLE:
 
         Palette_Effects_Table[i].entry_1		= b1;
         Palette_Effects_Table[i].range			= b2;
         Palette_Effects_Table[i].from_pal	= ptr1;
         Palette_Effects_Table[i].entry_n		= -1;  // Haven't begun yet
         Palette_Effects_Table[i].dsteps			= b3;
         Palette_Effects_Table[i].curr_dstep	= 0;   // Not currently delayed
         
         break;

      case CBANK:

         Palette_Effects_Table[i].entry_1		= b1;
         Palette_Effects_Table[i].entry_n		= b2;
         Palette_Effects_Table[i].dsteps			= b3;
         Palette_Effects_Table[i].range			= b2 - b1 + 1;
         Palette_Effects_Table[i].curr_dstep	= 0;   // Not currently delayed
                       
         break;

   }
   
   // If this is the only active effect currently installed, lets
   //   reset the timestamp and count from now

   num_active_effects++;  // Announce our presence, return handle

   return i;
   
}

errtype palette_remove_effect(byte id)
{
   if (Palette_Effects_Table[id].status == EMPTY) return ERR_NOEFFECT;

   Palette_Effects_Table[id].status = EMPTY;

   num_active_effects--;
   
   if (Palette_Effects_Table[id].effect == SHIFT) num_installed_shifts--;
   
   return OK;
}

/*
 * FREEZE and UNFREEZE ROUTINES
 *
 * These change the status of a table entry to "frozen".
 * The return code is ERR_RANGE if the entry was empty,
 * ERR_NOEFFECT if the entry was already frozen, and OK
 * if the table entry freeze went alright.
 *
 * Similarly for the UNFREEZE routines, except they also set the
 * timestamp to the time of unfreezing.
 */

errtype palette_freeze_effect(byte id)
{
   if (Palette_Effects_Table[id].status == EMPTY)  return ERR_RANGE;
   if (Palette_Effects_Table[id].status == FROZEN) return ERR_NOEFFECT;
   Palette_Effects_Table[id].status = FROZEN;
   
   num_active_effects--;
   
   return OK;
}

errtype palette_unfreeze_effect(byte id)
{
   if (Palette_Effects_Table[id].status == EMPTY)  return ERR_RANGE;
   if (Palette_Effects_Table[id].status != FROZEN) return ERR_NOEFFECT;

   Palette_Effects_Table[id].status = ACTIVE;
   
   num_active_effects++;
   
   return OK;
}

/*
 * QUERY and CHANGE_DELAY
 *
 * The former lets you query the status of an effect, the latter
 * lets you change its delay (but only for cycling effects!)
 */

PAL_STATUS palette_query_effect(byte id)
{
   return Palette_Effects_Table[id].status;
}

void palette_change_delay(byte id, short delay)
{
   if (Palette_Effects_Table[id].status != EMPTY)
      Palette_Effects_Table[id].dsteps = delay;

   return;
}

/* Palette INITIALIZE and SHUTDOWN routines
 *
 * DESCRIPTION: (1) Initializes the data structures internal to the palette
 * library.  It also takes an argument specifying how many timestamp increments
 * equal one step.  Passing values <= 0 can be hazardous to your health.
 *
 * DESCRIPTION: (2) Shutdown frees malloc'd memory.
 */

void palette_set_rate(short ts)
{
   timestamps_per_step = ts;
}

void palette_initialize(short tbl_size)
{
   int i;

   Palette_Effects_Table_Size = tbl_size;
   
   // Malloc the table

   Palette_Effects_Table = (PAL_TABLE_ENTRY *)
      NewPtr((int) tbl_size * sizeof(PAL_TABLE_ENTRY));
//¥¥¥No error check here
  
   // Initialize Table
   
   for (i = 0; i < Palette_Effects_Table_Size; i++)
      Palette_Effects_Table[i].status = EMPTY;

   palette_set_rate(1);
   
   return;
}
                     
/*
 * Palette_shutdown()
 *
 * Call this at the end of your program to free up the effects table.
 */

void palette_shutdown()
{
   DisposePtr((Ptr)Palette_Effects_Table);

   return;
}

/*
 * SHADOWMAP and DELTA ARRAY routines
 *
 * The initializer reserves a specified portion of the shadow map for
 * a single palette shift effect, and sets its entries to fixed point
 * mockups of the "from" colormap.  It also calculates what the delta
 * array for that portion will be, based on the source and destination
 * palettes, and the number of steps.
 *
 * Woe to the programmer who overlaps palette segments with multiple
 * shift effects!
 */

void palette_init_smap(short first, short last, uchar *from, uchar *to,
                       short num_steps)
{
   int i, j;
   fix x0, x1, x2, y;
   
   for (i = first; i <= last; i++) {

         j = i - first;
      
         Shadow_Fixed_Cmap[3*i]   = fix_make(from[3*j],0);
         Shadow_Fixed_Cmap[3*i+1] = fix_make(from[3*j+1],0);
         Shadow_Fixed_Cmap[3*i+2] = fix_make(from[3*j+2],0);

         // Get fixed point difference for r,g,b between src/dest palettes
         
         x0 = fix_make(to[3*j],0)   - Shadow_Fixed_Cmap[3*i];
         x1 = fix_make(to[3*j+1],0) - Shadow_Fixed_Cmap[3*i+1];
         x2 = fix_make(to[3*j+2],0) - Shadow_Fixed_Cmap[3*i+2];

         // Now divide the r,g,b diffs by #steps to figure out fixed deltas
         
         y = fix_make(num_steps,0);
         
         Delta_Cmap[3*i]   = fix_div(x0, y);
         Delta_Cmap[3*i+1] = fix_div(x1, y);
         Delta_Cmap[3*i+2] = fix_div(x2, y);
   }      
}

/*
 * Palette_Swap_shadow()
 *
 * Does a cycle bank on the shadow map and the delta map.
 */

#ifdef REAL_PAL_SWAP_SHAD
void palette_swap_shadow(int s, int n, int d)
{
   // used to be static, too big, what to do, what to do.... what to do...
   fix Shadow_smap[768];
   fix Shadow_dmap[768];
   int i;
   
   // Copy the originals to the shadow maps

   for (i = 3*s; i < (s+d)*3; i++) {
      Shadow_smap[i] = Shadow_Fixed_Cmap[i];
      Shadow_dmap[i] = Delta_Cmap[i];
   }

   for (i = 3*s; i < (s+n-d)*3; i++) {
      Shadow_Fixed_Cmap[i] = Shadow_Fixed_Cmap[(d*3)+i];
      Delta_Cmap[i] = Delta_Cmap[(d*3)+i];
   }

   for (i = (s+n-d)*3; i < (s+n)*3; i++) {
      Shadow_Fixed_Cmap[i] = Shadow_smap[i-((n-d)*3)];
      Delta_Cmap[i] = Shadow_dmap[i-((n-d)*3)];
   }
   
   return;
}
#endif
