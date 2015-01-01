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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include "common.h"
#include "stringx.h"

// magic values, based on experimentation 
// see http://ficus.cnidr.org:8080/metrics.html
STRINGINDEX STRING::InitialBufferLength = 32;
STRINGINDEX STRING::BufferLengthIncr = 20;
GDT_BOOLEAN STRING::DoubleBufferOnCopy = GDT_FALSE;

void 
STRING::SetMinInitBufLen(STRINGINDEX InitBufLen) 
{
  InitialBufferLength = InitBufLen;
}

	
void 
STRING::SetBufLenIncr(STRINGINDEX BufLenIncr) 
{
  BufferLengthIncr = BufLenIncr;
}


void 
STRING::SetDoDoubleBufLen(GDT_BOOLEAN DoDoubling) 
{
  DoubleBufferOnCopy = DoDoubling;
}


//this is where allocation and copying policy lives
void 
STRING::StrBuffAlloc(STRINGINDEX BufferSizeRequest) 
{

    if (BufferSizeRequest <= BufferSize)
      return;

    if (Buffer) 
      delete [] Buffer;

    BufferSize = BufferSizeRequest;
    Buffer = new UCHR[BufferSize];

    return;


#ifdef METRICS
  if (BufferSizeRequest > Length) {
    NumTimesExpanded++;
    TotalStringExpns += (BufferSizeRequest - Length);
  }
#endif

  if ((!Buffer)                                // added test for Buffer
      || (BufferSizeRequest > BufferSize)) {
    if (Buffer) {
      delete [] Buffer;
    }

    if (BufferSize != 0) {

#ifdef METRICS
      NumTimesDeleted++;
#endif

      if (DoubleBufferOnCopy) 
	BufferSize =  BufferSizeRequest * 2;
      else 
	BufferSize = (BufferSizeRequest + BufferLengthIncr);

    } else {
      BufferSize = BufferSizeRequest > InitialBufferLength ?
	BufferSizeRequest : InitialBufferLength;
    }

    Buffer = new UCHR[BufferSize];
  }
}


void 
STRING::Copy(const UCHR *CString, STRINGINDEX CLength) 
{
  if ( !CString )
    CLength = 0;

  StrBuffAlloc(CLength + 1);

  if (CLength > 0) {
    memcpy(Buffer, CString, CLength);
  }

  Length = CLength;
  Buffer[Length] = '\0';

}


STRING::STRING() 
{
  BufferSize = InitialBufferLength;
  Buffer = new UCHR[BufferSize];
  Length = 0;
  Buffer[Length] = '\0';
}


STRING::STRING(const STRING& OtherString) 
{
  Buffer = (UCHR*)NULL;
  Length = BufferSize = 0; 
  Copy(OtherString.Buffer, OtherString.Length);		
}


STRING::STRING(const CHR* CString) 
{
  Buffer = (UCHR*)NULL;
  Length = BufferSize = 0; 
  Copy((UCHR *)CString, strlen(CString));
}


STRING::STRING(const UCHR* CString) 
{
  Buffer = (UCHR*)NULL;
  Length = BufferSize = 0; 
  Copy(CString, strlen((CHR *)CString));
}


STRING::STRING(const CHR* NewBuffer, const STRINGINDEX BufferLength) 
{
  Buffer = (UCHR*)NULL;
  Length = BufferSize = 0; 
  Copy((UCHR *)NewBuffer, BufferLength);

}


STRING::STRING(const UCHR* NewBuffer, const STRINGINDEX BufferLength) 
{
  Buffer = (UCHR*)NULL;
  Length = BufferSize = 0; 
  Copy(NewBuffer, BufferLength);
}


STRING::STRING(const INT IntValue) 
{
  Buffer = (UCHR*)NULL;
  Length = BufferSize = 0;
  CHR s[256];
  sprintf(s, "%i", IntValue);
  Copy((UCHR *)s, strlen(s));
}


STRING& STRING::operator=(const CHR* CString) 
{
  if (CString)
    Copy((UCHR *)CString, strlen(CString));
  return *this;
}


STRING& STRING::operator=(const GDT_BOOLEAN BoolValue) 
{
  CHR s[256];
  sprintf(s, "%i", (INT)BoolValue);
  *this = s;
  return *this;
}


STRING& STRING::operator=(const INT IntValue) {
  CHR s[256];
  sprintf(s, "%i", IntValue);
  *this = s;
  return *this;
}


