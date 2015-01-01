/************************************************************************
 
 The contents of this file are subject to the Mozilla Public License
 Version 1.1 (the "License"); you may not use this file except in
 compliance with the License. You may obtain a copy of the License at
 http://www.mozilla.org/MPL/
 
 Software distributed under the License is distributed on an "AS IS"
 basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 License for the specific language governing rights and limitations
 under the License.
 
 The Original Code is Isearch.
 
 The Initial Developer of the Original Code is Clearinghouse for
 Networked Information Discovery and Retrieval.
 
 Portions created by Netscape Communications are Copyright (C) 2001-2002.
 All Rights Reserved.
 
 Portions created by the Clearinghouse for Networked Information
 Discovery and Retrieval are Copyright (C) 1994. All Rights Reserved.
 
************************************************************************/

/* Class VLIST - Doubly Linked Circular List Base Class */

#ifndef VLIST_HXX
#define VLIST_HXX

#include "gdt.h"
#include "defs.h"

class VLIST {
public:
	VLIST();
	virtual void   Clear();
	virtual void   EraseAfter(const INT Index);
	virtual void   Reverse();
	virtual INT    GetTotalEntries() const;
	virtual ~VLIST();
protected:
	virtual void   AddNode(VLIST* NewEntryPtr);
	virtual VLIST* GetNodePtr(const INT Index) const;
	virtual VLIST* GetNextNodePtr() const;
private:
        INT EntryCount;
	VLIST* Next;
	VLIST* Prev;
};

#endif
