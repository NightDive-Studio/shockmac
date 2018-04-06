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
// Here is the high and mighty Dirac Frame header file, courtesy of Mr. Robert Fermier...
// ======================================================================================


//	Struct...
//	---------
typedef struct {

fix	mass,
	hardness,
	roughness,
	gravity;
	
fix	corners[10][4];

} Dirac_frame;

physics_handle EDMS_make_Dirac_frame( Dirac_frame *d, State *s );
void           EDMS_get_Dirac_frame_viewpoint( physics_handle ph, State *s );
void           EDMS_set_Dirac_frame_parameters( physics_handle ph, Dirac_frame *d );
void           EDMS_get_Dirac_frame_parameters( physics_handle ph, Dirac_frame *d );
void           EDMS_control_Dirac_frame( physics_handle ph, fix forward, fix pitch, fix yaw, fix roll );
