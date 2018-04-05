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
 * $Source: r:/prj/cit/src/RCS/status.c $
 * $Revision: 1.77 $
 * $Author: kevin $
 * $Date: 1994/11/29 11:50:25 $
 */

// Source code for handling the status indicators for the
// Citadel game screen
//
// Source is divided into two segments; first comes all the
// biorhythm stuff (upper left hand corner of the game screen)
// and second, the upper right hand healthy/suit-energy measuring
// bar graphs

#include <math.h>
 
#include "player.h"
#include "status.h"
#include "tools.h"
#include "colors.h"
#include "mainloop.h"
#include "gameloop.h"
#include "newmfd.h"
#include "gamescr.h"
#include "faketime.h"
#include "fullscrn.h"
#include "citres.h"
#include "gamesys.h"
#include "physics.h"
#include "objsim.h"
#include "biotrax.h"
#include "statics.h"
#include "otrip.h"

#include "gamesys.h"
#include "gr2ss.h"


// Look ma, a new biorhythm option.
#define BOTTOMLESS               0x04 // things that fall off the bottom don't get drawn.

//��� For now, turn these on so it won't try to draw any biorhythm stuff
#define TIMING_PROCEDURES_OFF
//#define TIMING_CALLBACK_OFF

// Struct to contain information about tail

typedef struct {
   bool     free;
   int      *data;
   uchar    height[MAX_BIO_LENGTH];
   int      head;
   bool     tail;
   uchar    tail_length;
   uchar    color_length;
   int      update_time;
   int      counter;
   int      max_value;
   uchar    special;
   bool     active; 
} bio_data_block;


bool gBioInited = FALSE;

//KLC - chg for new art   uchar status_background[(DIFF_BIO_WIDTH+4)*(DIFF_BIO_HEIGHT+2)];
uchar status_background[(266+4)*(44+2)];
uchar bio_data_buffer[NUM_BIO_TRACKS * sizeof(bio_data_block)];

#define FRAME_RATE_SCALE   20
#define LOOPLINE_SCALE  0x2F

bio_data_block    *bio_data;

int bio_time_id;

int curr_bio_x, curr_bio_y;
int curr_bio_w, curr_bio_h;
short curr_bio_mode;
Ref curr_bio_ref;

static uchar    track_colors[NUM_BIO_TRACKS]
= {
      0x66,    // turquoise
      0x57,    // yellow green?
      0x73,    // blue
      0x40,    // orange
      0x4A,    // yellow
      0x28,    // gold
      0x33,    // red
      0x20,    // purple
};

/* bio keeps private canvas so we don't have to save/restore stuff from
   the real screen canvas. */
static grs_canvas bio_canvas;


#ifdef SYNCH_BIORHYTHMS
grs_bitmap bio_bitmap;

#define BIO_BITMAP_SIZE ((GAMESCR_BIO_X + GAMESCR_BIO_WIDTH) * (GAMESCR_BIO_Y + GAMESCR_BIO_HEIGHT))
uchar bio_bitmap_bits[BIO_BITMAP_SIZE];
#endif

// Internal Prototype

bool under_bio(int x);
void ss_save_under_set_pixel(int color, short i, short j);
void bio_set_pixel(int color, short x, short y);
void bio_restore_pixel(grs_bitmap *bmp, short x, short y);
void bio_vline(int color, int x, int y, int y1);
void status_bio_update_screenmode();
bool status_track_free(int track);
bool status_track_active(int track);
void status_track_activate(int track, bool active);
void draw_lower_tracks(int track_number, int location);
void gamescr_bio_func(void);
void diff_bio_func(void);
void draw_one_location_tracks(int location);
void draw_bio_height(int track_number, int draw_location);
errtype clear_bio_tracks(void);
void clear_tail(int track_number, int delete_location);
int FIND_OVERLAP(int x, int y);

void (*bio_funcs[])(void) = {gamescr_bio_func, diff_bio_func};


