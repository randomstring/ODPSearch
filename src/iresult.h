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

/* Class IRESULT - Internal Search Result */

#ifndef IRESULT_H
#define IRESULT_H

#include <math.h>
#include "stringx.h"
#include "mdtrec.h"

class IRESULT {
public:
        // IRESULT();
	IRESULT(MDTREC *Mdt);
	IRESULT& operator=(const IRESULT& OtherIresult);
	void SetMdtIndex(const int NewMdtIndex);
	int GetMdtIndex() const;
	void SetHitCount(const int NewHitCount);
	void IncHitCount();
	void IncHitCount(const int AddCount);
        void IncRealHitCount(const int RealCount);
	int GetHitCount() const;
	void SetScore(const float NewScore);
	void IncScore(const float AddScore);
	void AndIncScore(const float AddScore);
	void OrIncScore(const float AddScore);
	float GetScore() const;

#if 0
  	unsigned char GetPriceBucket() const;
  	unsigned int  GetShoppingCat() const;
        unsigned char IRESULT::GetPopularity() const;
#endif
        long GetPathNameHash() const;
        long GetFileNameHash() const;

	GPTYPE GetGlobalFileStart() const;
	GPTYPE GetGlobalFileEnd() const;
	GPTYPE GetRecordLength() const;

  	unsigned short GetFields() const;
	unsigned char  IsCool() const;
  	unsigned char  IsDeleted() const;
  	unsigned char  IsAdult() const;
  	unsigned char  GetCatDepth() const;

	~IRESULT();

	MDTREC *MdtTable;
	int MdtIndex;
        int HitCount;         // count of total term hits
        int RealHitCount;     // count of different terms marched
  
        unsigned char Field;

	float Score;
};

typedef IRESULT* PIRESULT;

#endif
