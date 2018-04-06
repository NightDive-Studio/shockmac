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
 * $Source: r:/prj/cit/src/RCS/pathfind.c $
 * $Revision: 1.13 $
 * $Author: xemu $
 * $Date: 1994/09/06 23:44:56 $
 */

#define __PATHFIND_SRC

#include <string.h>

#include "pathfind.h"
#include "player.h"
#include "faketime.h"
#include "tilename.h"
#include "otrip.h"
#include "objgame.h"
#include "objprop.h"
#include "gameobj.h"
#include "objbit.h"
#include "cybmem.h"
#include "doorparm.h"

//------------
//  PROTOTYPES
//------------
errtype find_path(char path_id);
short tile_height(MapElem *pme, char dir, bool floor);
uchar pf_obj_height(MapElem *pme, uchar old_z);


// So, the paradigm is that a given creature makes a pathfind request
// At the next suitable time, the pathfinder fills in all the pending requests
// If you want a step from your path and your request hasn't been filled yet,
// then you just wait.

// Each path is a Source location and a set of NSEW movements, 64 move locations
// There are MAX_PATHS available paths, so if lots of things are already pathfinding
// then new requests cannot be made.

// Which char is it?
#define HIGH_STEP(stepnum) ((stepnum) >> 2)
#define LOW_STEP(stepnum) ((stepnum) & 0x3)
#define PATH_CHAR(pathid,stepnum) (paths[(pathid)].moves[HIGH_STEP(stepnum)])
#define PATH_STEP(pathid,stepnum) ((PATH_CHAR(pathid,stepnum) >> (LOW_STEP(stepnum) << 1)) & 0x3)
// Good god, there has got to be a better way to do this! -- Rob
// Clear out the old set of 2 bits, and or in the new 2 bits
#define SET_PATH_STEP(pathid,stepnum,newval)  do {PATH_CHAR(pathid,stepnum) = \
      PATH_CHAR(pathid,stepnum) & ~(0x3 << (LOW_STEP(stepnum) << 1)) | ((newval) << (LOW_STEP(stepnum) << 1)); \
      }  while (0)
#define CLEAR_PATH(pathid) do {LG_memset ((void *)(paths[(pathid)].moves), 0, NUM_PATH_STEPS/4); } while (0)



// Note that a dest_z of 0 means to ignore that whole concept
// dest_z and start_z are in objLoc height coordinates, since that is the easiest API
char request_pathfind(LGPoint source, LGPoint dest, uchar dest_z, uchar start_z, bool priority)
{
   char i = 0;

   // Find the first available path number
   while ((i < MAX_PATHS) && (used_paths & (1 << i)))
      i++;

   // If no paths free, return -1;
   if (i == MAX_PATHS)
   {
      return(-1);
   }
   if (PointsEqual(source,dest))
   {
      return(-1);
   }

   // Grab the path
   used_paths |= (1 << i);

   // Init the path struct
   paths[i].source = source; 
   paths[i].dest = dest;
   // Path height units and API height units are the same, so don't
   paths[i].dest_z = dest_z;
   paths[i].start_z = start_z;
   paths[i].num_steps = priority ? -2 : -1;
   paths[i].curr_step = -1;  // to indicate that we haven't been filled yet

   // clear it
   CLEAR_PATH(i);

   // go
   return(i);
}


// Updates pt to reflect the next step on path path_id from step number step_num.
// pt must already reflect the location reached by the path in step_num steps.
// A step_num of -1 indicates to use the curr_step contained in the path.
// Unlike next_step_on_path, this procedure does not affect the paths array at all,
// although it does modify it's pt parameter.
// Returns direction of that next step
char compute_next_step(char path_id, LGPoint *pt, char step_num)
{
   char movecode = -22;
   if (step_num == -1)
      step_num = paths[path_id].curr_step;
   if (step_num != -1)
   {
      movecode = PATH_STEP(path_id,paths[path_id].num_steps - step_num - 1);
      switch(movecode)
      {
         case 0: pt->y++; break; // N
         case 1: pt->x++; break; // E
         case 2: pt->y--; break; // S
         case 3: pt->x--; break; // W
         default:
            break;
      }
   }
   return(movecode);
}


