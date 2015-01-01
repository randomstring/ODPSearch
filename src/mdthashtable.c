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

/* =================================================
 * mdthashtable.hxx
 *
 * by: Bryn Dole  Jan 5, 1999
 *
 * C++ class that maps the individual MDT (multi-document
 * table) record ids to the actual record data.
 *
 * The MDT table is a list of indexed files. 
 * Each record has the filename, path,
 * document type, key, and global and local offsets
 * into the data files.
 * 
 * This class uses the ISEARCH.mdt file to store the
 * mdt values indirectly, By storing an index (integer
 * offset to the actual filename/path/document-type/key.
 * Which are strings.
 *
 * This was implemented to eliminate the huge waste of
 * space used by the saving the pathname for each
 * record despite the fact that the path was reused many
 * times.
 */

#include <stdio.h>
#include "mdthashtable.h"

MDTHASHTABLE::MDTHASHTABLE() {}

void
MDTHASHTABLE::Init(char *FileName) {

  char *fn;

  fn = (char *)malloc(strlen(FileName)+5);

  sprintf(fn,"%s.fn",FileName);
  fileNameTable.Init(fn,10039);

  /* current number of categories is 36000 (Jan 8, 1999)  tablesize = 100003
   * current number of categories is 75000 (May 1, 1999)  tablesize = 500009
   * current number of categories is 327000 (Oct 1, 2000) tablesize = 500009
   * this allows for some growth before the hash table gets crowded...
   */
  sprintf(fn,"%s.pn",FileName);
  pathNameTable.Init(fn,500009);

  free(fn);
  
  readondisktable_on_add = 1;
}

MDTHASHTABLE::~MDTHASHTABLE() {};

long 
MDTHASHTABLE::AddFileName(const STRING& NewFileName) {
  return fileNameTable.Add((char *)NewFileName.Buffer);
}

char *
MDTHASHTABLE::GetFileName(long i) {
  return fileNameTable.Get(i);
}

long 
MDTHASHTABLE::AddPathName(const STRING& NewPathName) {

  if (readondisktable_on_add) {
      /* if a pre-existing pathname table exists, read it in */
      pathNameTable.ReadOnDiskHashTable();
      readondisktable_on_add = 0;
  }

  return pathNameTable.Add((char *)NewPathName.Buffer);
}

char *
MDTHASHTABLE::GetPathName(long i) {
  return pathNameTable.Get(i);
}



