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

/* Description:    CGI utilities */

#ifndef _CGIUTIL_HXX
#define _CGIUTIL_HXX

#include "gdt.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>

#define CGI_MAXENTRIES 100
#define POST 0
#define GET 1
#define TTY 2

class CGIAPP {
  char* name[CGI_MAXENTRIES];
  char* value[CGI_MAXENTRIES];
  int entry_count;
  int Method;

public:
  void GetInput();
  CGIAPP();
  void Display();
  char* GetName(int i);
  char* GetValue(int i);
  char* GetValueByName(const char *name);
  char* GetEnvByName(const char *field);
  ~CGIAPP();

};

void plustospace(char* p);
void unescape_url(char* p);
void unescape_string(char* p);
void escape_url(unsigned char* url, unsigned char* out);
void spacetoplus(char* str);
int  x2c(char* p);
unsigned char *c2x(unsigned char what);

#endif
