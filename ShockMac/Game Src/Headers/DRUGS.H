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
#ifndef __DRUGS_H
#define __DRUGS_H

/*
 * $Source: n:/project/cit/src/inc/RCS/drugs.h $
 * $Revision: 1.11 $
 * $Author: xemu $
 * $Date: 1994/05/13 21:11:25 $
 *
 * $Log: drugs.h $
 * Revision 1.11  1994/05/13  21:11:25  xemu
 * reflex
 * 
 * Revision 1.10  1994/02/01  04:37:07  mahk
 * DRUGS !
 * 
 * Revision 1.9  1993/09/02  23:07:28  xemu
 * angle me baby
 * 
 * Revision 1.8  1993/08/11  20:53:43  spaz
 * Changed drug identifying #define's
 * 
 * Revision 1.7  1993/07/28  16:57:02  mahk
 * Added drug2triple & triple2drug
 * 
 * Revision 1.6  1993/07/26  21:24:47  spaz
 * moved drug id's to #define's here
 * 
 * Revision 1.5  1993/07/19  11:39:48  mahk
 * Added inventory stuff
 * 
 * Revision 1.4  1993/07/08  13:29:35  spaz
 * Moved some #define's around.
 * 
 * Revision 1.3  1993/07/01  19:16:38  spaz
 * Added constants and prototypes for generic drug.c funcs.
 * 
 * Revision 1.2  1993/07/01  17:19:37  spaz
 * Added NUM_DRUGZ #define, and drugs_time[] static array.
 * 
 * Revision 1.1  1993/07/01  16:26:23  mahk
 * Initial revision
 * 
 *
 */

// Defines

#define DRUG_STAMINUP 0
#define DRUG_SIGHT    1
#define DRUG_LSD      2
#define DRUG_MEDIC    3
#define DRUG_REFLEX   4
#define DRUG_GENIUS   5
#define DRUG_DETOX    6

// Includes

// C Library Includes

// System Library Includes

// Master Game Includes

// Game Library Includes

// Game Object Includes


// Defines

// Prototypes

char* get_drug_name(int type, char* buf);
void drug_use(int type);
void drug_wear_off(int type);
void drug_effect(int type);
void drugs_update();
void drugs_init();
void drug_startup(bool visible);
void drug_closedown(bool visible);
int drug2triple(int type);
int triple2drug(int triple);

// Globals

#endif // __DRUGS_H

