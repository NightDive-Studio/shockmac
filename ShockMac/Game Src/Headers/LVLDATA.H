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
#ifndef __LVLDATA_H
#define __LVLDATA_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/lvldata.h $
 * $Revision: 1.6 $
 * $Author: tjs $
 * $Date: 1994/09/03 23:43:36 $
 *
 * $Log: lvldata.h $
 * Revision 1.6  1994/09/03  23:43:36  tjs
 * Automaps are in level_gamedata instead of being Malloc'd
 * 
 * Revision 1.5  1994/08/11  18:31:40  mahk
 * New enviro line
 * 
 * Revision 1.4  1994/03/31  20:45:15  xemu
 * zero grav for bio
 * 
 * Revision 1.3  1994/02/08  02:00:21  mahk
 * Added bio_h and rad_h
 * 
 * Revision 1.2  1994/01/26  23:35:24  mahk
 * New bio and rad regime.
 * 
 * Revision 1.1  1993/11/19  05:30:36  mahk
 * Initial revision
 * 
 *
 */

// Includes
#include "amap.h" // for our scheme to put automaps in here.

// Game system data for each level. 

typedef struct _level_data
{
   short size; // size of this structure.  
   uchar mist;
   uchar gravity;
   struct _hazard
   {
      uchar rad;
      uchar bio; // post-exposure damage, or gravity level
      uchar zerogbio; // if this is true, bio is interpreted as zero gravity
      uchar bio_h;
      uchar rad_h;
   } hazard;
   ulong exit_time;  // timestamp at which we exited the level
   curAMap auto_maps[NUM_O_AMAP];
} LevelData;

extern LevelData level_gamedata;

#define OLD_LEVEL_GAMEDATA_SIZE 5 // for misc_saveload versions less than 5

#endif // __LVLDATA_H

