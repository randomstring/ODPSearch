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

/* A set of search results. */

#include <stdlib.h>

#include "defs.h"
#include "stringx.h"
#include "common.h"
#include "vlist.h"
#include "strlist.h"
#include "record.h"
#include "mdtrec.h"
#include "mdt.h"
#include "idbobj.h"
#include "iresult.h"
#include "irset.h"
#include "dtreg.h"
#include "catids.h"
#include "mdthashtable.h"

#ifndef DOC_TYPE_KEYWORD 
#define DOC_TYPE_KEYWORD 99
#endif

extern MDTHASHTABLE ht;
extern MDTREC *MdtTable;

// now we don't need any path-prefix
// const int pathprefixlen = 20;

int IrsetIndexCompare(const void* x, const void* y) 
{
  INT4 Difference = ( (*((PIRESULT)y)).MdtIndex - 
		     (*((PIRESULT)x)).MdtIndex );
  if (Difference < 0) {
    return (-1);
  } else {
    if (Difference == 0) {
      return(0);
    } else {
      return 1;
    }
  }
}

IRSET::IRSET(const PIDBOBJ DbParent) 
{
  AbsoluteMaxEntries = MAX_RESULT_ENTRIES;

  Init(DbParent, 2000);
}

IRSET::IRSET(const PIDBOBJ DbParent, const int start_size) 
{
  int s = start_size;
  AbsoluteMaxEntries = MAX_RESULT_ENTRIES;

  if (s > AbsoluteMaxEntries) 
    s = AbsoluteMaxEntries;
  else if (s < 100)
    s = 100;
  
  Init(DbParent, s);
}


IRSET::IRSET(const PIDBOBJ DbParent, const int start_size, const int AbsMax) 
{
  int s = start_size;

  if (AbsMax > 0) {
    AbsoluteMaxEntries = AbsMax;
  }

  if (s > AbsoluteMaxEntries) 
    s = AbsoluteMaxEntries;
  else if (s < 100)
    s = 100;
  
  Init(DbParent, s);
}


void IRSET::Init(const PIDBOBJ DbParent, int start_size) 
{
  TotalEntries = 0;
  MaxEntries = start_size;
  Table = new IRESULT[MaxEntries](DbParent->GetMainMdt()->MdtTable);
  Parent = DbParent;
  MinScore=999999.0;
  MaxScore=0.0;
  int ScoreSort=0;		// 1 if sorted by score
  dtr = (void *)new DTREG(0);

  doctype = Parent->GetGlobalDocType();

  HashInit();
}

void
IRSET::HashInit() {
  int i;

#ifdef DEBUG_HASH_COLLISIONS
  collisions = 0;
#endif

  // lets use a power of two for fast hashing
  // 2^15 = 32768
  tableSize = 32768;

  // allow for zero length table to disable hash table function
  if (tableSize > 0) {
    iht = (i_list **) malloc(tableSize * sizeof(i_list *));

    // zero the table
    memset((void *)iht, 0, sizeof(i_list *) *tableSize);
  }
}

inline int
IRSET::Hash(int mdt_index) {
  // thankfully the mdt_index is the filenumber and this means
  // we have a high level of dispirstion built in
  //
  // This is the worlds fasted hash funtion.
  // 0x7FFF = 2^15 - 1
  return (mdt_index & 0x7FFF);
}

int
IRSET::HashAdd(int mdt_index) {

  // XXX - probably want to just add this to the the regular Add() function

  int hash;
  i_list *node = NULL;

  // check to see if this str/offet pair already exists
  // in the hash table
  hash = Hash(mdt_index);
  node = iht[hash];
  while (node && (mdt_index != node->mdt_index)) 
    node = node->next;

  if (!node) {
    // we have a new entry, add it.

    // insert node into the hashtable in front
    node = (i_list *) malloc(sizeof(i_list));
    node->mdt_index = mdt_index;
    node->irset_index = TotalEntries;
    node->next = iht[hash];

#ifdef DEBUG_HASH_COLLISIONS
    if (iht[hash])
      collisions++;
#endif

    iht[hash] = node;

  }

  return (node->irset_index);

}