bool under_bio(int x)
{
      if ( ((x >= 16) && (x <= 22)) ||
	   ((x >= 69) && (x <= 74)) ||
	   ((x >= 114) && (x <= 208)) ||
	   ((x >= 252) && (x <= 258)) ||
	   ((x >= 298) && (x <= 303)) )
	 return(TRUE);
      return(FALSE);
}

// ---------------------------------------------------------
// bio_set_pixel(int color, int x, int y)
// 
// calls gr_set_pixel as normal, unless the biorhythms is
// going "under" something

void ss_save_under_set_pixel(int color, short i, short j)
{
   extern LGPoint LastCursorPos;

   if (LastCursor!=NULL) {
      if ((LastCursorPos.x+SaveUnder.bm.w>i)&&(LastCursorPos.x<=i)) {
         if ((LastCursorPos.y+SaveUnder.bm.h>j)&&(LastCursorPos.y<=j)) {
            grs_bitmap *bm=(grs_bitmap *)(LastCursor->state);
            int k=(i-LastCursorPos.x)+SaveUnder.bm.row*(j-LastCursorPos.y);
            SaveUnder.bm.bits[k]=color;
            k=(i-LastCursorPos.x)+bm->row*(j-LastCursorPos.y);
            if (bm->bits[k]) {
               return;
            }
         }
      }
   }
   gr_set_pixel(color,i,j);
//   mprintf("x:%i, y:%i, w:%i, h:%i\n",LastCursorPos.x,LastCursorPos.y,SaveUnder.bm.w,SaveUnder.bm.h);
}

void bio_set_pixel(int color, short x, short y)
{
   short x0,x1,y0,y1,i,j;

//KLC - don't need   if ((curr_bio_mode == DIFF_BIO) && (under_bio(x)))
//      return;
//   if (convert_use_mode)
//   {
      x0 = SCONV_X(x); y0 = SCONV_Y(y);
      x1 = SCONV_X(x+1); y1 = SCONV_Y(y+1);
      for (i=x0; i < x1; i++)
         for (j=y0; j < y1; j++)
            ss_save_under_set_pixel(color,i,j);
//   }
//   else
//      ss_save_under_set_pixel(color,x,y);
}

// ---------------------------------------------------------
//  Restore the pixels from the offscreen background bitmap.
// ---------------------------------------------------------
void bio_restore_pixel(grs_bitmap *bmp, short x, short y)
{
	int		x0, y0;
	int		x1, y1;
	uchar	*pp;
	int		i, j;
	
	// Determine where x and y really are on the Mac screen.
	x0 = SCONV_X(x); y0 = SCONV_Y(y);
	x1 = SCONV_X(x+1); y1 = SCONV_Y(y+1);
	
	// OK, this sets a pointer to the background bitmap at the same location.  I just happen
	// to know that the difference between the on-screen and offscreen bitmaps is (10,2).
	pp = bmp->bits + (bmp->row * (y0-2) + (x0-10));

	// Restore the pixels directly from the offscreen background bitmap.
	for (j=y0; j < y1; j++)
	{
		for (i=x0; i < x1; i++)
			ss_save_under_set_pixel(*pp++,i,j);
		pp += bmp->row - 2;
	}
}

// ---------------------------------------------------------
// bio_vline(int x, int y, int y1)
// 
// calls bio_vline as normal, unless the biorhythms is
// going "under" something

void bio_vline(int color, int x, int y, int y1)
{
   if ((curr_bio_mode == DIFF_BIO) && (under_bio(x)))
      return;
#ifdef SVGA_SUPPORT
   if (convert_use_mode != 0)
   {
      short x0,x1,i,j;
      x0 = SCONV_X(x);
      x1 = SCONV_X(x+1);
      y = SCONV_Y(y);
      y1 = SCONV_Y(y1);
      if (y>y1) {
         int foo=y;
         y=y1;
         y1=foo;
      }
      for (i=x0; i<x1;i++)
         for (j=y; j<=y1; j++)
            ss_save_under_set_pixel(color,i,j);
   }
   else
#endif
   {
      if (y>y1) {
         int foo=y;
         y=y1;
         y1=foo;
      }
      for (;y<=y1;y++)
         ss_save_under_set_pixel(color,x,y);
   }
}

