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
#define __MAP_SRC
/*
 * $Source: r:/prj/cit/src/RCS/map.c $
 * $Revision: 1.15 $
 * $Author: xemu $
 * $Date: 1994/11/21 06:09:42 $
 *
 */

#include "map.h"
#include "schedule.h"
#include "statics.h"

//------------------------
//  Defines
//------------------------
#define MAP_SCHEDULE_SIZE 128

//------------------------
//  Prototypes
//------------------------
errtype _map_init_elem(FullMap *fmap, int i, int j);


errtype _map_init_elem(FullMap *fmap, int i, int j)
{
   MapElem *me = FULLMAP_GET_XY(fmap,i,j);
   // clear tiletype
   me_tiletype_set(me, 0);

   // clear tmaps[3]
   me_tmap_flr_set(me, 0);
   me_tmap_wall_set(me, 0);
   me_tmap_ceil_set(me, 0);

   // clear flags
   me_flags_set(me, 0);

   // clear heights[2]
   me_height_flr_set(me, 0);
   me_height_ceil_set(me, 0);

   // clear param
   me_param_set(me, 0);

   // clear objRef
   me_objref_set(me, 0);

   me_templight_flr_set(me,0);
   me_templight_ceil_set(me,0);

   me_clearsolid_set(me,0);
   me_subclip_set(me,0xFF); // secret gnosis good
   me_rotflr_set(me,0);
   me_rotceil_set(me,0);
   me_hazard_bio_set(me,0);
   me_hazard_rad_set(me,0);
   me_flicker_set(me,0);

   return(OK);
}

FullMap* map_create(int xshf, int yshf,int zshf, bool cyb)
{
	int i,j;
	FullMap* fmap = (FullMap *)NewPtr(sizeof(FullMap));
	fmap->x_shft = xshf;
	fmap->y_shft = yshf;
	fmap->z_shft = zshf;
	fmap->x_size = 1 << xshf;
	fmap->y_size = 1 << yshf;
	fmap->cyber = cyb;
	fmap->map = (MapElem *)static_map;
	for (i=0;i<fmap->x_size;i++)
	{
		for (j=0;j<fmap->y_size;j++)
		{
			_map_init_elem(fmap,i,j);
		}
	}
	fmap->x_scale = fmap->y_scale = fmap->z_scale = 0;
	for (i = 0; i < NUM_MAP_SCHEDULES; i++)
		schedule_init(&fmap->sched[i],MAP_SCHEDULE_SIZE,FALSE);
	return fmap;
}

bool map_set_default(FullMap *fmap)
{
   global_fullmap=fmap;
   global_map=fmap->map;
   return TRUE;
}

void map_init(void)
{
   FullMap* ourmap = map_create(DEFAULT_XSHF,DEFAULT_YSHF,DEFAULT_ZSHF,FALSE);
   map_set_default(ourmap);
}

void map_free(void)
{
	extern errtype schedule_free(Schedule* s);

	for (int i = 0; i < NUM_MAP_SCHEDULES; i++)
		schedule_free(&global_fullmap->sched[i]);
	DisposePtr((Ptr)fm_map(global_fullmap));
	DisposePtr((Ptr)global_fullmap);
}
