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

/* Description:	Internal Search Result Set */

#ifndef IRSET_H
#define IRSET_H

#include "idbobj.h"

typedef struct I_LIST {
    int           irset_index;    /* irset table index */
    int           mdt_index;      /* mdt index value */
    struct I_LIST *next;          /* next list item */
} i_list;

class IRSET {
public:
    IRSET(const PIDBOBJ DbParent);
    IRSET(const PIDBOBJ DbParent,int start_size);
    IRSET(const PIDBOBJ DbParent, const int start_size, const int AbsMax);
    void Init(const PIDBOBJ DbParent,int start_size);
    IRSET& operator=(const IRSET& OtherIrset);
    int GetOperandType() const { return TypeRset; };
    IRSET* Duplicate() const;
    IRSET* Duplicate();
    void AddEntry(const IRESULT& ResultRecord, const int AddHitCounts);
    void FastAddEntry(const IRESULT& ResultRecord, const int AddHitCounts);
    void GetEntry(const int Index, PIRESULT ResultRecord) const;
    void Expand();
    void CleanUp();
    void Resize(const int Entries);
    int GetTotalEntries() const;
    int GetHitTotal() const;
    void ComputeScores(const float TermWeight, const int total_count );
    void OldComputeScores(const float TermWeight, const int total_count );
    
    void Or(IRSET *OtherIrset);
    void And(IRSET *OtherIrset);
    void AndNot(IRSET *OtherIrset);
    
    // Near is hardcoded to be 50 characters
    void Near(const IRSET & OtherIrset) { CharProx(OtherIrset, 50); }
    void CharProx(const IRSET& OtherIrset, const int Distance);
    void SortByScore();
    void SortByIndex();
    void SetParent(PIDBOBJ const NewParent);
    float GetMaxScore();
    float GetMinScore();
    PIDBOBJ GetParent() const;
    
    ~IRSET();
    void HashInit();
    int Hash(int  mdt_index);
    int HashAdd(int mdt_index);
    int HashGet(int mdt_index);
    int GetScaledScore(const float UnscaledScore, const int ScaleFactor);
    float GetScaledScore(const float UnscaledScore, const float ScaleFactor);
    
    int      tableSize;
    struct I_LIST **iht;
    unsigned char doctype;
    void*    dtr;
    
    PIDBOBJ  Parent;
    PIRESULT Table;
    int      TotalEntries;
    int      MaxEntries;
    int      ScoreSort;
    float    MaxScore,MinScore;
    int      AbsoluteMaxEntries;
    
#ifdef DEBUG_HASH_COLLISIONS
    int collisions;
#endif
};

typedef IRSET* PIRSET;

#endif
