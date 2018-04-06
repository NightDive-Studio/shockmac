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
 * the texttool, a generic text editor shell sort of thing
 * only infinitely lamer
 */


#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "lg.h"
//#include <mprintf.h>
#include "texttool.h"

#define _tt_top(tid) if (tid==NULL) tid=cur_tt; _tt=tid;

#define StrDraw  _tt->lfont->s_draw
#define CharDraw _tt->lfont->c_draw
#define StrWid   _tt->lfont->s_wid
#define CharWid  _tt->lfont->c_wid
#define LenClr   _tt->lfont->l_clr
#define CharClr  _tt->lfont->c_clr
#define StrClr   _tt->lfont->s_clr
#define LGCursor   _tt->lfont->cursor

#define LineExist(_lt,lnum) ((lnum>=0)&&(lnum<_lt->max_h)&&(_lt->lines[lnum]!=NULL))
#define LastChar(_lt,lnum)  (_lt->lines[lnum][_lt->line_info[lnum].stl-1])

// internal prototypes
void _tt_build_cheat(long line_num);
void _tt_new_line(long line_num);
TextTool *tt_full_build(TTRect *pos, TTState *es, TTFontInfo *ttf, void *output_data, char *keymap, void *(d_func)(void *, LGRect *));
void tt_resize(TextTool *tt, int wid, int height);
void _tt_display_line(long line_num, long p_left, long p_right);
void _tt_show_line(long line_num, long p_left, long p_right);
int _tt_word_len(int which_word, char *s, long pos);
void _tt_resize_line(long line_num, long new_len);
void _tt_break_line(long line_num, long break_pt);
void _tt_rem_front(long line_num, long rem_pos);
void _tt_rem_mid(long line_num, long left_c, long right_c);
bool _tt_wrap_check(long *line_num, long *cur_pos);
bool _tt_add_char(long *line_num, long *cur_pos, char c);
bool _tt_del_chars(long *line_num, long *cur_pos, int cnt);
bool _tt_chg_line(int how);
bool _tt_chg_colu(int how);
void _tt_return(void);
ulong _tt_check_cursor_position(void);
int _tt_do_event(long tt_event);

// static local coolness
static TextTool *cur_tt=NULL, *_tt=NULL;

// Must choose one of these font headers to actually use....  and then, of course
// make sure you link appropriately also.  Is there a more graceful way to deal?
// #include <monofont.h>
// #include <realfont.h>
#include "tngfont.h"
#include "fakefont.h"

static TTRect     TTDefRect  = {{0,0},80,24};
static TTState    TTDefState = {0,66,0,TTS_WRAP|TTS_FULL,1,TTEV_NULL};
//static TTFontInfo TTDefFont  =
//       {mono_s_wid, mono_s_draw, mono_c_wid, mono_c_draw, mono_s_clr, mono_l_clr, mono_c_clr, mono_cursor, 1, 1, TTFI_FIXED };
//static TTKeyMap   TTDefKMap;

TTFontInfo TTDefFont  =
       {fnt_s_width, fnt_s_draw, fnt_c_width, fnt_c_draw, fnt_s_clr, fnt_l_clr, fnt_c_clr, fnt_cursor, 7, 7, TTFI_PROP|TTFI_SPACE };



// public useful stuff
TextTool *tt_full_build(TTRect *pos, TTState *es, TTFontInfo *ttf, void *output_data, char *keymap, void (*d_func)(void *, LGRect *))
{
   TextTool *new_tt;
   char *dummy;
   dummy = keymap;

   new_tt=(TextTool *)NewPtr(sizeof(TextTool));
   _tt_top(new_tt);
   _tt->max_w=_tt->max_h=_tt->cur_w=_tt->cur_h=0;
   if (pos!=NULL) _tt->scr_loc=*pos;
   else           _tt->scr_loc=TTDefRect;
   _tt->disp_x=_tt->disp_y=0;                // focus on upper left corner
   if (ttf!=NULL) _tt->lfont=ttf;
   else           _tt->lfont=&TTDefFont;
   _tt->output_data = output_data;
   _tt->display_func = d_func;
   _tt->lines=(char **)NewPtr(1);
   _tt->line_info=(TTCheats *)NewPtr(1);
   _tt->disp_rows=(_tt->scr_loc.h+_tt->lfont->height-1)/_tt->lfont->height;
   _tt_new_line(0);
   if (es!=NULL)  _tt->es=*es;
   else           _tt->es=TTDefState;
   return new_tt;
}