// Note that this is NOT an idempotent procedure.... as you call it
// it moves "source" along the path and increments the path step
// Since we still have the path information, it is possible to go
// backwards along the path, but I won't write that until it seems
// needed.
// Returns the direction one travels in to get to this next step
char next_step_on_path(char path_id, LGPoint *next, char *steps_left)
{
   char retval;
   *next = paths[path_id].source;
   retval = compute_next_step(path_id, next, -1);  // -1 for "use path data"
   paths[path_id].source = *next;
   paths[path_id].curr_step++;
   if (PointsEqual(*next,paths[path_id].dest))
   {
      *steps_left = 0;
   }
   else
   {
      *steps_left = paths[path_id].num_steps - paths[path_id].curr_step;
   }
   return(retval);
}


#define LOOKAHEAD_STEPS 3
// Checks whether or not we have skipped ahead to some square within LOOKAHEAD_STEPS
// of the "current" location for the specified path.  If so, jumps the path to that point and 
// returns TRUE.
bool check_path_cutting(LGPoint new_sq, char path_id)
{
   LGPoint pt;
   char count;
   // Hey, first check if we've finished the darn path...
   if ((path_id == -1) || (paths[path_id].num_steps == 0))
      return(FALSE);
   if (PointsEqual(new_sq,paths[path_id].dest))
   {
      paths[path_id].num_steps = 0;
      return(TRUE);
   }

   // Now do the standard lookahead check...
   pt = paths[path_id].source;
   for (count = 0; count < LOOKAHEAD_STEPS; count++)
   {
      compute_next_step(path_id, &pt, paths[path_id].curr_step + count);
      if (PointsEqual(pt,new_sq))
      {
         // hey look, we cut ahead to a more advanced point in our pathfinding...
         paths[path_id].source = new_sq;
         paths[path_id].curr_step += count + 1;
         return(TRUE);
      }
   }

   // We didn't find anything, how sad.
   return(FALSE);
}


#define PATHFIND_INTERVAL  (CIT_CYCLE >> 2)

// We could probably save a ulong by just doing this strictly on 
// the clock rather than actually doing it right.
ulong last_pathfind_time =0;
// priority means only check priority requests, but always check
errtype check_requests(bool priority_only)
{
   char i;
   // Don't bother checking unless it has been at least N ticks,
   if (priority_only || (player_struct.game_time > last_pathfind_time))
   {
      // If it is time to check, go through all the paths, find the
      // unfilled requests among them, and go satisfy them.  
      for (i=0; i < MAX_PATHS; i++)
      {
         if (paths[i].num_steps == -2)
            find_path(i);
         if (!priority_only)
         {
            if (paths[i].num_steps == -1)
               find_path(i);
         }
      }

      // Set requirements for next cycle of checking
      if (!priority_only)
         last_pathfind_time = player_struct.game_time + PATHFIND_INTERVAL;
   }
   return(OK);
}


errtype delete_path(char path_id)
{
   // To delete, just mark the path as unused and zero out its data
   if ((path_id < 0) || (path_id >= MAX_PATHS))
      return(ERR_NOEFFECT);
   LG_memset(&paths[path_id],0,sizeof(Path));
   used_paths &= ~(1 << path_id);
   return(OK);
}


// Resets appropriate secret pathfinding state not saved
// in the save game
errtype reset_pathfinding()
{
   last_pathfind_time = 0;
   return(OK);
}


// THE ACTUAL GRUNGY PART

// spt is a "point" which only has 8 bits each for x & y.
typedef short spt;
#define SPT_X(s)   ((s) & 0xFF)
#define SPT_Y(s)   ((s) >> 8)   
#define SPT_X_SET(s,newx) ((s) = ((s) & 0xFF00) | (newx))
#define SPT_Y_SET(s,newy) ((s) = ((s) & 0x00FF) | ((newy) << 8))
#define PT2SPT(pt)   ((((pt).y & 0xFF) << 8) | ((pt).x & 0xFF))

#define FORALLINSPTLIST(pspt, iter, loop) for (iter = pspt[0], i=0; \
   SPT_X(pspt[i]) != 0; i++, iter = pspt[i])

//#define CLEARSPTLIST(pspt, num, loop) do { for (loop=0; loop < num; loop++) { pspt[loop] = 0; } } while (0)
#define CLEARSPTLIST(pspt, num) LG_memset(pspt,0,sizeof(spt)*num)

// A given element in the pathfind buffer is 8 bits
// 5 bits of Z
// 2 bits of from-directionality
// 1 bit  of whether or not it's been visited
#define PFE_Z_MASK   0x1F
#define PFE_DIR_MASK 0x60
#define PFE_USE_MASK 0x80       
#define PFE_DIR_SHIFT   5
#define PFE_USE_SHIFT   7     

