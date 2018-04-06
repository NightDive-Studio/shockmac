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
 * $RCSfile: fixtest.c $
 * $Author: rex $
 * $Date: 1993/11/11 13:51:32 $
 *
 * program to test fixed-point math functions.
 *
 * $Log: fixtest.c $
 * Revision 1.8  1993/11/11  13:51:32  rex
 * Added atofix() test
 * 
 * Revision 1.7  1993/09/17  13:03:38  dfan
 * fast_pyth_dist
 * 
 * Revision 1.6  1993/07/30  12:43:00  dfan
 * fix_exp
 * 
 * Revision 1.5  1993/04/19  13:31:52  dfan
 * individual sin & cos functions
 * 
 * Revision 1.4  1993/03/03  11:51:11  dfan
 * float conversion
 * 
 * Revision 1.3  1993/01/28  12:29:03  dfan
 * sqrt test
 * 
 * Revision 1.2  1993/01/22  09:57:57  dfan
 * Tests the new functions now
 * 
 * Revision 1.1  1992/09/16  20:18:11  kaboom
 * Initial revision
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "fix.h"
#include <FixMath.h>

#define MAX_ARGS 8

#define sign(x) ((x)>0?1:-1)

typedef struct command {
	char *str;
	uchar cmd;
} command;

int num_args;
int args[MAX_ARGS];
int args_neg[MAX_ARGS];
char cmd;

#define CMD_NONE		0
#define CMD_ADD			1
#define CMD_SUB			2
#define CMD_MUL			3
#define CMD_DIV			4
#define CMD_SINCOS		5
#define CMD_FASTSINCOS	6
#define CMD_ATAN2		7
#define CMD_DIST		8
#define CMD_ASIN		9
#define CMD_ACOS		10
#define CMD_SQRT		11
#define CMD_FLOAT		12
#define CMD_SIN			13
#define CMD_COS			14
#define CMD_EXP			15
#define CMD_FASTDIST    16
#define CMD_ATOFIX		17
#define CMD_MULDIV		18
#define CMD_MUL24		19
#define CMD_DIV24		20
#define CMD_HELP		98
#define CMD_QUIT		99


#define NUM_CMD_STRS	42
command cmd_list[] =
{
	"+",				CMD_ADD,
	"add",				CMD_ADD,

	"-",				CMD_SUB,
	"sub",				CMD_SUB,

	"*", 				CMD_MUL,
	"mul",				CMD_MUL,
	
	"/",				CMD_DIV,
	"div",				CMD_DIV,

	"sc",				CMD_SINCOS,
	"sincos",			CMD_SINCOS,

	"s",				CMD_SIN,
	"sin",				CMD_SIN,

	"c",				CMD_COS,
	"cos",				CMD_COS,

	"fsc",				CMD_FASTSINCOS,
	"fastsincos",		CMD_FASTSINCOS,

	"as",				CMD_ASIN,
	"asin",				CMD_ASIN,

	"ac",				CMD_ACOS,
	"acos",				CMD_ACOS,

	"a",				CMD_ATAN2,
	"a2",				CMD_ATAN2,
	"atan2",			CMD_ATAN2,

	"d",				CMD_DIST,
	"dist",				CMD_DIST,

	"fd",				CMD_FASTDIST,
	"fastdist",			CMD_FASTDIST,

	"sqrt",				CMD_SQRT,

	"exp", 				CMD_EXP,
	"e",				CMD_EXP,

	"f",				CMD_FLOAT,

	"af",				CMD_ATOFIX,
	
	"md",				CMD_MULDIV,
	"muldiv",			CMD_MULDIV,

	"*24", 				CMD_MUL24,
	"mul24",			CMD_MUL24,

	"/24", 				CMD_DIV24,
	"div24",			CMD_DIV24,

	"?",				CMD_HELP,
	"help",				CMD_HELP,

	"q",				CMD_QUIT,
	"quit",				CMD_QUIT,
	"exit",				CMD_QUIT
};

bool check_args (int num)
{
	if (num_args >= num)
		return TRUE;
	else
	{
		printf ("Need %d args\n", num);
		return FALSE;
	}
}

