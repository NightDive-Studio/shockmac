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
#ifndef __TILECAM_H
#define __TILECAM_H

/*
 * $Source: n:/project/cit/src/inc/RCS/tilecam.h $
 * $Revision: 1.1 $
 * $Author: mahk $
 * $Date: 1993/09/03 14:30:23 $
 *
 * $Log: tilecam.h $
 * Revision 1.1  1993/09/03  14:30:23  mahk
 * Initial revision
 * 
 *
 */

// Includes

// Defines

typedef void (*camera_setfunc)(struct _tilecamera *tc);
// Updates the state of the camera


typedef struct _tilecamera
{
   long bcolor, fcolor;
   fix x,y,theta;
   bool show;
   camera_setfunc func;
   void* data;
} TileCamera;


// Prototypes

// Globals

#endif // __TILECAM_H

