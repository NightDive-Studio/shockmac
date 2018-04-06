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
 * $Source: r:/prj/cit/src/RCS/cone.c $
 * $Revision: 1.48 $
 * $Author: tjs $
 * $Date: 1994/08/29 00:18:59 $
 */

#include <string.h>
#include <stdlib.h>

#include "cone.h"
#include "map.h"

#include "fr3d.h"
#include "frparams.h"
#include "frspans.h"
#include "frflags.h"
#include "tools.h"

extern uint _fr_curflags;
#define PRINT_PYRAMID

// temp macros
#define GAME_HEIGHT (fix_make(-(1 << SLOPE_SHIFT_D), 0))

#define MAP_X        (fix_make(MAP_XSIZE, 0)-fix_make(0,1))
#define MAP_Y        (fix_make(MAP_YSIZE, 0)-fix_make(0,1))
#define FIX_ZERO     (fix_make(0, 0))
#define MAX_PTS      8
 
#define FIX_EPSILON (0x00000010)

#define FIX_SQRT_MAX 0x005a8279
#define THREE_QUARTERS_PI  ((FIXANG_PI * 3)/4)

#define print_point(pt) (print_fix_point(pt.x, pt.y))

// Returns the z component of a cross product
#define FIX_CROSS_DIRECTION(line, point) \
   (fix_mul((line.end.x - line.start.x), (point.y - line.start.y)) - \
    fix_mul((point.x - line.start.x), (line.end.y - line.start.y)))

// Does a (x <= y <= z) check
//#define IN_BETWEEN(x,y,z) (((x) <= (y)) && ((y) <= (z)))
#define IN_BETWEEN(x,y,z) ((((x) <= (y)) && ((y) <= (z))) || \
                            (((x) >= (y)) && ((y) >= (z))))

#ifdef min
#undef min
#endif
#define min(x,y) (((x) < (y)) ? (x) : (y))

#ifdef max
#undef max
#endif
#define max(x,y) (((x) > (y)) ? (x) : (y))

typedef struct {
   fix x;
   fix y;
} fix_point;

typedef struct {
   fix_point   start;
   fix_point   end;
} fix_line;

// Allocation for the view cone list
fix view_cone_list[MAX_PTS*2];
int view_count = 0;
g3s_vector main_view_vectors[4];

// wow is this ugly
extern g3s_vector viewer_position;
extern g3s_angvec viewer_orientation;

// prototypes
void reverse_poly_list(int index, fix *new_pts);
bool clockwise_poly(int index, fix *poly_pts);
int insert_viewer_position(int index, fix *new_pts, fix_point viewer_point);
int radius_fix(int index, fix *new_pts, fix_point viewer);
void intersect_cone_sides(fix *vlist, int n, fix y_min, fix y_max, int v_left, int v_right, int v_max);

// -----------------------------------------------------
// reverse_poly_list()
//
// reverses the poly list

void reverse_poly_list(int index, fix *new_pts)
{
   fix   temp_pts[MAX_PTS * 2];
   int   i;
   int   n;

   // Copy over the raw data to the temp list
   LG_memcpy(temp_pts, new_pts, sizeof(fix) * 2 * index);

   // copy the data back, but in reverse order
   // (fastest way????)
   for (i=0,n=(index-1); i< index; i++, n--)
   { 
      new_pts[i*2] = temp_pts[n*2];
      new_pts[i*2+1] = temp_pts[n*2+1];
   }
}

// -------------------------------------------
// clockwise_poly()
//
// Returns TRUE if polygon's verticies are in clockwise order.
// Special cases: If there are less than 3 verticies, will return TRUE.
// If there are three colinear points, returns TRUE.
// 
// Note: Does not handle points that are really really close together. 

bool clockwise_poly(int index, fix *poly_pts)
{
   fix         temp_pts[MAX_PTS * 2];
   fix_line    poly_line;
   fix_point   point;
   fix         cross_prd;
   bool        clockwise;
   bool        extra_div = TRUE;

   if (index <3)
      return(TRUE);  // Is a point or line clockwise??? - hmmmmmm, why not?

   LG_memcpy(temp_pts, poly_pts, sizeof(fix) * 2 * index);
   while (extra_div)
   {
      int      i;

      extra_div = FALSE;
      for (i=0;i<(index*2); i++)
         if (temp_pts[i] > FIX_SQRT_MAX)
         {
            extra_div = TRUE;
            break;
         }
      if (extra_div)
         for (i=0; i<(index*2); i++)
            temp_pts[i] = (temp_pts[i] > FIX_ZERO) ? temp_pts[i] >> 4 : -((fix_abs(temp_pts[i])) >> 4);
   }

   poly_line.start.x = temp_pts[0];
   poly_line.start.y= temp_pts[1];// fix_div(poly_pts[1], FIX_SQRT_MAX);
   poly_line.end.x = temp_pts[2]; //fix_div(poly_pts[2], FIX_SQRT_MAX);
   poly_line.end.y = temp_pts[3]; // fix_div(poly_pts[3], FIX_SQRT_MAX);
   point.x = temp_pts[4]; // fix_div(poly_pts[4], FIX_SQRT_MAX);
   point.y = temp_pts[5]; // fix_div(poly_pts[5], FIX_SQRT_MAX);

   cross_prd = FIX_CROSS_DIRECTION(poly_line, point);
   if (cross_prd == FIX_ZERO)
   {
      if (index == 3)      // Another Straight line - this is different though
         clockwise = TRUE;
      else
      {
         point.x = temp_pts[6]; // fix_div(poly_pts[6], FIX_SQRT_MAX);
         point.y = temp_pts[7]; // fix_div(poly_pts[7], FIX_SQRT_MAX);
         cross_prd = FIX_CROSS_DIRECTION(poly_line, point);
         if (cross_prd == FIX_ZERO)
         {
            // Warning(("We've got a problem: Four colinear points.\n"));
            clockwise = TRUE;
         }
         else
            clockwise = (cross_prd < FIX_ZERO);
      }
   }
   else
      clockwise = (cross_prd < FIX_ZERO);

   return(clockwise);
}