// okay, this is now hairy enough that I should probably comment it
void parse (char *str, bool command)
{
	char *c;										// pointer to current char in str
	int val;										// value of current arg
	bool neg = FALSE;							// is this arg negative?
	bool frac = FALSE;						// is this arg really a fraction (after decimal point)?
	int i;										// counter
	int den;										// denominator of fraction

	// Prepare for death
	num_args = 0;
	c = str;
	while (isspace(*c)) c++;
	if (command) cmd = CMD_NONE;
	if (*c == NULL) return;

	// Look for matching commands, and then skip over it
	if (command)
	{
		for (i = 0; i < NUM_CMD_STRS; i++)
		{
//			if (strnicmp (cmd_list[i].str, c, strlen (cmd_list[i].str)) == 0 &&
			if (strncmp (cmd_list[i].str, c, strlen (cmd_list[i].str)) == 0 &&
				  isspace(*(c+strlen(cmd_list[i].str))))
				break;
		}

		if (i < NUM_CMD_STRS)
		{
			c += strlen (cmd_list[i].str);
			cmd = cmd_list[i].cmd;
		}
		else
			return;
	}
	while (isspace(*c)) c++;

	// Stupid first time around stuff
	val = 0;
	if (*c == '-')
	{
		args_neg[0] = neg = TRUE; c++;
	}
	else
		args_neg[0] = neg = FALSE;

	while (*c != NULL)
	{
		// We have now gotten to the next non-whitespace char
		if (isdigit(*c))
		{
			// Update our numbers
			val = val * 10 + (*c++ - '0');
			den *= 10;
		}
		else
		{
			val *= (neg ? -1 : 1);
			if (frac)
			{
				val = (val << 16) / den;	// convert to fraction of 0x10000
//24				val = (val << 8) / den;	// convert to fraction of 0x100
				// Hoo boy, is this ugly
				// If the user inputs -4.75, that's really an integer part of -5
				// and a fractional part of .25.  So deal accordingly.
				if (args_neg[num_args-1] && val != 0)
				{
					args[num_args-1] --;
					val = 0xffff - val + 1;
//24					val = 0xff - val + 1;
				}
			}
			args[num_args++] = val;			// store the val away

			// are we about to do a decimal part?
			if (*c == '.')
			{
				frac = TRUE;
				den = 1;
			}
			else
				frac = FALSE;

			// prepare for the next argument
			c++;
			args_neg[num_args] = neg = FALSE;
			if (num_args == MAX_ARGS) break;
			while (isspace(*c)) c++;
			val = 0;
			if (*c == '-')
			{
				args_neg[num_args] = neg = TRUE; c++;
			}
		}	
	}

//	for (i = 0; i < num_args; i++)
//		printf ("%d ", args[i]);
//	printf ("\n");
}

void test_add (void)
{
	fix a, b, c;
	char ans[80];

	if (!check_args(4)) return;
	a = fix_make (args[0],args[1]);
	b = fix_make (args[2],args[3]);
	c = a + b;
	fix_sprint (ans, c);
	puts (ans);
}

void test_sub (void)
{
	fix a, b, c;
	char ans[80];

	if (!check_args(4)) return;
	a = fix_make (args[0],args[1]);
	b = fix_make (args[2],args[3]);
	c = a - b;
	fix_sprint (ans, c);
	puts (ans);
}

void test_mul (void)
{
	fix a, b, c;
	char ans[80];

	if (!check_args(4)) return;
	a = fix_make (args[0],args[1]);
	b = fix_make (args[2],args[3]);
	c = fix_mul(a, b);
	fix_sprint (ans, c);
	puts (ans);
}

void test_div (void)
{
	fix a, b, c;
	char ans[80];
	wide	d;
	long	rem;

	if (!check_args(4)) return;
	a = fix_make (args[0],args[1]);
	b = fix_make (args[2],args[3]);
	c = fix_div(a, b);
	printf("0x%08x    %d   ", c, gOVResult);
	fix_sprint (ans, c);
	puts (ans);
}



void test_mul_div (void)
{
	fix a, b, c, d;
	char ans[80];

	if (!check_args(6)) return;
	a = fix_make (args[0],args[1]);
	b = fix_make (args[2],args[3]);
	c = fix_make (args[4],args[5]);
	d = fix_mul_div(a, b, c);
	fix_sprint (ans, d);
	puts (ans);
}


void test_mul24 (void)
{
	fix24 a, b, c;
	char ans[80];

	if (!check_args(4)) return;
	a = fix24_make (args[0],args[1]);
	b = fix24_make (args[2],args[3]);
	c = fix24_mul (a, b);
	fix24_sprint (ans, c);
	puts (ans);
}


void test_div24(void)
{
	fix24 a, b, c;
	char ans[80];

	if (!check_args(4)) return;
	a = fix24_make (args[0],args[1]);
	b = fix24_make (args[2],args[3]);
	c = fix24_div (a, b);
	fix24_sprint (ans, c);
	puts (ans);
}


void test_sincos (void)
{
	fixang th;
	fix s, c;
	char sstr[80], cstr[80];

	if (!check_args(1)) return;
	th = args[0] * 0x10000 / 360;
	fix_sincos (th, &s, &c);
	fix_sprint (sstr, s);
	fix_sprint (cstr, c);
	printf ("sin %s cos %s\n", sstr, cstr);
}

void test_sin (void)
{
	fixang th;
	fix s;
	char sstr[80];

	if (!check_args(1)) return;
	th = args[0] * 0x10000 / 360;
	s = fix_sin (th);
	fix_sprint (sstr, s);
	printf ("sin %s\n", sstr);
}

void test_cos (void)
{
	fixang th;
	fix c;
	char cstr[80];

	if (!check_args(1)) return;
	th = args[0] * 0x10000 / 360;
	c = fix_cos (th);
	fix_sprint (cstr, c);
	printf ("cos %s\n", cstr);
}