#define PFE_OBJ_ZSHIFT  3
// PFE_Z just returns the raw stored z value (5 bits)
// PFE_Z_MAPHT returns the value as a map height (5 bits)
// PFE_Z_OBJHT returns the value as an object height (8 bits)
#define PFE_Z(ppfe) (*(ppfe) & PFE_Z_MASK)
#define PFE_Z_MAPHT(ppfe)  PFE_Z(ppfe)
#define PFE_Z_OBJHT(ppfe)  PFE_Z(ppfe) << PFE_OBJ_ZSHIFT
#define OBJZ_TO_PFEZ(zval) ((zval) >> PFE_OBJ_ZSHIFT)
#define PFEZ_TO_OBJZ(zval) ((zval) << PFE_OBJ_ZSHIFT)
#define MAPZ_TO_PFEZ(zval) (zval)
#define PFEZ_TO_MAPZ(zval) (zval)

// like above, but setting...
#define PFE_Z_SET_RAW(ppfe,z) (*(ppfe) = ((*(ppfe) & ~PFE_Z_MASK) | (z)))
#define PFE_Z_SET_MAPHT(ppfe,mapz)  PFE_Z_SET_RAW(ppfe,mapz)
#define PFE_Z_SET_OBJHT(ppfe,objz)  PFE_Z_SET_RAW(ppfe,objz >> PFE_OBJ_ZSHIFT)

#define PFE_DIR(ppfe) ((*(ppfe) & PFE_DIR_MASK) >> PFE_DIR_SHIFT)
#define PFE_DIR_SET(ppfe,d) (*(ppfe) = (*(ppfe) & ~PFE_DIR_MASK) | ((d) << PFE_DIR_SHIFT))

#define PFE_USED(ppfe) ((*(ppfe) & PFE_USE_MASK) >> PFE_USE_SHIFT)
#define PFE_USED_SET(ppfe,d) (*(ppfe) = (*(ppfe) & ~PFE_USE_MASK) | ((d) << PFE_USE_SHIFT))

// Hmm, I think this will work.... pathfind_buffer is a big chunk
// of memory, and we want to access it like a big array of uchars...      
#define PFE_GET_XY(x,y) ((uchar *)pathfind_buffer + x + (y * MAP_XSIZE))

// l1 and l2 are two lists of spts, and expand_into_list gets
// pointed to whichever is the current actual expand_into_list (the
// other being prepped to be the expand_into_list next time).
#define EXPAND_LIST_SIZE   64
spt *expand_into_list, *expand_from_list, *exp_l1, *exp_l2;
char expand_count;
uchar *pathfind_buffer;


bool map_connectivity(spt sq1, spt sq2, char dir, uchar flr1, uchar *new_z, uchar dest_z);
bool expand_one_square(spt sq, char path_id);
bool expand_fill_list(char path_id);


// Returns the height at the edge of the tile pme, in the direction dir
// Return value is in map units!
short tile_height(MapElem *pme, char dir, bool floor)
{
   uchar retval;
   if (floor)
      retval = me_height_flr(pme);
   else
      retval = MAP_HEIGHTS - me_height_ceil(pme);
   switch(me_tiletype(pme))
   {
      case TILE_SOLID:
         return(-1);
         break;
      case TILE_SOLID_NW:
         if ((dir == 2) || (dir ==1))
            return(-1);
         break;
      case TILE_SOLID_NE:
         if ((dir == 2) || (dir ==3))
            return(-1);
         break;
      case TILE_SOLID_SE:
         if ((dir == 0) || (dir ==3))
            return(-1);
         break;
      case TILE_SOLID_SW:
         if ((dir == 0) || (dir ==1))
            return(-1);
         break;
      // Ask doug how to do the sloping cases right...
      case TILE_SLOPEUP_N:
         if (dir == 2)
            break;
         if (dir == 0)
            retval += me_param(pme);
         else
            retval += me_param(pme) / 2;
         break;
      case TILE_SLOPEUP_S:
         if (dir == 0)
            break;
         if (dir == 2)
            retval += me_param(pme);
         else
            retval += me_param(pme) / 2;
         break;
      case TILE_SLOPEUP_E:
         if (dir == 3)
            break;
         if (dir == 1)
            retval += me_param(pme);
         else
            retval += me_param(pme) / 2;
         break;
      case TILE_SLOPEUP_W:
         if (dir == 1)
            break;
         if (dir == 3)
            retval += me_param(pme);
         else
            retval += me_param(pme) / 2;
         break;

   }
   return(retval);
}


