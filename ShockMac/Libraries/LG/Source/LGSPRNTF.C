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
// lgsprntf.c -- no-floats baby sprintf

// Tim Stellmach
// LookingGlass Technologies 1993

/*
* $Header: r:/prj/lib/src/lg/rcs/lgsprntf.c 1.12 1994/08/15 16:52:29 tjs Exp $
* $Log: lgsprntf.c $
 * Revision 1.12  1994/08/15  16:52:29  tjs
 * length specifiers 'l' & 'h' supported, as far as that goes.
 * 
 * Revision 1.11  1994/08/15  13:12:33  jak
 * Don't write though null pointer.
 * 
 * Revision 1.10  1994/08/14  15:48:51  tjs
 * Precision support for strings.
 * 
 * Revision 1.9  1994/08/12  02:27:56  tjs
 * Let's try that again.
 * 
 * Revision 1.8  1994/08/12  02:17:32  tjs
 * %x=%p
 * 
 * Revision 1.7  1994/02/27  02:25:58  tjs
 * Changed format to const
 * 
 * Revision 1.6  1994/02/01  05:57:36  tjs
 * Added binary numbers, optimization for small decimal integers.
 * 
 * Revision 1.5  1993/12/17  12:10:16  tjs
 * Fixed bug with %%
 * 
 * Revision 1.4  1993/11/04  14:08:17  tjs
 * Added alternate form for bools.
 * 
 * Revision 1.3  1993/11/04  11:46:11  tjs
 * Added error message for %S if function not installed, precision
 * handling for fixed-point numbers.
 * 
 * Revision 1.2  1993/11/04  09:42:35  tjs
 * Fixed bug with decimal point placement in fixed-point numbers.
 * 
 *
 *
 *
*/

#include <string.h>
#include <limits.h>
#include <ctype.h>

#include "lgsprntf.h"
#include "fix.h"

#define MAX_FIX_PRECIS 4

static int int_to_str(char *buf, int val);
static int uint_to_str(char *buf, uint val, int base, char alph);

static char *boolstring[] = {"FALSE","TRUE"};
static int pten[MAX_FIX_PRECIS+1]={ 1, 10, 100, 1000, 10000 };

static char *(*sprintf_str_func)(ulong strnum)=NULL;

// For small positive integers, just indirect into the big
// magic array and copy the two bytes you find there.  This
// common usage is thus blindingly fast.

#ifdef LGSPF_SMALLINT_OPT

static char digiarray[] = "\
00112233445566778899\
10111213141516171819\
20212223242526272829\
30313233343536373839\
40414243444546474849\
50515253545556575859\
60616263646566676869\
70717273747576777879\
80818283848586878889\
90919293949596979899";

#endif


// note that in the future we may include STAGE_LEN
// if we start to support these things.  And everyone will
// drive electric cars.
typedef enum { STAGE_TEXT, STAGE_FLAGS, STAGE_FWID, STAGE_PRECISION } lgsStage;

typedef enum { SMALL, DEFAULT, BIG } bigness;

// lg_sprintf()
// should have the save behaviour as sprintf(), but supports only the
// %d, %u, %s, %c, %x, %X, %o, %n, and %% conversion characters, and does not 
// support the space and + flags.
// In addition, instead of supporting floats, we support formatting 
// of fixes and fix24's, using the conversion characters %f and %F, 
// respectively.  These are currently always formatted with four digits
// after the decimal place.  Also supports %b conversion characters for
// boolean values, formatting them as "TRUE" or "FALSE".  For those of you
// who wisely use a string system of some kind, you can refer to a
// string number with the %S conversion; this requires you install a
// function mapping string numbers to char *'s using (get ready)
// lg_sprintf_install_stringfunc() -- see below.

int lg_sprintf(char *buf, const char *format, ...)
{
   int chars;
   va_list arglist;

   va_start(arglist, format);
   chars=lg_vsprintf(buf, format, arglist);
   va_end(arglist);

   return(chars);
}


// lg_vsprintf()
// just like vsprintf(), except different from it in just those ways
// that lg_sprintf() is different from sprintf().  So there.

