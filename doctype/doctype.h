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

/* Generic document type class */

#ifndef DOCTYPE_H
#define DOCTYPE_H

#include "defs.h"
#include "common.h"
#include "mdtrec.h"
#include "iresult.h"
#include "idbobj.h"


class DOCTYPE {
public:
    DOCTYPE(IDBOBJ* DbParent);
    virtual GPTYPE ParseWords(unsigned char* DataBuffer, 
			      int DataLength, 
			      int DataOffset, 
			      unsigned int FileId,
			      int *fdList);
    virtual void ScoreHits(IRESULT *results, 
			   float TermWeight, 
			   int ResultSetSize, 
			   int AbsoluteMaxEntries, 
			   float *MaxScore, 
			   float *MinScore);
    virtual float ScoreHit(IRESULT result, float weight);
    virtual unsigned int TopLevelCat(char *q);
    virtual void SetNumBuckets(int num);
    virtual int GetBucket(unsigned char c);

    virtual void GetMdtData(unsigned char* DataBuffer, 
			    INT4 DataLength, 
			    GPTYPE DataOffset,
			    unsigned int pathName,
			    unsigned int fileName,
			    char *path,			
			    char *filename,
			    MDTREC *mdtrec);

    virtual unsigned char NewField(unsigned char current, unsigned char n);

    virtual ~DOCTYPE();
    
protected:
    IDBOBJ* Db;
    
private:
    int NumBuckets;
    int BucketSize;
};

typedef DOCTYPE* PDOCTYPE;

#endif