// --------------------------------------------------------------------
// insert_viewer_position()
// 
// Inserts the viewer_point into the polygon, if the viewer_point does
// not lie within the polygon. 
//
// Requires: verticies of polygon to be in clockwise order

int insert_viewer_position(int index, fix *new_pts, fix_point viewer_point)
{
   fix_line    poly_line;
   fix_point   vpoint;
   fix         temp_pts[MAX_PTS * 2];
   fix         *current_pt;
   fix         cross_prd;
   int         i;
   int         insert;
   
   bool        extra_div = TRUE;
   bool        inside = FALSE;

   // Reduce values to a reasonable number, so cross product is happy
   // Copy over the raw data to the temp list
   LG_memcpy(temp_pts, new_pts, sizeof(fix) * 2 * index);
   vpoint.x = viewer_point.x;
   vpoint.y = viewer_point.y;
      
   while (extra_div)
   {
      extra_div = FALSE;
      for (i=0; i<(index*2); i++)
         if (temp_pts[i] > FIX_SQRT_MAX)
         {
            extra_div = TRUE;
            break;
         }
      if ((!extra_div) && ((viewer_point.x > FIX_SQRT_MAX) || (viewer_point.y > FIX_SQRT_MAX)))
         extra_div = TRUE;

      if (extra_div)
      {
         // Reduce values to a reasonable number, so cross product is happy
         for (i=0; i<(index*2); i++)
            temp_pts[i] = (temp_pts[i] > FIX_ZERO) ? temp_pts[i] >> 4 : -((fix_abs(temp_pts[i])) >> 4);

         vpoint.x = (vpoint.x > FIX_ZERO) ? vpoint.x >> 4 : -((fix_abs(vpoint.x)) >> 4);
         vpoint.y = (vpoint.y > FIX_ZERO) ? vpoint.y >> 4 : -((fix_abs(vpoint.y)) >> 4);
      }
   }

   poly_line.end.x = temp_pts[(index-1)*2];
   poly_line.end.y = temp_pts[(index-1)*2+1];

   insert = index; // Represents the point being in the polygon.

   for (i=0, current_pt = temp_pts; i<index;i++)
   {
      poly_line.start = poly_line.end;
      poly_line.end.x = *(current_pt++);
      poly_line.end.y = *(current_pt++);

      cross_prd = FIX_CROSS_DIRECTION(poly_line, vpoint);
      if (cross_prd == FIX_ZERO)
      {
         // Do something smart about colinear stuff.
         if ((IN_BETWEEN(poly_line.start.x, vpoint.x, poly_line.end.x)) &&
             (IN_BETWEEN(poly_line.start.y, vpoint.y, poly_line.end.y)))
         {
            return(index);
         }
         else if ((IN_BETWEEN(vpoint.x, poly_line.start.x, poly_line.end.x)) &&
             (IN_BETWEEN(vpoint.y, poly_line.start.y, poly_line.end.y)))
         {
            current_pt -= 4;
            *(current_pt++) = vpoint.x;
            *(current_pt++) = vpoint.y;
            return(index);
         }
         else
         {
            current_pt -= 2;
            *(current_pt++) = vpoint.x;
            *(current_pt++) = vpoint.y;
            return(index);
         }
      }
      else if (cross_prd > FIX_ZERO)
      {
         fix         slope, inv_slope;
         fix         inter, inv_inter;
         fix         interx, intery;
         bool        local_inside;

         // check if we don't have to compute the intersection point
         if (poly_line.end.x == poly_line.start.x)
         {
            interx = poly_line.start.x;
            intery = vpoint.y;
         }
         else if (poly_line.end.y == poly_line.start.y)
         {
            interx = vpoint.x;
            intery = poly_line.start.y;
         }
         else
         {
            slope = fix_div((poly_line.end.y - poly_line.start.y), (poly_line.end.x - poly_line.start.x));
            inv_slope = -fix_div(fix_make(1,0), slope);
            inter = poly_line.end.y - fix_mul(poly_line.end.x, slope);
            inv_inter = vpoint.y - fix_mul(vpoint.x, inv_slope);

            interx = fix_div((inter - inv_inter), (inv_slope - slope));
            intery = fix_mul(slope, interx) + inter;
         }

         local_inside = (IN_BETWEEN(poly_line.start.x, interx, poly_line.end.x) &&
               IN_BETWEEN(poly_line.start.y, intery, poly_line.end.y));
         
         if (inside)
         {
            if (local_inside)
            {
//            	Warning(("Outside two vectors of polygon: Insert - %d. Index - %d.\n", insert, i));
//               Warning(("vpoint:\n"));
//               warning_fix_point(vpoint.x, vpoint.y);
//               Warning(("Modified Poly List:\n"));
 //              warning_poly_list(index, temp_pts);
//               Warning(("Viewer Point:\n"));
//               warning_fix_point(viewer_point.x, viewer_point.gY);
//               Warning(("Original Poly List:\n"));
//               warning_poly_list(index, new_pts);
//               Warning(("Finding Art would probably be a very good thing!!!!\n"));
//               mprintf(("View vectors:\n"));
//               print_view_vectors(0, 0, 0L);
            }
         }
         else
         {
            insert = i;
            inside = local_inside;
         }
      }
   }

   if (insert != index)
   {
      // shift data over, so we can insert the viewer point

      current_pt = new_pts + (insert*2);
      LG_memmove(current_pt+2, current_pt, sizeof(fix) * 2 * (index - insert));
      *(current_pt) = viewer_point.x;
      *(current_pt+1) = viewer_point.y;
      insert = index + 1;
   }
   return(insert);
}

