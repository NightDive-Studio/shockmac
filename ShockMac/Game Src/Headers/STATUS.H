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
#ifndef __STATUS_H
#define __STATUS_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/status.h $
 * $Revision: 1.24 $
 * $Author: tjs $
 * $Date: 1994/08/10 02:35:07 $
 *
 * $Log: status.h $
 * Revision 1.24  1994/08/10  02:35:07  tjs
 * death.
 * 
 * Revision 1.23  1994/05/12  00:38:43  dc
 * c:\sspro\util\promode 54 bio for diff screen really
 * 
 * Revision 1.22  1994/05/12  00:30:57  dc
 * diff screen bio defines
 * 
 * Revision 1.21  1994/02/18  18:17:40  minman
 * fixed art offset
 * 
 * Revision 1.20  1994/02/16  08:43:11  mahk
 * Changed left margin of biorhythm.  Should have no noticeable effect.
 * 
 * Revision 1.19  1993/11/23  02:09:50  xemu
 * moved upper right around some
 * 
 * Revision 1.18  1993/11/22  19:55:16  xemu
 * minor readjustments
 * 
 * Revision 1.17  1993/09/18  00:15:45  xemu
 * made room for eye/lean
 * 
 * Revision 1.16  1993/09/13  23:48:22  xemu
 * enhanced biorhythms
 * 
 * Revision 1.15  1993/09/08  19:21:15  minman
 * moved down biorhythms
 * 
 * Revision 1.14  1993/09/02  23:08:42  xemu
 * angle me baby
 * 
 * Revision 1.13  1993/08/20  16:10:27  spaz
 * changed prototype for status_vitals_update()
 * 
 * Revision 1.12  1993/08/12  22:44:31  spaz
 * attempted to change coords to benefit mankind
 * 
 * Revision 1.11  1993/08/05  22:33:56  spaz
 * Fixed biorhythm tracking within art (thanks, Art!),
 * and threw in #define's neccessary for getting dynamic
 * status bar graphs in the upper right hand corner
 * 
 * Revision 1.10  1993/06/10  18:36:12  minman
 * made little optimizations and commented some
 * 
 * Revision 1.9  1993/06/09  16:53:23  minman
 * moved startup and shutdown code into status.c (from screen.c)
 * 
 * Revision 1.8  1993/06/08  23:00:28  minman
 * made little optimizations
 * 
 * Revision 1.7  1993/06/08  22:10:34  minman
 * status_bio_add now takes a tail length argument
 * 
 * Revision 1.6  1993/06/08  14:42:41  minman
 * made modifications to NO_HEIGHT define
 * 
 * Revision 1.5  1993/06/08  01:05:46  minman
 * overlapping lines work, and so do deletions of tails
 * 
 * Revision 1.4  1993/06/07  22:45:58  minman
 * allowed track to remember color
 * 
 * Revision 1.3  1993/06/07  04:44:31  minman
 * biorhythm works
 * 
 * Revision 1.2  1993/05/14  15:50:07  xemu
 * draw & update
 * 
 * Revision 1.1  1993/04/30  14:36:25  xemu
 * Initial revision
 * 
 *
 */

// Includes

// C Library Includes

// System Library Includes

// Master Game Includes

// Game Library Includes

// Game Object Includes


// Defines
#define NUM_BIO_TRACKS     8

#define GAMESCR_BIO         0
#define GAMESCR_BIO_X       5
#define GAMESCR_BIO_Y       1
//#define GAMESCR_BIO_WIDTH   149
#define GAMESCR_BIO_WIDTH   131
#define GAMESCR_BIO_HEIGHT  17

#define DIFF_BIO         1
#define DIFF_BIO_X       6
#define DIFF_BIO_Y       181
#define DIFF_BIO_WIDTH   307
#define DIFF_BIO_HEIGHT  17

#define STATUS_CHI_AMP 8

