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
#ifndef __KB_H 
#define __KB_H 
/*
 * $Source: n:/project/lib/src/input/RCS/kbs.h $
 * $Revision: 1.1 $
 * $Author: kaboom $
 * $Date: 1994/02/12 18:28:21 $
 *
 * Types for keyboard system.
 *
 * This file is part of the input library.
 */

#ifndef __KBS_H
#define __KBS_H
typedef struct {
   uchar code;
   uchar state;
   uchar ascii;				// Added for Mac version
   uchar modifiers;			//   "    "   "     "
} kbs_event;
#endif /* !__KBS_H */

/*
 * $Source: n:/project/lib/src/input/RCS/kbdecl.h $
 * $Revision: 1.4 $
 * $Author: kaboom $
 * $Date: 1994/02/12 18:21:29 $
 *
 * Declarations for keyboard library.
 *
 * $Log: kbdecl.h $
 * Revision 1.4  1994/02/12  18:21:29  kaboom
 * Moved event structure.
 * 
 * Revision 1.3  1993/04/29  17:19:59  mahk
 * added kb_get_cooked
 * 
 * Revision 1.2  1993/04/28  17:01:48  mahk
 * Added kb_flush_bios
 * 
 * Revision 1.1  1993/03/10  17:16:41  kaboom
 * Initial revision
 * 
 */

#ifdef __INLINE_FUNCTIONS__
#define kb_state(code) (kbd_lowmem_start[KBD_ARRAY_START+code]&KBA_STATE)
#else
extern uchar kb_state(uchar code);
#endif

#define kb_init kb_startup
#define kb_close kb_shutdown
extern int kb_startup(void *init_buf);
extern int kb_shutdown(void);

extern kbs_event kb_next(void);
extern kbs_event kb_look_next(void);
extern void kb_flush(void);
extern uchar kb_get_state(uchar kb_code);
extern void kb_clear_state(uchar kb_code, uchar bits);
extern void kb_set_state(uchar kb_code, uchar bits);
extern void kb_set_signal(uchar code, uchar int_no);
extern int kb_get_flags();
extern void kb_set_flags(int flags);
extern void kb_generate(kbs_event e);
//extern void kb_flush_bios(void);				// For Mac version
#define kb_flush_bios	kb_flush
extern bool kb_get_cooked(ushort* key);
#define KBA_STATE (1)
#define KBA_REPEAT (2)
#define KBA_SIGNAL (4)
#define __KBD_INC (1)
extern char * kbd_lowmem_start;
#define __KBERR_INC (1)
#define KBE_ALLOC_LOWMEM (0)
#define KBE_FREE_LOWMEM (1)
#define KBE_MEM_THRASHED (2)
#define KBE_REAL_HANDLER (3)
#define KBE_PROT_HANDLER (4)
#define KBE_MEM_LOCK (5)
#define KBE_MEM_UNLOCK (6)
#define KBF_BLOCK (1   )
#define KBF_CHAIN (2   )
#define KBF_SIGNAL (4   )
#define KBD_GLOBAL_START (0)
#define KBD_QUEUE_HEAD (KBD_GLOBAL_START)
#define KBD_LAST_CODES (KBD_QUEUE_HEAD+4)
#define KBD_OLD_REAL_HANDLER (KBD_LAST_CODES+4)
#define KBD_STATUS_FLAGS (KBD_OLD_REAL_HANDLER+4)
#define KBD_GLOBAL_SIZE (KBD_STATUS_FLAGS+4)
#define KBD_QUEUE_START (KBD_GLOBAL_SIZE)
#define KBD_QUEUE_SIZE (1024)
#define KBD_QUEUE_END (KBD_QUEUE_START+KBD_QUEUE_SIZE)
#define KBD_ARRAY_START (KBD_QUEUE_END)
#define KBD_ARRAY_SIZE (256)
#define KBD_ARRAY_END (KBD_ARRAY_START+KBD_ARRAY_SIZE)
#define KBD_SIGLIST_START (KBD_ARRAY_END)
#define KBD_SIGLIST_SIZE (256)
#define KBD_SIGLIST_END (KBD_SIGLIST_START+KBD_SIGLIST_SIZE)
#define KBD_LOWBUF_SIZE (KBD_SIGLIST_END)
#define KBD_HANDLER_START (KBD_LOWBUF_SIZE)
#define KBC_SHIFT_PREFIX (0x0e0)
#define KBC_PAUSE_PREFIX (0x0e1)
#define KBC_PAUSE_DOWN (0x0e11d)
#define KBC_PAUSE_UP (0x0e19d)
#define KBC_PRSCR_DOWN (0x02a)
#define KBC_PRSCR_UP (0x0aa)
#define KBC_PAUSE (0x07f)
#define KBC_NONE (0x0ff)
#define KBS_UP (0)
#define KBS_DOWN (1)
#endif /* !__KB_H */