int lg_vsprintf(char *buf, const char *format, va_list arglist)
{
   fix arg_fix;
   fix24 arg_fix24;
   bool arg_bool;
   char *arg_str;
   uint arg_uint;
   int arg_int;
   char fix_frac_buf[5];

   bool ladjust, altform, pspec, this_is_len;
   bigness big;
   char pad_char;
   char src_char;
   int dest_ind, src_ind, len, int_part, frac_part, mult;
   int fwid, precis, newchars, fshift, shift_ind, prefix;
   lgsStage stage;

   if (buf==NULL) return 0;

   dest_ind=src_ind=0;
   stage=STAGE_TEXT;
   big=DEFAULT;

   while( src_char=format[src_ind++] ) {
      this_is_len=FALSE;
      if( stage!=STAGE_TEXT ) {
         switch( src_char ) {
            case '.':
               if(stage<STAGE_PRECISION) stage=STAGE_PRECISION;
               break;
            case '-':
               if(stage==STAGE_FLAGS)
                  ladjust=TRUE;
               break;
            case '#':
               if(stage==STAGE_FLAGS)
                  altform=TRUE;
               break;
            case '0':
               if(stage==STAGE_FLAGS) {
                  pad_char='0';
                  break;
               }
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
               if(stage<STAGE_FWID) stage=STAGE_FWID;
               if(stage==STAGE_FWID)
                  fwid=fwid*10+(src_char-'0');
               else {
                  pspec=TRUE;
                  precis=precis*10+(src_char-'0');
               }
               break;
            case '%': // actual percent character
               buf[dest_ind++]='%';
               stage=STAGE_TEXT;
               break;
            case 'h':
               this_is_len=TRUE;
               big=SMALL;
               break;
            case 'L':
            case 'l':
               this_is_len=TRUE;
               big=BIG; // of course, currently this is ingored.
               break;
            case 'n':
               newchars=fwid=0;
               *(va_arg(arglist,int*))=dest_ind;
               break;
            case 'i':
            case 'd': // int
               arg_int=va_arg(arglist,int);
#ifdef LGSPF_SMALLINT_OPT
               if(arg_int>=0 && arg_int<100) {
                  *((ushort*)buf)=((ushort*)digiarray)[arg_int];
                  newchars=(arg_int<10)?1:2;
               }
               else
#endif
                    {
                  newchars=int_to_str(buf+dest_ind,arg_int);
               }
               break;
            case 'u': // unsigned int
               arg_uint=va_arg(arglist,uint);
               if(big==SMALL)
                  arg_uint &= USHRT_MAX;
#ifdef LGSPF_SMALLINT_OPT
               if(arg_uint<100) {
                  *((ushort*)buf)=((ushort*)digiarray)[arg_uint];
                  newchars=(arg_uint<10)?1:2;
               }
               else
#endif
                    {
                  newchars=uint_to_str(buf+dest_ind,arg_uint,10,0);
               }
               break;
            case 'B': // binary
               arg_uint=va_arg(arglist,uint);
               buf[dest_ind]='0';
               buf[dest_ind+1]='b';
               newchars=2+uint_to_str(buf+dest_ind+2,arg_uint,2,0);
               break;
            case 'p':
               src_char='X';
            case 'x':
            case 'X':
               arg_uint=va_arg(arglist,uint);
               if(big==SMALL)
                  arg_uint &= USHRT_MAX;
               if(altform && arg_uint!=0) {
                  buf[dest_ind]='0';
                  buf[dest_ind+1]=src_char;
                  newchars=2+uint_to_str(buf+dest_ind+2,arg_uint,16,src_char-'X'+'A');
               }
               else
                  newchars=uint_to_str(buf+dest_ind,arg_uint,16,src_char-'X'+'A');
               break;
            case 'o': // unsigned, octal
               arg_uint=va_arg(arglist,uint);
               if(big==SMALL)
                  arg_uint &= USHRT_MAX;
               if(altform) {
                  buf[dest_ind]='0';
                  newchars=1+uint_to_str(buf+dest_ind+1,arg_uint,8,0);
               }
               else
                  newchars=uint_to_str(buf+dest_ind,va_arg(arglist,uint),8,0);
               break;
            case 'c': // char
               buf[dest_ind]=(unsigned char)va_arg(arglist,int);
               newchars=1;
               break;
            case 'b': // bool
               arg_bool=!!va_arg(arglist,bool);
               if(altform) {
                  buf[dest_ind]=boolstring[arg_bool][0];
                  newchars=1;
               }
               else {
                  strcpy(buf+dest_ind,boolstring[arg_bool]);
                  // yeah, this relies on the strings being of lengths
                  // 5 and 4 respectively.  So sue me.
                  newchars=5-arg_bool;
               }
               break;
            case 'S': // string number
               if(sprintf_str_func) {
                  arg_str=sprintf_str_func(va_arg(arglist,ulong));
                  goto string_copy;
               }
               else {
                  buf[dest_ind]=0;
                  return -1;
               }
               break;
            case 's': // string
               arg_str=va_arg(arglist,char*);
string_copy:
               if(arg_str) {
                  if(pspec)
                     strncpy(buf+dest_ind,arg_str,precis);
                  else
                     strcpy(buf+dest_ind,arg_str);
                  newchars=strlen(arg_str);
                  if(pspec && precis<newchars) newchars=precis;
               }
               break;
            case 'F':
            case 'f': // fix
               if(!pspec || (precis>MAX_FIX_PRECIS)) precis=MAX_FIX_PRECIS;
               mult=pten[precis];
               if(src_char=='f') {
                  arg_fix=va_arg(arglist,fix);
                  int_part=fix_int(arg_fix);
                  frac_part=fix_int(fix_round(fix_frac(arg_fix)*mult));
               }
               else {
                  arg_fix24=va_arg(arglist,fix24);
                  int_part=fix24_int(arg_fix24);
                  frac_part=fix24_int(fix24_round(fix24_frac(arg_fix24)*mult));
               }
               if(frac_part>=mult) {
                  frac_part=0;
                  int_part++;
               }
               if((int_part<0)&&(frac_part!=0)) {
                  int_part=int_part+1;
                  frac_part=mult-frac_part;
                  if(int_part==0)
                     buf[dest_ind++]='-';
               }
               newchars=int_to_str(buf+dest_ind,int_part);
               if(precis!=0) {
                  buf[dest_ind+newchars]='.';
                  len=int_to_str(fix_frac_buf,frac_part);
                  memset(buf+dest_ind+newchars+1,'0',precis);
                  strcpy(buf+dest_ind+newchars+precis+1-len,fix_frac_buf);
                  newchars+=precis+1;
               }
               else if(altform) {
                  buf[dest_ind+newchars]='.';
                  newchars++;
               }
               break;               
            }

         if(isalpha(src_char) && !this_is_len) {
            stage=STAGE_TEXT;
            if(newchars<fwid) {
               fshift=fwid-newchars;
               if(ladjust) {
                  memset(buf+dest_ind+newchars,' ',fshift);
               }
               else {
                  // non-numeric fields are never 0-padded
                  if(src_char=='s'||src_char=='S'||
                     src_char=='c'||src_char=='b')
                        pad_char=' ';

                  // set length of prefix not to be right-shifted
                  prefix=0;
                  if(pad_char=='0') {
                     if(buf[dest_ind]=='-')
                        prefix=1;
                     else if(altform) {
                        if(src_char=='o')
                           prefix=1;
                        else if(src_char=='x'||src_char=='X')
                           prefix=2;
                     }
                  }

                  for(shift_ind=dest_ind+fwid-1;shift_ind>=dest_ind+fshift+prefix;shift_ind--)
                     buf[shift_ind]=buf[shift_ind-fshift];
                  memset(buf+dest_ind+prefix,pad_char,fshift);
               }
               dest_ind+=fwid;
            }
            else
               dest_ind+=newchars;
         }
      }
      else {
         if( src_char=='%' ) {
            stage=STAGE_FLAGS;
            pad_char=' ';
            fwid=precis=0;
            ladjust=altform=pspec=FALSE;
         }
         else
            buf[dest_ind++]=src_char;
      }
   }

   buf[dest_ind]='\0';

   return(dest_ind);
}

