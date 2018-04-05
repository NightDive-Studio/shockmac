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
#ifndef __PHYSUNIT_H
#define __PHYSUNIT_H

/*
 * $Source: n:/project/cit/src/inc/RCS/physunit.h $
 * $Revision: 1.8 $
 * $Author: xemu $
 * $Date: 1994/05/24 23:33:39 $
 *
 *
 */

// Includes

// Defines
#define PHYSICS_RADIUS_UNIT 96    // radius units per square
#define PHYS_PEP_UNIT 1           // Objprop units per physics unit
#define PHYS_HARDNESS_UNIT 1      // Objprop units per physics unit
#define PHYS_ROUGHNESS_UNIT   1   // Objprop units per physics unit

// 10 is much more correct
#define BROKEN_NEW_WAY
#ifdef BROKEN_NEW_WAY
#define PHYS_MASS_UNIT    10
#define PHYS_MASS_C_NUM   80
#define PHYS_MASS_C_DEN   33
#endif

//#define BROKEN_OLD_WAY
#ifdef BROKEN_OLD_WAY
// what in hell is this...?
#define PHYS_MASS_UNIT  1000       // tenths of kilograms to players
#define PHYS_MASS_C_NUM    1
#define PHYS_MASS_C_DEN    1
#endif

// Prototypes

// Globals

#endif // __PHYSUNIT_H

