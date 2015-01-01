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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>            
#include <sys/types.h>
#include <sys/uio.h>  
#include "common.h"
#include "idb.h"
#include "mdthashtable.h"

IDB::IDB(const STRING& NewPathName, const STRING& NewFileName) {

  DocTypeReg = new DTREG(this);

  GlobalDocType = DOC_TYPE_INVALID;

  Initialize(NewPathName, NewFileName);

  if (GlobalDocType == DOC_TYPE_INVALID) {
      GlobalDocType = GetGlobalDocType();
  }

}

IDB::IDB(const STRING& NewPathName, const STRING& NewFileName, const STRING DocTypeStr) {

  DocTypeReg = new DTREG(this);

  GlobalDocType = DocTypeReg->GetDocTypeFromString(DocTypeStr);

  Initialize(NewPathName, NewFileName);

  SetGlobalDocType(GlobalDocType);

}

void
IDB::Initialize(const STRING& NewPathName, const STRING& NewFileName) {

  STRING fn;

  Title = NewFileName;

  /* initialize the MDT hashtable */
  fn = NewPathName;
  fn.Cat(NewFileName);

  ht.Init((char*)fn.Buffer);

  DebugMode = 0;
  DebugSkip = 0;
  TotalRecordsQueued = 0;
  DbPathName = NewPathName;
  AddTrailingSlash(&DbPathName);
  DbFileName = NewFileName;
  RemovePath(&DbFileName);

  /* initialize restriced search parameters */
  RestrictTopCat  = 0;
  RestrictAdult   = 1;
  RestrictKids    = 0;
  RestrictPathname = NULL;
  RestrictFilename = NULL;
  RestrictFilenameHash = 0;
  
  SetWrongEndian();

  /* Create INDEX */
  STRING IndexFN;
  ComposeDbFn(&IndexFN, DbExtIndex);
  MainIndex = new INDEX(this, IndexFN);

  /* Create and load MDT */
  STRING MDTFN;
  ComposeDbFn(&MDTFN, DbExtMdt);
  STRING FileStem;
  GetDbFileStem(&FileStem);
  MainMdt = new MDT((char *)FileStem.Buffer, IsWrongEndian());

  if (MainMdt->MdtFd < 0) {
      /*  failed to open the MDT database file */
      return;
  }

}

void
IDB::SetMaxRecordLength(int MaxLength) {

  MaxRecordLength = MaxLength;
}

int
IDB::GetMaxRecordLength() {

  return (MaxRecordLength);

}

void
IDB::SetRestrictTopCat(unsigned char topcat) {

  RestrictTopCat = topcat;
}

void
IDB::SetRestrictFilename(char * Filename) {

  RestrictFilename = Filename;
  RestrictFilenameHash = 0;
}

void
IDB::SetRestrictPathname(char * Pathname) {

  RestrictPathname = Pathname;
  RestrictTopCat = GetDocTypePtr(GlobalDocType)->TopLevelCat(Pathname);
}

void
IDB::SetRestrictAdult(int i) {

  RestrictAdult = i;
}

void
IDB::SetRestrictKids(unsigned short i) {

  RestrictKids = i;
}


int
IDB::GetRestrictAdult() {

  return (RestrictAdult);
}

void 
IDB::SetWrongEndian() {

    /* XXX - we should have a test like this:
     *   WrongEndian = (IsDbBigEndian() != IsBigEndian()) ? GDT_TRUE : GDT_FALSE;
     */

    WrongEndian = GDT_FALSE;
}

SIZE_T 
IDB::GpFwrite(GPTYPE* Ptr, SIZE_T Size, SIZE_T NumElements, 
	      FILE* Stream) const {
    SIZE_T val;
    val = fwrite((char*)Ptr, Size, NumElements, Stream);
    if (val < NumElements) {
	printf("ERROR: Can't Complete Write!\n");
    }
    return val;
}


SIZE_T 
IDB::GpFread(GPTYPE* Ptr, SIZE_T Size, SIZE_T NumElements, 
	     FILE* Stream) const {
  SIZE_T x = fread((char*)Ptr, Size, NumElements, Stream);
  return x;
}

GDT_BOOLEAN 
IDB::IsDbCompatible() const {
    /* XXX - we sould do some tests here */
    return GDT_TRUE;
}


void 
IDB::ComposeDbFn(STRING *StringBuffer, const CHR *Suffix) const {
    GetDbFileStem(StringBuffer);
    StringBuffer->Cat(Suffix);
}


void 
IDB::GetDbFileStem(STRING *StringBuffer) const {
    *StringBuffer = DbPathName;
    StringBuffer->Cat(DbFileName);
}


DOCTYPE*
IDB::GetDocTypePtr(unsigned char DocType) const {
    DOCTYPE *DoctypePtr = DocTypeReg->GetDocTypePtr(DocType);
    if (DoctypePtr) {
	return DoctypePtr;
    } else {
	return DocTypeReg->GetDocTypePtr(0);
    }
}


