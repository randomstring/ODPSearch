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

#ifndef INDEX_H
#define INDEX_H

#include "defs.h"
#include "idbobj.h"
#include "stringx.h"
#include "mdt.h"
#include "irset.h"

#define NULL_TERM     0x0000
#define AND_TERM      0x0001
#define OR_TERM       0x0002
#define ANDNOT_TERM   0x0004
#define DOMAIN_TERM   0x0008

typedef struct SEARCH_QUERY {
    STRING Term;
    int TermLength;
    unsigned int type;
    int first;
    int last;
    int result_size;
    IRSET *result_set;
    unsigned char field;
    unsigned char docfield;
} search_query;


class INDEX {
    friend class IDB;

public:
    INDEX(IDB *DbParent, const STRING& NewFileName);
    void   AddRecordList(PFILE RecordListFp);
    PIRSET Search(search_query* QueryTerms, int AbsMax);
    PIRSET MultiTermSearch(const STRING& SearchTerm, 
			   int first, 
			   int last,
			   unsigned char field,
			   unsigned char docfield,
			   PIRSET WorkingSet,
			   int AbsMax);
    int Match(const UCHR *QueryTerm, 
	      const int TermLength, 
	      const GPTYPE gp, 
	      const INT4 Offset=0);
    void     SetDocTypePtr(DOCTYPE* NewDocTypePtr) { DocTypePtr = NewDocTypePtr; }
    DOCTYPE *GetDocTypePtr() { return DocTypePtr; }
    void PrintKeywords(int mincount);

    int GetIndirectBuffer(const GPTYPE Gp, UCHR *Buffer, const int len, const int BufferLen);
    ~INDEX();
    
private:
    void ResultRange(index_entry *Index_Table,
		     int file_length,
		     const STRING& Term, 
		     int *result_first,
		     int *result_last);
    int OpenBFB();   

    GPTYPE BuildGpList(unsigned char Doctype,
		       int StartingPosition,
		       unsigned char *MemoryData,
		       int MemoryDataLength,
		       unsigned int FileId,
		       int *fdList);

    GDT_BOOLEAN GetIndirectBuffer(const GPTYPE Gp, 
				  UCHR *Buffer);

    GDT_BOOLEAN GetIndirectBuffer(const GPTYPE Gp, 
				  UCHR *Buffer, 
				  const int len);
    
    GPTYPE  BuildGpList(unsigned char Doctype, 
			int StartingPosition,
			UCHR *MemoryData, 
			int MemoryDataLength, 
			GPTYPE *MemoryIndex, 
			int MemoryIndexLength);

    STRING IndexFileName;
    IDB *Parent;
    DOCTYPE  *DocTypePtr;
    unsigned char *BFB;
    long  BFBLength;
};

typedef INDEX* PINDEX;

#endif