// ---------------------------------------------------------
// status_bio_set(short bio_mode)
//
// Tell the biorhythm code which screen we are on

short bios_x[2] = { GAMESCR_BIO_X,  DIFF_BIO_X };
short bios_y[2] = { GAMESCR_BIO_Y,  DIFF_BIO_Y };
short bios_w[2] = { GAMESCR_BIO_WIDTH,  DIFF_BIO_WIDTH };
short bios_h[2] = { GAMESCR_BIO_HEIGHT,  DIFF_BIO_HEIGHT };
Ref bio_refs[2] = { GAMESCR_BIO_REF, DIFF_BIO_REF };

grs_bitmap bio_background_bitmap;

void status_bio_set(short bio_mode)
{
   FrameDesc *f;
   int      i;
#ifdef SVGA_SUPPORT
   uchar old_over = gr2ss_override;
#endif

   curr_bio_mode = bio_mode;
      
   // Assuming there are only 8 bio tracks!!!!!
   /* set biorhythm positions & clipping rectangle. */
   curr_bio_x = bios_x[bio_mode];  
   curr_bio_y = bios_y[bio_mode];
   curr_bio_w = bios_w[bio_mode];  
   curr_bio_h = bios_h[bio_mode];
#ifdef SVGA_SUPPORT
   gr2ss_override = OVERRIDE_ALL;
#endif
   ss_cset_cliprect(&bio_canvas, STATUS_BIO_X, STATUS_BIO_Y,
      STATUS_BIO_X + STATUS_BIO_WIDTH, STATUS_BIO_Y + STATUS_BIO_HEIGHT);
#ifdef SVGA_SUPPORT
   gr2ss_override = old_over;
#endif

   curr_bio_ref = bio_refs[bio_mode];
   f = (FrameDesc *)RefLock(STATUS_RESID);
   bio_background_bitmap = f->bm;

   // let's try to do the right thing!
   bio_background_bitmap.bits = status_background;

   LG_memcpy(bio_background_bitmap.bits,(char *)(f+1),sizeof(char) * f->bm.w * f->bm.h);

   bio_data = (bio_data_block *) bio_data_buffer;
   for (i=0; i<NUM_BIO_TRACKS; i++)
      bio_data[i].free = TRUE;

   RefUnlock(STATUS_RESID);
   bio_funcs[curr_bio_mode]();
}

// ---------------------------------------------------------
// status_bio_init()
//
// Do the init stuff for the biorhythm. 2d should be initialized and
// screen canvas should be unchanged when this is called.

void status_bio_update_screenmode()
{
   bio_canvas=*grd_screen_canvas;      /* make copy for int routine */
}

void status_bio_init(void)
{
   status_bio_update_screenmode();

#ifndef TIMING_PROCEDURES_OFF
   bio_time_id=tm_add_process((void (*)()) status_bio_update,0,TMD_FREQ/140);
#endif
}

void status_bio_start(void)
{
   if (!full_game_3d)
      gBioInited = TRUE;
   
#ifndef TIMING_PROCEDURES_OFF
   tm_activate_process(bio_time_id);
#endif 
}

void status_bio_end(void)
{
   gBioInited = FALSE;
//   Free(bio_background_bitmap.bits);
#ifndef TIMING_PROCEDURES_OFF
   tm_deactivate_process(bio_time_id);
#endif
}

// -----------------------------------------------------------
// status_bio_draw()
//
// Draw the biorhythm background

void status_bio_draw(void)
{
   int               i;

   // Draw the background map
//KLC - chg for new art   ss_bitmap(&bio_background_bitmap, STATUS_BIO_X, STATUS_BIO_Y);
	gr_bitmap(&bio_background_bitmap, SCONV_X(STATUS_BIO_X), SCONV_Y(STATUS_BIO_Y));
	
   // Go from left to right and draw all the tracks
   for (i=0; i < STATUS_BIO_LENGTH; i++)
      draw_one_location_tracks(i);
}