// --------------------------------------------------------
// radius_fix()
//
// Takes the points and a center view point and rearranges the
// points so they are in order in a circle (counter-clockwise)

int radius_fix(int index, fix *new_pts, fix_point viewer)
{
   fix_line    poly_line;
   fix_point   test_pt;
   int         i, j, k;
   int         new_index;
   int         counter;
   bool        middle;
   bool        second, third;
   fix         x, y, x2, y2;
   fix         *current_pt;
   fix         *insert_pt;
   fix         tempx, tempy;
   fix         temp_pts[MAX_PTS * 2];
   fixang      pt_ang[MAX_PTS];
   fixang      tempang;
   fix         cross_prd;

   if (index > 4)
      ; // Warning(("Too many verticies\n"));
   if (index < 3)
      return(index);

   // Find the ArcTans for each vertex, but also divide it by the
   // larger of the two values to get values under 1. Don't normalize
   // cause it's not necessary and it's expensive.
   for (i=0;i<index;i++)
   {
      x = (new_pts[i*2] - viewer.x);
      y = (new_pts[(i*2)+1] - viewer.y);
      if (fix_abs(y) > fix_abs(x))
      {
         x = fix_div(x,fix_abs(y));
         y = (y < FIX_ZERO) ? fix_make(-1,0) : fix_make(1,0);
      }
      else
      {
         y = fix_div(y, fix_abs(x));
         x = (x < FIX_ZERO) ? fix_make(-1,0) : fix_make(1,0);
      }
      pt_ang[i] = fix_atan2(y, x);
   }

   // Sort the list of angles
   for (i=1; i<index; i++)
   {
      for (j=(index-1); j>=i; j--)
      {
         if (pt_ang[j-1] > pt_ang[j])
         {
            k = j*2;
            tempx = new_pts[k-2]; tempy = new_pts[k-1]; tempang = pt_ang[j-1];
            new_pts[k-2] = new_pts[k]; new_pts[k-1] = new_pts[k+1]; pt_ang[j-1] = pt_ang[j];
            new_pts[k] = tempx; new_pts[k+1] = tempy; pt_ang[j] = tempang;
         }
      }
   }

   // Check for overlap
   LG_memcpy(temp_pts, new_pts, sizeof(fix) * 2 * index);
   for (i=1, j=0; i<index; i++)
   {
      if (pt_ang[i-1] >= FIXANG_PI)
         break;
      if ((pt_ang[i-1]+THREE_QUARTERS_PI) < pt_ang[i])
      {
         for (j=0; j<(index-i);j++)
         {
            new_pts[j*2] = temp_pts[(i+j)*2];
            new_pts[j*2+1] = temp_pts[(i+j)*2+1];
         }
         for (k=0; k<(index-j);k++)
         {
            new_pts[(j+k)*2] = temp_pts[k*2];
            new_pts[(j+k)*2+1] = temp_pts[k*2+1];
         }

         break;
      }
   }

   // Check the first pair of verticies for colinearity. (is that a word??)
   current_pt = new_pts;
   x = *(current_pt);      y = *(current_pt+1);
   x2 = *(current_pt+2);   y2 = *(current_pt+3);

   new_index = 1;
   insert_pt = current_pt+2;
   middle = FALSE;
   if (!((x==x2) || (y==y2)))
   {
      if (index == 3)
      {
         // If there are 3 verticies, then we save the middle to be
         // checked with the last vertex.
         middle = TRUE;
         current_pt +=2;
      }
      else
      {
         new_index++;
         current_pt += 4;
         insert_pt += 2;
      }
   }
   else
   {
      // Remove colinear vertex
      current_pt +=4;
   }

   // Check the last pair of verticies.
   // If we only have three verticies, and we have already removed
   // one vertex, we do not need to do the following since the last
   // vertex does not belong to a pair.
   // Unless of course, we did not remove a vertex, and therefore 
   // the middle vertex joins the last vertex as the pair.
   if ((index != 3) || middle)
   {
      x = *(current_pt);      y = *(current_pt+1);
      x2 = *(current_pt+2);   y2 = *(current_pt+3);
   
      // Again, check if we have a vertex on the same axis, and punt if the
      // "first" is inward of the second one(outer most).
      if ((x==x2) || (y==y2))
      {
         // Punt the inner vertex
         current_pt +=2;
         *(insert_pt++) = *(current_pt++);
         *(insert_pt++) = *(current_pt++);
         new_index++;
      }
      else
      {
         for (i=0; i<4;i++)
            *(insert_pt++) = *(current_pt++);
         new_index +=2;
      }
   }
   else // We have three verticies and we removed one vertex.
   {
      *(insert_pt++) = *(current_pt++);
      *(insert_pt++) = *(current_pt++);
      new_index++;
   }

   if (new_index > 2)
   {
      bool extra_div = TRUE;

      // This will remove convexness from the polygon
      // We are assuming counter-clockwise, due to sorting by angles.

      second = third = TRUE;

      LG_memcpy(temp_pts, new_pts, sizeof(fix) * 2 * index);

      // shrink down values to compensate for fix-point limitations
      while (extra_div)
      {
         extra_div = FALSE;
         for (i=0;i<(index*2); i++)
            if (temp_pts[i] > FIX_SQRT_MAX)
            {
               extra_div = TRUE;
               break;
            }
         if (extra_div)
            for (i=0; i<(index*2); i++)
               temp_pts[i] /= 10;
      }

      poly_line.start.x = temp_pts[0];
      poly_line.start.y = temp_pts[1];
      poly_line.end.x = temp_pts[4];
      poly_line.end.y = temp_pts[5];
      test_pt.x = temp_pts[2];
      test_pt.y = temp_pts[3];

      cross_prd = FIX_CROSS_DIRECTION(poly_line, test_pt);
      if (cross_prd >= FIX_ZERO)
         second = FALSE;

      if (new_index == 4)
      {
         poly_line.end.x = temp_pts[6];
         poly_line.end.y = temp_pts[7];
      
         cross_prd = FIX_CROSS_DIRECTION(poly_line, test_pt);
         if (cross_prd >= FIX_ZERO)
            second = FALSE;

         test_pt.x = temp_pts[4];
         test_pt.y = temp_pts[5];

         cross_prd = FIX_CROSS_DIRECTION(poly_line, test_pt);
         if (cross_prd >= FIX_ZERO)
            third = FALSE;
         else
         {
            poly_line.start.x = temp_pts[2];
            poly_line.start.y = temp_pts[3];

            cross_prd = FIX_CROSS_DIRECTION(poly_line, test_pt);
            if (cross_prd >= FIX_ZERO)
               third = FALSE;
         }
      }

      counter = (second) ? 2 : 1;
      insert_pt = new_pts + (2 * counter);

      if (third)
      {
         *(insert_pt++) = new_pts[4];
         *(insert_pt++) = new_pts[5];
         counter++;
      }
      
      if (new_index == 4)
      {
         *(insert_pt++) = new_pts[6];
         *(insert_pt++) = new_pts[7];
         counter++;
      }
   }
   else
      counter = new_index;

   return(counter);
}


