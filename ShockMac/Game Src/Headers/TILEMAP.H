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
#ifndef __TILEMAP_H
#define __TILEMAP_H

/*
 * $Source: n:/project/cit/src/inc/RCS/tilemap.h $
 * $Revision: 1.19 $
 * $Author: mahk $
 * $Date: 1993/09/03 14:29:09 $
 *
 * $Log: tilemap.h $
 * Revision 1.19  1993/09/03  14:29:09  mahk
 * Moved tile cameras out to there own files.  
 * 
 * Revision 1.18  1993/09/02  23:08:48  xemu
 * angle me baby
 * 
 * Revision 1.17  1993/08/17  18:32:08  mahk
 * Changed texture2color
 * 
 * Revision 1.16  1993/06/21  13:23:50  mahk
 * Changed HEIGHT2COLOR
 * 
 * Revision 1.15  1993/06/08  22:03:52  mahk
 * Changed TEXTURE2COLOR
 * ls
 * 
 * Revision 1.14  1993/06/05  07:44:22  mahk
 * Added tilemap cameras.
 * 
 * Revision 1.13  1993/06/02  10:26:47  mahk
 * Added event dispatching
 * 
 * Revision 1.12  1993/05/26  19:39:45  mahk
 * Added TEXTURE2COLOR
 * 
 * Revision 1.11  1993/05/25  19:01:35  mahk
 * Modified height color palette
 * 
 * Revision 1.10  1993/05/25  17:41:11  mahk
 * added HEIGHT2COLOR
 * ls
 * 
 * Revision 1.9  1993/05/21  22:51:54  xemu
 * fixed typo
 * 
 * Revision 1.8  1993/05/21  21:31:55  mahk
 * new improved highlights.
 * 
 * Revision 1.7  1993/05/21  20:28:45  mahk
 * We are now fully fullmap-compliant
 * 
 * Revision 1.6  1993/05/20  11:56:31  mahk
 * Implmented highlights
 * 
 * Revision 1.5  1993/05/20  09:01:34  mahk
 * Added tilemap resize & move.  Implemented a currently somewhat broken version of 
 * the 3d toggle button.  Added tilemap cursor.  
 * 
 * Revision 1.4  1993/05/18  14:59:26  mahk
 * Changed the prototype for TileMapGetZoom
 * 
 * Revision 1.3  1993/05/17  15:57:25  mahk
 * Added a cursor and collected drawfunc and data into a func.
 * 
 * Revision 1.2  1993/05/14  16:51:55  mahk
 * Mods to deal with tile editor.
 * 
 * Revision 1.1  1993/05/12  15:47:43  mahk
 * Initial revision
 * 
 *
 */

// Includes
#include "map.h"
#include "colors.h"
#include "tilecam.h"

// Defines
extern long height_colors[MAP_HEIGHTS];

#define MAX_HIGHLIGHTS     8
#define NEW_HIGHLIGHT      8

#define SOLID_TERR_COLOR      (BLUE_BASE + 0x8)
#define HEIGHT2COLOR(h)  (height_colors[h])
#define TEXTURE2COLOR(t) ((t)*2+32)
#define CAMERA_COLOR (BROWN_BASE+5)

typedef bool (*highlight_func)(FullMap* map, MapElem* elem, LGPoint square, void* data);

typedef void (*tile_drawfunc)(struct _tilemap* t, LGPoint square, MapElem* elem, LGPoint pix);


typedef struct _tiledraw
{
   tile_drawfunc func;
   void* data;
} TileDraw;


#define NUM_CAMERAS 2


typedef struct _tilemap
{
   LGRegion  reg;
   MapElem *map;
   FullMap *fmap;
   ushort zoom; // pixels per tile.
   LGPoint topleft;
   TileDraw draw;
   bool  showcursor;
   LGPoint cursor;           // Cursor for keyboard input.
   uchar highlights[MAP_ROWS][MAP_COLS];
   uchar hilitebits;
   TileCamera cameras[NUM_CAMERAS];
   bool cameras_used[NUM_CAMERAS];
}  TileMap;

// user-defined event types
#define CURSOR_CHANGE  0   // signalled when the cursor changes
#define ZOOM_CHANGE    1   // signalled when zoom factor changes. 

typedef struct _tilemap_event
{
   UIEVFRONT
   short subtype;
   TileMap* tilemap;
   UIEVBACK(sizeof(TileMap*));
}  TileMapEvent;



// Prototypes

errtype TileMapInit(TileMap* t, LGRegion* parent, LGRect* boundingrect, int z,
                    FullMap* fmap, ushort zoompix,
                  LGPoint topleft, TileDraw draw);
// Initialize a tilemap.  
// Cursor defaults to on and in the top left.  
// camera defaults to off. 