// -----------------------------------------------
// Accessors for the "active" field. 

bool status_track_free(int track)
{
   return bio_data[track].free;
}

bool status_track_active(int track)
{
   return bio_data[track].active;
}

void status_track_activate(int track, bool active)
{
   bio_data[track].active = active;
}

// 
// ----------------------------------------------------------------------
// status_bio_add()
//
// *var           - address of variable to be tracked
// max_value      - scale factor for value of variable
// update_time    - "bio time blocks" between update (can be any value)
// track_number   - track which this biorhythm should take (0 to NUM_BIO_TRACKS-1)
// tail_length    - length of tail (value * STATUS_BIO_TAIL)
// special        - special characteristics of this biorhythm
//
// Add a variable to be tracked by the biorhythm monitor.  
// Track the NULL pointer to clear out a track slot.
//

errtype status_bio_add(int *var, int max_value, int update_time, int track_number, int tail_length, uchar special)
{
   bio_data_block    *new_block;
   uchar             value;
   int               var_value;

   if (var == NULL)     // are we trying to clear out a track slot??
   {
      if (bio_data[track_number].free == TRUE)
	 return(ERR_NOEFFECT);
      else
      {
	      bio_data[track_number].free = TRUE;
         bio_data[track_number].active = FALSE;
	      return(OK);
      }
   }

   // Make sure we have a valid entry
   if ((bio_data[track_number].free == FALSE) ||   // unused track?
	 (track_number >= NUM_BIO_TRACKS) ||       // correct track number?
	 (tail_length < 1) ||                      // non-negative tail length?
	 (tail_length > MAX_TAIL_LENGTH))          // too long of a tail?
      return(ERR_NOEFFECT);
   else
   {
      // Initialize the new biorhythm

      new_block = bio_data + track_number;
      new_block->free = FALSE;
      new_block->data = var;
      LG_memset(&(new_block->height), INVALID_HEIGHT, sizeof(uchar) * MAX_BIO_LENGTH);

      // We must first check if the variable is greater than max value, if so make it max_value
      var_value = (*var > max_value) ? max_value : *var;
      
      // Then check that it's not below 0
      if (var_value < 0)
   	 var_value = 0;

      value = (var_value * STATUS_BIO_PEAK) / max_value;

      // Set the end of the bio line to be the same as the initial
      // This way - we don't have to check if we've started a tail
      // when we erase the first value.
      new_block->height[STATUS_BIO_LENGTH-1] = value | INVALID_HEIGHT;

      // Initialize the struct
      new_block->head = 0;       // head represents the first "new" x location
      new_block->tail = FALSE;
      new_block->tail_length = tail_length * STATUS_BIO_TAIL;
      new_block->color_length = tail_length * COLOR_LENGTH;
      new_block->update_time = update_time;
      new_block->counter = 0;
      new_block->max_value = max_value;
      new_block->special = special;
      new_block->active = TRUE;

      return(OK);
   }
}

errtype clear_bio_tracks()
{
   int i;
   for (i=0; i < NUM_BIO_TRACKS; i++)
      status_bio_add(NULL,0,0,i,0,0);
   return(OK);
}

extern void change_bio_vars(void);

//#define FIND_OVERLAP(x,y) (((x) - y + STATUS_BIO_LENGTH) % STATUS_BIO_LENGTH)
int FIND_OVERLAP(int x, int y)
{
   return (((x) - y + STATUS_BIO_LENGTH) % STATUS_BIO_LENGTH);
}


