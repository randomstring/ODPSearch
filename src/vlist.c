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

#include <iostream.h>
#include "vlist.h"

VLIST::VLIST() {
  EntryCount = 0;
  Next = this;	// a circle of one
  Prev = this;
}

int VLIST::GetTotalEntries() const {
  return EntryCount;
}

void 
VLIST::AddNode(VLIST* NewEntryPtr) {
  EntryCount++;
  Prev->Next = NewEntryPtr;	// set last node to point to new node
  NewEntryPtr->Prev = Prev;	// set new node to point to last node
  Prev = NewEntryPtr;		// set top node to point to new node
  NewEntryPtr->Next = this;	// set new node to point to top node
}


VLIST*  VLIST::GetNodePtr(const int Index) const {
  int x = 1;
  VLIST* p = Next;	// start at first real node
  while (p != this) {	// exit when we reach full circle
    if (x == Index) {
      return p;	// found it
    }
    p = p->Next;
    x++;
  }
  return 0;	// Index was never matched
}


VLIST* VLIST::GetNextNodePtr() const {
  return Next;
}

void VLIST::Clear() {
  EntryCount = 0;
  if (this->Next != this) {
    Prev->Next = 0;	// disattach circle to isolate nodes
    Prev = 0;
    delete this->Next;	// delete all subsequent nodes
    Next = this;	// reattach circle of one
    Prev = this;
  }
}


void VLIST::EraseAfter(const int Index) {
  int x = 1;
  VLIST* p = Next;	// start at first real node
  while (p != this) {	// exit when we reach full circle
    if (x == Index) {
      if (p->Next != this) {
	Prev->Next = 0;	// disattach circle to isolate nodes
	Prev = 0;
	delete p->Next;	// delete all subsequent nodes
	p->Next = this;	// reattach circle
	this->Prev = p;
	EntryCount = x;
	return;
      }
    }
    p = p->Next;
    x++;
  }
}


void 
VLIST::Reverse() {
  VLIST* p = this;
  VLIST* nextp;
  VLIST* temp;
  do {
    nextp = p->Next;
    temp = p->Prev;	// reverse pointers
    p->Prev = p->Next;
    p->Next = temp;
    p = nextp;
  } while (p != this);
}


VLIST::~VLIST() {
  // Disattach from previous node
  if (Prev != 0)
    Prev->Next = 0;
  //	Prev = 0;	// not necessary
  // Delete next node
  if (Next) {
    delete Next;
  }
}
