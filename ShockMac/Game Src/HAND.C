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
 * $Source: r:/prj/cit/src/RCS/hand.c $
 * $Revision: 1.38 $
 * $Author: minman $
 * $Date: 1994/11/17 18:19:18 $
 *
 */

// Includes
#include "handart.h"
#include "objclass.h"
#include "weapons.h"
#include "player.h"
#include "handart.h"
#include "hand.h"
#include "fullscrn.h"
#include "screen.h"
#include "faketime.h"


typedef struct {
   char   handart_frame;
   char  x_offset;
   char   y_offset;
} handart_frame_info;

#define HANDART_NUM (get_nth_from_triple(MAKETRIP(CLASS_GUN,player_struct.weapons[player_struct.actives[ACTIVE_WEAPON]].type,player_struct.weapons[player_struct.actives[ACTIVE_WEAPON]].subtype)))

#define GAMESCR_HANDART_Y  76
#define FULLSCREEN_HANDART_Y 168 

#define HANDART_X_BASE  100
#define HANDART_ID_BASE RES_handArt_0

#define FULL_MIDDLE_SCREEN (FULL_VIEW_WIDTH/2)
#define SCREEN_MIDDLE_SCREEN (SCREEN_VIEW_WIDTH/2)

#define MAX_HAND2HAND_FRAMES  5
#define NUM_FRAMES            2

#define PR24_COUNT         5       
#define LASER_EPEE_COUNT   5

#define HAND_BOB           3
#define BOB_MIN            0
#define BOB_MAX            6

#define HAND_BOBX          4
#define BOBX_MIN           0
#define BOBX_MAX           8

#define BOB_THRESHOLD      (fix_make(0,0x2800))

// damn is this ugly - but hey - we can save lots of space!!!

handart_frame_info hand2hand_info[NUM_HANDTOHAND_GUN][MAX_HAND2HAND_FRAMES] =
{
   {{0, 10, -2}, {1,0,7},{2,-14,21},{3,-26,26},{1,3,7}},       // pr-24
   {{1, -7, -1}, {0,10,-20},{2,-30,4},{3,-49,20},{1,-16,19}},       // laser epee
};

#define NUM_PROJ_GUN (NUM_PISTOL_GUN+NUM_AUTO_GUN)

LGPoint pistol_hand_info[NUM_PROJ_GUN][NUM_FRAMES] =
{
   {{0, 25},{-4, 19}},       // pistol 
   {{0, 25},{-3, 20}},       // dartgun
   {{0, 24},{-2, 18}},       // magnum
   {{0, 23},{-2, 16}},       // assault rifle
   {{0, 23},{-1, 21}},       // riot gun
   {{0, 21},{-15, 11}},       // flechette
   {{0, 23},{-2, 17}},       // skorpion
};

#define NUM_ENERGY_GUN (NUM_GUN-NUM_PROJ_GUN)
byte energy_hand_info[NUM_ENERGY_GUN][NUM_FRAMES] =
{
   {21, 25},       // magpulse
   {23, 27},       // rail gun
   {0,0},         // filler - hand2hand
   {0,0},         // filler
   {24, 24},       // sparq beam
   {24, 24},       // blaster
   {21, 21},       // ion rifle
   {24, 24},       // stungun
   {21, 21},       // plasma rifle
};


#define BOB_TIME (CIT_CYCLE >> 4)

ubyte handart_count = 2;
ubyte hand_bobbing = HAND_BOB;
ubyte hand_bobx = HAND_BOBX;
bool bob_up = TRUE;
bool bob_left = TRUE;
void reset_handart_count(int wpn_num);


// -----------------------------------------
// get_handart()
//