// Install a string system function for use with the %S conversion
// specifier.  The function in question must take a ulong and return
// a char* (like the resource system's RefGet(), by some coincidence)
// and you are responsible for any necessary memory management for the
// string it returns.
void lg_sprintf_install_stringfunc(char *(*func)(ulong strnum))
{
   sprintf_str_func=func;
}




// ==== static functions follow =======================

// private local functions used for writing integers and uints into 
// strings.  The int version calls the uint version after doing any
// necessary setup for negative numbers.  The uint version does the
// real work, writing the integer into the string in reverse digit
// order and then reversing it in place.  The integer version always
// formats its argument in decimal, whereas the uint version takes
// an argument for the number base to use and is somewhat optimized
// for base 8 and base 16, using shifts and masks instead of divides
// and remainders.

static int int_to_str(char *buf, int val)
{
   int len;

   if(val<0) {
      buf[0]='-';
      // special case for INT_MIN, since -INT_MIN may not be a
      // valid integer
      if(val==INT_MIN) {
         // Don't rely on sign of remainders of negative dividends.
         // In a perfect world, compiler would fold the consants and
         // include only the appropriate block of code.
         if((-5)%2<=0) {
            len=int_to_str(buf+1,-(INT_MIN/10));
            buf[1+len]='0'-(INT_MIN%10);
         }
         else {
            len=int_to_str(buf+1,-(INT_MIN/10)-1);
            buf[1+len]='0'+10-(INT_MIN%10);
         }
         buf[2+len]='\0';
         return(2+len);
      }         
      return(1+int_to_str(buf+1,-val));
   }

   return(uint_to_str(buf,(uint)val,10,0));
}

static int uint_to_str(char *buf, uint val, int base, char alph)
{
   int ind, rev;
   char tmp;

   // alph is the first letter of the alphabet: 'a' for lowercace
   // and 'A' for upper.  Subtract 10 to find value to add to
   // hex digits.
   alph-=10;

   if(val==0) {
      buf[0]='0';
      buf[1]='\0';
      return(1);
   }

   ind=0;
   // convert in reverse order of digits, checking for base 8 & 16
   // to avoid division.
   switch(base) {
      case 8:
         while(val>0) {
            tmp=val&0x7;
            tmp=tmp+'0';
            buf[ind++]=tmp;
            val=val>>3;
         }
         break;
      case 16:
         while(val>0) {
            tmp=val&0xF;
            tmp=tmp+(tmp<10?'0':alph);
            buf[ind++]=tmp;
            val=val>>4;
         }
         break;
      default:
         while(val>0) {
            tmp=val%base;
            tmp=tmp+(tmp<10?'0':alph);
            buf[ind++]=tmp;
            val=val/base;
         }
         break;
   }

   // reverse string in place.
   for(rev=0;rev<(ind>>1);rev++) {
      tmp=buf[rev];
      buf[rev]=buf[ind-rev-1];
      buf[ind-rev-1]=tmp;
   }

   buf[ind]='\0';

   return ind;
}