#define CRITTERS_OPEN_UNLOCKED_DOORS

bool pf_check_doors(MapElem *pme, char dir, ObjID *open_door)
{
   ObjRefID curr;
   ObjID id, which_obj = OBJ_NULL;
   curr = me_objref(pme);
   *open_door = OBJ_NULL;
   while (curr != OBJ_REF_NULL)
   {
      id = objRefs[curr].obj;
      if (objs[id].obclass == CLASS_DOOR)
      {
//         Warning(("contemplating id %x, loc = %x, %x, dir = %d\n",id,objs[id].loc.x,objs[id].loc.y,dir));
         switch(dir)
         {   
            case 0: // N
               if (((objs[id].loc.y & 0xFF) >= 0x80) && !(objs[id].loc.h & 0x40))
                  which_obj = id;
               break;
            case 1: // E
               if (((objs[id].loc.x & 0xFF) >= 0x80) && (objs[id].loc.h & 0x40))
                  which_obj = id;
               break;
            case 2: // S
               if (((objs[id].loc.y & 0xFF) <= 0x80) && !(objs[id].loc.h & 0x40))
                  which_obj = id;
               break;
            case 3: // W
               if (((objs[id].loc.x & 0xFF) <= 0x80) && (objs[id].loc.h & 0x40))
                  which_obj = id;
               break;
         }
      }
      curr = objRefs[curr].next;
   }
   if (which_obj != OBJ_NULL)
   {
      // If there is a door in the way, and it is closed, and
      // it is either locked or requires access, we can't get through
      if ((DOOR_CLOSED(which_obj)) && ((ObjProps[OPNUM(which_obj)].flags & TERRAIN_OBJECT)!=0) && 
          ((QUESTBIT_GET(objDoors[objs[which_obj].specID].locked)) ||
           (objDoors[objs[which_obj].specID].access_level)))
      {
         return(FALSE);
      }
      else
         *open_door = which_obj;
   }
   return(TRUE);
}

// Returns whether or not the two squares can be freely traveled
// between with respect to door-like objects in the squares.
bool pf_obj_doors(MapElem *pme1, MapElem *pme2, char dir, ObjID *open_door)
{
   bool retval;
//   Warning(("Top of pf_obj_door!\n"));
   retval = pf_check_doors(pme1, dir, open_door);
//   Warning(("A: *open_door = %x\n",*open_door));
   if (retval && (*open_door == OBJ_NULL))
   {
      retval = pf_check_doors(pme2, (dir + 2) % 4, open_door);
//      Warning(("B: *open_door = %x\n",*open_door));
   }
   return(retval);
}


// Returns the height (not downshifted) attainable by entering the 
// square at height old_z (downshifted).  Specifically, accounts for 
// bridges and repulsorlifts keeping the player elevated.

// Wow, this really doesn't deal with bridges right at all, I don't think
// Which is to say, I'm pretty sure it confuses doors with bridges outrageously
// maybe that is worth a specific hack...

// old_z is almost certainly in PFEZ units, as is the return value
uchar pf_obj_height(MapElem *pme, uchar )
{
   ObjRefID curr;
   ObjID id;
   uchar retval = MAPZ_TO_PFEZ(me_height_flr(pme));

   curr = me_objref(pme);
//   Spew(DSRC_AI_Pathfind, ("pf_o_ht: initial retval = %x\n",retval));
   while (curr != OBJ_REF_NULL)
   {
      id = objRefs[curr].obj;
#ifdef PATHFIND_REPULSORS
      if (ID2TRIP(id) == REPULSOR_TRIPLE)
      {
         // Check to see if height is sufficient for entry
         if ((objTraps[objs[id].specID].p2 < old_z) &&
             (objTraps[objs[id].specID].p3 > old_z))
         {
            retval = max(retval,objTraps[objs[id].specID].p3);
//            Spew(DSRC_AI_Pathfind, ("pf_o_ht: repulsor retval = %x\n",retval));
         }
      }
      else 
#endif
      if (ObjProps[OPNUM(id)].flags & TERRAIN_OBJECT)
      {
         switch(ObjProps[OPNUM(id)].render_type)
         {
            case FAUBJ_TL_POLY:
            case FAUBJ_TPOLY:
            case FAUBJ_SPECIAL:
               retval = max(retval, OBJZ_TO_PFEZ(objs[id].loc.z));
//               Spew(DSRC_AI_Pathfind, ("pf_o_ht: obj retval = %x (id = %x)\n",retval,id));
               break;
         }
      }
      curr = objRefs[curr].next;
   }
   return(retval);
}


