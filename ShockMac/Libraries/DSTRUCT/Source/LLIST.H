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
//		Llist.H		Double-linked list header file
//		Rex E. Bradford (REX)
/*
* $Header: n:/project/lib/src/dstruct/RCS/llist.h 1.5 1993/04/19 11:36:13 rex Exp $
* $Log: llist.h $
 * Revision 1.5  1993/04/19  11:36:13  rex
 * Arggh! More void* to make llist work!
 * 
 * Revision 1.4  1993/04/19  09:35:40  rex
 * Fixed llist macros again with void* so work with queues too (arrggh)
 * 
 * Revision 1.3  1993/04/19  09:26:41  rex
 * Fixed llist macros to do casting, so as to not get ptr warnings
 * 
 * Revision 1.2  1993/04/16  12:02:57  rex
 * Added llist_insert_before() and llist_insert_after(), dropped head arg
 * from llist_remove()
 * 
 * Revision 1.1  1993/04/16  11:04:26  rex
 * Initial revision
 * 
 * Revision 1.3  1993/04/15  16:25:40  rex
 * Made basic llist, derived Llist from it.
 * 
 * Revision 1.2  1993/01/12  17:54:37  rex
 * Modified includes in preparation for turning into library
 * 
 * Revision 1.1  1992/08/31  17:01:28  unknown
 * Initial revision
 * 
*/

#ifndef LLIST_H
#define LLIST_H

#include "lg_types.h"

//	--------------------------------------------------------------
//		LOW-LEVEL LINKED LIST
//	--------------------------------------------------------------

//	List node (data must follow)

typedef struct _llist {
	struct _llist *pnext;		// ptr to next node or NULL if at tail
	struct _llist *pprev;		// ptr to prev node or NULL if at head
										// real data follows, here
} llist;

//	Queue node (sorted list, 1st item is short priority)

typedef struct _queue {
	struct _queue *pnext;		// ptr to next node or NULL if at tail
	struct _queue *pprev;		// ptr to prev node or NULL if at head
	short priority;				// higher numbers go to head of list
										// real data follows, here
} queue;

//	List header

typedef struct _llist_head {
	llist head;						// head list item (not really in list)
	llist tail;						// tail list item (not really in list)
} llist_head;

//	Initialize a list header (must be done before use)

#define llist_init(plh) { \
	(plh)->head.pnext = llist_end(plh); \
	(plh)->tail.pprev = llist_beg(plh); \
	}

//	Add a new list node to head of list

#define llist_add_head(plh,pll) { \
	(pll)->pnext = llist_head(plh); \
	(pll)->pprev = llist_beg(plh); \
	((plh)->head.pnext)->pprev = (llist *) pll; \
	(plh)->head.pnext = (llist *) pll; \
	}

//	Add a new list node to tail of list

#define llist_add_tail(plh,pll) { \
	(pll)->pnext = llist_end(plh); \
	(pll)->pprev = llist_tail(plh); \
	((plh)->tail.pprev)->pnext = (llist *) pll; \
	(plh)->tail.pprev = (llist *) pll; \
	}

//	Insert before specified node

#define llist_insert_before(pll,pnode) { \
	(pll)->pprev = (queue *) (pnode)->pprev; \
	(pll)->pnext = (queue *) pnode; \
	(pnode)->pprev->pnext = (queue *) pll; \
	(pnode)->pprev = (queue *) pll; \
	}

//	Insert after specified node

#define llist_insert_after(pll,pnode) { \
	(pll)->pprev = (queue *) pnode; \
	(pll)->pnext = (queue *)(pnode)->pnext; \
	(pnode)->pnext->pprev = (queue *) pll; \
	(pnode)->pnext = (queue *) pll; \
	}

//	Add in priority order

void llist_insert_queue(llist_head *plh, queue *plq);

//	Move to new spot in queue (after inserting new priority)

bool llist_move_queue(llist_head *plh, queue *plq);

//	Remove node

#define llist_remove(pll) { \
	((pll)->pprev)->pnext = (pll)->pnext; \
	((pll)->pnext)->pprev = (pll)->pprev; \
	}

//	Get ptr to head or tail list nodes

#define llist_head(plh) (llist*)((plh)->head.pnext)
#define llist_tail(plh) (llist*)((plh)->tail.pprev)

//	Determine if list empty

#define llist_empty(plh) (llist_head(plh)==llist_end(plh))

//	Get # nodes in list

int llist_num_nodes(llist_head *plh);

//	Get next & prev nodes from a node

#define llist_next(pll) (llist*)((pll)->pnext)			// get ptr to next node
#define llist_prev(pll) (llist*)((pll)->pprev)		// get ptr to prev node

//	Get beginning and end items (used to check when traversing)

#define llist_beg(plh) (llist*)(&((plh)->head))
#define llist_end(plh) (llist*)(&((plh)->tail))

//	Iterate across all items from head to tail

#define forallinlist(listtype,plh,pll) for (pll = \
	(listtype *) llist_head(plh); pll != llist_end(plh); pll = llist_next(pll))

//	Iterate across all items from tail to head

#define forallinlistrev(listtype,plh,pll) for (pll = \
	(listtype *) llist_tail(plh); pll != llist_beg(plh); pll = llist_prev(pll))

//	--------------------------------------------------------------
//		HIGH-LEVEL LINKED LIST OF FIXED SIZE ITEMS (MANAGES STORAGE)
//	--------------------------------------------------------------

//	List header

typedef struct {
	llist head;							// head list item (not really in list)
	llist tail;							// tail list item (not really in list)
//	struct _llist_head;				// llist header (head, tail, numnodes)
	llist *pfree;						// ptr to next free element or NULL if no more
	llist *pNodeStore;				// ptr to first node store block, they're linked
											// (node store is list ptrs followed by data block)
	ushort nodeSize;					// size of each node
	short numNodesPerBlock;	// # nodes in list storage (including free ones)
} LlistHead;

//	Forgive the void pointers, C-- sucks

void LlistInit(LlistHead *plh, ushort nodeSize, short numNodesPerBlock);
void *LlistAddHead(LlistHead *plh);						// add 1st free to head, return ptr
void *LlistAddTail(LlistHead *plh);						// add 1st free to tail, return ptr
void *LlistAddQueue(LlistHead *plh, short prior);	// add in priority order
bool LlistMoveQueue(LlistHead *plh, void *pnode, short newprior);	// move pri
void LlistFree(LlistHead *plh, void *pnode);			// free node
void LlistFreeAll(LlistHead *plh);						// free all nodes
void LlistDestroy(LlistHead *plh);						// destroy list, reclaim storage

#define LlistHead(plh) (llist_head(plh))				// get ptr to head
#define LlistTail(plh) (llist_tail(plh))				// get ptr to tail
#define LlistFirstFree(plh) ((plh)->pfree)			// get ptr to first free

#define LlistEmpty(plh) (llist_empty(plh))		// determine if list empty
#define LlistNumNodes(plh) (llist_num_nodes((llist_head *) plh))	// # active

#define LlistNext(pll) (llist_next(pll))				// get ptr to next node
#define LlistPrev(pll) (llist_prev(pll))				// get ptr to prev node

#define LlistBeg(plh) (llist_beg(plh))				// beginning of list
#define LlistEnd(plh) (llist_end(plh))				// end of list

#define FORALLINLIST(listtype,plh,pll) forallinlist(listtype,plh,pll)
#define FORALLINLISTREV(listtype,plh,pll) forallinlistrev(listtype,plh,pll)

#endif