STRING& STRING::operator=(const LONG LongValue) {
  CHR s[256];
  sprintf(s, "%li", LongValue);
  *this = s;
  return *this;
}


STRING& STRING::operator=(const DOUBLE DoubleValue) {
  CHR s[256];
  sprintf(s, "%f", DoubleValue);
  *this = s;
  return *this;
}


STRING& STRING::operator=(const STRING& OtherString) {
  //if he is me, and me is he, oh gee!
  if (&OtherString == this) 
    return *this;
  Copy(OtherString.Buffer, OtherString.Length);

  return *this;
}


void 
STRING::Set(const UCHR* NewBuffer, const STRINGINDEX BufferLength) {
  Copy(NewBuffer, BufferLength);
}


STRING::operator const char *() const {
  return (const char *)Buffer;
}


STRING::operator const unsigned char *() const {
  return (const unsigned char *)Buffer;
}


STRING& STRING::operator+=(const UCHR Character) {
  Cat(Character);
  return *this;
}


STRING& STRING::operator+=(const CHR* CString) {
  Cat(CString);
  return *this;
}


STRING& STRING::operator+=(const STRING& OtherString) {
  Cat(OtherString);
  return *this;
}


INT STRING::operator==(const STRING& OtherString) const {
  return Equals(OtherString);
}


INT STRING::operator==(const CHR* CString) const {
  return Equals(CString);
}


INT STRING::operator!=(const STRING& OtherString) const {
  return !(Equals(OtherString));
}


INT STRING::operator!=(const CHR* CString) const {
  return !(Equals(CString));
}


INT STRING::operator^=(const STRING& OtherString) const {
  return CaseEquals(OtherString);
}


INT STRING::operator^=(const CHR* CString) const {
  return CaseEquals(CString);
}


INT 
STRING::Equals(const STRING& OtherString) const {
  if (Length != OtherString.Length) {
    return 0;
  }
  return ( memcmp(Buffer, OtherString.Buffer, Length) == 0 );
}


INT 
STRING::Equals(const CHR* CString) const {
  if (CString == NULL) 
    return 0;
  if (Length != (STRINGINDEX)strlen(CString)) {
    return 0;
  }
  return ( memcmp(Buffer, CString, Length) == 0 );
}

INT 
STRING::CaseEquals(const STRING& OtherString) const {
  if (Length != OtherString.Length) {
    return 0;
  }
  STRINGINDEX x;
  for (x=0; x<Length; x++) {
    if (tolower(Buffer[x]) != tolower(OtherString.Buffer[x])) {
      return 0;
    }
  }
  return 1;
}


INT 
STRING::CaseEquals(const CHR* CString) const {
  const CHR *p1 = CString;
  const UCHR *p2 = Buffer;
  INT Match = 1;
  STRINGINDEX x;
  for (x = 0; ( (x < Length) && *p1 ); x++) {
    if ( (tolower(*p1) - tolower(*p2)) != 0) {
      Match = 0;
      break;
    }
    else {
      p1++; p2++;
    }
  }
  if (Match) {
    if ( (x == Length) && !*p1)
      return 1;
  }
  
  return 0;
}


void 
STRING::Print() const {
  printf("%s", Buffer);
}


void 
STRING::Print(PFILE FilePointer) const {
  STRINGINDEX x;
  for (x=0; x<Length; x++)
    fprintf(FilePointer, "%c", Buffer[x]);
}


// can this be const STRING& ?
ostream& operator<<(ostream& os, const STRING& str) {
  os.write(str.Buffer, str.Length);
  return os;
}

/*
istream& operator>>(istream& is, STRING& str) {
  CHR buf[256];
  //  is >> buf;
  is.getline(buf,255);
  str = buf;
  return is;
}
*/

INT 
STRING::GetInt() const {
  return atoi(*this);
}


LONG
STRING::GetLong() const {
  return atol(*this);
}


DOUBLE 
STRING::GetFloat() const {
  return atof(*this);
}


GDT_BOOLEAN 
STRING::FGet(PFILE FilePointer, const STRINGINDEX MaxCharacters) {
  CHR* pc = new CHR[MaxCharacters+2];
  CHR* p;
  GDT_BOOLEAN Ok;
  if (fgets(pc, MaxCharacters+1, FilePointer)) {
    p = pc + strlen(pc) - 1;
    while ( (p >= pc) && ( (*p == '\n') || (*p == '\r') ) ) {
      *(p--) = '\0';
    }
    *this = pc;
    Ok = GDT_TRUE;
  } else {
    *this = "";
    Ok = GDT_FALSE;
  }
  delete [] pc;
  return Ok;
}