// Tells whether or not we can get from one square (at a given z)
// to a new square, and if so, what our new z will be.  This will
// start out very very stupid and hopefully get smarter as it 
// needs to.

#define PF_HEIGHT 1
#define PF_CLIMB  1

// flr1, new_z, and dest are all in PFE Z units
bool map_connectivity(spt sq1, spt sq2, char dir, uchar flr1, uchar *new_z, uchar )
{
   MapElem *pme1, *pme2;
   ObjID temp;
   short flr2,ceil2;
   bool retval;

   pme1 = MAP_GET_XY(SPT_X(sq1),SPT_Y(sq1));
   pme2 = MAP_GET_XY(SPT_X(sq2),SPT_Y(sq2));
   flr2 = MAPZ_TO_PFEZ(tile_height(pme2, (dir+2)%4, TRUE));
   ceil2 = MAPZ_TO_PFEZ(tile_height(pme2, (dir+2)%4, FALSE));
   if (flr2 == -1)
      return(FALSE);

#ifdef ALLOW_DESTZ_OVERIDE
   // Allow final destination overriding, and downshift z
   if (dest_z)
      flr2 = dest_z;
#endif

   if ((ceil2 < flr1 + PF_HEIGHT) || (flr2 > flr1 + PF_CLIMB))
   {
      retval = FALSE;
   }
   else
   {
      retval = TRUE;
      *new_z = pf_obj_height(pme2,flr1);
   }
   if (retval)
      retval = pf_obj_doors(pme1,pme2,dir,&temp);

   return(retval);
}


// Expand out a single square.  Don't let through any expansions
// that point to places that other expansions have been to or
// that we can't reach.
// Returns whether or not we reached the destination.
bool expand_one_square(spt sq, char path_id)
{
   spt newsq, dest = PT2SPT(paths[path_id].dest);
   char i;
   uchar *ppfe, *ppfe2;
   
   ppfe2 = PFE_GET_XY(SPT_X(sq),SPT_Y(sq));
//   Spew(DSRC_AI_Pathfind, ("expanding %x\n",sq));
   for (i=0; i < 4; i++)
   {
      newsq = sq;
      switch(i)
      {
         case 0: newsq = SPT_Y_SET(newsq,SPT_Y(newsq)+1); break;  // N
         case 1: newsq = SPT_X_SET(newsq,SPT_X(newsq)+1); break;  // E
         case 2: newsq = SPT_Y_SET(newsq,SPT_Y(newsq)-1); break;  // S
         case 3: newsq = SPT_X_SET(newsq,SPT_X(newsq)-1); break;  // W
      }
      ppfe = PFE_GET_XY(SPT_X(newsq),SPT_Y(newsq));
      if (!PFE_USED(ppfe)) // dont bother if we can already get there
      {
         uchar new_z, dest_z=0;
         if (newsq == dest)
            dest_z = OBJZ_TO_PFEZ(paths[path_id].dest_z);
         if (map_connectivity(sq,newsq,i,PFE_Z(ppfe2), &new_z, dest_z))
         {
            PFE_USED_SET(ppfe,TRUE);
            // return value from map_conn is already in PFE Z units
            PFE_Z_SET_RAW(ppfe,new_z);
            PFE_DIR_SET(ppfe,i);
            expand_into_list[expand_count++] = newsq;
  //          Spew(DSRC_AI_Pathfind,("can reach %x\n",newsq));
            if (newsq == dest)
               return(TRUE);
         }
//         else
//            Spew(DSRC_AI_Pathfind, ("%x does not connect\n",newsq));
      }
//      else
//         Spew(DSRC_AI_Pathfind, ("%x already used\n",newsq));
   }
   return(FALSE);
}

// Go through the list of last-iteration's reached squares, and
// generate a new list of places to go to. 
// Returns whether or not we reached the destination.
bool expand_fill_list(char path_id)
{
   spt s;
   char i;
   bool done = FALSE;
   expand_count = 0;
   CLEARSPTLIST(expand_into_list,EXPAND_LIST_SIZE);
   FORALLINSPTLIST(expand_from_list, s, i)
   {
      done = expand_one_square(s,path_id);
      if (done)
         break;
   }
//   Spew(DSRC_AI_Pathfind, ("expand into: "));
//   FORALLINSPTLIST(expand_into_list, s, i)
//   {
//      Spew(DSRC_AI_Pathfind, ("%x ",s));
//   }
//   Spew(DSRC_AI_Pathfind, ("\n"));
   return(done);
}

