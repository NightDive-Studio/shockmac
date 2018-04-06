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
#ifndef __CYBRND_H
#define __CYBRND_H

/*
 * $Source: n:/project/cit/src/inc/RCS/cybrnd.h $
 * $Revision: 1.5 $
 * $Author: minman $
 * $Date: 1993/12/15 22:56:25 $
 *
 * $Log: cybrnd.h $
 * Revision 1.5  1993/12/15  22:56:25  minman
 * added effect random var
 * 
 * Revision 1.4  1993/09/02  23:07:26  xemu
 * angle me baby
 * 
 * Revision 1.3  1993/08/17  14:17:55  minman
 * added make-info random number
 * 
 * Revision 1.2  1993/08/12  19:45:06  minman
 * added grenade random no
 * 
 * Revision 1.1  1993/08/12  19:22:58  minman
 * Initial revision
 * 
 *
 */

#ifdef __CYBRND_SRC
RNDSTREAM_STD(damage_rnd);
RNDSTREAM_STD(grenade_rnd);
RNDSTREAM_STD(obj_make_rnd);
RNDSTREAM_STD(effect_rnd);
#else
extern RndStream damage_rnd;
extern RndStream grenade_rnd;
extern RndStream obj_make_rnd;
extern RndStream effect_rnd;
#endif

// so i feel like using prime numbers - got a problem with that????
#define DAMAGE_SEED  23
#define GRENADE_SEED  31
#define OBJMAKE_SEED 29
#define EFFECT_SEED 37

void rnd_init(void);

#endif // __CYBRND_H