Ref get_handart(int *x_offset, int *y_offset, short mouse_x, short mouse_y)
{
   int      view_base_y;
   short    screen_height;
   short    factor;
   short    hand_x, hand_y;
   ubyte    frame;
   ubyte    type;
//   byte     offset=HAND_BOB;
   State    new_state;
//   RefTable *prt;

#ifdef HANDART_ADJUST
   extern ubyte hcount;
#endif

   switch (player_struct.weapons[player_struct.actives[ACTIVE_WEAPON]].type)
   {
      case (GUN_SUBCLASS_PISTOL):
      case (GUN_SUBCLASS_AUTO):
         frame = handart_show-1;
         hand_x = pistol_hand_info[HANDART_NUM][frame].x;
         hand_y = pistol_hand_info[HANDART_NUM][frame].y;
         break;
      case (GUN_SUBCLASS_HANDTOHAND):
         type = player_struct.weapons[player_struct.actives[ACTIVE_WEAPON]].subtype;
         frame = hand2hand_info[type][handart_show-1].handart_frame;
#ifdef HANDART_ADJUST
         if (hcount)
            frame = hcount-1;
#endif
         hand_x = hand2hand_info[type][handart_show-1].x_offset;
         hand_y = hand2hand_info[type][handart_show-1].y_offset;

         EDMS_get_state(objs[PLAYER_OBJ].info.ph, &new_state); // look - we have to use get_state to get velocity
         hand_x += hand_bobx;
         hand_y += hand_bobbing;

         if ((fix_abs(new_state.X_dot) > BOB_THRESHOLD) || (fix_abs(new_state.Y_dot) > BOB_THRESHOLD) ||
            (fix_abs(new_state.gamma_dot) > BOB_THRESHOLD) || (fix_abs(new_state.beta_dot) > BOB_THRESHOLD))
         {
            if (player_struct.last_bob+BOB_TIME < player_struct.game_time)
            {
               if (bob_up)
               {
                  if (hand_bobbing >= BOB_MAX)
                  {
                     bob_up = FALSE;
                     hand_bobbing --;
                  }
                  else
                     hand_bobbing ++;
               }
               else
               {
                  if (hand_bobbing <= BOB_MIN)
                  {
                     bob_up = TRUE;
                     hand_bobbing ++;
                  }
                  else
                     hand_bobbing --;
               }
               if (bob_left)
               {
                  if (hand_bobx <= BOBX_MIN)
                  {
                     bob_left = FALSE;
                     hand_bobx++;
                  }
                  else hand_bobx--;
               }
               else
               {
                  if (hand_bobx >= BOBX_MAX)
                  {
                     bob_left = TRUE;
                     hand_bobx--;
                  }
                  else hand_bobx++;
               }
               player_struct.last_bob = player_struct.game_time;
            }
         }
         break;
      default:
         frame = hand_x = 0;
         hand_y = energy_hand_info[HANDART_NUM-NUM_PROJ_GUN][0];
         break;
   }

   if (full_game_3d)
   {
      // old code - do we care if inventory is up????
//      if (full_visible & FULL_INVENT_MASK)
//         return(NULL);

      mouse_x -= FULL_VIEW_X; mouse_y -= FULL_VIEW_Y;

      view_base_y = FULLSCREEN_HANDART_Y;
      screen_height = FULL_VIEW_HEIGHT/3;
      
      if (mouse_x < 10)
         mouse_x = 10;
      else if (mouse_x > (FULL_VIEW_WIDTH-10))
         mouse_x = (FULL_VIEW_WIDTH-10);

      factor = abs(mouse_x - FULL_MIDDLE_SCREEN)/2;
      factor += ((mouse_y-40)/2);
      if (factor <0)
         factor = 0;

      *x_offset = (((mouse_x - FULL_MIDDLE_SCREEN)*factor)/FULL_MIDDLE_SCREEN + FULL_MIDDLE_SCREEN + FULL_VIEW_X - 10)+hand_x;
   }
   else
   {
      mouse_x -= SCREEN_VIEW_X;      mouse_y -= SCREEN_VIEW_Y;
      view_base_y = GAMESCR_HANDART_Y;
      screen_height = SCREEN_VIEW_HEIGHT/4;
      if (mouse_x < 10)
         mouse_x = 10;
      else if (mouse_x > (SCREEN_VIEW_WIDTH-10))
         mouse_x = (SCREEN_VIEW_WIDTH-10);

      if (mouse_y < 0)
         mouse_y = 0;
      else if (mouse_y > SCREEN_VIEW_HEIGHT)
         return(NULL);
      else if (mouse_y > (SCREEN_VIEW_HEIGHT - 10))
         view_base_y++;

      factor = abs(mouse_x - SCREEN_MIDDLE_SCREEN)/3;
      factor += (mouse_y-15);
      if (factor <0)
         factor = 0;

      *x_offset = ((mouse_x - SCREEN_MIDDLE_SCREEN)*factor)/SCREEN_MIDDLE_SCREEN+SCREEN_MIDDLE_SCREEN-15+hand_x;
   }
   *y_offset = view_base_y+(mouse_y/screen_height)+hand_y;

   reset_handart_count(player_struct.actives[ACTIVE_WEAPON]);

/* KLC - don't need this check
   prt = ResReadRefTable(HANDART_ID_BASE + HANDART_NUM);
   if (!(RefIndexValid(prt,frame)))
   {
      frame = prt->numRefs - 1;
      Warning(("ACK PAIN HATE!\n"));
   }
   ResFreeRefTable(prt);
*/
   return(MKREF((HANDART_ID_BASE+HANDART_NUM),frame));
}

// --------------------------------------
// notify_draw_handart()
//

void notify_draw_handart(void)
{
   // once a fire frame has been shown, set handart_fire to TRUE
   // - this is so we definitely show the fire frame, otherwise it would looooook very goooooofy - minman

   handart_fire = TRUE;
}

void reset_handart_count(int wpn_num)
{
  if (player_struct.weapons[wpn_num].type == GUN_SUBCLASS_HANDTOHAND)
   {
      extern ubyte toggle_hand;
      ubyte hit = (handart_count &= 0x80);
      handart_count = (player_struct.weapons[wpn_num].subtype == 0) ? PR24_COUNT : LASER_EPEE_COUNT;
      if (hit)
         handart_count = ((handart_count-2) | 0x80);
   }
   else
      handart_count = 2;
}
