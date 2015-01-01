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

/*
 * Data structure to store result data. 
 */

#include "defs.h"
#include "stringx.h"
#include "vlist.h"
#include "mdtrec.h"
#include "mdt.h"
#include "iresult.h"
#include "mdthashtable.h"
#include "catids.h"

IRESULT::IRESULT(MDTREC *Mdt) {
    
    MdtTable = Mdt;

    MdtIndex = 0;
    HitCount = 0;
    RealHitCount = 1;
    Field        = 0;
    Score = 0.0;
}

IRESULT& IRESULT::operator=(const IRESULT& OtherIresult) {
    MdtIndex = OtherIresult.MdtIndex;
    HitCount = OtherIresult.HitCount;
    RealHitCount = OtherIresult.RealHitCount;
    Field        = OtherIresult.Field;

    Score = OtherIresult.Score;
    return *this;
}

void IRESULT::SetMdtIndex(const int NewMdtIndex) {
    MdtIndex = NewMdtIndex;
}

int IRESULT::GetMdtIndex() const {
    return MdtIndex;
}

void IRESULT::SetHitCount(const int NewHitCount) {
    HitCount = NewHitCount;
}

void IRESULT::IncHitCount() {
    HitCount++;
}

void IRESULT::IncHitCount(const int AddCount) {
    HitCount += AddCount;
}

void IRESULT::IncRealHitCount(const int RealCount) {
    RealHitCount += RealCount;
}

int IRESULT::GetHitCount() const {
    return HitCount;
}

void IRESULT::SetScore(const float NewScore) {
    Score = NewScore;
}

void IRESULT::IncScore(const float AddScore) {
    Score += AddScore;
}

void IRESULT::AndIncScore(const float AddScore) {
    Score += AddScore;
    Score *= 1.5;
}

void IRESULT::OrIncScore(const float AddScore) {
    /* Take the Maximum of both scores */
    if (Score < AddScore)
	Score = AddScore;
}

float IRESULT::GetScore() const {
    return Score;
}

long IRESULT::GetPathNameHash() const {
    if (MdtTable)
	return MdtTable[MdtIndex-1].pathName;
    else {
	cerr << "NO HASHTABLE! (GetPathNameHash)\n";
	return 0;
    }
}

long IRESULT::GetFileNameHash() const {
    if (MdtTable)
	return MdtTable[MdtIndex-1].fileName;
    else {
	cerr << "NO HASHTABLE! (GetFileNameHash)\n";
	return 0;
    }
}

GPTYPE IRESULT::GetGlobalFileStart() const {
    if (MdtTable)
	return (MdtTable[MdtIndex-1].GlobalFileStart);
    else {
	cerr << "NO HASHTABLE! (GetGlobalFileStart)\n";
	return 0;
    }
}

GPTYPE IRESULT::GetGlobalFileEnd() const {
    if (MdtTable)
	return (MdtTable[MdtIndex-1].GlobalFileStart + MdtTable[MdtIndex-1].FileLength);
    else {
	cerr << "NO HASHTABLE! (GetGlobalFileEnd)\n";
	return 0;
    }
}

GPTYPE IRESULT::GetRecordLength() const {
    if (MdtTable)
	return (MdtTable[MdtIndex-1].FileLength);
    else {
	cerr << "NO HASHTABLE! (GetRecordLenth)\n";
	return 0;
    }
}

unsigned char IRESULT::IsDeleted() const {
    if (MdtTable)
	return ((MdtTable[MdtIndex-1].Fields & TOP_CAT_MASK) == TOP_CAT_Deleted);
    else {
	cerr << "NO HASHTABLE! (IsDeleted)\n";
	return 0;
    }
}

unsigned char IRESULT::IsCool() const {
    if (MdtTable)
	return ((MdtTable[MdtIndex-1].Fields & COOL_MASK) == COOL_MASK);
    else {
	cerr << "NO HASHTABLE! (IsCool)\n";
	return 0;
    }
}

unsigned char IRESULT::IsAdult() const {
    if (MdtTable)
	return ((MdtTable[MdtIndex-1].Fields & TOP_CAT_MASK) == TOP_CAT_Adult);
    else {
	cerr << "NO HASHTABLE! (IsAdult)\n";
	return 0;
    }
}

unsigned short IRESULT::GetFields() const {
    if (MdtTable)
	return (MdtTable[MdtIndex-1].Fields);
    else {
	cerr << "NO HASHTABLE! (GetFields)\n";
	return 0;
    }
}
unsigned char IRESULT::GetCatDepth() const {

    int CatDepth = 0;

    if (MdtTable)
	CatDepth = ((MdtTable[MdtIndex-1].Fields & CAT_DEPTH_MASK) >> CAT_DEPTH_SHIFT);
    else {
	cerr << "NO HASHTABLE! (GetCatDepth)\n";
	return 0;
    }

    if (CatDepth)
	return CatDepth;
    else
	return 1;
}


IRESULT::~IRESULT() { }
