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
#define safe_EDMS_get_state(ph,pst) if (ph != -1) EDMS_get_state(ph,pst)
#define safe_EDMS_kill_object(ph) if (ph != -1) EDMS_kill_object(ph)
#define safe_EDMS_ai_control_robot(ph,a,b,c,d,e,f) if (ph != -1) EDMS_ai_control_robot(ph,a,b,c,d,e,f)
#define safe_EDMS_get_pelvic_viewpoint(ph,st) if (ph != -1) EDMS_get_pelvic_viewpoint(ph,st)
