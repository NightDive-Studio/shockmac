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
 * $Source: r:/prj/cit/src/RCS/email.c $
 * $Revision: 1.116 $
 * $Author: xemu $
 * $Date: 1994/11/25 08:14:55 $
 */
 
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "Prefs.h"

#include "invdims.h"
#include "invent.h"
#include "mfdint.h"
#include "mfdext.h"
#include "mfddims.h"
#include "tools.h"
#include "gamestrn.h"
#include "objapp.h"
#include "objsim.h"
#include "wares.h"
#include "mfdgadg.h"
#include "sideicon.h"
#include "wrapper.h"
#include "input.h"
#include "emailbit.h"
#include "cit2d.h"
#include "citres.h"
#include "criterr.h"

#include "cybstrng.h"
#include "gamescr.h"
#include "mfdart.h"
#include "otrip.h"
#include "invpages.h"
#include "shodan.h"
#include "sfxlist.h"
#include "musicai.h"
#include "faketime.h"
#include "popups.h"
#include "loops.h"
#include "gr2ss.h"

#include "vmail.h"

#ifdef AUDIOLOGS
#include "audiolog.h"
#endif


// -------
// DEFINES
// -------

extern errtype inventory_draw_new_page(int num);
extern bool full_game_3d;

extern void push_inventory_cursors(LGCursor* newcurs);
extern void pop_inventory_cursors(void);

LGCursor email_cursor;
grs_bitmap email_cursor_bitmap;
bool email_cursor_currently=FALSE;
bool shodan_sfx_go = FALSE;

#define MFD_EMAILMUG_FUNC 14
#define EMAIL_BASE_ID   RES_email0

#define CHAR_SOFTSP 2

#define BASE_VMAIL 256

#define MUGSHOT_IDX     0
#define TITLE_IDX       1
#define SENDER_IDX      2
#define SUBJECT_IDX     3
#define MESSAGE_IDX     4
#define EMAIL_MESSAGE_IDX     (really_an_email ? MESSAGE_IDX : 0)

#define EMAIL_INACTIVE 0xFF

#define FOOTER_MORE_MASK 0x1
#define FOOTER_PAGE_MASK 0x2

// -------
// GLOBALS
// -------

//#define current_email (player_struct.actives[ACTIVE_EMAIL])
#define current_email (player_struct.current_email)

//==========================================
//       INVENTORY PANEL TEXT DISPLAY
//==========================================

extern LGRegion* inventory_region;
extern grs_canvas *pinv_canvas;

#define MESSAGE_COLOR 0x5A
#define MORE_COLOR    0x4C

#define MESSAGE_X (1)
#define MESSAGE_Y (2)

#define BOTTOM_MARGIN   10
#define EMAIL_INTERCEPT 0xFE
#define EMAIL_DONE     0xFF

bool email_big_font=TRUE;

char   email_buffer[256];
#define EMAIL_BUFSIZ (sizeof(email_buffer))
ubyte   next_text_line = EMAIL_DONE;
ubyte   last_text_line = 0;
ubyte   email_curr_page;
short   old_invent_page=0;
Id current_email_base = EMAIL_BASE_ID;

static uchar   intercept_hack_num;
static uchar   email_flags;

#define EMAIL_FLAG_BEEN_READ 0x1
#define EMAIL_FLAG_TRANSITORY 0x2

int email_font=RES_tinyTechFont;

// ------------
//  PROTOTYPES
// ------------
char* get_email_string(int id, char *text, int siz);
int get_sender_emailnum(int num);
void set_email_flags(int n);
char* get_email_title_string(int n, char *text, int siz);
void apply_email_macros(char *text, char *newval);
void email_intercept(void);
char* email_draw_string(char* text,short* x, short* y,bool last);
void free_email_buffer(void);
void draw_more_string(int x, int y, uchar footermask);
void email_draw_text(short email_id, bool really_an_email);
void email_page_exit(void);
bool email_invpanel_input_handler(uiEvent* ev, LGRegion* r, void* data);
void parse_email_mugs(char* mug, uchar* mcolor, ushort mugnums[NUM_MFDS], bool setup);
void mfd_emailmug_expose(MFD* mfd, ubyte control);
bool mfd_emailmug_handler(MFD *m, uiEvent *ev);
void select_email(int num, bool scr);
void read_email(Id new_base, int num);
void add_email_handler(LGRegion* r);
void mfd_emailware_expose(MFD* mfd, ubyte control);
bool mfd_email_button_handler(MFD* m, LGPoint bttn, uiEvent* ev, void* data);
errtype mfd_emailware_init(MFD_Func* f);
void email_turnon(bool visible,bool real_start);
void email_turnoff(bool visible,bool real_stop);
void update_email_ware(void);
char* email_name_func(void* dp, int num, char* buf);
uchar email_color_func(void* dp, int num);
void email_slam_hack(short which);


char* get_email_string(int id, char *text, int siz)
{
   get_string(id,text,siz);
#ifdef TOUPPER_EMAILS
   strtoupper(text);
#endif
   return text;
}


#define MAX_SENDERS (sizeof(player_struct.email_sender_counts)/sizeof(player_struct.email_sender_counts[0]))