// ----------------------------------------------------------------
// find_view_area()
//
// modifies an array of points to represents the view area in clockwise order 
// *count will have the number of points in the array.

bool find_view_area(fix *cone_list, fix floor_val, fix roof_val, int *count, fix radius)
{
   int         i;
   fix         tx, tz;
   fix         *new_pts;
   int         index = 0;
   fix         ratiox, ratioy, ratioz;
   g3s_vector  my_view[4];
   fix         radius_square;
   fix         x_val;
   fix         z_val;
   fix         len;
   fix         *current_pt;
   fix_point   viewer_point;
   grs_clip    old_clip;
   fix         height = 0;
//   char        fix_buffer[80];

   if (radius <= fix_make(0,0))
   {
      *count = 0;
      return (FALSE);
   }

   radius_square = (radius >= FIX_SQRT_MAX) ? FIX_MAX : fix_mul(radius, radius);

   new_pts = cone_list;
   viewer_point.x = viewer_position.gX;
   viewer_point.y = viewer_position.gZ;

   g3_get_view_pyramid(my_view);

   if (((_fr_curflags & FR_CURVIEW_MASK) == FR_CURVIEW_STRT) && !(_fr_curflags & FR_HACKCAM_FLAG))
      LG_memcpy(main_view_vectors, my_view, sizeof(g3s_vector) * 4);

   // check if we're looking completely up, or completely down
   // if so, we can do something fast
   if ((my_view[0].gY > FIX_ZERO) && (my_view[1].gY > FIX_ZERO)
      && (my_view[2].gY > FIX_ZERO) && (my_view[3].gY > FIX_ZERO))
   {
      height = floor_val;
      for (i=0;i<4;i++)
      {
         if (my_view[i].gY < fix_make(0,0x0080))
            my_view[i].gY = fix_make(0,0x0080);
      }
   }
   else if ((my_view[0].gY < FIX_ZERO) && (my_view[1].gY < FIX_ZERO)
      && (my_view[2].gY < FIX_ZERO) && (my_view[3].gY < FIX_ZERO))
   {
      height = roof_val;
      for (i=0;i<4;i++)
      {
         if (my_view[i].gY > -fix_make(0,0x0080))
            my_view[i].gY = -fix_make(0,0x0080);
      }
   }

   if (height != 0)
   {
      index = 4;
      // Find all the "raw" values without clipping
      for (i=0; i<4;i++)
      {
         ratioy = fix_div((height - viewer_position.gY),my_view[i].gY);
         x_val = fix_mul(ratioy, my_view[i].gX);
         z_val = fix_mul(ratioy, my_view[i].gZ);

         if (radius_square == FIX_MAX)
         {
            new_pts[i*2]      = viewer_position.gX + x_val;
            new_pts[i*2+1]    = viewer_position.gZ + z_val;
         }
         else
         {
            // deals with fix_point limitations - must shift down so we don't square over 65536
            if ((fix_abs(x_val) > FIX_SQRT_MAX) || (fix_abs(z_val) > FIX_SQRT_MAX))
            {
               len = fix_mul((fix_abs(x_val)>>9), (fix_abs(x_val)>>9))+fix_mul((fix_abs(z_val)>>9), (fix_abs(z_val)>>9));
               if (len <= (radius_square>>18))
               {
                  new_pts[i*2]=viewer_position.gX+x_val;
                  new_pts[i*2+1]=viewer_position.gZ+z_val;
               }
               else
               {
                  len = fix_sqrt(len) << 9;
                  new_pts[i*2]=viewer_position.gX+fix_mul(fix_div(radius,len), x_val);
                  new_pts[i*2+1]=viewer_position.gZ+fix_mul(fix_div(radius,len), z_val);
               }
            }
            else
            {
               len = fix_mul(x_val,x_val) + fix_mul(z_val, z_val);
               if (len <= radius_square)
               {
                  new_pts[i*2]      = viewer_position.gX + x_val;
                  new_pts[i*2+1]    = viewer_position.gZ + z_val;
               }
               else
               {
                  len = fix_sqrt(len);
                  new_pts[i*2]      = viewer_position.gX + fix_mul(fix_div(radius, len), x_val); //(fix_div(fix_mul(x_val, radius), len);
                  new_pts[i*2+1]    = viewer_position.gZ + fix_mul(fix_div(radius, len), z_val); //fix_div(fix_mul(z_val, radius), len);
               }
            }
         }
      }
      // Make the polygon clockwise, if it isn't already
      if (!clockwise_poly(index, new_pts))
      {
         reverse_poly_list(index, new_pts);
      }
   }
   else
   {
      index = 4;
      current_pt = new_pts;
      for (i=0; i<4; i++)
      {
         // Check for duplicate verticies
         if ((my_view[i].gX == my_view[(i+1)%4].gX) && (my_view[i].gZ == my_view[(i+1)%4].gZ))
         {
            index--;
            continue;
         }
         if (radius_square == FIX_MAX)
         {
            // Find direction of this vector
            if (my_view[i].gX < FIX_ZERO)
            {
               tx = FIX_ZERO;   ratiox = fix_div(FIX_ZERO - viewer_position.gX, my_view[i].gX);
            }
            else if (my_view[i].gX > FIX_ZERO)
            {
               tx = MAP_X;      ratiox = fix_div(MAP_X - viewer_position.gX, my_view[i].gX);
            }
            else
            {
               tx = 0;          ratiox = FIX_MIN;
            }
           
            if (my_view[i].gZ < FIX_ZERO)
            {
               tz = FIX_ZERO;   ratioz = fix_div(FIX_ZERO - viewer_position.gZ, my_view[i].gZ);
            }
            else if (my_view[i].gZ > FIX_ZERO)
            {
               tz = MAP_Y;      ratioz = fix_div(MAP_Y - viewer_position.gZ, my_view[i].gZ);
            }
            else
            {
               tz = 0;          ratioz = FIX_MIN;
            }
            if ((ratiox == FIX_MIN) && (ratioz == FIX_MIN))
            {
               index--;
               continue;
            }

            if ((ratiox < FIX_ZERO) && (ratioz < FIX_ZERO))
            {
               if ((viewer_position.gX < MAP_X) && (viewer_position.gX > FIX_ZERO)
                  && (viewer_position.gZ < MAP_Y) && (viewer_position.gZ > FIX_ZERO))
               {
                  // Warning(("Negative Ratios for cone clip - inside the map!!!!\n"));
                  // print_view_vectors(0, 0L, 0);
                  // Warning(("Go Find Art!\n"));
               }
               index--;
               continue;
            }

            if (ratiox < ratioz)
               tx = viewer_position.gX + fix_mul(ratioz, my_view[i].gX);
            else if (ratiox > ratioz)
               tz = viewer_position.gZ + fix_mul(ratiox, my_view[i].gZ);
         }
         else
         {
            if (fix_abs(my_view[i].gX) > fix_abs(my_view[i].gZ))
            {
               tx = (my_view[i].gX < 0) ? -radius : radius;
               tz = fix_mul(fix_div(tx, my_view[i].gX), my_view[i].gZ);
            }
            else
            {
               tz = (my_view[i].gZ < 0) ? -radius : radius;
               tx = fix_mul(fix_div(tz, my_view[i].gZ), my_view[i].gX);
            }

            len = fix_sqrt(fix_mul(tx, tx) + fix_mul(tz, tz));
            tx = viewer_position.gX + fix_mul(fix_div(radius, len), tx); //fix_div(fix_mul(tx, radius), len);
            tz = viewer_position.gZ + fix_mul(fix_div(radius, len), tz); // fix_mul(tz, radius), len);
         }
         *(current_pt++) = tx;  *(current_pt++) = tz;
      }

      if (index < 2)
         ; // Warning(("HEY - Only one point for cone - this is bad....\n"));
 
      index = radius_fix(index, new_pts, viewer_point);
      reverse_poly_list(index, new_pts);
   }

   index = insert_viewer_position(index, new_pts, viewer_point);


   // Clip the polygon
   old_clip.f = grd_fix_clip;
   gr_set_fix_cliprect(FIX_ZERO, FIX_ZERO, MAP_X, MAP_Y);
   index = gr_clip_fix_poly(index, new_pts, new_pts);
   grd_fix_clip = old_clip.f;


// I don't think we need this stuff, and when it gets called - it
// causes bad things to happen!!!
//   if (!clockwise_poly(index, new_pts))
//   {
//      reverse_poly_list(index, new_pts);
//      mprintf("I guess we need this darn thing, or is it a bug????\n\n");
//   }

   *count = index;

   if (index > 2)
      return(TRUE);
   else
      return(FALSE);
}