bool tt_toast(TextTool *old_tt)
{
   int i;
   for (i=0; i<old_tt->max_h; i++)
      DisposePtr((Ptr)old_tt->lines[i]);
   DisposePtr((Ptr)*(old_tt->lines));
   DisposePtr((Ptr)old_tt->line_info);
   return TRUE;
}

// set this tt to be the default one
bool tt_set(TextTool *def_tt)
{
   cur_tt=def_tt;
   return TRUE;
}

void tt_move(TextTool *tt, int xoff, int yoff)
{
   _tt_top(tt);
   _tt->scr_loc.crn.pt.x=xoff; _tt->scr_loc.crn.pt.y=yoff;
}

void tt_resize(TextTool *tt, int wid, int height)
{
   _tt_top(tt);
   _tt->scr_loc.w=wid; _tt->scr_loc.h=height;
   // do stuff here... recompute the universe, etc...
}

// move to a new line
// delete a line

// build a cheat for one line
void _tt_build_cheat(long line_num)
{
   TTCheats *loc=&(_tt->line_info[line_num]);
   int x_pix=0, x_chr=0, a_wid;
   char *sb=_tt->lines[line_num], *s;

   // recheck carefully for proportional font stuff...
   // should probably modularize out the selector for optimal expose events

   a_wid=strlen(s=sb);
   loc->stl=a_wid;
   // find the left edge
   while ((x_pix<_tt->disp_x)&&(x_chr<loc->stl))
    { x_pix+=CharWid(*s++); x_chr++; }
   if (x_chr==loc->stl)
    { loc->chr[0]=TTC_NOTHING; loc->chr[1]=0; loc->pix[0]=0; loc->pix[1]=0; return; }
   if (x_pix>_tt->disp_x)
    { x_pix-=CharWid(*--s); --x_chr; }
   loc->chr[0]=x_chr; loc->pix[0]=x_pix;
   // find right edge

// bug if string width is 1, basically   

   x_pix+=CharWid(*s++); x_chr++;
   while ((x_pix<_tt->disp_x+_tt->scr_loc.w)&&(x_chr<loc->stl))
    { x_pix+=CharWid(*s++); x_chr++; }
   if (x_chr==loc->stl)
    { x_pix-=CharWid(*--s); --x_chr; }
   loc->chr[1]=x_chr; loc->pix[1]=x_pix;

// mprintf("line %d (%s): cheat %d %d, %d %d\n",line_num,sb,loc->chr[0],loc->pix[0],loc->chr[1],loc->pix[1]);
}

// inserts a new line at line_num, inserts appropriately if necessary
void _tt_new_line(long line_num)
{
   _tt->max_h++;
//еее   _tt->lines=(char **)Realloc(_tt->lines,sizeof(char *)*_tt->max_h);
//еее   _tt->line_info=(TTCheats *)Realloc(_tt->line_info,sizeof(TTCheats)*_tt->max_h);
   DisposePtr((Ptr)_tt->lines);
   _tt->lines = (char **)NewPtr(sizeof(char *)*_tt->max_h);
   DisposePtr((Ptr)_tt->line_info);
   _tt->line_info = (TTCheats *)NewPtr(sizeof(TTCheats)*_tt->max_h);
   if (line_num<_tt->max_h-1)                 /* insert case */
   {
      LG_memmove(&_tt->line_info[line_num+1],&_tt->line_info[line_num],sizeof(TTCheats)*(_tt->max_h-line_num-1));
      LG_memmove(&_tt->lines[line_num+1],&_tt->lines[line_num],sizeof(char *)*(_tt->max_h-line_num-1));
   }
   _tt->line_info[line_num].wid=TTL_INIT;     		/* totally empty at first */
   _tt->line_info[line_num].flg|=TTC_FLG_RET; 		/* should be a return there */
   _tt->lines[line_num]=(char *)NewPtr(TTL_INIT);   /* and it is empty (\0) */
   _tt->lines[line_num][0]='\0';
   _tt_build_cheat(line_num);                 /* so we need a cheat for it */
}

