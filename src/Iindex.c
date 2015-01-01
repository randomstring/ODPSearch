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

/* Command line program for creating/indexing the data. */

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "common.h"
#include "dtreg.h"
#include "index.h"
#include "idb.h"

class IDBC : public IDB {
public:
	IDBC(const STRING& NewPathName, const STRING& NewFileName, STRING DocTypeStr) 
		: IDB(NewPathName, NewFileName, DocTypeStr) { };
        int Verbose;

protected:
	void IndexingStatus(const int StatusMessage, const STRING *FileName,
			    const int Count) const 
		{
			switch (StatusMessage) {
			case (IndexingStatusParsingFiles):
				printf("   Parsing files ...\n");
				break;
			case (IndexingStatusParsingDocument):
			        if (Verbose) {
				  printf("   Parsing ");
				  FileName->Print();
				  printf(" ...\n");
				}
				break;
			case (IndexingStatusIndexing):
				printf("   Indexing %i words ...\n", Count);
				break;
			case (IndexingStatusMerging):
				printf("   Merging index ...\n");
				break;
			}
		};
};

typedef IDBC* PIDBC;
STRING DocumentType;
unsigned char DocType;
int Verbose = 0;

int main(int argc, char** argv) {
        DTREG dtreg(0);
	printf("Iindex v%s\n", IsearchVersion);
	if (argc < 2) {
		printf("-d (X)   Use (X) as the root name for database files.\n");
		printf("-v       verbose, print parsed files\n");
		printf("-V       Print the version number.\n");
		printf("-t (X)   Index as files of document type (X).\n");
		printf("-f (X)   Read list of file names to be indexed from file (X).\n");
		printf("Document Types Supported:");
		printf("\t%s\n",dtreg.GetDocTypeList());
		return 0;
	}

	if (!setlocale(LC_CTYPE,"")) {
		printf("Warning: Failed to set the locale!\n");
	}

	FILE *fp;
	STRING Flag;
	STRING DBName;
	STRING FileList;
	int DebugFlag = 0;
	int x = 0;
	int LastUsed = 0;
	
	while (x < argc) {
		if (argv[x][0] == '-') {
			Flag = argv[x];
			if (Flag.Equals("-v")) {
			        Verbose = 1;
				LastUsed = x;
			}
			if (Flag.Equals("-d")) {
				if (++x >= argc) {
					printf("ERROR: No database name specified after -d.\n\n");
					return 0;
				}
				DBName = argv[x];
				LastUsed = x;
			}
			if (Flag.Equals("-f")) {
				if (++x >= argc) {
					printf("ERROR: No file name specified after -f.\n\n");
					return 0;
				}
				FileList = argv[x];
				LastUsed = x;
			}
			if (Flag.Equals("-t")) {
				if (++x >= argc) {
					printf("ERROR: No document type name specified after -t.\n\n");
					return 0;
				}
				DocumentType = argv[x];
				DocumentType.UpperCase();
				LastUsed = x;
			}
			if (Flag.Equals("-V")) {
				return 0;
			}
			if (Flag.Equals("-debug")) {
				DebugFlag = 1;
				LastUsed = x;
			}
			
		}
		x++;
	}
  
	if (DBName.Equals("")) {
		DBName = IsearchDefaultDbName;
	}
  
	x = LastUsed + 1;
  
	int NumFiles = argc - x;
	int z = x;
  
	if ( (FileList.Equals("")) && (NumFiles == 0) ) {
		printf("ERROR: No files specified for indexing!\n");
		return 0;
	}
  
	if ( (!FileList.Equals("")) && (NumFiles != 0) ) {
		printf("ERROR: Unable to handle -f and file names at the same time.\n");
		return 0;
	}
	
	STRING PathName, FileName;
	
	printf("Building document list ...\n");
	
	PIDBC pdb;
	STRING DBPathName, DBFileName;
  
	DBPathName = DBName;
	DBFileName = DBName;
	RemovePath(&DBFileName);
	RemoveFileName(&DBPathName);

	//
	// Delete any old files left arround
	//
	STRING KillFile;
	PCHR cKillFile;
	
	KillFile = DBName;
	KillFile.Cat(".mdt");
	StrUnlink(KillFile);
	
	KillFile = "rm -f ";
	KillFile.Cat(DBName);
	KillFile.Cat(".[0-9]*");
	cKillFile = KillFile.NewCString();
	system(cKillFile);
	delete cKillFile;

	KillFile = "rm -f ";
	KillFile.Cat(DBName);
	KillFile.Cat(".inx*");
	cKillFile = KillFile.NewCString();
	system(cKillFile);
	delete cKillFile;

	pdb = new IDBC(DBPathName, DBFileName, DocumentType);
	pdb->Verbose = Verbose;
  
	if (!pdb->ValidateDocType(DocumentType)) {
		printf("ERROR: Unknown document type specified.\n");
		delete pdb;
		return 0;
	}
	
	if (DebugFlag) {
		pdb->DebugModeOn();
	}

	// Set Global Document Type to match -t if there isn't already one
	if (DocumentType.GetLength() >0) {
		unsigned char  GlobalDoctype;
		GlobalDoctype = pdb->GetGlobalDocType();
		if (GlobalDoctype == DOC_TYPE_INVALID) {
		        DocType = dtreg.GetDocTypeFromString(DocumentType);
			pdb->SetGlobalDocType(DocType);
		}
	}
	
	printf("Building database ");
	DBName.Print();
	printf(":\n");
  
	if ( !FileList.Equals("") ) {
 	        // call index directly with optimized filelist code

	        // pdb->SetGlobalDocType(dtreg.GetDocTypeFromString(DocumentType));

		// printf("Global Document type = %d\n",pdb->GetGlobalDocType());

	        if (FileList.Equals("-")) {
		    pdb->Index(stdin);
		}
		else {
		    fp = fopen(FileList, "r");
		    if (!fp) {
		        printf("ERROR: Can't find file list (-f).\n");
			delete pdb;
			return 0;
		    }
		    pdb->Index(fp);
		}
	}
	else {
	    printf("Must specify file list with -f.\n");
	    exit(1);
	}
	
	delete pdb;
	printf("Database files saved to disk.\n");
  
	return 0;
}
