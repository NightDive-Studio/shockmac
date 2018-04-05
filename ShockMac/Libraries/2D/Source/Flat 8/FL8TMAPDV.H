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
#if (defined(powerc) || defined(__powerc))	

#ifdef __cplusplus
extern "C"
{
#endif
extern fix fix_mul_asm_safe(fix a, fix b);
#ifdef __cplusplus
}
#endif

#else

extern asm fix fix_mul_asm_safe(fix a, fix b);

#endif

// InvDIv = 0 to use divs in mappers, !=0 to use inverse multiplies
#define InvDiv 1

// this macro does a safe divide on a light, which we can shift up for more precision
#define fix_mul_asm_safe_light(a,b) ((fix_mul_asm_safe(a,b)+0x00FF)>>8)
