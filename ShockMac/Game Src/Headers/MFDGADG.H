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
#ifndef __MFDGADG_H
#define __MFDGADG_H

/*
 * $Source: n:/project/cit/src/inc/RCS/mfdgadg.h $
 * $Revision: 1.5 $
 * $Author: mahk $
 * $Date: 1994/02/16 09:27:14 $
 *
 * $Log: mfdgadg.h $
 * Revision 1.5  1994/02/16  09:27:14  mahk
 * Fixed prototype bug.
 * 
 * Revision 1.4  1994/02/16  09:23:57  mahk
 * Added more goofy operations.
 * 
 * Revision 1.3  1993/12/08  10:31:59  mahk
 * Changed to mfdint.h
 * 
 * Revision 1.2  1993/10/20  05:47:23  mahk
 * Added a slider gadget.
 * 
 * Revision 1.1  1993/09/15  10:50:30  mahk
 * Initial revision
 * 
 *
 */

// Includes
#include "mfdint.h"
#include "mfdext.h"
#include "mfddims.h"

//--------------------------------------------------------------------------------
// MFD GADGETS
// 
// As the name suggests, this file is a set of simple gadgets usable in MFDs.  


// -------------------
// BUTTON ARRAYS
// -------------------

// A button array is a matrix of buttons. MFD button arrays do not
// draw themselves, only handle input.  

// Defines

typedef bool (*MFDBttnCallback)(MFD* mfd, LGPoint button, uiEvent* ev, void *data);

// Prototypes
errtype MFDBttnArrayInit(MFDhandler* h, LGRect* r, LGPoint bdims, LGPoint bsize, MFDBttnCallback cb, void* cbdata);
// Initialize h to handle a buttonarray in rect r of bdims.x buttons across by bdims.y buttons down.  bsize
// describes the pixel dimensions of each button.  Whenever a button is clicked on, cb will be called with the 
// coordinates of the button, the mouse event, and the value of cbdata.  

errtype MFDBttnArrayShutdown(MFDhandler* h);
// shuts down a button array.  

errtype MFDBttnArrayResize(MFDhandler* h, LGRect* r, LGPoint bdims, LGPoint bsize);
// Changes the dimensions of an mfd button array. 

// Globals

// ------------------
// SLIDERS 
// ------------------

// A slider is a linear "analog" control.  

typedef bool (*MFDSliderCallback)(MFD* mfd, short val, uiEvent* ev, void *data);

errtype MFDSliderInit(MFDhandler* h, LGRect* r, MFDSliderCallback cb, void* data);
// Create a (horizontal) slider in a particular sub-rect of the MFD.  
// Whenever the slider is adjusted, the callbad will be called with the 
// mouse event and the horizontal (relative) position of the slider. 
#endif // __MFDGADG_H

