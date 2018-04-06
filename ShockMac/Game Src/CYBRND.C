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
/*
 * $Source: n:/project/cit/src/RCS/cybrnd.c $
 * $Revision: 1.6 $
 * $Author: minman $
 * $Date: 1994/01/25 00:12:05 $
 *
 * $Log: cybrnd.c $
 * Revision 1.6  1994/01/25  00:12:05  minman
 * added config vars for the random seeds and
 * does a random selection of seeds if seeds are
 * not specified
 * 
 * Revision 1.5  1993/12/15  22:57:49  minman
 * added effect var
 * 
 * Revision 1.4  1993/09/02  23:00:21  xemu
 * angle!
 * 
 * Revision 1.3  1993/08/17  14:17:23  minman
 * added make-info random number
 * 
 * Revision 1.2  1993/08/12  19:44:56  minman
 * added grenade random no
 * 
 * Revision 1.1  1993/08/12  19:22:41  minman
 * Initial revision
 * 
 *
 */

#define __CYBRND_SRC

#include "cybrnd.h"

RNDSTREAM_STD(start_rnd);

void rnd_init(void)
{
   long  random_seed;
   long  seed;

   // try to get a completely random value - take the bottom two bytes to be
   // the random seed
//   _bios_timeofday(0, &random_seed);
   random_seed = TickCount();
   RndSeed(&start_rnd, (random_seed & 0x0000FFFF));

   // use config seed - otherwise use a random seed
   //KLC - For Mac, just hard-code values from the config file.
//   seed = (config_get_value("damage_seed", CONFIG_INT_TYPE, &data, &count)) ?
//      data : RndRange(&start_rnd, 0, 0xFFFF);
   seed = RndRange(&start_rnd, 0, 0xFFFF);
   RndSeed(&damage_rnd, seed);

//   seed = (config_get_value("grenade_seed", CONFIG_INT_TYPE, &data, &count)) ?
//      data : RndRange(&start_rnd, 0, 0xFFFF);
   seed = RndRange(&start_rnd, 0, 0xFFFF);
   RndSeed(&grenade_rnd, seed);

//   seed = (config_get_value("obj_make_seed", CONFIG_INT_TYPE, &data, &count)) ?
//      data : RndRange(&start_rnd, 0, 0xFFFF);
   seed = RndRange(&start_rnd, 0, 0xFFFF);
   RndSeed(&obj_make_rnd, seed);

//   seed = (config_get_value("effect_seed", CONFIG_INT_TYPE, &data, &count)) ?
//      data : RndRange(&start_rnd, 0, 0xFFFF);
   seed = RndRange(&start_rnd, 0, 0xFFFF);
   RndSeed(&effect_rnd, seed);
}