#ifdef MOO
// returns whether there are characters on the line in the specified range
// sets c_l and c_r to the character counts for the pix counts in p_l and p_r
// based on the string in l_n
bool _tt_pix_cnv(long l_n, long p_l, long p_r, long *c_l, long *c_r)
{
      

}
#endif

// rebuild a range of cheats

// display one line starting at some coordinate
// display a rectangle of the area

// do appropriate gadgety things to display the line.
void _tt_display_line(long line_num, long p_left, long p_right)
{
   LGRect disp_rect;
   int llin;
 
   long dummy;
   dummy = p_left;
   dummy = p_right;

   llin=line_num-_tt->disp_y;
   disp_rect.ul.x = _tt->scr_loc.crn.pt.x+_tt->line_info[line_num].pix[0];
   disp_rect.ul.y = _tt->scr_loc.crn.pt.y+llin*_tt->lfont->height;
   disp_rect.lr.x = _tt->scr_loc.w - 1;
   disp_rect.lr.y = disp_rect.ul.y + _tt->lfont->height;
//   mprintf ("disp_rect = (%d, %d)(%d, %d)\n",disp_rect.ul.x, disp_rect.ul.y, disp_rect.lr.x, disp_rect.lr.y);
   tt_display_all(_tt, &disp_rect);  
}

// for real, display a partial (or full) line
void _tt_show_line(long line_num, long p_left, long p_right)
{
   int llin=line_num-_tt->disp_y, cy=line_num;
//   long c_left, c_right;
   bool blnk_line=TRUE;

   if ((line_num<_tt->disp_y)||(line_num>=_tt->disp_y+_tt->disp_rows)) return;
   if (p_left==-1) p_left=0; if (p_right==-1) p_right=_tt->scr_loc.w-1;
   if (LineExist(_tt,cy))
   {
      if (_tt->line_info[cy].chr[0]==TTC_REBUILD)
         _tt_build_cheat(cy);
#ifdef LOUD
      mono_setxy(60,llin);
      mprintf("%2.2d %2.2d %2.2d %2.2d          ",
         _tt->line_info[cy].chr[0],_tt->line_info[cy].chr[1],_tt->line_info[cy].wid,_tt->line_info[cy].stl);
#endif
//    _tt_pix_cnv(_tt->lines[cy],p_left,p_right,&c_left,&c_right);
      if (_tt->line_info[cy].chr[0]!=TTC_NOTHING)
      {
         char *st_base, *s, c;
         long wid;
	
  	      st_base=_tt->lines[cy];
  	      s=st_base+_tt->line_info[cy].chr[1]+1;
  	      c=*s; *s='\0';
         wid=StrDraw(st_base+_tt->line_info[cy].chr[0],
                     _tt->scr_loc.crn.pt.x+_tt->line_info[cy].pix[0],
                     _tt->scr_loc.crn.pt.y+llin*_tt->lfont->height);
         *s=c; blnk_line=FALSE;
         if (p_left+wid<p_right)
            LenClr(p_right-p_left-wid,_tt->scr_loc.crn.pt.x+wid,_tt->scr_loc.crn.pt.y+llin*_tt->lfont->height);
      }
   }
   if (blnk_line) LenClr(p_right-p_left,_tt->scr_loc.crn.pt.x,_tt->scr_loc.crn.pt.y+llin*_tt->lfont->height);
}

// do appropriate UI things to display the whole thing
void tt_display_all(TextTool *tt, LGRect *r)
{
   if (tt->display_func != NULL)
      tt->display_func(tt->output_data, r);
}

