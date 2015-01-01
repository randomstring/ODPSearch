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

/* Class IDBOBJ: Database object virtual class */

#ifndef IDBOBJ_H
#define IDBOBJ_H

#include "mdt.h"

class IDBOBJ {
    friend class INDEX;
    friend class IRSET;
    friend class DOCTYPE;

public:
    IDBOBJ() { };
    virtual ~IDBOBJ() { };
    
    virtual char *      GetRestrictDir() { return NULL; };
    virtual int         GetRestrictAdult() { return 1; };
    unsigned int        GetTopLevelRestrictDir() { return 0; };
    int                 GetMaxRecordLength() { return 0; };
    
    virtual void        ComposeDbFn(STRING *StringBuffer, 
				    const CHR *Suffix) const { };

    virtual FILE       *ffopen(const STRING& FileName, const CHR *Type) 
	{ return 0; };
    virtual INT         ffclose(FILE *FilePointer) { return 0; };
    
    virtual SIZE_T      GpFwrite(GPTYPE* Ptr, SIZE_T Size, SIZE_T NumElements, 
				 FILE* Stream) const {
	cerr << "Bad call to IDBOBJ::GpFwrite()\n";
	return 0;
    };
    
    virtual SIZE_T      GpFread(GPTYPE* Ptr, SIZE_T Size, SIZE_T NumElements, 
				FILE* Stream) const {
	cerr << "Bad call to IDBOBJ::GpFread()\n";
	return 0;
    };

    virtual unsigned char GetGlobalDocType() {
	cerr << "Bad call to IDBOBJ::GetGlobalDocType()\n";
	return 0;
    };

    virtual void        GetDbFileStem(STRING *StringBuffer) const { };
  //virtual int         IsStopWord(CHR* WordStart, INT WordMaximum) const = 0;
    
    STRING Title;
    
protected:
    virtual void        IndexingStatus(const INT StatusMessage,
				       const STRING *FileName, 
				       const INT WordCount) const { };
private:
    virtual MDT        *GetMainMdt() { return 0; };
    
    virtual     unsigned int ParseWords(unsigned char Doctype, 
					unsigned char* DataBuffer, 
					int DataLength, 
					int DataOffset,
					unsigned int FileId,
					int *fdList) = 0;
};

typedef IDBOBJ* PIDBOBJ;

#endif
