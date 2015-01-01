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

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <unistd.h>
#include "defs.h"
#include "stringx.h"
#include "common.h"
#include "record.h"
#include "mdtrec.h"
#include "mdt.h"

// MdtTable is a shared global
MDTREC *MdtTable = NULL;

MDT::MDT(const char *DbFileStem, const GDT_BOOLEAN WrongEndian) 
{
    STRING Fn;

    Changed = GDT_FALSE;
    
    MdtWrongEndian = WrongEndian;
    FileStem = strdup(DbFileStem);
    
    int   fd;
    long  file_length = 0;
    
    ReadOnly = GDT_TRUE;
    TotalEntries = 0;
    
    // Open on-disk MDT
    umask(000);
    ReadOnly = GDT_FALSE;
    Fn = FileStem;
    Fn += DbExtMdt;
    MdtFd = open((char *)Fn.Buffer, O_RDONLY);
    if (MdtFd < 0) {
	ReadOnly = GDT_FALSE;
	MdtFd = open((char *)Fn.Buffer, O_RDWR|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH); 
    }
    else {
	// mmap MDT file for easier and faster access
	TotalEntries = lseek(MdtFd, 0L, SEEK_END) / sizeof(MDTREC);
	MdtTable = (MDTREC *) mmap(NULL, TotalEntries * sizeof(MDTREC),PROT_READ, MAP_SHARED, MdtFd, 0);
    }
    
    if (MdtFd < 0) {
	perror(Fn);
	MdtFd = -1;
	return ;
    }
    
}

void MDT::AddEntry(const MDTREC& MdtRecord) 
{
    if (ReadOnly == GDT_TRUE) {
	fprintf(stderr,"ERROR: mdt file was not opened for write.\n");
	return;
    }
    
    // Add to on-disk MDT
    // write out the MDT record
    write(MdtFd, (char*)&MdtRecord, sizeof(MDTREC));	
    
    TotalEntries++;
    Changed = GDT_TRUE;
}

void MDT::GetEntry(const SIZE_T Index, MDTREC* MdtrecPtr) const 
{
    if ((Index > 0) && (Index <= TotalEntries) ) {
	memcpy((void *)MdtrecPtr, (void *) (MdtTable +  (Index - 1)), sizeof(MDTREC));
    }
}

void MDT::SetEntry(const SIZE_T Index, const MDTREC& MdtRecord) 
{
    if (ReadOnly == GDT_TRUE) {
	return;
    }
    if ( (Index > 0) && (Index <= TotalEntries) ) {
	// Save on-disk record
	lseek(MdtFd, (Index - 1) * sizeof(MDTREC), SEEK_SET);
	write(MdtFd, (char*)&MdtRecord,sizeof(MDTREC));
	Changed = GDT_TRUE;
    }
}

SIZE_T MDT::GetTotalEntries() const 
{
    return TotalEntries;
}

int MDT::GetChanged() const 
{

    return Changed;
}

MDT::~MDT() 
{
    if (MdtTable && (ReadOnly == GDT_TRUE)) {
	munmap((char *) MdtTable, TotalEntries * sizeof(MDTREC));
    }

  if (MdtFd >= 0)
      close(MdtFd);

  if (FileStem)
      free(FileStem);

}
