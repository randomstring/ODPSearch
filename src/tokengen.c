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

#include <ctype.h>

#include "gdt.h"
#include "defs.h"
#include "stringx.h"
#include "vlist.h"
#include "strlist.h"
#include "tokengen.h"

SIZE_T TOKENGEN::GetTotalEntries(void) {
        DoParse();
        return TokenList.GetTotalEntries();
}

void TOKENGEN::SetQuoteStripping(GDT_BOOLEAN DoItOrNot) {
        DoStripQuotes = DoItOrNot;
        HaveParsed = GDT_FALSE;
        DoParse();
}

TOKENGEN::TOKENGEN(const STRING &InString) 
        : DoStripQuotes (GDT_FALSE), HaveParsed(GDT_FALSE)
{
        InCharP = InString.NewCString();
}

TOKENGEN::~TOKENGEN() {
        delete InCharP;
}

void TOKENGEN::DoParse(void) {

        if (HaveParsed)
                return;

        char *Next;
        STRING TokenStr;

        Next = InCharP;

        INT i;
        for ( i=1; (Next = nexttoken(Next, &TokenStr)); i++ ) {
#if 0
  	    cerr << "Token: "<< TokenStr << "\n";
#endif
	    TokenList.SetEntry(i, TokenStr);
	}
        
#if 0
	cerr << "PARSE DONE\n";
#endif
        HaveParsed = GDT_TRUE;
}


void TOKENGEN::GetEntry(const SIZE_T Index, STRING* StringEntry) {
  DoParse();
  TokenList.GetEntry(Index, StringEntry);
}

char *TOKENGEN::nexttoken(char *input, STRING *token) 
{
  int empty_token=1;
  int istoken=0;
  char *end;
  *token = "";

  if (input == NULL)
    return NULL;

  end = input + strlen(input);

  // cerr << "YYY = [" << input << "]\n";

  while(input < end) {

    // cerr << "XXX = [" << input << "]\n";

    // XXX - for some reason Solaris thinks that some parts of
    //       UTF-8 characters are spaces such as (%b1) in %c3%b1
    //       wich is UTF-8 for n tilda. (as in espan~ol)
    //
    // if ( isspace(*input) ) {

    if ( (*input == ' ') || (*input == '\t') || (*input == '\n') || 
	 (*input == '\r') || (*input == '\v') || (*input == '\f')) {
#if 0
      cerr << "ISPACE: " << *input << "\n";
#endif

      if (!istoken) {
	input++;
	continue;
      } else {
	input++;
	return input;
      }
    }
    else if ((*input == '"')  && (istoken) && (*(input-1) != ':')) {
      // lacking a space between token and a quote, insert virtual space
      return input;
    }    
    else if ( *input == '"') {
      //quoted strings are literals and should be returned as one token
      CHR *BeginQuote;
      if (!DoStripQuotes)
	BeginQuote = input;
      else {
	BeginQuote = ++input;
      }
      
      if (*(input+1) == 0) {
	// we're at the end of the string
	*token = "";
	return NULL;
      }

      // cerr << "string = [";
      empty_token =1;
      do {
	*token += *input;
	// cerr << *input;
	if (!isspace(*input) && (*input != '"')) 
	  empty_token = 0;
	input++;
      } while (*input != '"' && *input);

      // cerr << *input << "]\n";

      if (empty_token) {
	// empty quoted string, ignore
	*token="";
	input++;
	continue;
      }
      else if (*input == '"') {
	if (!DoStripQuotes)
	  *token += *input; 
	input++;
	istoken = 1;
      }	
      else {
	//if quotes aren't matched, parse it
	//as part of a single term.
	input = ++BeginQuote;
	token->EraseAfter(token->SearchReverse('"'));
      }
      continue;
    }
    else if ( *input == '&' || *input == '|' ) {
      //looks like a C-style operator
      if ( (*input == *(input+1)) || 
	  ( *input == '&' && *(input+1) == '!') ) {
	//it IS a C-style operator
	if (!istoken) {
	  //if we're looking for a token, grab it
	  //again, STRING = char, doesn't work.
	  *token += *input;
	  input++; 
	  *token += *input;
	  input++;
	  return input;
	} else {
	  return input;
	}
      }	else {
	//must be part of something else
	istoken = 1;
	*token += *input;
	input++;
	continue;
      }
    }
    
    istoken = 1;
    *token += *input;
    input++;
  }
    
  if (istoken)
    return input;
  else 
    return NULL;
}
  
