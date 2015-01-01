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

/* Commonly used functions */

#ifndef COMMON_H
#define COMMON_H

#include "defs.h"
#include "stringx.h"

#define ALPHA_MAX         100      
#define MAX_POPULARITY    100

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

// PANIC is a combination of the definitions from Firewall class and
// the macro from "POSIX Programmer's Guide" by Lewin
#ifdef __GNUC__
#ifndef HAS__FUNC__
#define HAS__FUNC__
#endif
#endif

//
// __HERE__ has to be a preprocessor macro
//
#ifndef __HERE__
#ifdef HAS__FUNC__
#define __HERE__ __FILE__, __LINE__, __FUNCTION__
#else
#define __HERE__ __FILE__, __LINE__
#endif
#endif

#ifndef PANIC
#define PANIC panic(__HERE__);
#endif

#ifdef HAS__FUNC__
void        panic(const char *filename, const char *func, long line);
#else
void        panic(const char *filename, long line);
#endif

void        AddTrailingSlash(PSTRING PathName);
void        RemovePath(PSTRING FileName);
void        RemoveFileName(PSTRING PathName);
void        RemoveFileExtension(PSTRING PathName);
LONG        GetFileSize(const STRING FileName);
LONG        GetFileSize(const CHR* FileName);
LONG        GetFileSize(FILE* FilePointer);
GDT_BOOLEAN IsFile(const STRING FileName);
GDT_BOOLEAN IsFile(const CHR* FileName);
void        ExpandFileSpec(PSTRING FileSpec);
GDT_BOOLEAN DBExists(const STRING FileSpec);
int         rename(const STRING From, const STRING To);
GDT_BOOLEAN IsAlnum(unsigned char c);
GDT_BOOLEAN IsUTF8_Start_Char(unsigned char c);
GDT_BOOLEAN IsUTF8_Continuation_Char(unsigned char c);
GDT_BOOLEAN IsUTF8_CJK_Char(unsigned char *s);
unsigned int utf8toi(unsigned char *s);
unsigned int TopLevelCat(unsigned char *q);
unsigned char pricebucket(float price);
float pricebucketrange(unsigned char bucket, int minmax);
unsigned int ShoppingCatLookup(char *cat);
const char *ShoppingCatIdLookup(int id);
int isAdultCat(int catid);

#endif