#define NULL_SENDER MAX_SENDERS

// scan through the mugshot for the super-secret "sender" code, which 
// starts with an S and is followed by a decimal number.  
// if we don't find one, return NULL_SENDER.  

int get_sender_emailnum(int num)
{
   char* s = get_temp_string(MKREF(EMAIL_BASE_ID+num, MUGSHOT_IDX));

   for(; *s != '\0'; s++)
      if (toupper(*s) == 'S')
      {
         int sid = atoi(s+1);
         return (sid < MAX_SENDERS && sid >= 0) ? sid : NULL_SENDER;
      }
   return NULL_SENDER;
}     

void set_email_flags(int n)
{
   int sender_num = get_sender_emailnum(n);
   int cnt;

   if (((player_struct.email[n] & EMAIL_SEQ) >> EMAIL_SEQ_SHF) > 0
      || sender_num == NULL_SENDER) return;
   cnt = ++player_struct.email_sender_counts[sender_num];
   player_struct.email[n] &= ~(EMAIL_SEQ << EMAIL_SEQ_SHF);
   player_struct.email[n] |=  cnt << EMAIL_SEQ_SHF;
}

char* get_email_title_string(int n, char *text, int siz)
{
   int cnt = (player_struct.email[n] & EMAIL_SEQ) >> EMAIL_SEQ_SHF; // get the email's sequence number.
   Ref title = MKREF(EMAIL_BASE_ID+n,TITLE_IDX);

   get_string(title,text,siz);
   if (cnt > 0) // if in fact the email has a sequence number, tack it on the end.
   {
      lg_sprintf(text,get_temp_string(title),cnt);
   }
   return text;
}

#define EMAIL_MACRO_PAD_CHARS 20
void apply_email_macros(char *text, char *newval)
{
   short cold=0,cnew=0,len;
   char buf[256];
   char i, stupid;
   int score;
   extern void second_format(int sec_remain, char *s);

   len=strlen(text);

   while (cold < len)
   {
      if (text[cold] == '$')
      {
         switch(text[cold+1])
         {
            // player's name
            case 'N':
            case 'n':
               strcpy(newval+cnew,player_struct.name);
#ifdef TOUPPER_EMAILS
               if(email_big_font)
                  strtoupper(newval+cnew);
#else
               if(islower(*(newval+cnew)))
                  *(newval+cnew)-='a'-'A';
#endif
               cnew += strlen(player_struct.name);
               break;
            // number of kills
            case 'K':
            case 'k':
               numtostring(player_struct.num_victories,buf);
               strcpy(newval+cnew, buf);
               cnew += strlen(buf);
               break;
            // time playing
            case 'T':
            case 't':
               second_format(player_struct.game_time / CIT_CYCLE, buf);
               strcpy(newval+cnew, buf);
               cnew += strlen(buf);
               break;
            // number of revivals
            case 'D':
            case 'd':
               numtostring(player_struct.num_deaths,buf);
               strcpy(newval+cnew, buf);
               cnew += strlen(buf);
               break;
            // difficulty index
            case 'C':
            case 'c':
               stupid = 0;
               for (i=0; i < 4; i++)
                  stupid += (player_struct.difficulty[i] * player_struct.difficulty[i]);
               numtostring(stupid,buf);
               strcpy(newval+cnew, buf);
               cnew += strlen(buf);
               break;
            // score
            case 'S':
            case 's':
               stupid = 0;
               for (i=0; i < 4; i++)
                  stupid += (player_struct.difficulty[i] * player_struct.difficulty[i]);
               // death is 10 anti-kills, but you always keep at least a third of your kills.
               score = player_struct.num_victories - min(player_struct.num_deaths * 10, player_struct.num_victories * 2 / 3);
               score = score * 10000;
               score = score - min(score * 2 / 3, ((player_struct.game_time / (CIT_CYCLE * 36)) * 100));
               score = score * (stupid + 1) / 37;  // 9 * 4 + 1 is best difficulty factor
               if (stupid == 36)
                  score += 2222222; // secret kevin bonus
               numtostring(score,buf);
               strcpy(newval+cnew, buf);
               cnew += strlen(buf);
               break;
            default:
               newval[cnew]='$';
               newval[cnew+1]=text[cold+1];
               break;
         }
         cold += 2;
      }
      else {
         newval[cnew]=text[cold];
         cold++;
         cnew++;
      }
   }
   newval[cnew]='\0';
   return;
}

void email_intercept(void)
{
   switch(intercept_hack_num)
   {
      default:
         inventory_clear();
         next_text_line = EMAIL_DONE;
         pop_inventory_cursors();
         email_cursor_currently=FALSE;
//         Free(email_cursor_bitmap.bits);
         read_email(0,intercept_hack_num);
         shodan_sfx_go = TRUE;
#ifdef AUDIOLOGS
         if (!audiolog_setting)
#endif
         {
            if (!digi_fx_playing(SFX_SHODAN_STRONG, NULL))
               play_digi_fx(SFX_SHODAN_STRONG, -1);
         }
         break;
   }
}

