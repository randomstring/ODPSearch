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

#ifndef MDT_H
#define MDT_H

#include "defs.h"
#include "mdtrec.h"

class MDT {
public:
	MDT(const char *DbFileStem, const GDT_BOOLEAN WrongEndian);
	void AddEntry(const MDTREC& MdtRecord);
	void GetEntry(const SIZE_T Index, MDTREC* MdtrecPtr) const;
	void SetEntry(const SIZE_T Index, const MDTREC& MdtRecord);

	SIZE_T GetTotalEntries() const;
	int  GetChanged() const;

	~MDT();

	int MdtFd;
        MDTREC *MdtTable;
	SIZE_T TotalEntries;
private:
	char *FileStem;
	SIZE_T MaxEntries;
        GDT_BOOLEAN Changed;
	GDT_BOOLEAN MdtWrongEndian;
	GDT_BOOLEAN ReadOnly;
};

typedef MDT* PMDT;

#endif