// fpr real, send full screen to output
void tt_show_all(TextTool *tt)
{
   long ay;
   _tt_top(tt);
   for (ay=0; ay<_tt->disp_rows; ay++)
      _tt_show_line(ay+_tt->disp_y,-1,-1);
   LGCursor(_tt->scr_loc.crn.pt.x+_tt->cur_w,_tt->scr_loc.crn.pt.y+_tt->cur_h-_tt->disp_y);
}

void tt_dump(TextTool *tt)
{
   long ay;
   _tt_top(tt);
   printf("DUMP:\n");
   for (ay=0; ay<_tt->max_h; ay++)
      printf("%5.5d %s\n", ay, _tt->lines[ay]);
}

// return the length of which_word (see defines TTWL_?) in char *s
int _tt_word_len(int which_word, char *s, long pos)
{
   char *p, *q, sw;
   int val=-1;                         /* so we can see if this is acting way zany */

   switch (which_word)
   {
   case TTWL_FIRST: p=strchr(s,' '); if (p==NULL) val=strlen(s); else val=p-s; break;
   case TTWL_LAST:  p=strrchr(s,' '); if (p==NULL) val=strlen(s); else val=s-p; break;
   case TTWL_CUR:   q=strchr(s+pos,' '); sw=*(s+pos); *(s+pos)='\0'; p=strrchr(s,' '); *(s+pos)=sw;
                    if ((q==NULL)&&(p==NULL)) val=strlen(s);
                    else if (q==NULL) val=strlen(s)-(p-s);
                    else if (p==NULL) val=q-s; else val=q-p; break;
   }
   return val;
}

void _tt_resize_line(long line_num, long new_len)
{
   long cur_len=_tt->line_info[line_num].wid;
   long n_num, new_targ;

   new_len+=2;                          // \0 and pad space
   n_num=(new_len-TTL_INIT)/TTL_BASE;   // num of chunks
   if (((new_len-TTL_INIT)%TTL_BASE)!=0) n_num++;
   new_targ=n_num*TTL_BASE+TTL_INIT;
   if (new_targ==cur_len) return;
   _tt->line_info[line_num].wid=new_targ;
//еее   _tt->lines[line_num]=Realloc(_tt->lines[line_num],new_targ);
   SetPtrSize((Ptr)_tt->lines[line_num],new_targ);
}

// fills line_num with s
void tt_fill_line(TextTool *tt, int how, long line_num, char *s)
{
   long r_len=strlen(s), loop;
   _tt_top(tt);
   if (line_num>=_tt->max_h)
      for (loop=_tt->max_h; loop<=line_num; loop++)
         _tt_new_line(loop);
   else
      if (how==TTF_INSWHOLE) _tt_new_line(line_num); /* insert a new line */
   if ((how==TTF_INSEND)||(how==TTF_INSFRONT))
   {
      if (_tt->line_info[line_num].stl>0)
		   r_len+=_tt->line_info[line_num].stl+2;      /* +2 for the space and the \0 */
      else
         how=TTF_INSWHOLE;                           /* if nothing there, simply replace */
   }
   _tt_resize_line(line_num,r_len);
   switch (how)
   {
   case TTF_REPLACE:
   case TTF_INSWHOLE: strcpy(_tt->lines[line_num],s); break;
   case TTF_INSEND:   strcat(_tt->lines[line_num],s); break;
   case TTF_INSFRONT:
      LG_memmove(_tt->lines[line_num]+strlen(s)+1,_tt->lines[line_num],_tt->line_info[line_num].stl+1);
      LG_memmove(_tt->lines[line_num],s,strlen(s));     /* dont need this \0 */
      _tt->lines[line_num][strlen(s)]=' ';
      break;
   }
   _tt_build_cheat(line_num);
   _tt_display_line(line_num,-1,-1);

   // should get gnosis of returns
}

// breaks off the first break_pt characters of line_num
void _tt_break_line(long line_num, long break_pt)
{
   if (break_pt>=_tt->line_info[line_num].stl) return;
   _tt->lines[line_num][break_pt]='\0';
   _tt_resize_line(line_num,break_pt);
   _tt_build_cheat(line_num);
   _tt_display_line(line_num,-1,-1);
}