#ifdef SYNCH_BIORHYTHMS
#ifdef SVGA_SUPPORT
// ------------------------------------------------------------------------
// status_bio_synchronous()
//
// Does the synchronous blitting of biorhythm canvas
void status_bio_synchronous()
{
   if (!convert_use_mode)
      return;
   gr_push_canvas(grd_screen_canvas);
   ss_scale_bitmap(&bio_bitmap,0,0,GAMESCR_BIO_WIDTH,GAMESCR_BIO_HEIGHT);
   gr_pop_canvas();
}
#endif
#endif

// ------------------------------------------------------------------------
// status_bio_update()
//
// Draw in the biorhythm stuff
// Does clever incremental redraw (but not yet)

void status_bio_update(void)
{
#ifndef TIMING_CALLBACK_OFF
   uchar             color;
   int               i;
   int               j;
   bio_data_block    *curr_blk;
   int               the_head;
   long              color_base;
   int               draw_location;
   int               var_value;
   static grs_canvas* old_canvas;
   
   if (!gBioInited)
      return;
   
   MouseLock++;
   if (MouseLock > 1) { MouseLock--; return; } 
   change_bio_vars();
   old_canvas = grd_canvas;
   gr_set_canvas(&bio_canvas);
   curr_blk = bio_data;

#ifdef SVGA_SUPPORT
   gr_push_state();
#endif
   for (i=0; i<NUM_BIO_TRACKS;i++, curr_blk++)
   {
      if (curr_blk->free == FALSE)
      {
	      // We must check to see if this track should be drawn now,
	      // or must it wait until it's time
	      if (curr_blk->counter < curr_blk->update_time)
	      {
	         curr_blk->counter++;
	      }
	      else
	      {
	         curr_blk->counter = 0;
	         color_base = track_colors[i]; // Prevent need to use index every time
	         the_head = curr_blk->head;    // Prevent need to look inside data structure every time

	         // We must first check if the variable is greater than max value, if so make it max_value
	         var_value = (*(curr_blk->data) > curr_blk->max_value) ? curr_blk->max_value : (*(curr_blk->data));

	         // Then check that it's not below 0
            curr_blk->height[the_head] = (var_value <= 0) ? 0 : (var_value * STATUS_BIO_PEAK) / curr_blk->max_value;

            if (curr_blk->special & BOTTOMLESS && curr_blk->height[the_head] == 0)
               curr_blk->height[the_head] = INVALID_HEIGHT;           
	    
	         // draw the head
	         draw_lower_tracks(i, the_head);
	 
	         // Draw the first trailer pixel - We know that this pixel must exist because we drew the first pixel
	         // when we started the biorhythm

	         draw_location = FIND_OVERLAP(the_head, 1);
	         curr_blk->height[draw_location] |= COLOR_BIT_SHIFT(1); // shift color
	         draw_lower_tracks(i, draw_location);
	 
	         if (curr_blk->tail == FALSE)
	         {
	            // Since we don't have a tail - 
	            // we don't know how long the biorhythm is 
	            // Let's find out what we have to dim!!!!
	    
	            for (j=0, draw_location=the_head-curr_blk->color_length;
                    j < the_head/curr_blk->color_length;
                    j++, draw_location -= curr_blk->color_length)
               {
		            color = curr_blk->height[draw_location] & COLOR_BIO_MASK;
		            if ((color != COLOR_BIT_SHIFT(j+1)) || (rand() > (RAND_MAX/2)))
		            {
		               curr_blk->height[draw_location] &= HEIGHT_BIO_MASK;
		               curr_blk->height[draw_location] |= COLOR_BIT_SHIFT(j+2);
		               draw_lower_tracks(i, draw_location);
		            }   
               }

	            // Have we gotten to the point where we can see the end of the tail????
	    
	            if (the_head == curr_blk->tail_length)
	            {
		            curr_blk->tail = TRUE;     // start the tail
		            clear_tail(i, 0);          // clear the first spot 
               }
	         }
            else
            {
	            // Since we have a tail - we know that we will have three spots to dim and one
	            // to delete from the biorhythm - let's dim the three spots first.

	            for (j=0, draw_location=FIND_OVERLAP(the_head, curr_blk->color_length);
                    j < (COLOR_CHANGES - 1); j++,
                    draw_location = FIND_OVERLAP(draw_location, curr_blk->color_length))
               {
	               color = curr_blk->height[draw_location] & COLOR_BIO_MASK;
	               if ((color != COLOR_BIT_SHIFT(j+1)) || (rand() > (RAND_MAX/2)))
	               {
	                  curr_blk->height[draw_location] &= HEIGHT_BIO_MASK;
	                  curr_blk->height[draw_location] |= COLOR_BIT_SHIFT(j+2);
	                  draw_lower_tracks(i, draw_location);
	               }
               }

	            // Let's delete the end of the tail
	            clear_tail(i, draw_location);
            }

            // Advance the head
            curr_blk->head = (curr_blk->head + 1) % STATUS_BIO_LENGTH;
         }
      }
   }

#ifdef SVGA_SUPPORT
   gr_pop_state();
#endif
   gr_set_canvas(old_canvas);
   MouseLock--;
#endif
}

