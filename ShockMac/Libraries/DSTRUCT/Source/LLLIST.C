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
//		lllist.C		Basic doubly-linked list
//		Rex E. Bradford (REX)
//
//		This module contains the few routines associated with the simple
//		llist structure which are not macros.  These include:
//
//		llist_insert_queue() - insert a node into a sorted list
//		llist_move_queue()   - move a node within a sorted list
//		llist_num_nodes()    - count # nodes in a list
//
//		See llist.h for the rest of the functionality, or llist.txt
//		for documentation on how to use llist's.
/*
* $Header: n:/project/lib/src/dstruct/RCS/lllist.c 1.2 1993/04/16 12:02:19 rex Exp $
* $log$
*/

#include "llist.h"

//	----------------------------------------------------------
//		LITTLE LINKED LIST ROUTINES (SEE MACROS IN LLIST.H TOO)
//	----------------------------------------------------------
//
//	llist_insert_queue() inserts a new queue item into list.

void llist_insert_queue(llist_head *plh, queue *plq)
{
	queue *pxx;

//	Point to element past new one in priority

	pxx = (queue *) llist_head(plh);
	while ((pxx != (queue *)llist_end(plh)) && (plq->priority <= pxx->priority))
		pxx = pxx->pnext;

//	Patch us in before this element

	llist_insert_before(plq,pxx);
}

//	---------------------------------------------------------
//
//	llist_move_queue() moves a queue item after inserting new priority.

bool llist_move_queue(llist_head *plh, queue *plq)
{
	bool moved;

//	See if priority warrants moving the queue node

	moved = FALSE;
	if ((plq->pprev != (queue *)llist_beg(plh)) && (plq->priority > plq->pprev->priority))
		moved = TRUE;
	else if ((plq->pnext != (queue *)llist_end(plh)) && (plq->priority < plq->pnext->priority))
		moved = TRUE;

//	Yes, detach from queue and re-insert

	if (moved)
	{
		llist_remove(plq);
		llist_insert_queue(plh, plq);
	}

//	Report whether item had to be moved or not

	return(moved);
}

//	----------------------------------------------------------
//
//	llist_num_nodes() counts # nodes in list.
//
//		plh = ptr to list header
//
//	Returns: # nodes in list

int llist_num_nodes(llist_head *plh)
{
	llist *pll;
	int num;

	num = 0;
	forallinlist(llist,plh,pll)
		++num;

	return(num);
}