void _tt_rem_front(long line_num, long rem_pos)
{
   int lin_len=_tt->line_info[line_num].stl;
   if (rem_pos>=lin_len) return;
   LG_memmove(_tt->lines[line_num],_tt->lines[line_num]+rem_pos, lin_len-rem_pos);
   _tt_resize_line(line_num,lin_len-rem_pos+1);
   _tt_build_cheat(line_num);
   _tt_display_line(line_num,-1,-1);
}

void _tt_rem_mid(long line_num, long left_c, long right_c)
{
   int lin_len=_tt->line_info[line_num].stl, rem_cnt=right_c-left_c+1;
   if ((rem_cnt>=lin_len)||(left_c>right_c)) return;
   LG_memmove(_tt->lines[line_num]+left_c,_tt->lines[line_num]+right_c, lin_len-right_c);
   _tt_resize_line(line_num,lin_len-rem_cnt);
   _tt_build_cheat(line_num);
   _tt_display_line(line_num,-1,-1);
}

// given that only line line_num changed, do wrap checks
bool _tt_wrap_check(long *line_num, long *cur_pos)
{
   char *s, *p, sw=0;                   /* pointers for manipulation, sw is the swap character */
   long ln=*line_num;
   bool wr=FALSE;
   // first, do we wrap back to the last line (should use pixwid, not stl)
   if (LineExist(_tt,ln-1)&&((_tt->line_info[ln-1].flg&TTC_FLG_RET)==0))
      if (_tt->line_info[ln-1].stl+1+_tt_word_len(TTWL_FIRST,_tt->lines[ln],0)<_tt->es.right_m)
         printf("Wrap back - note %d %d\n",_tt->line_info[ln-1].stl,_tt_word_len(TTWL_FIRST,_tt->lines[ln],0));
   // now, does our line wrap
   if (_tt->line_info[ln].stl>=_tt->es.right_m)
   {
      char *base=_tt->lines[ln];
      s=strrchr(base,' ');
      while ((s>base)&&(s-base>=_tt->es.right_m))
      {
         if (sw!=0) *p=sw;               /* put the savechar back */
         p=s; sw=*p; *p='\0'; s=strrchr(base,' ');   /* go back another word */
      }
      if (sw!=0) *p=sw;                  /* put any punted characters back */
      if (s>base)
      {                                  /* do the wrap */
         int brk=(++s)-base;             /* skip the space, figure out where to break */
         printf("Want to wrap -%s-...",s);
         tt_fill_line(NULL,TTF_INSFRONT,ln+1,s);
         _tt_break_line(ln,brk-1);
         _tt->line_info[ln].flg&=(~TTC_FLG_RET); /* punt the return */
         printf("Note strlen %d and stl %d for .%s./.%s.\n",strlen(s),_tt->line_info[(*line_num)+1].stl,s,_tt->lines[(*line_num)+1]);
         if ((*cur_pos)>brk)
          { (*line_num)++; (*cur_pos)-=brk; }
         wr=TRUE;
      }
   }
   // now, should we bring something in from the next line, and propagate down
   // send back ok error code
   return wr;
}

// currently returns whether the line wrapped? why? who knows.
bool _tt_add_char(long *line_num, long *cur_pos, char c)
{
   long new_stl=_tt->line_info[*line_num].stl, new_pos=*cur_pos;
   char *s=_tt->lines[*line_num];
   bool add_at_end=TRUE; 

   if (new_stl>*cur_pos)                         // insert
      if ((_tt->es.mode&TTS_OVER)==0)            // actually have to insert
         LG_memmove(&s[*(cur_pos)+1],&s[*cur_pos],new_stl-*cur_pos);
      else
         add_at_end=FALSE;                       // just a punch in
   if (add_at_end&&(_tt->es.max_w>0)&&(new_stl>=_tt->es.max_w))
      new_stl--;                                 // punt final character, we are out of space
   if (add_at_end)
  	  { s[*cur_pos]=c; c=0; new_pos=_tt->line_info[*line_num].stl=++new_stl; }
   if (new_stl>_tt->line_info[*line_num].wid-2)  // have to get more memory
      _tt_resize_line(*line_num,new_stl);
   if (new_stl>_tt->max_w) _tt->max_w=new_stl;
   _tt->lines[*line_num][new_pos]=c;             // put us down, R2
   (*cur_pos)++;                                 // go to next character
   _tt_build_cheat(*line_num);                   // we can do incremental cheats later
   if (_tt->es.mode&TTS_WRAP)
      if (_tt_wrap_check(line_num,cur_pos)) return TRUE;
   _tt_display_line(*line_num,-1,-1);
   return FALSE;
}

