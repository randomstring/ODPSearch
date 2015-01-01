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

/* Class MDTREC - Multiple Document Table Record */

#ifndef MDTREC_H
#define MDTREC_H

#include "defs.h"
#include "stringx.h"

class MDTREC {
public:
  MDTREC();
  MDTREC& operator=(const MDTREC& OtherMdtRec);
  void SetPopularity(unsigned char NewPopularity);
  unsigned char GetPopularity() const;

  void SetGlobalFileStart(const GPTYPE NewGlobalFileStart);
  GPTYPE GetGlobalFileStart() const;
  void SetFileLength(const unsigned short NewGlobalFileEnd);
  unsigned short GetFileLength() const;
  void SetDeleted();
  GDT_BOOLEAN GetDeleted() const;
  ~MDTREC();

  // real data is below - one of each entry per virtual file/record

#if 0
  unsigned int  ShoppingCat;
  unsigned char Popularity;  
  unsigned char PriceBucket;
  unsigned char MiscFields;
  unsigned char FormatAndDepth;
  GPTYPE GlobalFileEnd;
#endif

  unsigned int pathName;
  unsigned int fileName;

  GPTYPE GlobalFileStart;
  unsigned short FileLength;

  /* Fields = TopLevelCat | Kid_bits */
  unsigned short Fields;

};

typedef MDTREC* PMDTREC;

#endif