void TileMapSetDefault(TileMap* t);
// Set the default tilemap, which will be used in place of NULL
// in any of the following tilemap operations. 

errtype TileMapSetTopLeft(TileMap* t, LGPoint topleft);
// Changes the top left displayed square of the tilemap,
// redisplaying the tilemap.   

errtype TileMapSetMap(TileMap* t, FullMap* fmap);
// Changes the map which is displayed by the tilemap, redisplaying
// the tilemap.

errtype TileMapSetDraw(TileMap* t, TileDraw draw);
// Changes the function used to display mapsquares, 
// and redisplays the tilemap.

errtype TileMapGetDraw(TileMap* t, TileDraw* draw);
// Gets the function used to display mapsquares, and its data

errtype TileMapSetHighlight(TileMap* t, LGPoint square, int hilitenum, bool on);
// Sets the highlighted-ness of the specified square to the value of "on" for 
// highlight number hilitenum.  (there are MAX_HIGHLIGHTS possible hilitenums) 

errtype TileMapGetHighlight(TileMap* t, LGPoint square, int hilitenum, bool* on);
// Gets the highlighted-ness of the specified square for 
// highlight number hilitenum.  (there are MAX_HIGHLIGHTS possible hilitenums) 

errtype TileMapHighlight(TileMap* t, int hilitenum, highlight_func func, void* data);
// Sets the value of hilitenum for each square to the value returned by 
// the specified function when applied to that square.  If func is NULL, 
// clears hilitenum.  

errtype TileMapClearHighlights(TileMap* t);
// Clears ALL highlights for tilemap t.  

errtype TileMapFindHighlightNum(TileMap* tm, int* num);
// finds and allocates a free highlight number. 


errtype TileMapSetZoom(TileMap* t, ushort zoom);
// Changes the zoom factor  for t, redisplaying it. 

errtype TileMapGetZoom(TileMap* t,ushort* zoom);
// Gets the zoom facter of t. 

bool TileMapSquare2Pixel(TileMap* t, LGPoint in, LGPoint* out);
// if the square "in" is visible, sets *out to the upper left
// corner of it in screen coordinates, and returns true.  
// otherwise, returns false.

bool TileMapPixel2Square(TileMap* t, LGPoint in, LGPoint* out);
// If the LGPoint "in" is contained in a square of tilemap t, 
// set *out to that square in map coordinates, and return true.
// Otherwise, return false.  


bool TileMapRedrawPixels(TileMap* t, LGRect* r);
// If any part of r intersects with t in screen coordinates, 
// return true and redraw that intersection, otherwise
// return false.  the NULL rectangle represents all pixels. 

bool TileMapRedrawSquares(TileMap* t, LGRect* r);
// If any of the squares in r are visible in t, redraw those
// squares and return true.  otherwise, return false.  
// The NULL rectangle represents all squares.  

bool TileMapRedrawSquare(TileMap* t, LGPoint sq);
// Redraws a single square of t, returns whether that 
// square is visible, and thus was actually redrawn. 

errtype TileMapSetCursor(TileMap* t, LGPoint p);
// Puts the tilemap cursor at LGPoint p. 

errtype TileMapGetCursor(TileMap* t, LGPoint *p);
// Gets the tilemap cursor.  

errtype TileMapCursorOnOff(TileMap* t, bool onoff);
// Turns the cursor display on or off.  

errtype TileMapAddCamera(TileMap* t, TileCamera* tc, uint* id);
// Adds a new camera to the tilemap.  id will have the id number of the camera

errtype TileMapRemoveCamera(TileMap* t, uint id);
// Removes the camera with the specified id;

errtype TileMapGetCamera(TileMap* t, TileCamera* tc, uint id);
// Fills in tc with the current data on the camera with the specified id.

errtype TileMapSetCamera(TileMap* t, TileCamera* tc, uint id);
// Sets the data of the camera with the specified id to the contents of tc.

errtype TileMapUpdateCameras(TileMap* t);
// redraws all active cameras in t. 

errtype TileMapResize(TileMap* tm, LGPoint newdims);
// resizes the tilemap to the dimensions specified by newdims.
// redisplaying the tilemap.


errtype TileMapMove(TileMap* tm, LGPoint newloc, int z);
// Moves a tilemap to new coordinates relative to parent, 
// redisplaying the tilemap.

errtype TileMapInstallHandler(TileMap* tm, ulong evmask, uiHandlerProc proc, void* data, int* id);
// Installs an event  handler on the tilemap's LGRegion.  

errtype TileMapRemoveHandler(TileMap* tm, int id);
// removes a previously-installed event handler. 

// Globals


extern TileMap *TheTileMap;

#endif // __TILEMAP_H





 