int
IRSET::HashGet(int mdt_index) {

  int h = Hash(mdt_index);
  i_list *node = NULL;

  node = iht[h];
  while (node && (node->mdt_index != mdt_index))
      node = node->next;

  if (node)
    return (node->irset_index);
  else
    return -1;
}

float IRSET::GetMaxScore(){
  return(MaxScore);
}

float IRSET::GetMinScore(){
  return(MinScore);
}

IRSET& IRSET::operator=(const IRSET& OtherIrset) {
  if (Table) {
    delete [] Table;
  }
  Init(OtherIrset.GetParent(),OtherIrset.GetTotalEntries());
  int y = OtherIrset.GetTotalEntries();
  int x;
  IRESULT iresult(Parent->GetMainMdt()->MdtTable);

  for (x=1; x<=y; x++) {
    OtherIrset.GetEntry(x, &iresult);
    Table[x-1] = iresult;
  }
  return *this;
}

IRSET* IRSET::Duplicate() const {
  IRSET* Temp = new IRSET(Parent);
  *Temp = *(this);
  return Temp;
}


PIRSET IRSET::Duplicate(){
  IRSET * Temp= new IRSET(Parent);
  *Temp=*(this);
  return (Temp);
}


void IRSET::FastAddEntry(const IRESULT& ResultRecord, const int AddHitCounts) 
{
  int x;

  // fast add doesn't check for category match or adult searches
  // because this has already been done...

  x = HashGet(ResultRecord.MdtIndex);
  if (0 <= x) {
    // the entries already exists in the table
    // update the hitcounts
    if (AddHitCounts) {
      Table[x].IncScore(ResultRecord.Score);
      Table[x].IncHitCount(ResultRecord.HitCount);
      Table[x].IncRealHitCount(ResultRecord.RealHitCount);
    }
    Table[x].IncScore(ResultRecord.Score);

    return;
  }

  // otherwise its a new record, so add it
  if (TotalEntries == MaxEntries)
    Expand();
  if (TotalEntries < MaxEntries) {
    x = HashAdd(ResultRecord.MdtIndex);
    Table[TotalEntries] = ResultRecord;
    TotalEntries = TotalEntries + 1;
  }
#if 0
  else {
      cerr << "FULL: Dropping Entry = " << x << " "
	   << " MDT index= " << ResultRecord.MdtIndex << " "
	   << " Path number= " << ResultRecord.pathName << " "
	   << " File number= " << ResultRecord.fileName << "\n";
  }
#endif  
  
}

void IRSET::AddEntry(const IRESULT& ResultRecord, const int AddHitCounts) 
{
  int x;

  x = HashGet(ResultRecord.MdtIndex);

  if ((x >= 0) && (x < TotalEntries)) {
    // the entries already exists in the table
    // update the hitcounts
    if (AddHitCounts) {
      Table[x].IncHitCount(ResultRecord.HitCount);
    }
    Table[x].IncScore(ResultRecord.Score);
    Table[x].Field = (((DTREG *)dtr)->GetDocTypePtr(doctype))->NewField(Table[x].Field,ResultRecord.Field);

#if 0
    cerr << "DUP Entry = " << x << " "
	 << " MDT index= " << Table[x].MdtIndex << " "
	 << " Path number= " << Table[x].pathName << " "
	 << " File number= " << Table[x].fileName << "\n";
#endif

    return;
  }

  // otherwise its a new record, so add it

  if (TotalEntries == MaxEntries)
      Expand();
  if (TotalEntries < MaxEntries) {
      // only add the entry if we aren't past our max IRSET size
      // add the entry to the hash table
      x = HashAdd(ResultRecord.MdtIndex);
      
      // add the entry to the table
      Table[TotalEntries] = ResultRecord;

#if 0
      cerr << "Entry = " << TotalEntries << " "
	   << " MDT index= " << Table[TotalEntries].MdtIndex << "\n";
#endif  

      TotalEntries++;

  }
}