// ---------------------------------------------
// intersect_cone_sides
//

fix   span_lines[8];
byte  span_index[2];
fix   span_intersect[4];

void intersect_cone_sides(fix *vlist, int n, fix y_min, fix y_max, int v_left, int v_right, int v_max)
{
   fix         deltax, deltay;
   int         s_left, s_right;
   fix         y;

   // get the viewer's position - and take the bottom
//   y = fix_trunc(viewer_position.gZ);
   y=viewer_position.gZ;

   if ((y_min == y) || (y_max == y))
   {
      if (y_max == y)
      {
         v_left = v_right = v_max;

         while (vlist[2*((v_left+n-1)%n)+1] == y_max)
            v_left = (v_left+n-1)%n;
         while (vlist[2*((v_right+1)%n)+1] == y_max)
            v_right = (v_right+1)%n;
      }
      // do the left side
      span_lines[0] = vlist[2*((v_left+n-1)%n)] - vlist[2*v_left];
      span_lines[1] = vlist[2*((v_left+n-1)%n)+1] - vlist[2*v_left+1];
      span_lines[2] = vlist[2*((v_left+1)%n)] - vlist[2*v_left];
      span_lines[3] = vlist[2*((v_left+1)%n)+1] - vlist[2*v_left+1];
      span_index[0] = -(v_left+1);
      span_intersect[0] = vlist[2*v_left];
      span_intersect[1] = vlist[2*v_left+1];

      // do the right side
      span_lines[4] = vlist[2*((v_right+n-1)%n)] - vlist[2*v_right];
      span_lines[5] = vlist[2*((v_right+n-1)%n)+1] - vlist[2*v_right+1];
      span_lines[6] = vlist[2*((v_right+1)%n)] - vlist[2*v_right];
      span_lines[7] = vlist[2*((v_right+1)%n)+1] - vlist[2*v_right+1];
      span_index[1] = -(v_right+1);
      span_intersect[2] = vlist[2*v_right];
      span_intersect[3] = vlist[2*v_right+1];
   }
   else
   {
      s_left = v_left;      s_right = v_right;

      while (vlist[2*((s_left+1)%n)+1] < (y-FIX_EPSILON))
         s_left = (s_left+1)%n;

      while (vlist[2*((s_right+n-1)%n)+1] < (y-FIX_EPSILON))
         s_right = (s_right+n-1)%n;

      // first check if crossing is at upper point of vector
      if (y == vlist[2*((s_left+1)%n)+1])
      {
         span_lines[0] = vlist[2*s_left] - vlist[2*((s_left+1)%n)];
         span_lines[1] = vlist[2*s_left+1] - vlist[2*((s_left+1)%n)+1];
         span_lines[2] = vlist[2*((s_left+2)%n)] - vlist[2*((s_left+1)%n)];
         span_lines[3] = vlist[2*((s_left+2)%n)+1] - vlist[2*((s_left+1)%n)+1];
         span_index[0] = -((s_left+1)%n)-1;
         span_intersect[0] = vlist[2*((s_left+1)%n)];
         span_intersect[1] = vlist[2*((s_left+1)%n)+1];
      }
      else
      {
         // do the left side first
         deltax = vlist[2*((s_left+1)%n)] - vlist[2*s_left];
         deltay = vlist[2*((s_left+1)%n)+1] - vlist[2*s_left+1];

         span_lines[0] = -deltax;       span_lines[1] = -deltay;
         span_lines[2] = deltax;        span_lines[3] = deltay;
         span_index[0] = s_left;

         span_intersect[0] = vlist[2*s_left] + fix_mul(fix_div((y-vlist[2*s_left+1]), deltay), deltax);
                                             // fix_div(fix_mul((y-vlist[2*s_left+1]), deltax), deltay);
         span_intersect[1] = y;
      }

      // first check if crossing is at upper point of vector
      if (y == vlist[2*((s_right+n-1)%n)+1])
      {
         span_lines[4] = vlist[2*((s_right+n-2)%n)] - vlist[2*((s_right+n-1)%n)];
         span_lines[5] = vlist[2*((s_right+n-2)%n)+1] - vlist[2*((s_right+n-1)%n)+1];
         span_lines[6] = vlist[2*s_right] - vlist[2*((s_right+n-1)%n)];
         span_lines[7] = vlist[2*s_right+1] - vlist[2*((s_right+n-1)%n)+1];
         span_index[1] = -((s_right+n-1)%n)-1;
         span_intersect[2] = vlist[2*((s_right+n-1)%n)];
         span_intersect[3] = vlist[2*((s_right+n-1)%n)+1];
      }
      else
      {
         // do the right side next
         deltax = vlist[2*((s_right+n-1)%n)] - vlist[2*s_right];
         deltay = vlist[2*((s_right+n-1)%n)+1] - vlist[2*s_right+1];

         span_lines[4] = deltax;        span_lines[5] = deltay;
         span_lines[6] = -deltax;       span_lines[7] = -deltay;
         span_index[1] = (s_right+n-1)%n;

         span_intersect[2] = vlist[2*s_right] + fix_mul(fix_div((y-vlist[2*s_right+1]), deltay), deltax);
                                             //fix_div(fix_mul((y - vlist[2*s_right+1]), deltax), deltay);
         span_intersect[3] = y;
      }
   }
}