char* email_draw_string(char* text,short* x, short* y,bool last)
{
   short w,h;
   gr_set_fcolor(MESSAGE_COLOR);
   gr_char_size('X',&w,&h);
   while (isspace(*text))
   {
      if (*text == '\n') {
         *y += h, *x = 0;
      }
      else 
         *x += gr_char_width(*text);
      text++;
   }
   while(*y + BOTTOM_MARGIN < INVENTORY_PANEL_HEIGHT)
   {
      while (*x < INVENTORY_PANEL_WIDTH)
      {
         char temp = '\0';
         char *end;
         end = text;
         while(!isspace(*end) && *end != '\0' && *end != '\n')
            end++;
         temp = *end;
         *end = '\0';
         gr_string_size(text,&w,&h);
         if (*x + w > INVENTORY_PANEL_WIDTH)
         {
            short hyphensiz,dum;
            char sav;

            *end = temp;
            gr_char_size('-',&hyphensiz,&dum);
            for(;end>text;end--) {
               if(*end==CHAR_SOFTSP||*end=='-') {
                  sav=*end;
                  *end='\0';
                  gr_string_size(text,&w,&h);
                  if(*x+w+hyphensiz<=INVENTORY_PANEL_WIDTH) {
                     draw_shadowed_string(text,MESSAGE_X+*x,MESSAGE_Y+*y,full_game_3d);
                     draw_shadowed_string("-",MESSAGE_X+*x+w,MESSAGE_Y+*y,full_game_3d);
                     *end=sav;
                     text=end+1;
                     break;
                  }
                  else
                     *end=sav;
               }
            }
            break;
         }
         draw_shadowed_string(text,MESSAGE_X+*x,MESSAGE_Y+*y,full_game_3d);
         *x += w;
         *end = temp;
         if (temp == '\0' && end==text) return NULL;
         text = end;
         if (temp == '\n')
         {
            text++;
            break;
         }

         // now, assess the length of the whitespace after the token.
         while(isspace(*end) && *end != '\n') end++;
         if (*end == '\n')
         {
            text = end + 1;
            break;
         }
         temp = *end;
         *end = '\0';
         w = gr_string_width(text);
         *x+= w;
         *end = temp;
         if (*end == '\0') return NULL;
         text = end;
      }
      *x = 0;
      *y += h;
   }
   if (last)
   {
     gr_string_size(text,&w,&h);
     if (w < INVENTORY_PANEL_WIDTH) //  && *y + h < INVENTORY_PANEL_HEIGHT)
     {
        draw_shadowed_string(text,MESSAGE_X,*y+MESSAGE_Y,full_game_3d);
        *x = w; *y += h;
        return NULL;
     }
   }
   return text;
}

void free_email_buffer(void)
{
   email_buffer[0] = '\0';
}

#define PAGE_STR_BUFSIZE 16

// Footer mask should have 0x1 set to print "MORE", 0x2 set to
// print page number.

void draw_more_string(int x, int y, uchar footermask)
{
   gr_set_fcolor(MORE_COLOR);

   if(footermask & FOOTER_MORE_MASK)
      res_draw_string(email_font,REF_STR_More,x+MESSAGE_X,y+MESSAGE_Y);
   if(footermask & FOOTER_PAGE_MASK) {
      // print page number
      // in the future, this string will be in messages.txt
      // and everyone will drive electric cars.
      char pagen[PAGE_STR_BUFSIZE];
      short w,h,len;

      get_email_string(REF_STR_WordPage,pagen,PAGE_STR_BUFSIZE);
      len=strlen(pagen);
      pagen[len++]=' ';
      numtostring(email_curr_page,pagen+len);
      gr_string_size(pagen,&w,&h);
      draw_shadowed_string(pagen,INVENTORY_PANEL_WIDTH-w-2,y+MESSAGE_Y,full_game_3d);
   }
}

