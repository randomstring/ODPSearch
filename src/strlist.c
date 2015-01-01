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

/* Class STRLIST - String List */

#include <string.h>

#include "defs.h"
#include "stringx.h"
#include "vlist.h"
#include "strlist.h"

STRLIST::STRLIST() 
  : VLIST() 
{
}


STRLIST& STRLIST::operator=(const STRLIST& OtherStrlist) 
{
  Clear();
  SIZE_T x;
  SIZE_T y = OtherStrlist.GetTotalEntries();
  STRLIST* NodePtr;
  STRLIST* NewNodePtr;
  for (x=1; x<=y; x++) {
    NodePtr = (STRLIST*)(OtherStrlist.GetNodePtr(x));
    NewNodePtr = new STRLIST();
    NewNodePtr->String = NodePtr->String;
    VLIST::AddNode(NewNodePtr);
  }
  return *this;
}


void 
STRLIST::AddEntry(const STRING& StringEntry) 
{
  STRLIST* NodePtr = new STRLIST();
  NodePtr->String = StringEntry;
  VLIST::AddNode(NodePtr);
}


void 
STRLIST::AddEntry(const CHR* Entry) 
{
  STRLIST* NodePtr = new STRLIST();
  NodePtr->String = Entry;
  VLIST::AddNode(NodePtr);
}


void 
STRLIST::SetEntry(const SIZE_T Index, const STRING& StringEntry) 
{
  STRLIST* NodePtr = (STRLIST*)(VLIST::GetNodePtr(Index));
  if (NodePtr) {
    NodePtr->String = StringEntry;
  } else {
    if (Index > 0) {
      // Add filler nodes
      SIZE_T y = Index - GetTotalEntries();
      SIZE_T x;
      for (x=1; x<=y; x++) {
	NodePtr = new STRLIST();
	VLIST::AddNode(NodePtr);
      }
      NodePtr->String = StringEntry;
    }
  }
}


void 
STRLIST::GetEntry(const SIZE_T Index, STRING* StringEntry) const 
{
  STRLIST* NodePtr = (STRLIST*)(VLIST::GetNodePtr(Index));
  if (NodePtr) {
    *StringEntry = NodePtr->String;
  }
}


void 
STRLIST::Split(const CHR* Separator, const STRING& TheString) 
{
  STRINGINDEX Position;
  STRLIST NewList;
  STRING S, T;
  SIZE_T SLen = strlen(Separator);
  S = TheString;
  // parse S and build list of terms
  while ( (Position=S.Search(Separator)) != 0) {
    T = S;
    T.EraseAfter(Position - 1);
    S.EraseBefore(Position + SLen);
    NewList.AddEntry(T);
  }
  NewList.AddEntry(S);	// add the remaining entry
  *this = NewList;
}


void 
STRLIST::Split(const CHR Separator, const STRING& TheString) 
{
  STRINGINDEX Position;
  STRLIST NewList;
  STRING S, T;
  S = TheString;

  // parse S and build list of terms
  while ( (Position=S.Search(Separator)) != 0) {
    T = S;
    T.EraseAfter(Position - 1);
    S.EraseBefore(Position + 1);
    NewList.AddEntry(T);
  }
  NewList.AddEntry(S);	// add the remaining entry
  *this = NewList;
}


void 
STRLIST::Join(const CHR* Separator, STRING* StringBuffer) 
{
  STRING NewString;
  SIZE_T x;
  SIZE_T y = GetTotalEntries();
  for (x=1; x<=y; x++) {
    NewString += ((STRLIST*)(VLIST::GetNodePtr(x)))->String;
    if (x < y) {
      NewString += Separator;
    }
  }
  *StringBuffer = NewString;
}


SIZE_T 
STRLIST::SearchCase(const STRING& SearchTerm) 
{
  SIZE_T x = 1;
  STRLIST* p = (STRLIST*)(this->GetNextNodePtr());
  while (p != this) {
    if (p->String ^= SearchTerm) {
      return x;
    }
    x++;
    p = (STRLIST*)(p->GetNextNodePtr());
  }
  return 0;
}


void 
STRLIST::GetValue(const CHR* Title, STRING* StringBuffer) 
{
  *StringBuffer = "";
  SIZE_T x;
  STRINGINDEX Position, y;
  STRING S;
  STRING S2;
  SIZE_T TotalEntries = GetTotalEntries();
  for (x=1; x<=TotalEntries; x++) {
    S2 = ((STRLIST*)(VLIST::GetNodePtr(x)))->String;
    if ( (Position=S2.Search('=')) ) {
      S = S2;
      S.EraseAfter(Position - 1);
      while (S.GetChr(1) == ' ') {	// get rid of leading spaces
	S.EraseBefore(2);
      }
      while (S.GetChr((y=S.GetLength())) == ' ') {	// get rid of trailing spaces
	S.EraseAfter(y-1);
      }
      if (S ^= Title) {
	S = S2;
	S.EraseBefore(Position + 1);
	*StringBuffer = S;
	return;
      }
    }
  }
}


void 
STRLIST::GetValue(const STRING& Title, STRING* StringBuffer) 
{
  *StringBuffer = "";
  SIZE_T x;
  STRINGINDEX Position, y;
  STRING S;
  STRING S2;
  SIZE_T TotalEntries = GetTotalEntries();
  for (x=1; x<=TotalEntries; x++) {
    S2 = ((STRLIST*)(VLIST::GetNodePtr(x)))->String;
    if ( (Position=S2.Search('=')) ) {
      S = S2;
      S.EraseAfter(Position - 1);
      while (S.GetChr(1) == ' ') {	// trim leading spaces
	S.EraseBefore(2);
      }
      while (S.GetChr((y=S.GetLength())) == ' ') {  // trim trailing spaces
	S.EraseAfter(y-1);
      }
      if (S ^= Title) {
	S = S2;
	S.EraseBefore(Position + 1);
	*StringBuffer = S;
	return;
      }
    }
  }
}