void test_fastsincos (void)
{
	fixang th;
	fix s, c;
	char sstr[80], cstr[80];

	if (!check_args(1)) return;
	th = args[0] * 0x10000 / 360;
	fix_fastsincos (th, &s, &c);
	fix_sprint (sstr, s);
	fix_sprint (cstr, c);
	printf ("sin %s cos %s\n", sstr, cstr);
}

void test_atan2 (void)
{
	fix a, b, c;

	if (!check_args(4)) return;
	a = fix_make (args[0],args[1]);
	b = fix_make (args[2],args[3]);
	c = fix_atan2 (a, b);
	printf ("%04x\n", c);
}

void test_dist (void)
{
	fix a, b, c;
	char ans[80];

	if (!check_args(4)) return;
	a = fix_make (args[0],args[1]);
	b = fix_make (args[2],args[3]);
	c = fix_pyth_dist (a, b);
	fix_sprint (ans, c);
	puts (ans);
}

void test_fastdist (void)
{
	fix a, b, c;
	char ans[80];

	if (!check_args(4)) return;
	a = fix_make (args[0],args[1]);
	b = fix_make (args[2],args[3]);
	c = fix_fast_pyth_dist (a, b);
	fix_sprint (ans, c);
	puts (ans);
}

void test_asin (void)
{
	fix a, c;

	if (!check_args(2)) return;
	a = fix_make (args[0],args[1]);
	c = fix_asin (a);
	printf ("%04x\n", c);
}

void test_acos (void)
{
	fix a, c;

	if (!check_args(2)) return;
	a = fix_make (args[0],args[1]);
	c = fix_acos (a);
	printf ("%04x\n", c);
}

void test_sqrt (void)
{
	fix a, c;
	char ans[80];

	if (!check_args(2)) return;
	a = fix_make (args[0],args[1]);
	c = fix_sqrt (a);
	fix_sprint (ans, c);
	puts (ans);
}

void test_float (void)
{
	fix a;
	float c;

	if (!check_args(2)) return;
	a = fix_make (args[0],args[1]);
	c = fix_float (a);
	printf ("%f\n", c);
}

void test_exp (void)
{
	fix a, c;
	char ans[80];

	if (!check_args(2)) return;
	a = fix_make (args[0],args[1]);
	c = fix_exp (a);
	fix_sprint (ans, c);
	puts (ans);
}

void test_atofix(void)
{
	fix a;
	char buff[80];

	printf("Enter number: ");
	gets(buff);
	a = atofix(buff);
	fix_sprint(buff, a);
	puts(buff);
}


void help ()
{
	printf ("Enter all numbers with decimal point, e.g. 5.0, -2.57\n");
	printf ("Functions:\n");
	printf ("  + a.b c.d\n");
	printf ("  - a.b c.d\n");
	printf ("  * a.b c.d\n");
	printf ("  / a.b c.d\n");
	printf ("  muldiv a.b c.d e.f\n");
	printf ("  sincos a (in degrees)\n");
	printf ("  sin a\n");
	printf ("  cos a\n");
	printf ("  fastsincos a (in degrees)\n");
	printf ("  asin a.b\n");
	printf ("  acos a.b\n");
	printf ("  atan2 a.b c.d (y x)\n");
	printf ("  dist a.b c.d (sqrt ((a.b)^2 + (c.d^2)))\n");
	printf ("  sqrt a.b\n");
	printf ("  exp a.b\n");
}

void main ()
{
	char ans[80];

	printf ("Enter a calculation with the operator first.  For example\n\n");
	printf ("+ 5.0 3.14\n* 0.324 -15.1\n\n");
	printf ("'?' for more help, 'cmd-Q' to quit\n\n");
	
	while (TRUE)
	{	
		printf (": ");
		fgets (ans, sizeof(ans), stdin);
		parse (ans, TRUE);
		if (cmd != CMD_NONE)
		{
			switch (cmd)
			{
			case CMD_ADD:		test_add ();		break;
			case CMD_SUB:		test_sub ();		break;
			case CMD_MUL:		test_mul ();		break;
			case CMD_DIV:		test_div ();		break;
			case CMD_SINCOS:	test_sincos ();		break;
			case CMD_SIN:		test_sin ();		break;
			case CMD_COS:		test_cos ();		break;
			case CMD_FASTSINCOS:test_fastsincos ();	break;
			case CMD_ATAN2:		test_atan2();		break;
			case CMD_DIST:		test_dist();		break;
			case CMD_FASTDIST:	test_fastdist();	break;
			case CMD_ASIN:		test_asin();		break;
			case CMD_ACOS:		test_acos();		break;
			case CMD_FLOAT:		test_float();		break;
			case CMD_SQRT:		test_sqrt();		break;
			case CMD_EXP:		test_exp();			break;
			case CMD_ATOFIX:	test_atofix();		break;
			case CMD_MULDIV:	test_mul_div();		break;
			case CMD_MUL24:		test_mul24();		break;
			case CMD_DIV24:		test_div24();		break;
			case CMD_HELP:		help();				break;
			case CMD_QUIT:		exit (0);
			default:	printf ("Does not compute\n");	break;
			}
		}
	}
}
