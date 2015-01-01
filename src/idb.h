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

/* 
 * This is a monsterous "Uber-class" that ties everything together.
 *
 * This is precisely why a) object-oriented code sucks and b) why dumb
 * people shouldn't write code. (is it obvious I didn't design this mess?)
 */


#ifndef IDB_H
#define IDB_H

#include <stdio.h>
#include <string.h>
#include "dtreg.h"
#include "index.h"
#include "record.h"
#include "mdthashtable.h"

class IDB 
  : public IDBOBJ {
    friend class INDEX;
    friend class IRSET;
    friend class DOCTYPE;

public:
    IDB(const STRING& NewPathName, const STRING& NewFileName, STRING DocTypeStr);
    IDB(const STRING& NewPathName, const STRING& NewFileName);

    void         FreeMmap();

    void         Initialize(const STRING& NewPathName,
			    const STRING& NewFileName);

    GDT_BOOLEAN   IsDbCompatible() const;
    char *        GetAllDocTypes() const { return (DocTypeReg->GetDocTypeList()); }

    /* set the directory to restrict searches to */
    void          SetRestrictAdult(int i);
    int           GetRestrictAdult();
    void          SetRestrictKids(unsigned short Kids);
    void          SetRestrictTopCat(unsigned char TopCat);
    void          SetRestrictFilename(char *filename);
    void          SetRestrictPathname(char *pathname);
    void          SetMaxRecordLength(int);
    int           GetMaxRecordLength();

    DOCTYPE      *GetDocTypePtr(const unsigned char DocType) const;
    GDT_BOOLEAN  ValidateDocType(STRING DocType) const;
    GDT_BOOLEAN  ValidateDocType(unsigned char DocType) const;

    int          GetTotalRecords() const { return MainMdt->GetTotalEntries(); }

    IRSET        *Search(search_query *SearchQuery);
    IRSET        *Search(search_query *SearchQuery, int AbsMax);

    void         DebugModeOn() { DebugMode = 1; }
    void         DebugModeOff() { DebugMode = 0; }
    void         SetDebugSkip(const INT Skip) { DebugSkip = Skip; }

    void         ComposeDbFn(STRING *StringBuffer, const CHR *Suffix) const;
    void         GetDbFileStem(STRING *StringBuffer) const;
    char *       GetDbVersionNumber() const;
    void         GetIsearchVersionNumber(STRING *StringBuffer) const { *StringBuffer = IsearchVersion; }

    void         Index(FILE *fp);

    /* This method called during indexing to build GP list for document. */
    unsigned int ParseWords(unsigned char Doctype, 
			   unsigned char* DataBuffer, 
			   int DataLength, 
			   int DataOffset,
			   unsigned int FileId,
			   int *fdList);

    FILE       *ffopen(const STRING& FileName, const CHR *Type) { return fopen((char *)FileName.Buffer, Type); }
    int         ffclose(FILE *FilePointer) { return fclose(FilePointer); }
    GDT_BOOLEAN IsWrongEndian() const { return WrongEndian; }
    void        SetWrongEndian();
    SIZE_T      GpFwrite(GPTYPE* Ptr, SIZE_T Size, SIZE_T NumElements, 
			 FILE* Stream) const;
    SIZE_T      GpFread(GPTYPE* Ptr, SIZE_T Size, SIZE_T NumElements, 
			FILE* Stream) const;
    GDT_BOOLEAN GetDocumentDeleted(const INT Index) const;
    SIZE_T      CleanupDb();
    void        SetGlobalDocType(unsigned char NewGlobalDocType);
    unsigned char GetGlobalDocType();
    INDEX*      GetMainIndex() { return MainIndex; }
    MDT*        GetMainMdt() { return MainMdt; }
    ~IDB();

    MDT        *MainMdt;
    unsigned char GlobalDocType;
    STRING      DbPathName, DbFileName;

protected:
    void IndexingStatus(const INT StatusMessage, const STRING *FileName,
			const INT Count) const {};
private:
    INDEX      *MainIndex;
    int         DebugMode, DebugSkip;
    DTREG      *DocTypeReg;
    int         TotalRecordsQueued;
    GDT_BOOLEAN DbInfoChanged;
    GDT_BOOLEAN WrongEndian;

public:
    unsigned char  RestrictTopCat;
    int            RestrictAdult;
    unsigned short RestrictKids;
    char *         RestrictPathname;
    char *         RestrictFilename;
    long           RestrictFilenameHash;
    int            MaxRecordLength;

    MDTHASHTABLE  ht;

};

typedef IDB* PIDB;

#endif