GDT_BOOLEAN 
STRING::FGetMultiLine(PFILE FilePointer, const STRINGINDEX MaxCharacters) {
  // Gets a string from multiple lines, using Unix-like "\" at the end of
  // the line for a continuation character
  CHR* pc = new CHR[MaxCharacters+2];
  CHR* p;
  GDT_BOOLEAN Ok=GDT_FALSE;
  STRING Buf="";

  while (fgets(pc, MaxCharacters+1, FilePointer)) {
    Ok=GDT_TRUE;
    p = pc + strlen(pc) - 1;
    // Get to the last useful character in the buffer we just read
    while ( (p >= pc) && ( (*p == '\n') || (*p == '\r') ) ) {
      *(p--) = '\0';
    }

    // If the last character is a backslash, we have to store this line
    // into Buf and go get another one.  Otherwise, we just got the last
    // line.
    if (*p == '\\') {
      *(p--) = '\0';
      Buf.Cat(pc);
    } else {
      Buf.Cat(pc);
      break;
    }

  }

  if (Ok)
    *this = Buf;
  else
    *this="";

  delete [] pc;
  return Ok;
}


STRINGINDEX 
STRING::GetLength() const {
  return Length;
}


UCHR 
STRING::GetChr(STRINGINDEX Index) const {
  if ( (Index > 0) && (Index <= Length) )
    return Buffer[Index-1];
  else
    return 0;	// generate ER
}


void 
STRING::SetChr(const STRINGINDEX Index, const UCHR NewChr) {
  if (Index > 0) {
    if (Index > Length) {
      STRINGINDEX x;
      for (x=1; x<(Index-Length); x++) {
	Cat(' ');
      }
      Cat(NewChr);
    } else {
      Buffer[Index-1] = NewChr;
    }
  }
}


void 
STRING::Cat(const UCHR Character) 
{
  if (BufferSize >= Length+2) {
    Buffer[Length] = Character;

#ifdef METRICS
    NumTimesExpanded++;		//this is usually done in StrBuffAlloc
    TotalStringExpns++;		//ditto
#endif

  } else {
    UCHR *Temp = Buffer;
    Buffer = (UCHR*)NULL;
    StrBuffAlloc(Length + 2);
    if (Length>0)
      memcpy(Buffer, Temp, Length);
    Buffer[Length] = Character;
    if (Temp)
      delete [] Temp;
  }
  Length++;
  Buffer[Length] = '\0';
#ifdef METRICS
  TotalStringLength++;
  TotalNumStrings++;
#endif
}


void 
STRING::Cat(const CHR* CString) {
  Cat(CString, strlen(CString));
}


void 
STRING::Cat(const CHR* CString, STRINGINDEX CLength) {
  if (CLength == 0) {
    return;
  }

  if (BufferSize >= (CLength + Length + 1)) {
    memcpy(Buffer + Length, CString, CLength);

#ifdef METRICS
    NumTimesExpanded++;		//this is usually done in StrBuffAlloc
    TotalStringExpns += CLength; //ditto
#endif

  } else {
    UCHR *Temp = Buffer;
    Buffer = (UCHR*)NULL;
    StrBuffAlloc(Length + CLength + 1);
    if (Length>0)
      memcpy(Buffer, Temp, Length);
    memcpy(Buffer + Length, CString, CLength);
    if (Temp)
      delete [] Temp;
  }
  Length += CLength;
  Buffer[Length] = '\0';
#ifdef METRICS
  TotalStringLength += Length;
  TotalNumStrings++;
#endif
}


void 
STRING::Cat(const STRING& OtherString) {
  if (OtherString.Length == 0)
    return;
  Cat((CHR *)OtherString.Buffer, OtherString.Length);
}


