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

#include <stdlib.h>
#include <stdio.h>

#include "lg.h"
#include "array.h" 


//void main(int argc, char* argv[])
void main(void)
{
   errtype err;
   Array a;
   int i,j;
//   if (argc > 1) srand(atoi(argv[1]));
   array_init(&a,sizeof(int),10);
   for(i = 0; i < 5; i++)
   {
      err = array_newelem(&a,&j);
      printf("Added an element: i = %d, j = %d, err = %d\n",i,j,err);
   }
   for (i = 0; i < 40; i++)
      if (rand()%3 > 0)
      {
         err = array_newelem(&a,&j);
         printf("Added an element: j = %d, err = %d vec = %x fvec = %x\n",j,err,a.vec,a.freevec);
      }
      else
      {
         j = rand()%(a.fullness);
         err = array_dropelem(&a,j);
         printf("dropped elem %d, err = %d\n",j,err);

      }
   printf("vecsize = %d\n",a.vecsize);
   array_destroy(&a);
   printf("vecsize = %d\n",a.vecsize);

}