// --------------------------------------------
// simple_cone_clip_pass()
//
// Cone clips the area, and calls store_x_span on the
// contents of the cone.
//

void simple_cone_clip_pass(void)
{
   int      n;
   int      i;
   byte     v_min;                     // vertex with smallest y coord 
   byte     v_max;                     // vertex with largest y coord 
   byte     v_left, v_right;           // current left & right vertices
//   byte     v_prev;                    // previous vertex
   int      y;                         // current scanline 
   int      y_top;
   fix      left, right;               // the left and right values on scan line, making sure scan line does not go past end points 
   fix      y_min, y_max;              // min & max vertex y coords 
   fix      y_left, y_right;           // ending y for left & right edges 
   fix      x_min, x_max;              // min & max x coords 
   fix      x_left, x_right;           // scanline x intersections 
   fix      m_prev;                    // previous slopes
   fix      m_left=fix_make(-1,0);
   fix      m_right=fix_make(1,0);           // look - slopes for right/left edges
   fix      d;                         // difference for slope computations
   fix      x_abs_left, x_abs_right;   // min or max value of endpoint for that line of the polygon 
   fix      x_outer_left, x_outer_right; // used to determine if that line is horizontal
   fix      x_shift;
   bool     right_line, left_line;     // looking for line
   bool     right_repeat, left_repeat; // looking for repeat on the line 

   // get the view polygon - if there's not a valid cone, then just return.
   if (!find_view_area(view_cone_list, fix_make(0,0), GAME_HEIGHT, &n, fix_make(_frp.view.radius,0)))
   {
      // so if we don't have a valid cone, let's check if we're in the map first before
      // spewing a warning message

      if ((viewer_position.gX < MAP_X) && (viewer_position.gX > FIX_ZERO)
         && (viewer_position.gZ < MAP_Y) && (viewer_position.gZ > FIX_ZERO))
      {
         // Warning(("Not a valid cone found and we're inside the map!\n"));
      }
      return;
   }

   view_count = n;

   // initialize these to weenie values.
   x_min = y_min = FIX_MAX;
   x_max = y_max = 0;

   // find the y coordinate of the highest and lowest vertices; save the
   // vertex number of the highest. 
   for (i=0; i<n; i++)
   {   
      if (view_cone_list[2*i] < x_min)  x_min = view_cone_list[2*i];
      if (view_cone_list[2*i] > x_max)  x_max = view_cone_list[2*i];
      if (view_cone_list[2*i+1] < y_min)
      {
         y_min = view_cone_list[2*i+1];
         v_min = i;
      }
      if (view_cone_list[2*i+1] > y_max)
      {
         y_max = view_cone_list[2*i+1];
         v_max = i;
      }
   }

   /* check if this is a horizontal line. */
   if (fix_int (y_min) == fix_int (y_max))
   {
      cone_span_set(fix_int(y_min), fix_int(x_min), fix_int(x_max));
      return;
   }

   /* we want to set v_left and v_right to be leftmost and rightmost vertices
      with y = y_min.  usually, both are v_min, but if there is a horizontal
      edge at y = y_min, they will be different. */

   v_left = v_right = v_min;
   while (view_cone_list[2*((v_left+1)%n)+1] == y_min)
      v_left = (v_left+1)%n;
   while (view_cone_list[2*((v_right+n-1)%n)+1] == y_min)
      v_right = (v_right+n-1)%n;

   intersect_cone_sides(view_cone_list, n, y_min, y_max, v_left, v_right, v_max);

   // Check if top line is the max line - if so then decrement;
   y_top = fix_int(y_max);
   if (y_top == MAP_YSIZE)
      y_top = MAP_YSIZE-1;

   /* draw each span, starting at y_min. */
   for (y=fix_int (y_min); y<y_top; y++)
   {
      /* process completed left edge(s). */
      left_line = left_repeat = FALSE;
      x_outer_left = fix_make(-1,0);
      while (fix_int (view_cone_list[2*v_left+1]) == y)
      {
         m_prev = m_left;
         x_left = view_cone_list[2*v_left];
         y_left = view_cone_list[2*v_left+1];
         v_left = (v_left+1)%n;
         if (left_repeat)
         {
            x_outer_left = min(x_outer_left, x_left);
            left_line = TRUE;
         }   
         else
         {
            x_outer_left = x_left;
            if (m_prev > fix_make(0,0))
            {
               left_line = TRUE;
               x_outer_left -= fix_mul(fix_frac(y_left), m_prev);           // save the left point's X coordinate
            }

            left_repeat = TRUE;            // signal that if we do this again - we've repeated
         }
         x_abs_left = min(view_cone_list[2*v_left], x_left);       
         d = view_cone_list[2*v_left+1]-y_left;

         // if the next point is above the current - calculate the slope
         if (fix_int (view_cone_list[2*v_left+1]) > fix_int (y_left))
         {   
            m_left = fix_div (view_cone_list[2*v_left]-x_left, d);

            x_shift = 0;

            // this gets to the leftmost point of the line - either on top or bottom of the pixel
            d = (m_left < 0) ? (fix_make(1,0) - fix_frac(y_left)) : fix_frac(y_left);
            x_shift = fix_abs(fix_mul(d,m_left));

            // shift over - if the line before does for any left over
            if (m_prev > fix_make(0,0))
            {
               d = fix_abs(fix_mul(fix_frac(y_left), m_prev));
               x_shift = max(d, x_shift);
            }
            x_left -= x_shift;
         }
      }

      right_line = right_repeat = FALSE;
      x_outer_right = fix_make(-1,0);

      /* process completed right edge(s). */
      while (fix_int (view_cone_list[2*v_right+1]) == y)
      {
         m_prev = m_right;
         x_right = view_cone_list[2*v_right];
         y_right = view_cone_list[2*v_right+1];
         v_right = (v_right+n-1)%n;
         if (right_repeat)
         {
            x_outer_right = max(x_outer_right, x_right);
            right_line = TRUE;
         }
         else
         {
            x_outer_right = x_right;
            if (m_prev < fix_make(0,0))
            {
               x_outer_right += fix_abs(fix_mul(fix_frac(y_right), m_prev));
               right_line = TRUE;
            }
            right_repeat = TRUE;
         }
         x_abs_right = max(view_cone_list[2*v_right],x_right);
         d = view_cone_list[2*v_right+1]-y_right;

         // if the next point is above the current - calculate the slope
         if (fix_int (view_cone_list[2*v_right+1]) > fix_int (y_right))
         {
            m_right = fix_div (view_cone_list[2*v_right]-x_right, d);

            d = (m_right > 0) ? (fix_make(1,0) - fix_frac(y_right)) : fix_frac(y_right);
            x_shift = fix_abs(fix_mul(d,m_right));

            if (m_prev < fix_make(0,0))
            {
               d = fix_abs(fix_mul(fix_frac(y_right), m_prev));
               x_shift = max(d, x_shift);
            }
            x_right += x_shift;
         }
      }
      /* draw this scanline and calculate x intersections with next. */
      if (fix_int (x_left) <= fix_int (x_right))
      {
         left = x_left;
         right = x_right;

         // checking that we don't go pass the endpoints - x_abs_left
         left = max(x_left, x_abs_left);
         right = min(x_right,x_abs_right);

         // checking for horizontal lines (using x_outer_left)
         // if so, take the leftmost or rightmost point.
         if (left_line)
            left = min(left, x_outer_left);
         if (right_line)
            right = max(right, x_outer_right);

         cone_span_set(y, fix_int(left), fix_int(right));
      }
      x_left += m_left;
      x_right += m_right;
   }

   // This is for the top scan line - which shouldn't be done with the
   // above, because we'd get just the vertex point, if we've got
   // a broad angle. (kindof difficult to explain)
   if (fix_int (x_left) <= fix_int (x_right))
   {
      // checking that we don't go pass the endpoints - x_abs_left
      left = max(x_left, x_abs_left);
      right = min(x_right,x_abs_right);

      cone_span_set(y, fix_int(left), fix_int(right));
   }
}