void 
STRING::Insert(const STRINGINDEX InsertionPoint, const STRING& OtherString) {
  STRINGINDEX StringLength = OtherString.Length;
  if (StringLength == 0) {
    return;
  }
  if (Length == 0)  {
    StrBuffAlloc(StringLength + 1);
    memcpy(Buffer, OtherString.Buffer, StringLength);
  }
  else if (BufferSize >= (StringLength + Length + 1)) {
    INT RemnantSize = Length - InsertionPoint + 1;
    UCHR *EndFirstBit = Buffer + InsertionPoint - 1;
    UCHR* Remnant = new UCHR[RemnantSize];
    memcpy(Remnant, EndFirstBit, RemnantSize);
    memcpy(EndFirstBit, OtherString.Buffer, StringLength);
    memcpy(EndFirstBit + StringLength, Remnant, RemnantSize);
    delete [] Remnant;
#ifdef METRICS
    NumTimesExpanded++;
    TotalStringExpns += StringLength;
#endif
  }
  else {
    UCHR* Temp = Buffer;
    Buffer = (UCHR*)NULL;
    StrBuffAlloc(Length + StringLength + 1);
    
    //index of the rest of the string
    INT ToCopy = InsertionPoint - 1;
    
    //pointer to the rest of the string
    UCHR *EndBit = Buffer + InsertionPoint - 1;
    
    //how many characters remain
    INT RemnantSize = Length - InsertionPoint + 1;
    memcpy(Buffer, Temp, ToCopy);
    memcpy(Buffer+ToCopy, OtherString.Buffer, StringLength);
    memcpy(EndBit + StringLength, Temp+ToCopy, RemnantSize);
    delete [] Temp;
  }
  Length += StringLength;
  Buffer[Length] = '\0';
#ifdef METRICS
  NumTimesCopied++;
  TotalNumStrings++;
  TotalStringLength += Length;
#endif
}


STRINGINDEX 
STRING::Search(const CHR* CString) const {
  char *p;
  p = strstr((char *)Buffer, CString);
  return p ? (p - (char *)Buffer + 1) : 0;
}

STRINGINDEX 
STRING::Bold_Search(const CHR* CString) const {
  char *p;
  int t;
  int CSlen = strlen(CString);
  char *ss  = (char *) malloc(CSlen +1);
  char *buf = (char *) malloc(strlen((CHR *)Buffer) +1);
  
  t = 0;
  while ( ss[t] = tolower(CString[t])) t++;
  t = 0;
  while ( buf[t] = tolower(Buffer[t])) t++;

  p = strstr((char *)buf, ss);
  t =  p ? (p - (char *)buf + 1) : 0;

  while (t) {
    if (t) {
      //
      // make sure instance of the word is preceded by:
      //  1. the begining of the string
      //  2. a space or punctuation
      //
      if ((t > 1) && isalnum(buf[t-2])) {
	p = strstr(++p, ss);
	t =  p ? (p - (char *)buf + 1) : 0;
	continue;
      }
    }
    if (t) {
      //
      // make sure instance of the word is followed by:
      //  1. the end of the string
      //  2. a space or punctuation
      //  3. XXX - need to add wildcarding for "word*" searches
      //      
      if ((buf[t + CSlen - 1] != 0) && isalnum(buf[t + CSlen -1]) ) {
	p = strstr(++p, ss);
	t =  p ? (p - (char *)buf + 1) : 0;
	continue;
      }
    }
    return t;
  }

  return t;
}

  
STRINGINDEX 
STRING::Search(const UCHR Character) const {
  char *p;
  p = strchr((char *)Buffer, Character);
  return p ? (p - (char *)Buffer + 1) : 0;
}


STRINGINDEX 
STRING::SearchReverse(const CHR* CString) const {
  STRINGINDEX x;
  STRINGINDEX n = (STRINGINDEX) strlen(CString);
  if (n > Length)
    return 0;
  if (n == 0)
    return 0;			// Generate warning ER?
  x = Length - n + 1;
  if (x > 0)
    do {
      x--;
      if (strncmp(CString, (char *)Buffer + x, n) == 0)
	return (x + 1);
    } while (x > 0);
  return 0;
}


STRINGINDEX 
STRING::SearchReverse(const UCHR Character) const {
  STRINGINDEX x;
  if (Length == 0)
    return 0;
  x = Length;
  if (x > 0)
    do {
      x--;
      if (Buffer[x] == Character)
	return (x+1);
    } while (x > 0);
  return 0;
}


INT 
STRING::Bold(const CHR* CStringSearch) {
  STRING NewString, S;
  STRINGINDEX Position;
  INT4 CSLen = strlen(CStringSearch);
  INT Count = 0;
  INT Offset = 0;
  NewString = *this;
  while ( (Position=Bold_Search(CStringSearch)) != 0) {
    Count++;
    NewString.Insert(Position + CSLen + Offset,"</B>");
    NewString.Insert(Position + Offset,"<B>");
    Offset += 7;
    Offset += Position +CSLen -1;
    EraseBefore(Position + CSLen);
  }
  *this = NewString;
  return Count;
}

