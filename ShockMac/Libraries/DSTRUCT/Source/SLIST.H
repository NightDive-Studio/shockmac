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
//		Slist.H		Singly-linked list header file
//		Rex E. Bradford (REX)
/*
* $Header: n:/project/lib/src/dstruct/RCS/slist.h 1.1 1993/05/03 10:53:29 rex Exp $
* $Log: slist.h $
 * Revision 1.1  1993/05/03  10:53:29  rex
 * Initial revision
 * 
*/

#ifndef SLIST_H
#define SLIST_H

#include "types.h"

//	--------------------------------------------------------------

//	Slist node

typedef struct _slist {
	struct _slist *psnext;	// ptr to next node or NULL if last one
									// real data follows, here
} slist;

//	Slist header

typedef struct _slist_head {
	struct _slist *psnext;				// ptr to 1st item in list
} slist_head;

//	Initialize an slist header (must be done before use)

#define slist_init(pslh) { (pslh)->psnext = NULL; }

//	Add a new slist node to head of list

#define slist_add_head(pslh,psl) { \
	(psl)->psnext = slist_head(pslh); \
	(pslh)->psnext = (slist *) psl; \
	}

//	Insert after specified node

#define slist_insert_after(psl,pnode) { \
	(psl)->psnext = (pnode)->psnext; \
	(pnode)->psnext = (slist *) psl; \
	}

//	Remove node (must specify prior node)

#define slist_remove(psl,pslbefore) { (pslbefore)->psnext = (psl)->psnext; }

//	Get ptr to head slist node

#define slist_head(pslh) (slist *)((pslh)->psnext)

//	Determine if list empty

#define slist_empty(pslh) (slist_head(pslh) == NULL)

//	Get next node

#define slist_next(psl) (slist *)((psl)->psnext)		// get ptr to next node

//	Iterate across all items

#define forallinslist(listtype,pslh,psl) for (psl = \
	(listtype *)slist_head(pslh); psl != NULL; psl = (listtype *)slist_next(psl))


#endif