void IRSET::GetEntry(const int Index, PIRESULT ResultRecord) const {
  if ((Index > 0) && (Index <= TotalEntries) ) {
    *ResultRecord = Table[Index-1];
  }
}

void IRSET::Expand() {
  if (TotalEntries < AbsoluteMaxEntries) 
    Resize(AbsoluteMaxEntries);
}

void IRSET::CleanUp() {
  Resize(TotalEntries);
}

void IRSET::Resize(const int Entries) {

  // only resize if we have to
  if (Entries <= MaxEntries)
    return;

  PIRESULT Temp = new IRESULT[Entries](Parent->GetMainMdt()->MdtTable );
  int RecsToCopy;
  int x;
  if (Entries >= TotalEntries) {
    RecsToCopy = TotalEntries;
  } else {
    RecsToCopy = Entries;
    TotalEntries = Entries;
  }
  for (x=0; x<RecsToCopy; x++) {
    // Not sure if Temp[x] = Table[x] is good enough.
    Temp[x] = Table[x];
  }
  if (Table)
    delete [] Table;
  Table = Temp;
  MaxEntries = Entries;
}

int IRSET::GetTotalEntries() const {
  return TotalEntries;
}

int IRSET::GetHitTotal() const {
  int x;
  int Total = 0;
  for (x=0; x<TotalEntries; x++) {
    Total += Table[x].HitCount;
  }
  return Total;
}

void IRSET::Or(IRSET * OtherIrset) {
  int x;
  int t = OtherIrset->GetTotalEntries();
  IRESULT OtherIresult(Parent->GetMainMdt()->MdtTable);
  for (x=1; x<= t; x++) {
    OtherIrset->GetEntry(x, &OtherIresult);
    FastAddEntry(OtherIresult, 1);

#if 0
      cerr << "OR Entry = " << x << " "
	   << " MDT index= " << OtherIresult.MdtIndex << " "
	   << " Path number= " << OtherIresult.pathName << " "
	   << " File number= " << OtherIresult.fileName << "\n";
#endif  


  }
}

// A faster version by Bryn Dole, needs something extra to actually work though.. see comment.
void IRSET::CharProx(const IRSET& OtherIrset, const int Distance) {

  IRESULT OtherIresult(Parent->GetMainMdt()->MdtTable);
  IRSET MyResult(Parent);
  int x = 1;
  int y;
  int count=0;
  int t = OtherIrset.GetTotalEntries();
  
  while (x <= t) {
    OtherIrset.GetEntry(x, &OtherIresult);

    y = HashGet(OtherIresult.MdtIndex);
    if (y >= 0) {
      // terms are in the same record

      // XXX - Need to add back a list of hits per record with the possition of
      // XXX - the terms so that we can see if the terms are within a cetain
      // XXX - distance of each other

      //if ( (abs(OtherIresult.GlobalFileStart - Table[y].GlobalFileEnd) <= Distance) ||
      //(abs(OtherIresult.GlobalFileEnd - Table[y].GlobalFileStart) <= Distance)) {

      // terms are withing X characters of each other
      Table[y].IncHitCount(OtherIresult.HitCount);
      Table[y].IncRealHitCount(OtherIresult.RealHitCount);
      Table[y].IncScore(OtherIresult.Score);
      MyResult.FastAddEntry(Table[y], 0);
      count++;
    }
    x++;
  }

  delete [] Table;
  TotalEntries= count;
  MaxEntries= MyResult.MaxEntries;
  Table = MyResult.Table;
  tableSize = MyResult.tableSize;
  MyResult.Table = NULL;
  iht = MyResult.iht;
}