// ---------------------------------------------------------------------------------------
// draw_bio_height()
//
// Note: requires bio_mouse_rect's x coordinates have already
// been set. We do this since we only draw_one_location_tracks
// calls this function.

void draw_bio_height(int track_number, int draw_location)
{
   bio_data_block    *curr_blk;
   long              color;
   int      x;
   int      y;
   int      y1;
   uchar    height;
   int      prevHeight;
   int      prevSpike;

   curr_blk = bio_data + track_number;

   // Extract the raw "height" information
   height = curr_blk->height[draw_location];

   if ((height & HEIGHT_BIO_MASK) == 0 && (curr_blk->special & BOTTOMLESS))
      return;

   // If we have an invalid height, continue no further
   if ((height & INVALID_HEIGHT) == INVALID_HEIGHT)
      return;

   // Extract the color offset
   color = track_colors[track_number] + ((height & COLOR_BIO_MASK) >> 5);

   // Extract the actual height
   height &= HEIGHT_BIO_MASK;

   // Get the previous height - to check for a spike
   prevHeight = prevSpike = curr_blk->height[FIND_OVERLAP(draw_location, 1)];
   prevHeight &= HEIGHT_BIO_MASK;

   x = STATUS_BIO_X_BASE + draw_location;
   y = STATUS_BIO_Y_BASE - height;

   if ((abs(height-prevHeight) < SPIKE_THRESHOLD) ||
      (curr_blk->special & NO_SPIKE) ||
      ((prevSpike & INVALID_HEIGHT) == INVALID_HEIGHT))
   {
      bio_set_pixel(color, x, y);
   }
   else
   {
      y1 = STATUS_BIO_Y_BASE - prevHeight;

      bio_vline(color, x, y, y1);
  }
}

// ----------------------------------------------------------
//  clear_tail()
//
//  KLC - greatly simplified this for Mac version, where the offscreen background
//  bitmap is the same size as onscreen.  When clearing, we simply copy the back-
//  ground bitmap directly back onto the screen.  bio_restore_pixel() handles
//  figuring out where to restore from.
// ----------------------------------------------------------
void clear_tail(int track_number, int delete_location)
{
	bio_data_block *curr_blk;
	int         prevHeight;
	int         prevLocation;
	int         height;
	int         x, y, y1;
	int         i, delta, base;
	
	curr_blk = bio_data + track_number;
	prevLocation = FIND_OVERLAP(delete_location, 1);
	
	prevHeight = curr_blk->height[prevLocation] & HEIGHT_BIO_MASK;
	height = curr_blk->height[delete_location] & HEIGHT_BIO_MASK;
	
	x = STATUS_BIO_X_BASE + delete_location;
	y = STATUS_BIO_Y_BASE - height;
	
	// First, check to see if we're restoring for a v-line spike.
	delta = abs(height-prevHeight);
	if ((delta >= SPIKE_THRESHOLD) &&
		 !((delete_location == 0) && (curr_blk->tail == FALSE)))
	{
		y1 = STATUS_BIO_Y_BASE - prevHeight;
		if (y>y1)
			base = y1;
		else
			base = y;
		for (i=0;i<delta+1; i++)
		{
			bio_restore_pixel(&bio_background_bitmap, x, base + i);
		}
	}
	else
	{
		bio_restore_pixel(&bio_background_bitmap, x, y);
	}
	
	// Invalidate the height at the delete location
	curr_blk->height[delete_location] |= INVALID_HEIGHT;
	
	draw_one_location_tracks(delete_location);
}

