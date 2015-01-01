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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>

#include "common.h"
#include "dtreg.h"
#include "index.h"
#include "idb.h"

class IDBC : public IDB {
public:
    IDBC(const STRING& NewPathName, const STRING& NewFileName) 
	:	IDB(NewPathName, NewFileName) {};

protected:
    void IndexingStatus(const INT StatusMessage, const STRING *FileName,
			const INT WordCount) const {};
};

typedef IDBC* PIDBC;

int main(int argc, char** argv) {
        DTREG dtreg(0);
	printf("Iutil v%s\n", IsearchVersion);
	if (argc < 2) {
		printf("Copyright (c) 1995, 1996, 1997 MCNC/CNIDR\n");
		printf("-d (X) Use (X) as the root name for database files.\n");
		printf("-V     Print the version number.\n");
		printf("-vi    View summary information about the database.\n");
		printf("-vf    View list of fields defined in the database.\n");
		printf("-v     View list of documents in the database.\n");
		printf("-k (X) View list of keywords in the database with mincount of X.\n");
		printf("-gt (X)   Set (X) as the global document type for the database.\n");
		printf("-gt0      Clear the global document type for the database.\n");
		printf("-optimize   Optimize database indexes.\n");
		printf("-m (X)   Load (X) megabytes of data at a time for optimizing\n");
		printf("Document Types Supported:");
		printf("\t%s\n",dtreg.GetDocTypeList());
		return 0;
	}

	if (!setlocale(LC_CTYPE,"")) {
		printf("Warning: Failed to set the locale!\n");
	}

	STRING GlobalDoctype;
	INT SetGlobalDoctype = 0;
	CHR Cwd[256];
	getcwd(Cwd, 255);
	STRING Flag;
	STRING DBName;
	STRING Temp;
	INT DebugFlag = 0;
	INT Skip = 0;
	INT EraseAll = 0;
	INT PathChange = 0;
	INT Cleanup = 0;
	INT OptimizerMemory=1; // 1 MB by default
	INT MinCount = 0;
	INT Keywords = 0;
	INT View = 0;
	INT ViewInfo = 0;
	INT ViewFields = 0;
	INT Optimize = 0;
	INT x = 0;
	INT LastUsed = 0;
	while (x < argc) {
		if (argv[x][0] == '-') {
			Flag = argv[x];
			if(Flag.Equals("-optimize")){
				Optimize=1;
				LastUsed=x;
			}
			if (Flag.Equals("-d")) {
				if (++x >= argc) {
					printf("ERROR: No database name specified after -d.\n");
					return 0;
				}
				DBName = argv[x];
				LastUsed = x;
			}
			if (Flag.Equals("-gt")) {
				if (++x >= argc) {
					printf("ERROR: No document type specified after -gt.\n");
					printf("       Use -gt0 if you want no document type.\n");
					return 0;
				}
				GlobalDoctype = argv[x];
				SetGlobalDoctype = 1;
				LastUsed = x;
			}
			if (Flag.Equals("-m")) {
				if (++x >= argc) {
					printf("ERROR: No memory size specified after -m.\n");
					return 0;
				}
				OptimizerMemory = atoi(argv[x]);
				printf("%i MB Memory Selected\n", OptimizerMemory);
				LastUsed = x;
			}
			if (Flag.Equals("-gt0")) {
				GlobalDoctype = "";
				SetGlobalDoctype = 1;
				LastUsed = x;
			}
			if (Flag.Equals("-debug")) {
				DebugFlag = 1;
				if (x+1 < argc) {
					Temp = argv[x+1];
					Temp.GetCString(Cwd, 256);
					if (isdigit(Cwd[0])) {
						Skip = Temp.GetInt();
						x++;
					}
				}
				LastUsed = x;
			}
			if (Flag.Equals("-erase")) {
				EraseAll = 1;
				LastUsed = x;
			}
			if (Flag.Equals("-v")) {
				View = 1;
				LastUsed = x;
			}
			if (Flag.Equals("-k")) {
				Keywords = 1;
				if (++x >= argc) {
				    printf("ERROR: No mincount specified after -k.\n");
				    return 0;
				}
				MinCount = atoi(argv[x]);
				LastUsed = x;
			}
			if (Flag.Equals("-vf")) {
				ViewFields = 1;
				LastUsed = x;
			}
			if (Flag.Equals("-vi")) {
				ViewInfo = 1;
				LastUsed = x;
			}
			if (Flag.Equals("-V")) {
				return 0;
			}
			if (Flag.Equals("-c")) {
				Cleanup = 1;
				LastUsed = x;
			}
		}
		x++;
	}
	
	if (DBName.Equals("")) {
		DBName = IsearchDefaultDbName;
	}
	
	x = LastUsed + 1;
	
	
	PIDBC pdb;
	STRING DBPathName, DBFileName;
	
	if (!DBExists(DBName)) {
		printf("Database ");
		DBName.Print();
		printf(" does not exist.\n");
		return 0;
	}
	
	struct stat info;
	STRING IndexFile;
	
	DBPathName = DBName;
	DBFileName = DBName;
	RemovePath(&DBFileName);
	RemoveFileName(&DBPathName);
	pdb = new IDBC(DBPathName, DBFileName);
	OptimizerMemory=OptimizerMemory*1024*1024; // in bytes
	
	if (DebugFlag) {
		pdb->DebugModeOn();
	}

#if 0
	if (Optimize) {
		IndexFile = DBName;
		IndexFile.Cat(".inx.1");
		PCHR CheckName;
		CheckName = IndexFile.NewCString();
		if (stat(CheckName, &info) !=0) {
			printf("Database ");
			DBName.Print();
			printf(" does not need optimizing.\n");
			delete CheckName;
			return 0;
		} else {
			delete CheckName;
			pdb->MergeIndexFiles(OptimizerMemory);
		}
	}
#endif

	if (ViewInfo) {
		STRING S;
		INT x, y, z;
		unsigned char gdt = DOC_TYPE_INVALID;
		pdb->GetDbFileStem(&S);
		printf("Database name: ");
		S.Print();
		printf("\n");
		gdt = pdb->GetGlobalDocType();
		printf("Global document type: %s\n", dtreg.GetStringFromDocType(gdt));
		y = pdb->GetTotalRecords();
		printf("Total number of documents: %i\n", y);
		z = 0;
		for (x=1; x<=y; x++) {
		    if (pdb->GetDocumentDeleted(x)) {
		        z++;
		    }
		}
		printf("Documents marked as deleted: %i\n", z);
	}
	
	if (!pdb->IsDbCompatible()) {
		printf("The specified database is not compatible with this version of Iutil.\n");
		printf("Please use matching versions of Iindex, Isearch, and Iutil.\n");
		delete pdb;
		return 0;
	}
  
	if (SetGlobalDoctype) {
		pdb->SetGlobalDocType(dtreg.GetDocTypeFromString(GlobalDoctype));
		if (GlobalDoctype == "") {
			printf("Global document type cleared.\n");
		} else {
			GlobalDoctype.UpperCase();
			printf("Global document type set to ");
			GlobalDoctype.Print();
			printf(".\n");
		}
	}
  
	if (EraseAll) {
		printf("Erase them yourself, this is no longer supported.\n");
		return 0;
	}
  
	if (Cleanup) {
		printf("Cleaning up database (removing deleted documents) ...\n");
		int x = pdb->CleanupDb();
		printf("%i document(s) were removed.\n", x);
	}

	if (ViewFields) {
		printf("The View Fields option is no longer supported.\n");
	}

	if (Keywords) {
	    pdb->GetMainIndex()->PrintKeywords(MinCount);
	}
	
	if (View) {
		unsigned char dt;
		dt = pdb->GetGlobalDocType();
	        printf("Document type = ");
	        if (dt == 0) {
		    printf("    (none) ");
		} else {
		    printf("%10s ",dtreg.GetStringFromDocType(dt));
		}

		printf("D=depth Start FileLength Path/Filename <Cool> <Kids> <Deleted>\n");
		RECORD Record;
		STRING S;
		int y = pdb->GetTotalRecords();
		int x;
		MDT *MainMdt = pdb->GetMainMdt();
		MDTREC mdtrec;
		for (x=1; x<=y; x++) {

		        MainMdt->GetEntry(x, &mdtrec);

			printf("D=%3d ",(mdtrec.Fields & CAT_DEPTH_MASK) >> CAT_DEPTH_SHIFT);

			printf("%9i %4i ", 
			       mdtrec.GlobalFileStart,
			       mdtrec.FileLength);

			S = pdb->ht.GetPathName(mdtrec.pathName);
			S.Cat("/");
			S.Cat(pdb->ht.GetFileName(mdtrec.fileName));
			S.Print();

			printf(" tc=%x",(mdtrec.Fields & TOP_CAT_MASK));

			if (mdtrec.Fields & COOL_MASK) {
			    printf(" COOL");
			}

			if (mdtrec.Fields & KID_MASK) {
			    printf(" Age=");
			    
			    if ((mdtrec.Fields & KID_MASK) & KID_SITE) 
				printf("kid/");
				
			    if ((mdtrec.Fields & KID_MASK) & TEEN_SITE) 
				printf("teen/");
				    
			    if ((mdtrec.Fields & KID_MASK) & MTEEN_SITE) 
				printf("mteen");
			}

			if (mdtrec.GetDeleted()) {
				printf(" DELETED");
			}

			printf("\n");
		}
	}
  
	delete pdb;
	return 0;
}
