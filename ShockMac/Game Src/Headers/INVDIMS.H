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
#ifndef __INVDIMS_H
#define __INVDIMS_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/invdims.h $
 * $Revision: 1.5 $
 * $Author: mahk $
 * $Date: 1994/08/19 05:07:22 $
 *
 * $Log: invdims.h $
 * Revision 1.5  1994/08/19  05:07:22  mahk
 * added one to inventory panel size.
 * 
 * Revision 1.4  1994/01/23  04:00:08  dc
 * w+h for message line
 * 
 * Revision 1.3  1993/11/12  19:44:13  mahk
 * Changed dimensions for purposes of 360 view
 * 
 * Revision 1.2  1993/09/18  03:55:12  mahk
 * Added message line coords as well.c
 * 
 * Revision 1.1  1993/09/17  16:59:25  mahk
 * Initial revision
 * 
 *
 */

// INVENTORY PANEL DIMENSIONS

#define GAME_MESSAGE_X 87
#define GAME_MESSAGE_Y 138
#define GAME_MESSAGE_W 145
#define GAME_MESSAGE_H 7

#define INVENTORY_PANEL_X  87  
#define INVENTORY_PANEL_Y  145
#define INVENTORY_PANEL_WIDTH    145
#define INVENTORY_PANEL_HEIGHT   50

#define INV_FULL_WD (INVENTORY_PANEL_WIDTH + INVENTORY_PANEL_X - GAME_MESSAGE_X)
#define INV_FULL_HT (INVENTORY_PANEL_HEIGHT + INVENTORY_PANEL_Y - GAME_MESSAGE_Y)

#endif // __INVDIMS_H