void email_draw_text(short email_id, bool really_an_email)
{
   short x = 0,y = 0;
   char* remains = NULL;
   char buf[256] = "";
#ifdef SVGA_SUPPORT
   uchar old_over;
#endif

   email_curr_page++;

   uiHideMouse(NULL);
   make_email_cursor(&email_cursor,&email_cursor_bitmap,email_curr_page,email_curr_page==1);
   if(!email_cursor_currently)
   {
      push_inventory_cursors(&email_cursor);
      email_cursor_currently=TRUE;
   }
   uiShowMouse(NULL);

   if (!ResInUse(email_id)) {
      return;
   }
   if (really_an_email)
   {
      if (current_email == EMAIL_INACTIVE)
      {
         current_email = EMAIL_INACTIVE;
         return;
      }
      if (next_text_line == EMAIL_INTERCEPT) {
         email_intercept();
         return;
      }
   }
   if (next_text_line == EMAIL_DONE) {
      return;
   }
   last_text_line = next_text_line;
#ifdef SVGA_SUPPORT
   old_over = gr2ss_override;
   gr2ss_override = OVERRIDE_ALL;
#endif
   gr_push_canvas(pinv_canvas);
   gr_set_font((grs_font*)ResLock(email_font));
   if (!full_game_3d)
      uiHideMouse(inventory_region->r);
   inventory_clear();
   if (*email_buffer != '\0')
   {
      ubyte line = EMAIL_MESSAGE_IDX + next_text_line;
      char* next = get_temp_string(MKREF(email_id,line));
      bool last;

      last = (next==NULL || next[0]=='\0');
      if ((remains = email_draw_string(email_buffer,&x,&y,last)) != NULL)
      {
         strncpy(buf,remains,sizeof(buf));
         remains = buf;
         goto more;
      }
      *email_buffer = '\0';
      x += gr_char_width(' ');
      if (last)
      {
         next_text_line = EMAIL_DONE;
         current_email_base = EMAIL_BASE_ID;
         goto done;
      }
   }
   while(remains == NULL || gr_string_width(remains) < INVENTORY_PANEL_WIDTH)
   {
      int len;
      char* next;
      bool last;
      char tmp[256];
      ubyte line = EMAIL_MESSAGE_IDX  + next_text_line++;
      if (remains == NULL)
         get_email_string(MKREF(email_id,line),buf,sizeof(buf));
      else
         get_email_string(MKREF(email_id,line),buf+strlen(remains),sizeof(buf)-strlen(remains));
      apply_email_macros(buf,tmp);
      strcpy(buf,tmp);
      if(buf[0]=='\0') {
         next_text_line = EMAIL_DONE;
         current_email_base = EMAIL_BASE_ID;
         goto done;
      }
      len = strlen(buf);
      if (!isspace(buf[len-1]))
         strcpy(buf+len," ");
      next = get_temp_string(MKREF(email_id,line+1));
      last = next[0] == '\0';
      remains = email_draw_string(buf,&x,&y,last);
      if (last)
      {
         if (remains == NULL)
         {
            ResUnlock(email_id);								//KLC - we're done with it.
            next_text_line = EMAIL_DONE;
            current_email_base = EMAIL_BASE_ID;
            goto done;
         }
         else goto more;
      }
      if (remains != NULL)
      {
         char buf2[sizeof(buf)];
         strcpy(buf2,remains);
         strcpy(buf,buf2);
         remains = buf;
      }
   }
   // Print the "more" message.  
 more:
   if (remains != NULL)
   {
      if (strlen(remains) >= EMAIL_BUFSIZ)
      {
         critical_error(0x3005);
      }
      strcpy(email_buffer,remains);
   }
   draw_more_string(x,y,FOOTER_MORE_MASK);
 done:
   if(next_text_line==EMAIL_DONE) {
      short w,h;

      if (email_flags&EMAIL_FLAG_TRANSITORY) {
         player_struct.email[current_email] &= ~(EMAIL_GOT|EMAIL_READ);
      }
      gr_char_size('X',&w,&h);
      x=0; y+=h;
      if(intercept_hack_num>0) {
         next_text_line = EMAIL_INTERCEPT;
         gr_set_fcolor(MORE_COLOR);
         draw_more_string(x,y,FOOTER_MORE_MASK);
      }
      else if (email_curr_page>1)
         draw_more_string(x,y,0);
   }
   ResUnlock(email_font);
   if (!full_game_3d)
      uiShowMouse(inventory_region->r);
   gr_pop_canvas();
#ifdef SVGA_SUPPORT
   gr2ss_override = old_over;
#endif
}


#define BAD_EMAIL_KEYFLAGS (KB_FLAG_SHIFT|KB_FLAG_CTRL|KB_FLAG_ALT|KB_FLAG_SPECIAL)

void email_page_exit(void)
{
   int mid;

   current_email=EMAIL_INACTIVE;
   pop_inventory_cursors();
   email_cursor_currently=FALSE;
//   Free(email_cursor_bitmap.bits);
   next_text_line = EMAIL_DONE;
   for(mid=0;mid<NUM_MFDS;mid++) {
      if(mfd_get_func(mid,player_struct.mfd_current_slots[mid])==MFD_EMAILMUG_FUNC)
         restore_mfd_slot(mid);
   }
}


bool email_invpanel_input_handler(uiEvent* ev, LGRegion*, void*)
{
   if (input_cursor_mode == INPUT_OBJECT_CURSOR) return FALSE;
   if (inventory_page != INV_EMAILTEXT_PAGE)
   {
      if(email_cursor_currently) {
         email_page_exit();
      }
      return FALSE;
   }
   if(ev->type==UI_EVENT_MOUSE_MOVE) return TRUE;
   if (current_email == EMAIL_INACTIVE) return FALSE;
   if (ev->type == UI_EVENT_MOUSE && !(ev->subtype & (MOUSE_LDOWN|MOUSE_RDOWN|MOUSE_CDOWN)))
      return TRUE;
//   if (ev->type == UI_EVENT_KBD_COOKED &&  (ev->subtype & BAD_EMAIL_KEYFLAGS))
//      return FALSE;
   if (ev->type == UI_EVENT_KBD_COOKED && !((ev->subtype & KB_FLAG_DOWN) != 0 && (ev->subtype & 0xFF) == ' '))
      return FALSE;
   if (next_text_line == EMAIL_DONE)
   {
      email_page_exit();
      inventory_draw_new_page(old_invent_page);
   }
   else email_draw_text(current_email_base + current_email,current_email_base == EMAIL_BASE_ID);
   return TRUE;
}


