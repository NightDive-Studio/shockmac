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
// $Source: n:/project/lib/src/2d/RCS/devtab.asm $
// $Revision: 1.2 $
// $Author: kaboom $
// $Date: 1993/05/18 12:14:53 $
//
// List of device driver function tables.
//
// This file is part of the 2d library.
//
// $Log: devtab.asm $
// Revision 1.2  1993/05/18  12:14:53  kaboom
// Changed to use dseg.inc for data segment declaration.
// 
// Revision 1.1  1993/04/29  16:45:14  kaboom
// Initial revision
// 

#include "MacDev.h"

typedef void (**ptr_type)();

void (**grd_device_table_list[])() = {(ptr_type) mac_device_table};