// learn this about return flag
bool _tt_del_chars(long *line_num, long *cur_pos, int cnt)
{
   int lin_wid=_tt->line_info[*line_num].stl, nlpos=*cur_pos, nrpos=*cur_pos;
//   int dir;
   char *s=_tt->lines[*line_num];

   if (cnt<0) nlpos+=cnt; else nrpos+=cnt;
   if ((nlpos>=0)&&(nrpos<=lin_wid))
   {                                   /* easy case... all on the line */
      LG_memmove(s+nlpos,s+nrpos,lin_wid-nrpos+1);
      _tt->line_info[*line_num].stl-=cnt;
      *cur_pos=nlpos;
      _tt_display_line(*line_num,-1,-1);
      _tt_build_cheat(*line_num);
   }
   else
   {                                   /* ugly scene, multi-line thingy */
      printf("Unhappy %d %d.. %d f %d\n",nlpos,nrpos,lin_wid,cnt);

//      cnt+=_tt->cur_w;
//      if (cnt>0) { cnt-=lin_wid; dir=1; } else dir=-1;
//      _tt_chg_line(dir);
//      if (cnt>0) _tt->cur_w=0; else _tt->cur_w=_tt->line_info[_tt->cur_h].stl;
//      _tt_chg_colu(cnt-dir);           /* extra bonus wrap character there... */
   }      
   return TRUE;
}

// return TRUE if we are out of space
bool _tt_chg_line(int how)
{
   bool edge=FALSE;                    /* edge of available space */
   how+=_tt->cur_h;
   if (how<0) { _tt->cur_h=0; edge=TRUE; }
   else if (how>=_tt->max_h)
   {
      if (_tt->es.mode&TTS_CGROW)
      {
	      for (_tt->cur_h=_tt->max_h; _tt->cur_h<=how; _tt->cur_h++)
   	      _tt_new_line(_tt->cur_h);
	      _tt->cur_w=0; _tt->cur_h--;      // back to the last line
      }
      else
       { edge=TRUE; _tt->cur_h=_tt->max_h-1; }
   }
   else
   {
      _tt->cur_h=how;
      if (_tt->cur_w>=_tt->line_info[how].stl)
         _tt->cur_w=_tt->line_info[how].stl;
   }
   return edge;
}

// returns if it hit the edge
bool _tt_chg_colu(int how)
{
   int dir, ncpos=_tt->cur_w+how, lin_wid=_tt->line_info[_tt->cur_h].stl;

   if ((ncpos>=0)&&(ncpos<=lin_wid))
      _tt->cur_w=ncpos;
   else
   {
      how+=_tt->cur_w;
      if (how>0) { how-=lin_wid; dir=1; } else dir=-1;
      if (_tt_chg_line(dir))
       { if (dir>0) _tt->cur_w=_tt->line_info[_tt->cur_h].stl; else _tt->cur_w=0; return TRUE; }
      if (how>0) _tt->cur_w=0; else _tt->cur_w=_tt->line_info[_tt->cur_h].stl;
      _tt_chg_colu(how-dir);           /* extra bonus wrap character there... */
   }
   return FALSE;
}