// faster AndNot() by Bryn Dole
void IRSET::AndNot(IRSET *OtherIrset) 
{
  IRESULT Iresult(Parent->GetMainMdt()->MdtTable);
  IRSET MyResult(Parent);
  int x = 1;
  int count=0;
  int t = GetTotalEntries();
  
  while (x <= t) {
    GetEntry(x, &Iresult);

    if (OtherIrset->HashGet(Iresult.MdtIndex) < 0) {
      // the entry was not found, so include it
      MyResult.FastAddEntry(Iresult, 0);
      count++;
    }
    x++;
  }

  delete [] Table;
  TotalEntries=count;
  MaxEntries=MyResult.MaxEntries;
  Table = MyResult.Table;
  tableSize = MyResult.tableSize;
  MyResult.Table = NULL;
  iht = MyResult.iht;
}

// Faster And() with merging implemented by Bryn Dole
void IRSET::And(IRSET * OtherIrset) 
{
  IRESULT OtherIresult(Parent->GetMainMdt()->MdtTable);
  IRSET MyResult(Parent);
  int x = 1;
  int y;
  int count=0;
  int t = OtherIrset->GetTotalEntries();
  
  while (x <= t) {
    OtherIrset->GetEntry(x, &OtherIresult);

    y = HashGet(OtherIresult.MdtIndex);
    if (y >= 0) {
      // the entry was found, so include it
      Table[y].IncHitCount(OtherIresult.HitCount);
      Table[y].IncRealHitCount(OtherIresult.RealHitCount);
      Table[y].IncScore(OtherIresult.Score);
      MyResult.FastAddEntry(Table[y], 0);

#if 0
      cerr << "AND Entry = " << count << " "
	   << " MDT index= " << Table[y].MdtIndex << " "
	   << " Path number= " << Table[y].pathName << " "
	   << " File number= " << Table[y].fileName << "\n";
#endif  

      count++;
    }
    x++;
  }

  delete [] Table;
  TotalEntries= count;
  MaxEntries= MyResult.MaxEntries;
  Table = MyResult.Table;
  tableSize = MyResult.tableSize;
  MyResult.Table = NULL;
  // free(iht); and all associated buckets
  iht = MyResult.iht;
}

void IRSET::ComputeScores(const float TermWeight, int total_count) {
  if (TotalEntries == 0) {
    return;
  }

  DTREG    dtreg(0);

  dtreg.GetDocTypePtr(doctype)->ScoreHits(Table, 
					  TermWeight, 
					  TotalEntries, 
					  AbsoluteMaxEntries, 
					  &MaxScore, 
					  &MinScore);
  
}

void IRSET::OldComputeScores(const float TermWeight, int total_count) {
  if (TotalEntries == 0) {
    return;
  }

  float Weight;
  float DocsInRs = TotalEntries;
  float DocsInDb = Parent->GetMainMdt()->GetTotalEntries();
  float InvDocFreq = DocsInDb / DocsInRs;
  float SumSqScores = 0;
  float SqrtSum;
  float Score;
  int    x;

  //
  // Set the Term's weight of the term inversely proportional to the 
  // number of hits
  //
  // Weight = [1/AbsoluteMaxEntries, 1 ]  = (0,1]
  if (total_count) {
      if (total_count > AbsoluteMaxEntries)
	  total_count = AbsoluteMaxEntries;
      Weight = ( 1.0 + (AbsoluteMaxEntries - total_count)) / (float)AbsoluteMaxEntries;
  }
  else {
      Weight = ( 1.0 + (AbsoluteMaxEntries - TotalEntries)) / (float)AbsoluteMaxEntries;
  }

  for (x=0; x<TotalEntries; x++) {
      // IncDocFreq measures the inverse frequency of the term in the entire DB
      Score = ((DTREG *)dtr)->GetDocTypePtr(doctype)->ScoreHit(Table[x] , InvDocFreq);
      Table[x].SetScore(Score);
      SumSqScores += (Score * Score);
  }

  SqrtSum = sqrt(SumSqScores);
  if (SqrtSum == 0.0) {
      SqrtSum = 1.0;
  }

  for (x=0; x<TotalEntries; x++) {
      // TermWeight & Weight apply to all members of a set
      Score= ( Table[x].Score / SqrtSum ) * Weight * TermWeight;
      if(Score>MaxScore)
	  MaxScore=Score;
      if(Score<MinScore)
          MinScore=Score;
      Table[x].SetScore(Score);
  }
}