INT 
STRING::Replace(const CHR* CStringSearch, const CHR* CStringReplace) {
  STRING NewString, S;
  STRINGINDEX Position;
  INT4 CSLen = strlen(CStringSearch);
  INT Count = 0;
  while ( (Position=Search(CStringSearch)) != 0) {
    Count++;
    S = *this;
    S.EraseAfter(Position-1);
    NewString += S;
    NewString += CStringReplace;
    EraseBefore(Position + CSLen);
  }
  NewString += *this;
  *this = NewString;
  return Count;
}


INT 
STRING::Replace(const CHR* CStringSearch, const STRING& CStringReplace) {
  STRING NewString, S;
  STRINGINDEX Position;
  INT4 CSLen = strlen(CStringSearch);
  INT Count = 0;
  while ( (Position=Search(CStringSearch)) != 0) {
    Count++;
    S = *this;
    S.EraseAfter(Position-1);
    NewString += S;
    NewString += CStringReplace;
    EraseBefore(Position + CSLen);
  }
  NewString += *this;
  *this = NewString;
  return Count;
}


void 
STRING::EraseBefore(const STRINGINDEX Index) {
  if (Index <= 1) {
    return;
  }
  if (Index > Length) {
    Length = 0;
    Buffer[Length] = '\0';
    return;
  }
  UCHR* Temp = new UCHR[BufferSize];
  INT4 CharsLeft = Length - Index + 1;
  memcpy(Temp, Buffer + Index - 1, CharsLeft);
  delete [] Buffer;
  Buffer = Temp;
  Length = CharsLeft;
  Buffer[Length] = '\0';
#ifdef METRICS
  TotalNumStrings++;
  TotalStringLength += Length;
  NumTimesCopied++;
#endif
}


void 
STRING::EraseAfter(const STRINGINDEX Index) {
  if (Index > Length)
    return;
  Length = Index;
  Buffer[Length] = '\0';
}


void 
STRING::UpperCase() {
  STRINGINDEX x;
  for (x=0; x<Length; x++) {
    Buffer[x] = toupper(Buffer[x]);
  }
}


void 
STRING::GetCString(CHR* CStringBuffer, const INT BufferSize) const {
  STRINGINDEX ShortLength = BufferSize - 1;
  if (Length < ShortLength)
    ShortLength = Length;
  if (Buffer)
    memcpy(CStringBuffer, Buffer, ShortLength);
  CStringBuffer[ShortLength] = '\0';
}


CHR* 
STRING::NewCString() const {
  return ((CHR*)NewUCString());
}


UCHR* 
STRING::NewUCString() const {
  unsigned char* p = (unsigned char*) malloc(Length+1);
  if (Buffer) {
    memcpy(p, Buffer, Length);
  }
  p[Length] = '\0';
  return p;
}

STRING::~STRING() {
  if (BufferSize)
    if (Buffer != (UCHR*)NULL)
      delete [] Buffer;
}

INT 
StrUnlink(const STRING& FileName) {
  return remove(FileName);
}


INT 
StrCaseCmp(const CHR* s1, const CHR* s2) {
  return StrCaseCmp((UCHR*)s1, (UCHR*)s2);
}


INT 
StrCaseCmp(const UCHR* s1, const UCHR* s2) {
  const UCHR* p1;
  const UCHR* p2;
  INT diff;
  p1 = s1;
  p2 = s2;
  while ((diff=(tolower(*p1) - tolower(*p2))) == 0) {
    if ( (*p1 == '\0') && (*p2 == '\0') ) {
      break;
    }
    p1++;
    p2++;
  }
  return diff;
}


INT 
StrNCaseCmp(const CHR* s1, const CHR* s2, const size_t n) {
  return StrNCaseCmp((UCHR*)s1, (UCHR*)s2, n);
}


INT 
StrNCaseCmp(const UCHR* s1, const UCHR* s2, const size_t n) {
  int diff = 0;
  size_t  x = n;
  while ( x-- && ((diff=tolower(*s1) - tolower(*s2)) == 0) && *s1) {
      s1++;
      s2++;
  }
  return diff;
}
