/*
 * list.h: lists interfaces
 *
 * Copyright (C) 2000 Gary Pennington and Daniel Veillard.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE AUTHORS AND
 * CONTRIBUTORS ACCEPT NO RESPONSIBILITY IN ANY CONCEIVABLE MANNER.
 *
 * Author: Gary.Pennington@uk.sun.com
 */

typedef struct _xmlLink xmlLink;
typedef xmlLink *xmlLinkPtr;

typedef struct _xmlList xmlList;
typedef xmlList *xmlListPtr;

typedef void (*xmlListDeallocator) (xmlLinkPtr lk);
typedef int  (*xmlListDataCompare) (const void *data0, const void *data1);
typedef int (*xmlListWalker) (const void *data, const void *user);

/* Creation/Deletion */
xmlListPtr	xmlListCreate		(xmlListDeallocator deallocator,
	                                 xmlListDataCompare compare);
void		xmlListDelete		(xmlListPtr l);

/* Basic Operators */
void *		xmlListSearch		(xmlListPtr l,
					 void *data);
void *		xmlListReverseSearch	(xmlListPtr l,
					 void *data);
int		xmlListInsert		(xmlListPtr l,
					 void *data) ;
int		xmlListAppend		(xmlListPtr l,
					 void *data) ;
int		xmlListRemoveFirst	(xmlListPtr l,
					 void *data);
int		xmlListRemoveLast	(xmlListPtr l,
					 void *data);
int		xmlListRemoveAll	(xmlListPtr l,
					 void *data);
void		xmlListClear		(xmlListPtr l);
int		xmlListEmpty		(xmlListPtr l);
xmlLinkPtr	xmlListFront		(xmlListPtr l);
xmlLinkPtr	xmlListEnd		(xmlListPtr l);
int		xmlListSize		(xmlListPtr l);

void		xmlListPopFront		(xmlListPtr l);
void		xmlListPopBack		(xmlListPtr l);
int		xmlListPushFront	(xmlListPtr l,
					 void *data);
int		xmlListPushBack		(xmlListPtr l,
					 void *data);

/* Advanced Operators */
void		xmlListReverse		(xmlListPtr l);
void		xmlListSort		(xmlListPtr l);
void		xmlListWalk		(xmlListPtr l,
					 xmlListWalker walker,
					 const void *user);
void		xmlListReverseWalk	(xmlListPtr l,
					 xmlListWalker walker,
					 const void *user);
void		xmlListMerge		(xmlListPtr l1,
					 xmlListPtr l2);
xmlListPtr	xmlListDup		(const xmlListPtr old);
int		xmlListCopy		(xmlListPtr cur,
					 const xmlListPtr old);
/* Link operators */
void *          xmlLinkGetData          (xmlLinkPtr lk);

/* xmlListUnique() */
/* xmlListSwap */