// should wrap to the next line
void _tt_return(void)
{
   bool line_gen=FALSE;


   switch (_tt->es.mode&TTS_MODE)
   {
   case TTS_FULL:
	   _tt->line_info[_tt->cur_h].flg|=TTC_FLG_RET;
	   if (_tt->cur_w<_tt->line_info[_tt->cur_h].stl)
   	{
	      tt_fill_line(NULL,TTF_INSWHOLE,_tt->cur_h+1,_tt->lines[_tt->cur_h]+_tt->cur_w);
	      _tt_break_line(_tt->cur_h,_tt->cur_w);
	      line_gen=TRUE;
	   }
	   _tt->cur_h++; _tt->cur_w=0;         /* go to beginning of next line */
	   if ((!line_gen)&&((_tt->es.mode&(TTS_OVER|TTS_READONLY))==0))
	      _tt_new_line(_tt->cur_h);
      break;
   case TTS_SINGLE:
   case TTS_LINES:
   case TTS_READONLY:
      break;
   }
}

// returns changes based on _tt->cur_w and h in terms of scrolling and such
ulong _tt_check_cursor_position(void)
{
   ulong changed=0;
   if (_tt->cur_h<_tt->disp_y)                          /* cursor off top of screen */
    { _tt->disp_y=_tt->cur_h; changed|=TTCHG_REDRAW; }  /* focus up there */
   if (_tt->cur_h>=_tt->disp_y+_tt->disp_rows)
    { _tt->disp_y=_tt->cur_h-_tt->disp_rows+1; changed|=TTCHG_REDRAW; }
   changed|=TTCHG_CURSOR;     // since mono_font has to keep moving it around all the time
   return changed;
}

// actually does things.  This takes events, which can come out of script files, macros, or keystroke parses
int _tt_do_event(long tt_event)
{
//   int old_w=_tt->cur_w, old_h=_tt->cur_h;
   int changed=0;

   if (tt_event&_ALL_M)
   {
      if ((tt_event&(_tt->es.mode&_ALL_M))==0)    /* is not an event in this mode */
         tt_event=TTEV_NULL;

      switch (tt_event)
      {
      case TTEV_RET:    _tt_return(); break;
      case TTEV_F_CHAR: _tt_chg_colu( _tt->es.r_cnt); break;
      case TTEV_B_CHAR: _tt_chg_colu(-_tt->es.r_cnt); break;
      case TTEV_F_LINE: _tt_chg_line( _tt->es.r_cnt); break;
      case TTEV_B_LINE: _tt_chg_line(-_tt->es.r_cnt); break;
      case TTEV_REPEAT: _tt->es.r_cnt*=4; break;
      case TTEV_DEL:    _tt_del_chars(&_tt->cur_h, &_tt->cur_w, _tt->es.r_cnt); break;
      case TTEV_BACKSP: _tt_del_chars(&_tt->cur_h, &_tt->cur_w,-_tt->es.r_cnt); break;
      case TTEV_KILL:   _tt_break_line(_tt->cur_h, _tt->cur_w); break;
      case TTEV_HOME:
      case TTEV_BOL:    _tt->cur_w=0; break;
      case TTEV_END:
      case TTEV_EOL:    _tt->cur_w=_tt->line_info[_tt->cur_h].stl; break;
      case TTEV_OVER:   if (_tt->es.mode&TTS_OVER) _tt->es.mode-=TTS_OVER; else _tt->es.mode+=TTS_OVER; break;
      case TTEV_WRAP:   if (_tt->es.mode&TTS_WRAP) _tt->es.mode-=TTS_WRAP; else _tt->es.mode+=TTS_WRAP; break;
      case TTEV_NULL:   break;
      }       // life is short and love is always over in the morning black wind come carry me far away
   }
   else                                                      /* normal key */
   {
      if (_tt->es.mode&TTS_READONLY) return TTCHG_NOCHANGE;  /* readonly mode, ignore non-event keys */
	   _tt_add_char(&_tt->cur_h,&_tt->cur_w,tt_event);
//        _tt_chg_line(1);
//     else
//        _tt_chg_colu(1);
   }
   if ((_tt->es.r_cnt!=1)&&(tt_event!=TTEV_REPEAT))
      _tt->es.r_cnt=1;                                       /* reset extra count */
//   if ((old_w!=_tt->cur_w)||(old_h!=_tt->cur_h)) changed|=TTCHG_CURSOR;
   changed|=_tt_check_cursor_position();
   _tt->es.last_ev=tt_event;
   return changed;
}