GDT_BOOLEAN 
IDB::ValidateDocType(STRING DocTypeString) const {
    unsigned char tmp;
    DTREG dtreg(0);
    tmp = dtreg.GetDocTypeFromString(DocTypeString);
    if (tmp == DOC_TYPE_INVALID)
	return GDT_FALSE;
    else
	return (DocTypeReg->GetDocTypePtr(tmp) != NULL) ? GDT_TRUE : GDT_FALSE;
}

GDT_BOOLEAN 
IDB::ValidateDocType(unsigned char DocType) const {
    return (DocTypeReg->GetDocTypePtr(DocType) != NULL)? GDT_TRUE : GDT_FALSE;
}


IRSET*
IDB::Search(search_query *SearchQuery) {

  return Search(SearchQuery,MAX_RESULT_ENTRIES);
}

IRSET*
IDB::Search(search_query *SearchQuery,int AbsMax) {
  if (!IsDbCompatible()) {
    return (new IRSET(this));
  }
  IRSET *RsetPtr;

  RsetPtr = MainIndex->Search(SearchQuery,AbsMax);

  return RsetPtr;
}


char *
IDB::GetDbVersionNumber() const {
    return(VERS);
}


/*
 * Read in file list, Parse the files, generate an unsorted list of records, call AddRecordList()
 */

void 
IDB::Index(FILE *filelist) {

    if (!IsDbCompatible()) {
      return;
    }

    /* read files one by one from the file list and add them */
    RECORD Record;
    
    /* Check whether we need to set the Global DocType */
    GDT_BOOLEAN SetGDocType;
    
    if (GlobalDocType == DOC_TYPE_INVALID) {
      printf("ERROR: no doctype set.\n");
    }
    
    MainIndex->AddRecordList(filelist);

    TotalRecordsQueued = 0;

}


#if 0
INT 
IDB::IsStopWord(CHR* WordStart, INT WordMaximum) const {
  return ( MainIndex->IsStopWord(WordStart, WordMaximum) );
}
#endif
   

GPTYPE 
IDB::ParseWords(unsigned char DocType, 
		unsigned char* DataBuffer, 
		int DataLength,
		int DataOffset, 
		unsigned int FileId,
		int *fdList)
{
    /* Redirect the call to this method to the appropriate doctype. */
  PDOCTYPE DocTypePtr;

  DocTypePtr = GetDocTypePtr(DocType);
  return ( DocTypePtr->ParseWords(DataBuffer, 
				  DataLength, 
				  DataOffset, 
				  FileId,
				  fdList) );
}
 
GDT_BOOLEAN 
IDB::GetDocumentDeleted(const INT Index) const {
  MDTREC Mdtrec;
  MainMdt->GetEntry(Index, &Mdtrec);
  return Mdtrec.GetDeleted();
}


SIZE_T 
IDB::CleanupDb() {
    /* nothing to be done anymore */
    return (0);
}

void 
IDB::SetGlobalDocType(unsigned char NewGlobalDocType) {
  GlobalDocType = NewGlobalDocType;

  if (GlobalDocType != DOC_TYPE_INVALID) {

    STRING Fn;
    char   buf[128];
    int   fd;

    GetDbFileStem(&Fn);
    Fn.Cat(".doctype");
    
    /* Open the document type file and write the document type */
    umask(000);
    fd = open((char *)Fn.Buffer, O_WRONLY | O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd < 0) {
	perror(Fn);
    }
    else {
	/* read the file */
	DTREG dtreg(0);
	sprintf(buf,"%s\n",dtreg.GetStringFromDocType(NewGlobalDocType));
	write(fd,buf,strlen(buf));

	close(fd);
    }
  }
}


unsigned char
IDB::GetGlobalDocType() {

  if (GlobalDocType == DOC_TYPE_INVALID) {

    STRING Fn;
    char   buf[128];
    int   fd;

    GetDbFileStem(&Fn);
    Fn.Cat(".doctype");
        
    /* Open the document type file and read the document type */
    umask(000);
    fd = open((char *)Fn.Buffer, O_RDONLY);
    if (fd < 0) {
	perror(Fn);
    }
    else {
	/* read the file */
	read(fd,buf,128);

	char *a = strstr(buf,"\n");
	if (a) {
	    a[0] = '\0';
	}

	DTREG dtreg(0);
	GlobalDocType = dtreg.GetDocTypeFromString(buf);
	close(fd);
    }
    
  }

  return GlobalDocType;
}


void
IDB::FreeMmap() {
  if (MainIndex) {
    delete MainIndex;
    MainIndex = NULL;
  }
  if (MainMdt) {
    delete MainMdt;
    MainMdt = NULL;
  }
}

IDB::~IDB() {

  if (MainIndex) {
    delete MainIndex;
  }
  if (MainMdt) {
    delete MainMdt;
  }
  delete DocTypeReg;
}


