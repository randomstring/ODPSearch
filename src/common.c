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

/* Some common function calls */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "catids.h"
#include "common.h"

#define DIR_SLASH '/'

void 
AddTrailingSlash(PSTRING PathName) 
{
  STRINGINDEX x;
  if ( ((x=PathName->GetLength()) > 1) &&
       (PathName->GetChr(x) != DIR_SLASH) ) {
    PathName->Cat(DIR_SLASH);
  }
}

void 
RemovePath(PSTRING FileName) 
{
  STRINGINDEX x;
  if ((x=FileName->SearchReverse(DIR_SLASH)) != 0) {
    FileName->EraseBefore(x+1);
  }
}

void 
RemoveFileName(PSTRING PathName) 
{
  STRINGINDEX x;
  x = PathName->SearchReverse(DIR_SLASH);
  if (x > 0)
    PathName->EraseAfter(x-1);
  else
    PathName->EraseAfter(x);
}


GDT_BOOLEAN 
DBExists(const STRING FileSpec) 
{
  struct stat info;
  GDT_BOOLEAN exists=GDT_FALSE;
  PCHR CheckName;
  STRING IndexFile;

  IndexFile = FileSpec;
  IndexFile.Cat(".mdt");
  CheckName = IndexFile.NewCString();
  if (stat(CheckName, &info) ==0) 
    exists = GDT_TRUE;

  delete CheckName;
  return(exists);
}

GDT_BOOLEAN
IsAlnum(unsigned char c)
{
    /*
     * Allow many more characters as alpha numeric
     */

    /* most common case */
    if ((( 'a' <= c ) && ( c <= 'z' )) ||
	(( 'A' <= c ) && ( c <= 'Z' ))) {
	return GDT_TRUE;
    }
    
    if ((c <= 32)    /* control character, space, '!' */
	|| (c == '!')
	|| (c == '\'')
	|| (c == ',')
	|| (c == '/')
	|| (c == '.')
	|| (c == '*')
	|| (c == '-')
	|| (c == '(')
	|| (c == ')')
	|| (c == ':')
	|| (c == ';')
	|| (c == '?')
	
	|| (c == '[')
	|| (c == '\\')
	|| (c == ']')
	
	|| (c == '{')
	|| (c == '}')
	|| (c == '~')
	
	|| (isspace(c))

	)
    { 
	return GDT_FALSE;
    }
    return GDT_TRUE;
}