// converts an input character into an event for the tt system
// returns -1 or the line selected (w/return)
long tt_parse_char(TextTool *tt, ushort key_code)
{
   char c = (key_code & 0xFF); 
   int ret;
   int event = TTEV_NULL;
   _tt_top(tt);

   if (key_code & TEXTTOOL_KB_FLAG_CTRL)
   {
      switch (c)
      {
      case 'a': event=TTEV_BOL; break;
      case 'b': event=TTEV_B_CHAR; break;
      case 'd': event=TTEV_DEL; break;
      case 'e': event=TTEV_EOL; break;
      case 'f': event=TTEV_F_CHAR; break;
      case 'g': event=TTEV_NULL; break;
      case 'h': event=TTEV_BACKSP; break;
      case 'k': event=TTEV_KILL; break;
      case 'm': event=TTEV_RET; break;
      case 'n': event=TTEV_F_LINE; break;
      case 'o': event=TTEV_OVER; break;
      case 'p': event=TTEV_B_LINE; break;
      case 'u': event=TTEV_REPEAT; break;
      case 'w': event=TTEV_WRAP; break;
      }
   }
   else if (key_code & TEXTTOOL_KB_FLAG_SPECIAL)
   {
      switch(c)
      {
         case 0x48: event = TTEV_B_LINE; break;
         case 0x50: event = TTEV_F_LINE; break;
         case 0x4b: event = TTEV_B_CHAR; break;
         case 0x4d: event = TTEV_F_CHAR; break;
      }
   }
   else
   {
      switch(c)
      {
         case 0xd: event = TTEV_RET; break;
         case 0x8: event = TTEV_BACKSP; break;
      }
   }
//   mprintf ("event = %d\n  TTEV_RET = %d  TTEV_NULL = %d\n",event,TTEV_RET,TTEV_NULL);
   if (event == TTEV_NULL)
   {
      if (isprint(c)) event=c; 
   }
   ret=_tt_do_event(event);
   if (ret&TTCHG_CURSOR) LGCursor(_tt->scr_loc.crn.pt.x+_tt->cur_w,_tt->scr_loc.crn.pt.y+_tt->cur_h-_tt->disp_y);
   if (ret&TTCHG_REDRAW) tt_display_all(_tt,NULL);
   return (event==TTEV_RET)?_tt->cur_h:-1;
}

long tt_parse_string(TextTool *tt, char *st)
{
   char c;
   int ret, i;
   int event;
   _tt_top(tt);

   for (i=0; i<strlen(st); i++)
   {
      c = st[i];
      event = TTEV_NULL;
      if (c == '\n')
      { event = TTEV_RET; }
      else
      { if (isprint(c)) event=c; }
      ret=_tt_do_event(event);
      if (ret&TTCHG_CURSOR) LGCursor(_tt->scr_loc.crn.pt.x+_tt->cur_w,_tt->scr_loc.crn.pt.y+_tt->cur_h-_tt->disp_y);
      if (ret&TTCHG_REDRAW) tt_display_all(_tt,NULL);
   }
   return (event==TTEV_RET)?_tt->cur_h:-1;
}

char *tt_get(TextTool *tt, long line_num)
{
   _tt_top(tt);
   return (line_num>=_tt->max_h)?NULL:_tt->lines[line_num];
}






// i received a phone call lying in my bed, the other night
// i recognized her voice, she asked me out, i said all right
// i pulled up along the curb, she walked in front of my car
// she was looking nice, she was looking nice, and i said
// it was interesting watching her face as it was pressed against my windshield
// it was interesting watching her face as it was pressed against my windshield
//                                                        against my windshield

// feel things going through my head, troubling me and making me nervous
// feel things going through my head, troubling me and making me nervous

//                          - "Windshield", Green Magnet School, _Blood Music_