int IrsetScoreCompare(const void* x, const void* y) {

  float Difference;
  int  rankDifference = ((*((PIRESULT)y)).RealHitCount - 
			 (*((PIRESULT)x)).RealHitCount );

  if (rankDifference < 0) {
    return (-1);
  } 
  else {
    if ( rankDifference > 0) {
      return (1);
    }
    else {
      // Use the regular score to break the tie
      
      Difference = ((*((PIRESULT)y)).Score - 
		    (*((PIRESULT)x)).Score );
      
      if (Difference < 0) {
	return (-1);
      } else {
	if (Difference == 0) {
	  // use the record number to break the tie
	  return ((*((PIRESULT)x)).MdtIndex - 
		  (*((PIRESULT)y)).MdtIndex );
	} else {
	  return 1;
	}
      }
    }
  }
}

void IRSET::SortByScore() {


#if 0
  // this only helps OR searches, which are almost never done now. 1/30/2002

  // give presidence to matches with the highest number of term hits
  int x;
  float Score;
  float NewMaxScore = MaxScore;
  for (x=0; x<TotalEntries; x++) {

    Score =  Table[x].Score + Table[x].RealHitCount * MaxScore;

    if( Score > NewMaxScore)
      NewMaxScore=Score;

    Table[x].Score = Score;

  }

  // adjust maximum score
  MaxScore=NewMaxScore;

#endif

  if (TotalEntries <= (AbsoluteMaxEntries / 2)) 
    qsort(Table, TotalEntries, sizeof(IRESULT), IrsetScoreCompare);

#if 0
  // fprintf(stderr,"Max score = %.12f\nMin Score = %.12f\n",NewMaxScore, NewMinScore);

  // Score debugging
  for (int x=0; (x < TotalEntries) && ( x < 1000); x++) {
    cerr << Table[x].RealHitCount << "  " 
	 << Table[x].HitCount  << "  " 
	 << Table[x].Score << "  "
	 << "\n";
  }
#endif

  ScoreSort=1;
}

void IRSET::SortByIndex() {
  qsort (Table, TotalEntries, sizeof(IRESULT), IrsetIndexCompare);
  ScoreSort=0;
}

void IRSET::SetParent(PIDBOBJ const NewParent) {
  Parent = NewParent;
}

PIDBOBJ IRSET::GetParent() const {
  return Parent;
}

int IRSET::GetScaledScore(const float UnscaledScore, const int ScaleFactor) {
	float Diff = MaxScore - MinScore;
	if (Diff <= 0) {
	  // no variance in scores
	  Diff=1;
	  // return max score
	  return (ScaleFactor);
	}
	return ( (int)( ((UnscaledScore - MinScore) * ScaleFactor) / Diff ) );
}

float IRSET::GetScaledScore(const float UnscaledScore, const float ScaleFactor) {
	float Diff = MaxScore - MinScore;
	if (Diff <= 0) {
	  // no variance in scores
	  Diff= 1.0;
	  // return max score

	  // XXX - remove this:
	  cerr << "BLARG: No spread in scores!\n";

	  return (ScaleFactor);
	}
	return ( (float)( ((UnscaledScore - MinScore) * ScaleFactor) / Diff ) );
}

IRSET::~IRSET() {

#ifdef DEBUG_HASH_COLLISIONS
  if (TotalEntries)
    cerr << "Total collisions = " << collisions << "   " 
	 << TotalEntries << "  " << (( 100 * collisions) / TotalEntries)
	 << "\n";
#endif

  if (Table)
    delete [] Table;
}