// ============================================
//            THE SELECTED EMAIL MFD
// ============================================

#define EMAILMUG_SLOT MFD_INFO_SLOT

#define EMAIL_SUBJECT_Y (MFD_VIEW_HGT-1)

#define LAST_MUG(mfd)   (*(short*)&player_struct.mfd_func_data[MFD_EMAILMUG_FUNC][mfd*2])

#define COLOR_ESC_CHAR 'c'
#define INTERCEPT_ESC_CHAR 'i'
#define TRANSITORY_ESC_CHAR 't'
#define WHOAMI_ESC_CHAR 's'

void parse_email_mugs(char* mug, uchar* mcolor, ushort mugnums[NUM_MFDS], bool setup)
{
   short i, fwid;
   char* s, *sfront;
   short lastmug = -1;
   uchar esc_param, different;
   extern void cap_mfds_with_func(uchar func, uchar max);
   extern int str_to_hex(char);
   char buf[64];

   s = buf;
   sfront = s;
   strcpy(s,mug);

   if(mug && *mug) {

      intercept_hack_num=0;
      while(!isdigit(*s) && *s!='\0') {

         fwid=0;

         if(*s==COLOR_ESC_CHAR||*s==INTERCEPT_ESC_CHAR||*s==WHOAMI_ESC_CHAR) {
            // goofy 2-digit hex parse
            esc_param=0;
            if(*(s+1) && *(s+2)) {
               esc_param=(str_to_hex(*(s+1))<<4)+str_to_hex(*(s+2));
               fwid=2;
            }
         }

         switch(*s) {
            case TRANSITORY_ESC_CHAR:
               email_flags|=EMAIL_FLAG_TRANSITORY;
               break;
            case COLOR_ESC_CHAR:
               if(mcolor) {
                  *mcolor=esc_param;
               }
               break;
            case INTERCEPT_ESC_CHAR:
               if(!(email_flags&EMAIL_FLAG_BEEN_READ))
                  intercept_hack_num=esc_param;
               break;
         }
         s+=fwid+1;
         while(!isalpha(*s) && !isdigit(*s) && *s!='\0') s++;
      }
   }
   different=0;
   for (i = 0; i < NUM_MFDS; i++)
   {
      if (!isdigit(*s))
      {
         mugnums[i] = lastmug;
         continue;
      }
      mugnums[i] = atoi(s);
      if(mugnums[i]!=lastmug)
         different++;
      lastmug = mugnums[i];
      while(isdigit(*s)) s++;
      while(!isdigit(*s) && *s != '\0') s++;
   }
   if(setup)
      cap_mfds_with_func(MFD_EMAILMUG_FUNC,different);
   
}

void mfd_emailmug_expose(MFD* mfd, ubyte control)
{
   bool full = control & MFD_EXPOSE_FULL;
   int msg = current_email_base+current_email;
   if (control == 0)  // MFD is drawing stuff
   {
      int hnd;
      // Do unexpose stuff here.
      if (shodan_sfx_go)
         shodan_sfx_go = FALSE;
      if (digi_fx_playing(SFX_SHODAN_STRONG, &hnd))
         snd_end_sample(hnd);
      return;
   }
   if (!ResInUse(msg))
   {
      current_email = EMAIL_INACTIVE;
   }
   if (current_email == EMAIL_INACTIVE)
   {
      mfd_notify_func(MFD_EMPTY_FUNC,EMAILMUG_SLOT,TRUE,MFD_EMPTY,TRUE);
      return;
   }
   if (control & MFD_EXPOSE) // Time to draw stuff
   {
      char buf[256];
      ushort mnums[NUM_MFDS];
      ushort mugnum;
      short mid = NUM_MFDS;
      int mug;
      uchar mcolor=MESSAGE_COLOR;
      parse_email_mugs((char*)RefGet(MKREF(msg,MUGSHOT_IDX)),&mcolor,mnums,FALSE);
      for (mid = 0; mid < NUM_MFDS; mid++)
         if (player_struct.mfd_current_slots[mid] == EMAILMUG_SLOT)
         {
            break;
         }
      if (mid > mfd->id) mid = 0;
      mugnum = mnums[mfd->id-mid];
      if (mugnum != LAST_MUG(mfd->id))
         full = TRUE;
      LAST_MUG(mfd->id) = mugnum;
      if (!full) goto out;

      mug = REF_IMG_EmailMugShotBase + mugnum;
      // clear update rects
      mfd_clear_rects();
      // set up canvas
      gr_push_canvas(pmfd_canvas);
      ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);

      // Clear the canvas by drawing the background bitmap
      if (!full_game_3d)
//KLC - chg for new art         ss_bitmap(&mfd_background, 0, 0);
         gr_bitmap(&mfd_background, 0, 0);

      // Slam in the mug shot, centered.  
      if ((mugnum<BASE_VMAIL)
#ifdef PLAYTEST
  && RefIndexValid((RefTable*)ResGet(REFID(mug)),REFINDEX(mug)))
#else
         )   // god, I love this job
