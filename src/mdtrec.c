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

/* Description:	Class MDTREC - Multiple Document Table Record */

#include <string.h>

#include "mdtrec.h"
#include "catids.h"

MDTREC::MDTREC() {
  Fields = 0;

  pathName = 0;
  fileName = 0;

  GlobalFileStart = 0;
  FileLength = 0;

}
 
MDTREC& 
MDTREC::operator=(const MDTREC& OtherMdtRec) {

    Fields          = OtherMdtRec.Fields;
    pathName        = OtherMdtRec.pathName;
    fileName        = OtherMdtRec.fileName;
    GlobalFileStart = OtherMdtRec.GlobalFileStart;
    FileLength      = OtherMdtRec.FileLength;
    
    return *this;
}

void 
MDTREC::SetGlobalFileStart(const GPTYPE NewGlobalFileStart) {
    GlobalFileStart = NewGlobalFileStart;
}


GPTYPE 
MDTREC::GetGlobalFileStart() const {
    return GlobalFileStart;
}


void 
MDTREC::SetFileLength(unsigned short NewFileLength) {
    FileLength = NewFileLength;
}


unsigned short
MDTREC::GetFileLength() const {
    return FileLength;
}

void 
MDTREC::SetDeleted() {
    /* clear category field */
    Fields &= ~(TOP_CAT_MASK);
    Fields |=  TOP_CAT_Deleted;  /* all zero */
}


GDT_BOOLEAN 
MDTREC::GetDeleted() const {
    return ((Fields & TOP_CAT_MASK) == TOP_CAT_Deleted) ? GDT_TRUE : GDT_FALSE;
}


MDTREC::~MDTREC() {
}