// ------------------------------------------------------
// draw_one_location_tracks()
//
// procedure draws all tracks at a location along 
// the biorhythm
// use this to draw to preserve ordering of tracks (overlapping)

void draw_one_location_tracks(int location)
{
   int      i;

   for (i=(NUM_BIO_TRACKS-1); i>=0; i--)
   {
      if (!bio_data[i].free && bio_data[i].active)
	 draw_bio_height(i, location);
   }
}


// --------------------------------------------
// draw_lower_tracks()
//
// procedure differs from draw_one_location_tracks in that it only
// draws tracks that are under the track number, including the track
// number given. Saves a little work.
//

void draw_lower_tracks(int track_number, int location)
{
   int      i;

   for (i=track_number;i>=0; i--)
   {
      if (!bio_data[i].free && bio_data[i].active)
	 draw_bio_height(i, location);
   }
}

// RANDOM SPEW STUFF
// moved here by doug to make things looking pretty


#define SIN_MAG   20

// Temporary biorhythm variables
ulong time1 = 0;
ulong time2 = 0;
ulong time3 = 0;
ulong time4 = 0;
ulong time5 = 0;
ulong time6 = 0;
ulong time7 = 0;
int bio_delta = 1;
extern int diff_sum;
extern int curr_ll;

// this simulates the heart beat!
int test_bio_var = 10;
int test_bio_var2 = 2;

// this simulates the sine wave
int test_bio_var3 = 19;

int bio_energy_var;
fix sinX = 0;
fix sinChi = 0;

void gamescr_bio_func(void)
{
   extern int rad_absorb,bio_absorb;
   int i;

   clear_bio_tracks();
   
   // KLC - the "update_time" parameter is halved for Mac version, because we're only
   // getting called 70 times/sec rather than 140.
   
   status_bio_add(&bio_energy_var, MAX_ENERGY, 2, ENERGY_TRACK, 2, 0);
   {
      short ver = player_struct.hardwarez[CPTRIP(ENV_HARD_TRIPLE)];
      if (ver >= 1)
         status_bio_add(&bio_absorb,24,2,BIOHAZARD_TRACK,3,BOTTOMLESS);
      if (ver >= 2)
         status_bio_add(&rad_absorb,24,2,RADIATION_TRACK,3,BOTTOMLESS);
   }
   status_bio_add(&test_bio_var, 20, 1, HEART_TRACK, 3, 0);
   status_bio_add(&test_bio_var3, SIN_MAG, 4, SINE_TRACK, 2, 0);

   for (i = 0; i < NUM_BIO_TRACKS; i++)
      status_track_activate(i,(player_struct.active_bio_tracks & (1 << i)) != 0);
}

void diff_bio_func(void)
{
   clear_bio_tracks();
   //                            max_value, update_time, track_number, tail_length, special
   status_bio_add(&diff_sum,     12,      6, 0, 2, 0);
   status_bio_add(&test_bio_var, 20,      2, 1, 3, 0);
   status_bio_add(&test_bio_var3,20,      8, 2, 2, 0);
   status_bio_add(&test_bio_var2,SIN_MAG, 4, 4, 3, 0);
}

uchar heart_beat = 0;
ulong heart_time = 0;
ulong heart_delay = 5;
bool flatline_heart;

uchar chi_amp=STATUS_CHI_AMP;

extern ubyte fatigue_threshold;

