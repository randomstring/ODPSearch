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

/* A simple parser that will find the start of every word. */

#ifndef SIMPLE_H
#define SIMPLE_H

#include "defs.h"
#include "doctype.h"
#include "catids.h"

class SIMPLE : public DOCTYPE {
public:
      SIMPLE(PIDBOBJ DbParent);
      float ScoreHit(IRESULT result, float weight);  
      unsigned int TopLevelCat(char *q);
      void GetMdtData(unsigned char* DataBuffer,
		      INT4 DataLength,
		      GPTYPE DataOffset,
		      unsigned int pathName,
		      unsigned int fileName,
		      char *path,
		      char *filename,
		      MDTREC *mdtrec);
 
      ~SIMPLE();
};

typedef SIMPLE* PSIMPLE;

#endif
