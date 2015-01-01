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

/*=================================================
 * mdthashtable.hxx
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
 * space used by the saving the fixed 
 *
 *=================================================
 */


#ifndef MDTHASHTABLE_H
#define MDTHASHTABLE_H

#include "hashtable.h"
#include "stringx.h"

class MDTHASHTABLE {
public:
  MDTHASHTABLE();
  ~MDTHASHTABLE();
  void Init(char *FileName);
  long AddFileName(const STRING& NewFileName);
  char *GetFileName(long i);
  long AddPathName(const STRING& NewPathName);
  char *GetPathName(long i);
private:
  // XXX -one day we will want to save the common path
  // xxx -prefix to save space.
  // char pathNamePrefix;
  int readondisktable_on_add;
  HASHTABLE fileNameTable;
  HASHTABLE pathNameTable;
};

#endif
