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
#ifndef __BIOTRAX_H
#define __BIOTRAX_H

/*
 * $Source: n:/project/cit/src/inc/RCS/biotrax.h $
 * $Revision: 1.1 $
 * $Author: mahk $
 * $Date: 1994/02/23 13:20:34 $
 *
 */



// Defines
#define ENERGY_TRACK 0
#define BIOHAZARD_TRACK (ENERGY_TRACK+1)
#define RADIATION_TRACK (BIOHAZARD_TRACK+1)
#define LOOPLINE_TRACK  (RADIATION_TRACK+1)
#define HEART_TRACK (NUM_BIO_TRACKS - 2)
#define SINE_TRACK  (NUM_BIO_TRACKS - 1)

// Prototypes

// Globals

#endif // __BIOTRAX_H

