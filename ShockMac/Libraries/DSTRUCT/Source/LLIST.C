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
//		LList.C		Doubly-linked list
//		Rex E. Bradford (REX)
//
//		This module, along with macros in llist.h and routines in
//		lllist.c, implements a "managed-storage" linked list of
//		homogenous (same-sized) nodes.  The llist module takes care
//		of allocation and deallocation of storage for nodes, using
//		Malloc() to allocate several nodes at a time (the amount is
//		user-specifiable on a per-list basis).  The storage is
//		dynamic; a list may grow without bound until Malloc() fails.
//		Sorted lists, or queues, are handled by this module as well.
//
//		See llist.h for the full interface, and llist.txt for documentation.
/*
* $Header: n:/project/lib/src/dstruct/RCS/llist.c 1.2 1993/04/16 12:01:57 rex Exp $
* $log$
*/

#include <string.h>
#include "lg.h"
#include "llist.h"
//#include "memall.h"

void LlistGrowList(LlistHead *plh);
void LlistInitNodeBlock(LlistHead *plh, llist *pNodeBlock, ushort blockSize);

//	---------------------------------------------------------
//		LLIST ROUTINES
//	---------------------------------------------------------
//
//	LlistInit() initializes a list.
//
//		plh              = ptr to list header
//		nodeSize         = size of each list node, including embedded _llist
//		numNodesPerBlock = # nodes to allocate in each Malloc() block

void LlistInit(LlistHead *plh, ushort nodeSize, short numNodesPerBlock)
{
//	Initialize basic part of linked list header

	llist_init(plh);

//	Set allocation params

	plh->nodeSize = nodeSize;
	plh->numNodesPerBlock = numNodesPerBlock;

//	Allocate initial list block

	plh->pNodeStore = NULL;
	LlistGrowList(plh);			// plh->pfree initialized by this call
}

//	--------------------------------------------------------
//
//	LlistAddHead() adds first free node to head of list.
//
//		plh = ptr to list header
//
//	returns: ptr to list node, with pnext & pprev already filled in

void *LlistAddHead(LlistHead *plh)
{
	llist *pll;

//	Check for room

	if (plh->pfree == NULL)
		LlistGrowList(plh);

//	Get next free node off free list

	pll = plh->pfree;
	plh->pfree = pll->pnext;

//	Insert at head of list

	llist_add_head(plh, pll);

//	Return ptr to element

	return(pll);
}

//	--------------------------------------------------------
//
//	LlistAddTail() adds first free node to tail of list.
//
//		plh = ptr to list header
//
//	returns: ptr to list node, with pnext & pprev already filled in

void *LlistAddTail(LlistHead *plh)
{
	llist *pll;

//	Check for room

	if (plh->pfree == NULL)
		LlistGrowList(plh);

//	Get next free node off free list

	pll = plh->pfree;
	plh->pfree = pll->pnext;

//	Insert at tail of list

	llist_add_tail(plh, pll);

//	Return ptr to element

	return(pll);
}

//	--------------------------------------------------------
//
//	LlistAddQueue() adds first free element in priority order.
//
//		plh   = queue header
//		prior = priority value for new node
//
//	returns: ptr to list node, with pnext,pprev,prior already filled in

void *LlistAddQueue(LlistHead *plh, short prior)
{
	queue *plq;

//	Check for room

	if (plh->pfree == NULL)
		LlistGrowList(plh);

//	Get next free node off free list

	plq = (queue *) plh->pfree;
	plh->pfree = (llist *) plq->pnext;

//	Insert in priority order

	plq->priority = prior;
	llist_insert_queue((llist_head *) plh, plq);

//	Return ptr to item

	return(plq);
}

//	--------------------------------------------------------
//
//	LlistMoveQueue() sets new queue nodes priority, moves.
//
//		plh      = ptr to queue header
//		pnode    = ptr to node to be moved
//		newprior = new priority value
//
//	returns: TRUE if item was actually moved in list, false if newprior
//		didn't cause it to switch places with other nodes.

bool LlistMoveQueue(LlistHead *plh, void *pnode, short newprior)
{
	((queue *) pnode)->priority = newprior;
	return(llist_move_queue((llist_head *)plh, (queue *)pnode));
}

//	--------------------------------------------------------
//
//	LlistFree() frees a list element.
//
//		plh    = ptr to list or queue header
//		pnode  = ptr to list node to free

void LlistFree(LlistHead *plh, void *pnode)
{
//	Remove from list

	llist_remove((llist *) pnode);

//	Return node to head of free list

	((llist *)pnode)->pnext = plh->pfree;
	plh->pfree = (llist *)pnode;
}

//	--------------------------------------------------------
//
//	LlistFreeAll() frees all elements in list.
//	Note: this does not reclaim list memory, use ListDestroy()
//
//		plh = ptr to list or queue header

void LlistFreeAll(LlistHead *plh)
{
	llist *pnb;
	ushort blockSize;

//	Init head & tail ptrs, zero num nodes

	plh->head.pnext = LlistEnd(plh);
	plh->tail.pprev = LlistBeg(plh);

//	Reset free list of nodes to span across all storage blocks

	blockSize = plh->nodeSize * plh->numNodesPerBlock;
	for (pnb = plh->pNodeStore; pnb != NULL; pnb = pnb->pnext)
		LlistInitNodeBlock(plh, pnb, blockSize);
}

//	--------------------------------------------------------
//
//	LlistDestroy() destroys a list.
//
//		plh = ptr to list or queue header
//	--------------------------------------------------------
//  For Mac version:  Use DisposePtr instead of Free.

void LlistDestroy(LlistHead *plh)
{
	llist *pnb;
	llist *pnbNext;

//	Free all storage blocks

	pnb = plh->pNodeStore;
	while (pnb)
	{
		pnbNext = pnb->pnext;
		DisposePtr((Ptr)pnb);
		pnb = pnbNext;
	}

//	Reinitialize list header (must re-init to use again!)

	LG_memset(plh, 0, sizeof(LlistHead));
}

//	--------------------------------------------------------
//		PRIVATE ROUTINES
//	--------------------------------------------------------
//
//	LlistGrowList() grows a linked list.  It adds a new
//	storage block (the number of nodes per block is set
//	when LlistInit() is called).
//
//		plh = ptr to list or queue header
//	--------------------------------------------------------
//  For Mac version:  Use NewPtr instead of Malloc.

void LlistGrowList(LlistHead *plh)
{
	ushort blockSize;
	llist *pNewStore;

//	Allocate new storage block

	blockSize = plh->nodeSize * plh->numNodesPerBlock;
	pNewStore = (llist *)NewPtr(sizeof(llist) + blockSize);

//	Link it in to the node store list

	pNewStore->pnext = plh->pNodeStore;
	plh->pNodeStore = pNewStore;

//	Build the free list chain

	LlistInitNodeBlock(plh, pNewStore, blockSize);
}

//	---------------------------------------------------------
//
//	LlistInitNodeBlock() initializes a node block.
//
//		plh = ptr to list or queue header
//		pNodeBlock = ptr to node block to initialize
//		blockSize  = size of the block

void LlistInitNodeBlock(LlistHead *plh, llist *pNodeBlock, ushort blockSize)
{
	llist *pll;

	plh->pfree = pNodeBlock + 1;	// point free to just past links
	for (pll = plh->pfree;
		pll < (llist *) (((char *) plh->pfree) + (blockSize - plh->nodeSize));
		pll = (llist *) (((char *) pll) + plh->nodeSize))
			pll->pnext = (llist *) (((char *) pll) + plh->nodeSize);
	pll->pnext = NULL;
}