#endif
      {
         grs_bitmap bm;
         bm.bits = NULL;
         extract_temp_res_bitmap(&bm,mug);
//KLC - chg for new art         ss_bitmap(&bm,(MFD_VIEW_WID-bm.w)/2,(MFD_VIEW_HGT-bm.h)/2);
         gr_bitmap(&bm,(SCONV_X(MFD_VIEW_WID)-bm.w)/2, (SCONV_Y(MFD_VIEW_HGT)-bm.h)/2);
      }

#ifdef AUDIOLOGS
      if (!audiolog_setting)
#endif
         if (shodan_sfx_go)
         {
            if (!digi_fx_playing(SFX_SHODAN_STRONG, NULL))
               play_digi_fx(SFX_SHODAN_STRONG, -1);
         }
      // Now, the text
      if (mugnum == mnums[0])
      {
         char *sub;
         short w,h;

         get_email_title_string(current_email,buf,sizeof(buf));
         strcat(buf,"\n");
         get_email_string(MKREF(msg,SENDER_IDX),buf+strlen(buf),sizeof(buf)-strlen(buf));
         mfd_full_draw_string(buf,0,0,mcolor,email_font,TRUE,TRUE);
         get_email_string(REF_STR_MessageSubject,buf,sizeof(buf));
         sub=buf+strlen(buf);
         get_email_string(MKREF(msg,SUBJECT_IDX),sub,sizeof(buf)-strlen(buf));
         // draw subject field only if subject string is non-null.
         if(*sub) {
            wrap_text(buf,MFD_VIEW_WID-1); 
            gr_string_size(buf,&w,&h);
            unwrap_text(buf);
            mfd_full_draw_string(buf,0,EMAIL_SUBJECT_Y-h,mcolor,email_font,TRUE,TRUE);
         }
      }

      mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
      if (mfd->id == MFD_LEFT && player_struct.mfd_current_slots[MFD_RIGHT] == EMAILMUG_SLOT)
      {
         mfd_notify_func(MFD_EMAILMUG_FUNC,EMAILMUG_SLOT,FALSE,MFD_ACTIVE,TRUE);
      }


      // Pop the canvas
      gr_pop_canvas();
      // Now that we've popped the canvas, we can send the 
      // updated mfd to screen
      mfd_update_rects(mfd);

   }
 out:
   return;
  
}

bool mfd_emailmug_handler(MFD *, uiEvent *ev)
{
   if (ev->type != UI_EVENT_MOUSE || !(ev->subtype & (MOUSE_LDOWN|MOUSE_RDOWN|MOUSE_CDOWN)))
      return FALSE;
   if (player_struct.hardwarez[HARDWARE_EMAIL] == 0)
   {
      string_message_info(REF_STR_NoDataReader);
      return TRUE;
   }
   read_email(0,current_email);
   return TRUE;
}

//==========================================================
//                DISPLAY A MESSAGE
//==========================================================

void select_email(int num, bool scr)
{
   int id;
   int mug_num;
   current_email_base = EMAIL_BASE_ID;
   id  = current_email_base+num;
   if (!ResInUse(id))
   {
      current_email = EMAIL_INACTIVE;
      return;
   }
   
   mug_num=atoi((char *)RefGet(MKREF(id,MUGSHOT_IDX)));
   ResUnlock(id);
   
   if (mug_num>=BASE_VMAIL)
      read_email(current_email_base, num);
   if (scr)
   {
      current_email = num;
      next_text_line = EMAIL_DONE;
      mfd_notify_func(MFD_EMAILMUG_FUNC,EMAILMUG_SLOT,TRUE,MFD_ACTIVE,TRUE);
      if (mug_num < BASE_VMAIL && inventory_page == INV_EMAILTEXT_PAGE)
         read_email(0,num);
   }
}


