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

#ifndef TOKENGEN_H
#define TOKENGEN_H

#include "gdt.h"
#include "vlist.h"
#include "strlist.h"

class TOKENGEN {
public:
	TOKENGEN(const STRING &InString);
	~TOKENGEN();
	void GetEntry(const SIZE_T Index, STRING* StringEntry);
	void SetQuoteStripping(GDT_BOOLEAN);
	SIZE_T GetTotalEntries(void);


private:
	CHR *nexttoken(CHR *input, STRING *token);
	STRLIST TokenList;
	GDT_BOOLEAN DoStripQuotes, HaveParsed;
	void DoParse(void);
	CHR *InCharP;
};

#endif
