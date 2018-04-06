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
#ifndef __POPUPS_H
#define __POPUPS_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/popups.h $
 * $Revision: 1.3 $
 * $Author: xemu $
 * $Date: 1994/08/16 22:13:23 $
 *
 */

#define POPUP_MFD_LEFT     0
#define POPUP_MFD_RIGHT    1
#define POPUP_DOWN         2
#define POPUP_ICON_LEFT    3
#define POPUP_ICON_RIGHT   4
#define NUM_POPUPS         5

void init_popups(void);
// initalizes popup cursors

void make_popup_cursor(LGCursor* c, grs_bitmap* bm, char* string, uint tmplt, bool allocate, LGPoint offset);
void make_email_cursor(LGCursor* c, grs_bitmap* bm, uchar page, bool init);

/* Modifies c and bm to be a cursor built from the specified 
 string and tmplt.  if allocate is true the bits for the cursor bitmap
 will be Malloc'ed.  Otherwise, bm must already  have a bits field set that
 points to enough memory for the cursor bitmap.  
 */

void load_string_array(Ref first, char *arry[], char buf[], int sz, int n);
/*
   Loads N seuquential strings (starting with <first>) into buf.
   Fills the array arry with pointers to each of the strings.
*/

extern bool 	popup_cursors;

#endif // __POPUPS_H