#define STATUS_BIO_X       curr_bio_x
#define STATUS_BIO_Y       curr_bio_y
#define STATUS_BIO_WIDTH   curr_bio_w
#define STATUS_BIO_HEIGHT  curr_bio_h

#define STATUS_START_OFFSET 0
#define STATUS_BIO_Y_DELTA  1
#define MAX_BIO_LENGTH      307
#define STATUS_BIO_LENGTH   (STATUS_BIO_WIDTH - STATUS_START_OFFSET)
#define STATUS_BIO_TAIL     30
#define STATUS_BIO_PEAK     (STATUS_BIO_HEIGHT - 3)    // 3 because of zany art size
#define STATUS_BIO_X_BASE   (STATUS_BIO_X + STATUS_START_OFFSET)
#define STATUS_BIO_Y_BASE   (STATUS_BIO_Y + STATUS_BIO_HEIGHT - STATUS_BIO_Y_DELTA - 2)
  
#define SPIKE_THRESHOLD    4
#define COLOR_CHANGES      6
#define COLOR_LENGTH       (STATUS_BIO_TAIL/COLOR_CHANGES)

#define MAX_TAIL_LENGTH    5
#define NO_HEIGHT          0x1f
#define INVALID_HEIGHT     0xE0

#define COLOR_BIO_MASK     0xE0     // Top Three bits signify depth of color
#define HEIGHT_BIO_MASK    0x1f     // Bottom Five bits signify height
#define BIT6               0x20     // First bit of the color field.

#define COLOR_BIT_SHIFT(x) ((x) << 5)

//#define FIND_OVERLAP(x,y) (((x) - y + STATUS_BIO_LENGTH) % STATUS_BIO_LENGTH)

#define STATUS_VITALS_X          184
#define STATUS_VITALS_Y          0
#define STATUS_VITALS_WIDTH      130
#define STATUS_VITALS_HEIGHT     17

#define STATUS_VITALS_X_BASE     (STATUS_VITALS_X+4)
#define STATUS_VITALS_Y_TOP      (STATUS_VITALS_Y+1)
#define STATUS_VITALS_Y_BOTTOM   (STATUS_VITALS_Y+11)
#define STATUS_VITALS_H          8
#define STATUS_VITALS_W          (STATUS_VITALS_WIDTH-9)

#define STATUS_X                 4
#define STATUS_Y                 1
#define STATUS_HEIGHT            20
#define STATUS_WIDTH             312

#define GAMESCR_BIO_REF          REF_IMG_bmBiorhythm
#define DIFF_BIO_REF             REF_IMG_bmDiffBio
#define STATUS_RESID             curr_bio_ref
#define STATUS_RES_VITALSID      REF_IMG_bmVitals
#define STATUS_RES_HEALTH_ID     REF_IMG_bmVitalInnardsTop
#define STATUS_RES_ENERGY_ID     REF_IMG_bmVitalInnardsBottom

// Special Status Biorhythm variables

#define NO_SPIKE                 0x01
#define SPIKE_NOISE              0x02

// Prototypes

// Draw the background for the biorhythm thing

void status_bio_set(short bio_mode);
void status_bio_init(void);
void status_bio_start(void);
void status_bio_end(void);

void status_bio_draw(void);
extern void status_vitals_init();

// Add a variable to be tracked by the biorhythm monitor.  
// Track the NULL pointer to clear out a track slot.
// special - parameters to set characteristics of track (not in use currently - for future use cause we're powerful)
errtype status_bio_add(int *var, int max_value, int update_time, int track_number, int tail_length, uchar special);

// Draw the biorhythm quasi-persistent thing.  Keep track of
// previous draws to do clever incremental strategies.
void status_bio_update(void);

// Draw in the health / energy diagram.  Keep track of
// previous draws to do clever incremental strategies.
errtype status_vitals_update(bool Full_Redraw);

// Globals

extern bool flatline_heart;
extern uchar chi_amp;

#endif //__STATUS_H