#define FATIGUE_RANGE (CONTROL_MAX_VAL - SPRINT_CONTROL_THRESHOLD)
#define ENERGY_ZERO 50

void change_bio_vars(void)
{
   // Demo for biorhythm
   int      fatigue_ratio;
   int      fatigue_amp;
   int      chi_per;

   if ((*tmd_ticks - heart_time) > heart_delay)
   {
      switch (heart_beat)
      {
      case 0:
         
         {
            int f = max(0,(player_struct.fatigue - (CIT_CYCLE*fatigue_threshold))/CIT_CYCLE);
            int heart_ratio = 3400/(f+7);
            if (heart_ratio > 500)
               heart_ratio = 500;
            else if (heart_ratio < 90)
               heart_ratio = 90;
            if (!flatline_heart && (*tmd_ticks - time1) > heart_ratio)
            {
               test_bio_var = 19;
               heart_delay = 5;
               time1 = heart_time = *tmd_ticks;
               heart_beat = 1;
            }
         }
         break;
         
      case 1:
         heart_beat = 2;
         test_bio_var = 20;
         heart_delay = 5;
         break;
      case 2:
         heart_beat = 3;
         test_bio_var = 10;
         heart_delay = 7;
         break;
      case 3:
         heart_beat = 4;
         test_bio_var = 2;
         heart_delay = 5;
         break;
      case 4:
         heart_beat = 5;
         test_bio_var = 1;
         heart_delay = 5;
         break;
      case 5:
         heart_beat = 0;
         test_bio_var = 10;
         heart_delay = 5;
         break;
      }
      heart_time = *tmd_ticks;
   }
   // bio_energy_var
   {
      static ubyte energy_spike = 0;
      static int last_val = 0;
      if (bio_energy_var != last_val)
         energy_spike = 3;
      if (energy_spike == 0)
      {
         bio_energy_var = ENERGY_ZERO + player_struct.energy_spend - player_struct.energy_regen;
      }
      else
      {
         energy_spike--;
      }
      last_val = bio_energy_var;
   }
   // MR. SINUSOID biorhythm
   if (player_struct.fatigue)
      fatigue_ratio = fatigue_amp = (150000/player_struct.fatigue);
   else
   {
      fatigue_ratio = 40;
      fatigue_amp = SIN_MAG;
   }

   if (fatigue_ratio < 5)
      fatigue_ratio = 5;
   else if (fatigue_ratio > 40)
      fatigue_ratio = 40;

   fatigue_amp -= 5;
   if (fatigue_amp < 0)
      fatigue_amp = 0;
   else if (fatigue_amp > SIN_MAG)
      fatigue_amp = SIN_MAG;

   if ((*tmd_ticks - time2) > fatigue_ratio)
   {
      sinX += FIX_UNIT/10;
      test_bio_var2 = (((FIX_UNIT- fix_fastsin(sinX))/2) * fatigue_amp) + 1 + ((SIN_MAG-fatigue_amp)/2);
      time2 = *tmd_ticks;
   }

   chi_per=chi_amp;
   if(player_struct.drug_status[CPTRIP(LSD_DRUG_TRIPLE)])
      chi_per<<=1;               
   if(player_struct.drug_status[CPTRIP(GENIUS_DRUG_TRIPLE)])
      chi_per>>=2;
   if (chi_per<2) {
      test_bio_var3 = 10;
      time3 = *tmd_ticks;
      sinChi=0;
   }
   else if ((*tmd_ticks - time3) > chi_per)
   {
      fix s;
      sinChi += FIX_UNIT/20;
      if(sinChi>6*FIX_UNIT) sinChi-=6*FIX_UNIT;
      if(sinChi<fix_make(3,0))
         s=-fix_mul(sinChi,sinChi-3*FIX_UNIT)*4/9;
      else
         s=fix_mul(sinChi-3*FIX_UNIT,sinChi-6*FIX_UNIT)*4/9;
      test_bio_var3 = 10 + fix_int(chi_amp*s);
      time3 = *tmd_ticks;
   }
}

