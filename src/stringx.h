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

/* Class STRING */

#ifndef STRINGX_H
#define STRINGX_H

#include <string.h>
#include <iostream.h>
#include "gdt.h"
#include "defs.h"

typedef size_t STRINGINDEX;
typedef STRINGINDEX* PSTRINGINDEX;

class STRING {
public:
  STRING();
  STRING(const STRING& OtherString);
  STRING(const CHR* CString);
  STRING(const UCHR* CString);
  STRING(const CHR* NewBuffer, const STRINGINDEX BufferLength);
  STRING(const UCHR* NewBuffer, const STRINGINDEX BufferLength);
  STRING(const INT IntValue);
  STRING&     operator=(const CHR* CString);
  STRING&     operator=(const GDT_BOOLEAN BoolValue);
  STRING&     operator=(const INT IntValue);
  STRING&     operator=(const LONG LongValue);
  STRING&     operator=(const DOUBLE DoubleValue);
  STRING&     operator=(const STRING& OtherString);
  operator    const char *() const;
  operator    const unsigned char *() const;
  STRING&     operator+=(const UCHR Character);
  STRING&     operator+=(const CHR* CString);
  STRING&     operator+=(const STRING& OtherString);
  INT         operator==(const STRING& OtherString) const;
  INT         operator==(const CHR* CString) const;
  INT         operator!=(const STRING& OtherString) const;
  INT         operator!=(const CHR* CString) const;
  INT         operator^=(const STRING& OtherString) const;
  INT         operator^=(const CHR* CString) const;
  void        Set(const UCHR* NewBuffer, const STRINGINDEX BufferLength);
  INT         Equals(const STRING& OtherString) const;
  INT         Equals(const CHR* CString) const;
  INT         CaseEquals(const STRING& OtherString) const;
  INT         CaseEquals(const CHR* CString) const;
  void        Print() const;
  void        Print(PFILE FilePointer) const;
  friend      ostream& operator<<(ostream& os, const STRING& str);
  INT         GetInt() const;
  LONG        GetLong() const;
  DOUBLE      GetFloat() const;
  GDT_BOOLEAN FGet(PFILE FilePointer, const STRINGINDEX MaxCharacters);
  GDT_BOOLEAN FGetMultiLine(PFILE FilePointer, 
			    const STRINGINDEX MaxCharacters);
  STRINGINDEX GetLength() const;
  UCHR        GetChr(STRINGINDEX Index) const;
  void        SetChr(const STRINGINDEX Index, const UCHR NewChr);
  void        Cat(const UCHR Character);
  void        Cat(const CHR* CString);
  void        Cat(const CHR* CString, STRINGINDEX CLength);
  void        Cat(const STRING& OtherString);
  void        Insert(const STRINGINDEX InsertionPoint, 
		     const STRING& OtherString);
  STRINGINDEX Bold_Search(const CHR* CString) const;
  STRINGINDEX Search(const CHR* CString) const;
  STRINGINDEX Search(const UCHR Character) const;
  STRINGINDEX SearchReverse(const CHR* CString) const;
  STRINGINDEX SearchReverse(const UCHR Character) const;
  INT         STRING::Bold(const CHR* CStringSearch);
  INT         Replace(const CHR* CStringSearch, const CHR* CStringReplace);
  INT         Replace(const CHR* CStringSearch, const STRING& CStringReplace);
  void        EraseBefore(const STRINGINDEX Index);
  void        EraseAfter(const STRINGINDEX Index);
  void        UpperCase();
  void        GetCString(CHR* CStringBuffer, const INT BufferSize) const;
  CHR*        NewCString() const;	// Remember to delete [] !!
  UCHR*       NewUCString() const;	// Remember to delete [] !!

  void        SetMinInitBufLen(STRINGINDEX InitBufLen);
  void        SetBufLenIncr(STRINGINDEX BufLenIncr);
  void        SetDoDoubleBufLen(GDT_BOOLEAN DoDoubling);
  void        StrBuffAlloc(STRINGINDEX BufferSizeRequest);
  ~STRING();
  UCHR               *Buffer;
private:
  void                Copy(const UCHR *CString, STRINGINDEX CLength);

  STRINGINDEX         Length;
  static STRINGINDEX  InitialBufferLength;
  static STRINGINDEX  BufferLengthIncr;
  static GDT_BOOLEAN  DoubleBufferOnCopy;
  STRINGINDEX         BufferSize;
};

typedef STRING* PSTRING;

INT StrUnlink(const STRING& FileName);
INT StrCaseCmp(const CHR* s1, const CHR* s2);
INT StrCaseCmp(const UCHR* s1, const UCHR* s2);
INT StrNCaseCmp(const CHR* s1, const CHR* s2, const size_t n);
INT StrNCaseCmp(const UCHR* s1, const UCHR* s2, const size_t n);

#endif /* STRINGX_H */