#define FIRST_CDATA_NUM 0x10f
void read_email(Id new_base, int num)
{
   int id;
   int mug_num;
#ifdef AUDIOLOGS
   errtype alog_rv = ERR_NOEFFECT;
#endif
//KLC - use a global preference now   ubyte terseness = player_struct.terseness;
   ubyte terseness = gShockPrefs.goMsgLength;
   
   email_curr_page = 0;
   if (new_base != 0)
      current_email_base = new_base;
   // no intercept if we are reading a paper.  Bit sloppy to do it
   // this way, but no sloppier than papers in general.
   // And if you thought the hack for papers was bad, wait until you see the one for datas... -- X
   if ((current_email_base==RES_paper0) || (num >= FIRST_CDATA_NUM)) {
      intercept_hack_num=0;
   }
   id = current_email_base+num;
   if (!ResInUse(id))
   {
      current_email = EMAIL_INACTIVE;
      return;
   }

   email_flags=0;
   if (current_email_base == EMAIL_BASE_ID)
   {
#ifdef AUDIOLOGS
      alog_rv = audiolog_play(num);
#endif
      if(player_struct.email[num] & EMAIL_READ)
         email_flags|=EMAIL_FLAG_BEEN_READ;
      player_struct.email[num] |= EMAIL_READ;
   }
   else terseness = 0;
   current_email = num;
   if (inventory_page >= 0)
      old_invent_page = inventory_page;
#ifdef AUDIOLOGS
   if ((alog_rv != OK) || (audiolog_setting == 2))
   {
#endif
      inventory_draw_new_page(INV_EMAILTEXT_PAGE);
      next_text_line = 0;
      if (terseness > 0) // let's be terse
      {
         // skip ahead to the terse version
         while(*get_temp_string(MKREF(current_email_base+num,MESSAGE_IDX+next_text_line)) != '\0')
            next_text_line++;
         next_text_line++;
      }
      free_email_buffer();
#ifdef AUDIOLOGS
   }
#endif

   if (current_email_base == EMAIL_BASE_ID)
   {
      player_struct.hardwarez_status[HARDWARE_EMAIL] &= ~(WARE_FLASH);
      QUESTBIT_OFF(0x12c);
      
      mug_num=atoi((char *)RefGet(MKREF(current_email_base+num,MUGSHOT_IDX)));
      ResUnlock(current_email_base+num);
      
      if (mug_num>=BASE_VMAIL)  // video email
      {
#ifdef AUDIOLOGS
         if ((alog_rv != OK) || (audiolog_setting == 2))
         {
#endif
            // draw the text for the vmail before playing vmail
            email_draw_text(current_email_base + current_email,current_email_base == EMAIL_BASE_ID);
            play_vmail(mug_num-BASE_VMAIL);
#ifdef AUDIOLOGS
         }
         else
         {
         }
#endif
      }
      else
      {
         mfd_notify_func(MFD_EMAILMUG_FUNC,EMAILMUG_SLOT,TRUE,MFD_ACTIVE,TRUE);
         if (current_email != EMAIL_INACTIVE)
         {
            int i;
            ushort mnums[NUM_MFDS];
            bool grab = TRUE; 
            parse_email_mugs((char *)RefGet(MKREF(current_email_base+num,MUGSHOT_IDX)),NULL,mnums,TRUE);
            for (i = 1; i < NUM_MFDS; i++)
            {
               if (mnums[i] != mnums[i-1])
               {
                  save_mfd_slot(i);
                  mfd_change_slot(i,EMAILMUG_SLOT);
                  grab = FALSE;     
               }
            }
            if (grab) {
               i=mfd_grab_func(MFD_EMAILMUG_FUNC,EMAILMUG_SLOT);
               save_mfd_slot(i);
               mfd_change_slot(i,EMAILMUG_SLOT);
            }
            else {
               save_mfd_slot(0);
               mfd_change_slot(0,EMAILMUG_SLOT);
            }
         }
      }
   }
#ifdef AUDIOLOGS
   if ((alog_rv != OK) || (audiolog_setting == 2))
   {
#endif
      email_draw_text(current_email_base + current_email,current_email_base == EMAIL_BASE_ID);
#ifdef AUDIOLOGS
   }
   else
   {
      if (_current_loop <= FULLSCREEN_LOOP) 
         chg_set_flg(INVENTORY_UPDATE);   
   }
#endif
}

//=======================================================
//                   INITIALIZATION
//=======================================================

void add_email_handler(LGRegion* r)
{
   int id;
   uiInstallRegionHandler(r,UI_EVENT_MOUSE_MOVE|UI_EVENT_MOUSE|UI_EVENT_KBD_COOKED,email_invpanel_input_handler,NULL,&id);
}





//=====================================================
//             THE EMAIL PAGE SELECT MFD
//=====================================================
#define MFD_EMAILWARE_FUNC 15
#define NUM_EMAIL_BUTTONS 3
#define BUTTON_LIT_STATE(mfd,butt) (player_struct.mfd_func_data[MFD_EMAILWARE_FUNC][NUM_EMAIL_BUTTONS*(mfd)+(butt)])

#define EMAIL_BARRAY_X   0
#define EMAIL_BARRAY_WD  (MFD_VIEW_WID)
#define EMAIL_BARRAY_Y (MFD_VIEW_HGT - res_bm_height(REF_IMG_EmailButt0)-2)  


static ubyte email_pages[] = { 50,7,8};
#define STRINGS_PER_WARE (REF_STR_wareSpew1 - REF_STR_wareSpew0)

void mfd_emailware_expose(MFD* mfd, ubyte control)
{
   extern void mfd_item_micro_hires_expose(bool full, int triple);
   extern void draw_mfd_item_spew(Ref id, int n);
   int i;
   bool full = control & MFD_EXPOSE_FULL;
   bool on = (control & (MFD_EXPOSE|MFD_EXPOSE_FULL)) != 0;
   ubyte s = player_struct.hardwarez_status[HARDWARE_EMAIL];
   if (control == 0)
   {
      int mfd_id = NUM_MFDS;

      // if we aren't showing the email hardware mfd any more, then 
      // turn off the MFD
      while(mfd_yield_func(MFD_EMAILWARE_FUNC,&mfd_id))
      {
         if (mfd_id != mfd->id)
            on = TRUE;
      }   
   }
   if (((s & WARE_ON) != 0) != on)
   {
      use_ware(WARE_HARD,HARDWARE_EMAIL);
   }

   if (control == 0) return;


   mfd_clear_rects();
   gr_push_canvas(pmfd_canvas);
   ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);

   // Lay down the "background" 