// So, for each map square we keep track of what square we took to get
// here.  Since our fill is breadth-first, we know if a square has already
// been reached, it has been reached by a quicker or equally quick path,
// so we only need 2 bits of "from-directionality".  We also keep track of
// our z, so that we can have better gnosis of connectivity (you can go down
// cliffs, but not up them, unless you are on a bridge, for example).

// The basic algorithm is that we expand our list of interesting squares
// out by one on each loop iteration, then check all of those squares for
// being our destination, and failing finding our target, assemble a new list
// of old squares to expand on the next iteration.

// When we find the target, we just follow the from-directionality backwards to 
// get the shortest path.  I fully admit this is far from the best, fastest,
// or cleverest algorithm in the world.  

// I don't think this algorithm deals at all with being able to jump over 
// one-square pits.  In fact, I worry whether or not 2 bits of directionality
// is sufficient to the task.  I guess we'll find out.

// use big_buffer rather than being le memory hog
#define PATHFIND_WITH_BIG_BUFFER
errtype find_path(char path_id)
{
   bool done = FALSE;
   char i,j,step_count=0;
   uchar *ppfe;

   // Malloc our expand lists & the buffer
   exp_l1 = (spt *)big_buffer;
   exp_l2 = (spt *)(big_buffer + (sizeof(spt) * EXPAND_LIST_SIZE));
   pathfind_buffer = (uchar *)(big_buffer + (sizeof(spt) * 2 * EXPAND_LIST_SIZE));

   // Clear the lists
   CLEARSPTLIST(exp_l1,EXPAND_LIST_SIZE);
   CLEARSPTLIST(exp_l2,EXPAND_LIST_SIZE);
   LG_memset(pathfind_buffer, 0, MAP_XSIZE * MAP_YSIZE * sizeof(uchar));
#ifdef REALLY_SLOW_PATHFIND_CLEARING
   for (i=0; i < MAP_XSIZE; i++)
   {
      for (j=0; j < MAP_YSIZE; j++)
      {   
         PFE_USED_SET(PFE_GET_XY(i,j),FALSE);
      }
   }
#endif

   // set up initial pointings
   expand_into_list = exp_l1;
   expand_from_list = exp_l2;

   // Prep the first one to be our source   
   expand_from_list[0] = PT2SPT(paths[path_id].source);
   ppfe = PFE_GET_XY(paths[path_id].source.x, paths[path_id].source.y);
   PFE_Z_SET_OBJHT(ppfe, paths[path_id].start_z);
   PFE_USED_SET(ppfe, TRUE);

   while (!done && step_count < NUM_PATH_STEPS)
   {
      // Expand out last iteration's list
      done = expand_fill_list(path_id);

      // If we haven't found it, swap pointers, etc.
      if (!done)
      {
         if (expand_count == 0)
         {
//            Warning(("expand_count = 0!\n"));
            step_count = NUM_PATH_STEPS;
         }
         if (expand_into_list == exp_l1)
         {
            expand_into_list = exp_l2;
            expand_from_list = exp_l1;
         }
         else
         {
            expand_into_list = exp_l1;
            expand_from_list = exp_l2;
         }
         step_count++;
      }
      else
      // If we HAVE found it, go poke in the right info into the path          
      {
         LGPoint currpt;
         i=0;
         currpt = paths[path_id].dest;
         while (!PointsEqual(currpt,paths[path_id].source))
         {
            j = PFE_DIR(PFE_GET_XY(currpt.x,currpt.y));
            SET_PATH_STEP(path_id,i,j);
            i++;

            // Compute one step backwards, according to our direction, j
            // so that we have a new currpt
            switch(j)
            {
               case 0: currpt.y--; break; // N, so backwards is S
               case 1: currpt.x--; break; // E, so backwards is W
               case 2: currpt.y++; break; // S, so backwards is N
               case 3: currpt.x++; break; // W, so backwards is E
            }
         }
         paths[path_id].num_steps = i;
      }
   }
   if (step_count >= NUM_PATH_STEPS)
   {
      paths[path_id].num_steps = 0;
//      Warning(("Failed to find path from (%x,%x) to (%x,%x)\n",paths[path_id].source.x,paths[path_id].source.y,
//         paths[path_id].dest.x, paths[path_id].dest.y));
   }

   return(OK);
}
