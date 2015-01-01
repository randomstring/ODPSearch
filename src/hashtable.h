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

// hashtable.hxx
//
// by: Bryn Dole   Jan 5, 1999
//
// A simple hash table for storing indexed strings
//
// NOTE: this is not a traditional hash table...


#ifndef HASHTABLE_HXX
#define HASHTABLE_HXX

#include <stdio.h>

typedef struct STR_LIST {
  char*           str;       /* string being matched */
  unsigned long   offset;    /* unique id/offset of the string */
  struct STR_LIST *next;     /* next list item */
} str_list;


class HASHTABLE {
public:
  HASHTABLE();
  ~HASHTABLE();
  void Init(char *fn, int size);  // intialize the hashtable
  int  ReadOnDiskHashTable();     // load on disk hash into memory
  long Add(char *str);            // add a string to the table, return file offset
  char *Get(long i);              // given offset, return string
  unsigned int Hash(char *str);   // hash function for in memory hashtable 

private:
  long offset;        // current offset into on-disk file
  char* filename;     // filename of table values
  int fd;             // file handle
  int mmapped;        // a boolean, true if the hashtable file is currently mmap'd
  int tableSize;      // size of in memory hashtable
  str_list **Table;   // in memory hashtable of value-index pairs

  char *OnDiskTable;  // pointer to mmap()'ed table of strings 
  long file_length;   // length of mmap()'d file
};

#endif