//KLC - chg for new art   mfd_item_micro_expose(TRUE,VIDTEX_HARD_TRIPLE);
   mfd_item_micro_hires_expose(TRUE,VIDTEX_HARD_TRIPLE);
   if (full)
   {
      uchar n = HARDWARE_EMAIL;
      uchar v = player_struct.hardwarez[n];
      draw_mfd_item_spew(REF_STR_wareSpew0 + STRINGS_PER_WARE*n,v);
   }


   // clear rects so that we don't draw it if we don't have to
   if (!full) mfd_clear_rects();

   for (i = 0; i < NUM_EMAIL_BUTTONS; i++)
   {
      bool lit = inventory_page == email_pages[i];
      if (full || BUTTON_LIT_STATE(mfd->id,i) != lit)
      {
         int id = (lit) ? REF_IMG_LitEmailButt0 + i : REF_IMG_EmailButt0 + i;
         short x = EMAIL_BARRAY_WD*i/NUM_EMAIL_BUTTONS+EMAIL_BARRAY_X;
         short y = EMAIL_BARRAY_Y;
         draw_res_bm(id,x,y);
         mfd_add_rect(x,y,x+res_bm_width(id),y+res_bm_height(id));
         BUTTON_LIT_STATE(mfd->id,i) = lit;
      }
   }
   gr_pop_canvas();
   mfd_update_rects(mfd);
}

bool mfd_email_button_handler(MFD*, LGPoint bttn, uiEvent*, void*)
{
   current_email_base = EMAIL_BASE_ID;
   old_invent_page = bttn.x;
   inventory_draw_new_page(email_pages[bttn.x]);
   mfd_notify_func(MFD_EMAILWARE_FUNC, MFD_ITEM_SLOT, FALSE, MFD_ACTIVE, FALSE);
   return TRUE;
}

errtype mfd_emailware_init(MFD_Func* f)
{
   int cnt = 0;
   LGPoint bsize;
   LGPoint bdims;                                     
   LGRect r;
   errtype err;
   bsize.x = res_bm_width(REF_IMG_EmailButt0);
   bsize.y = res_bm_height(REF_IMG_EmailButt0);
   bdims.x = NUM_EMAIL_BUTTONS;
   bdims.y = 1;
   r.ul.x = EMAIL_BARRAY_X;
   r.ul.y = EMAIL_BARRAY_Y;
   r.lr.x = r.ul.x + EMAIL_BARRAY_WD;
   r.lr.y = r.ul.y + bsize.y;
   err = MFDBttnArrayInit(&f->handlers[cnt++],&r,bdims,bsize,mfd_email_button_handler,NULL);
   if (err != OK) return err;
   f->handler_count = cnt;
   return OK;
}
//=====================================================
//                THE EMAIL WARE
//=====================================================
short last_email_taken = 0;

void email_turnon(bool ,bool real_start)
{
   bool flash=player_struct.hardwarez_status[HARDWARE_EMAIL]&WARE_FLASH;
   current_email_base = EMAIL_BASE_ID;
   player_struct.hardwarez_status[HARDWARE_EMAIL] &= ~(WARE_FLASH);
   QUESTBIT_OFF(0x12c);
   if (real_start)
   {
      inventory_draw_new_page(email_pages[flash?EMAIL_VER:last_email_taken]);
      set_inventory_mfd(MFD_INV_HARDWARE,HARDWARE_EMAIL,TRUE);
      mfd_change_slot(mfd_grab_func(MFD_EMAILWARE_FUNC,MFD_ITEM_SLOT),MFD_ITEM_SLOT);
   }
}

void email_turnoff(bool ,bool real_stop)
{
   if (real_stop)
   {
      int mfd_id = NUM_MFDS;
      while(mfd_yield_func(MFD_EMAILWARE_FUNC,&mfd_id))
      {
         restore_mfd_slot(mfd_id);
      }
   }
}

// every 30 nerd seconds, check to see if we have unread email and flash the email ware 
// if we do. 

#define FLASH_TIME_INTERVAL 30

void update_email_ware(void)
{
   int i;
   if ((player_struct.game_time >> APPROX_CIT_CYCLE_SHFT) % FLASH_TIME_INTERVAL != 0)
      return;

   for (i = 0; i < NUM_EMAIL_PROPER; i++)
   {
      uchar s = player_struct.email[i];
      if ((s & EMAIL_GOT) != 0 && (s & EMAIL_READ) == 0)
         goto found;
   }
   return;
found:
   player_struct.hardwarez_status[HARDWARE_EMAIL]|=WARE_FLASH;
}

//=====================================================
//               THE EMAIL INVENTORY PAGE
//=====================================================

#define BUFSZ 50

char* email_name_func(void*, int num, char* buf)
{
   return get_email_title_string(num,buf,BUFSZ);
}

uchar email_color_func(void* , int num)
{
   return(player_struct.email[num] & EMAIL_READ?0x5C:0x59);
}

// SHODAN wacky earth destroying hacking
void email_slam_hack(short which)
{
   void add_email_datamunge(short munge,bool select);

   add_email_datamunge(which,TRUE);
   read_email(EMAIL_BASE_ID,which);
}
