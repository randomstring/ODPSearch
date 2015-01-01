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

/* WARNING: this file no longer compiles or runs. The interfaces have depricated 
 * too much. Try the commands in cgi-bin
 * Just keeping this arround for historical reference. Maybe one day it'll be 
 * brought back into sync with the rest of the code.
 */


/* Description:	Command-line search utility */

#include <stdio.h>
#include <string.h>
#include <locale.h>

#include "common.h"
#include "dtreg.h"
#include "index.h"
#include "idb.h"


int main(int argc, char** argv) {
	if (argc < 2) {
		printf("Isearch v%s\n", IsearchVersion);
		printf("Copyright (c) 1995, 1996, 1997 MCNC/CNIDR\n");
		printf("-d (X)  # Search database with root name (X).\n");
		printf("-V  # Print the version number.\n");
		printf("-p (X)  # Present element set (X) with results.\n");
		printf("-q  # Print results and quit immediately.\n");
		printf("-t  # Print terse results and quit immediately.\n");
		printf("-prefix (X)  # Add prefix (X) to matched terms in document.\n");
		printf("-suffix (X)  # Add suffix (X) to matched terms in document.\n");
		printf("-byterange  # Print the byte range of each document within\n");
		printf("            # the file that contains it.\n");
		printf("-startdoc (X)  # Display result set starting with the (X)th\n");
		printf("               # document in the list.\n");
		printf("-enddoc (X)  # Display result set ending with the (X)th document\n");
		printf("             # in the list.\n");
		printf("-RECT{North South West East}  # Find targets that overlap\n");
		printf("                              # this geographic rectangle.\n");
		printf("-o (X)  # Document type specific option.\n");
		printf("(X) (Y) (...)  # Search for words (X), (Y), etc.\n");
		printf("               # [fieldname/]searchterm[*][:n]\n");
		printf("                # Prefix with fieldname/ for fielded searching.\n");
		printf("                # Append * for right truncation.\n");
		printf("                # Append :n for term weighting (default=1).\n");
		printf("                # (Use negative values to lower rank.)\n");
		printf("Examples: Isearch -d POETRY truth \"beaut*\" urn:2\n");
		printf("          Isearch -d STORIES cat dog mouse\n");
		printf("          Isearch -d STORIES '(cat or dog) and mouse'\n");
		printf("          Isearch -d PRUFROCK '(ether and table) or mermaids'\n");
		printf("          Isearch -d BIBLE '(Saul||Goliath)&&David'\n");
		printf("Document Types Supported:");
		DTREG dtreg(0);
		STRLIST DocTypeList;
		dtreg.GetDocTypeList(&DocTypeList);
		STRING s;
		INT x;
		INT y = DocTypeList.GetTotalEntries();
		for (x=1; x<=y; x++) {
			DocTypeList.GetEntry(x, &s);
			printf("\t ");
			s.Print();
		}
		printf("\n");
		return 0;
	}

	STRLIST DocTypeOptions;
	STRING Flag;
	STRING DBName;
	STRING ElementSet;
	STRING TermPrefix, TermSuffix;
	STRING StartDoc="", EndDoc="";
	INT DebugFlag = 0;
	INT QuitFlag = 0;
	INT ByteRangeFlag = 0;
	INT BooleanAnd = 0;
	INT RpnQuery = 0;
	INT SpatialRectFlag=0;
	INT x = 0;
	INT LastUsed = 0;
	GDT_BOOLEAN TerseFlag=GDT_FALSE;
	
	ElementSet = "B";
	while (x < argc) {
		if (argv[x][0] == '-') {
			Flag = argv[x];
			if (Flag.Equals("-o")) {
				if (++x >= argc) {
					printf("ERROR: No option specified after -o.\n");
					return 0;
				}
				STRING S;
				S = argv[x];
				DocTypeOptions.AddEntry(S);
				LastUsed = x;
			}
			if (Flag.Equals("-d")) {
				if (++x >= argc) {
					printf("ERROR: No database name specified after -d.\n");
					return 0;
				}
				DBName = argv[x];
				LastUsed = x;
			}
			if (Flag.Equals("-p")) {
				if (++x >= argc) {
					printf("ERROR: No element set specified after -p.\n");
					return 0;
				}
				ElementSet = argv[x];
				LastUsed = x;
			}
			if (Flag.Equals("-prefix")) {
				if (++x >= argc) {
					printf("ERROR: No prefix specified after -prefix.\n\n");
					return 0;
				}
				TermPrefix = argv[x];
				LastUsed = x;
			}
			if (Flag.Equals("-suffix")) {
				if (++x >= argc) {
					printf("ERROR: No suffix specified after -suffix.\n");
					return 0;
				}
				TermSuffix = argv[x];
				LastUsed = x;
			}
			if (Flag.Equals("-startdoc")) {
				if (++x >= argc) {
					printf("ERROR: No value specified after -startdoc.\n");
					return 0;
				}
				StartDoc = argv[x];
				LastUsed = x;
			}
			if (Flag.Equals("-enddoc")) {
				if (++x >= argc) {
					printf("ERROR: No value specified after -enddoc.\n");
					return 0;
				}
				EndDoc = argv[x];
				LastUsed = x;
			}
			if (Flag.Equals("-q")) {
				QuitFlag = 1;
				LastUsed = x;
			}
			if (Flag.Equals("-t")) {
				TerseFlag = GDT_TRUE;
				QuitFlag = 1;
				LastUsed = x;
			}
			if (Flag.Equals("-byterange")) {
				ByteRangeFlag = 1;
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
		//		cout << "ERROR: No database name specified!" << endl;
		//		return 0;
	}
	
	if(!TerseFlag) {
		printf("Isearch v%s\n", IsearchVersion);
	}

	if (!setlocale(LC_CTYPE,"")) {
		printf("Warning: Failed to set the locale!\n");
	}
  
	// this should be in the engine
	//PFILE fp;
	STRING temp;
	temp = DBName;
	temp.Cat(".inx");
  
	if (!DBExists(DBName)) {
		printf("Database ");
		DBName.Print();
		printf(" does not exist.\n");
		return 0;
	}
	
	x = LastUsed + 1;
	if (x >= argc) {
		return 0;
	}
	
	INT NumWords = argc - x;
	INT z = x;

	// the rest is the search query
	PSTRING WordList = new STRING[NumWords];
	search_query  Query[100];
	
	for (z=0; z<NumWords; z++) {
		WordList[z] = argv[z+x];

		Query[z].first = -1;
		Query[z].last = -1;
		Query[z].result_size = 0;
		Query[z].result_set  = NULL;
		Query[z].Term = argv[z+x];
		Query[z].type = AND_TERM;

	}

	Query[z].first = -1;
	Query[z].last = -1;
	Query[z].result_size = 0;
	Query[z].result_set  = NULL;
	Query[z].Term = "";
	Query[z].type = NULL_TERM;

	PIDB pdb=(PIDB)NULL;
	STRING DBPathName, DBFileName;
	STRING PathName, FileName;


	PRSET prset=(PRSET)NULL;
	PIRSET pirset=(PIRSET)NULL;
	RESULT result;
	INT t, n;
	
	DBPathName = DBName;
	DBFileName = DBName;
	RemovePath(&DBFileName);
	RemoveFileName(&DBPathName);
	pdb = new IDB(DBPathName, DBFileName, DocTypeOptions);
  
	if (DebugFlag) {
		pdb->DebugModeOn();
	}
  
	if (!pdb->IsDbCompatible()) {
		printf("The specified database is not compatible with this version of Isearch.\n");
		printf("Please use matching versions of Iindex, Isearch, and Iutil.\n");
		delete [] WordList;
		delete pdb;
		return 0;
	}
  
	if(!TerseFlag) {
		printf("Searching database ");
		DBName.Print();
		printf(":\n");
	}
	
	// if (BooleanAnd) { }

	pirset = pdb->NewSearch(Query);

	n = pirset->GetTotalEntries();
	pirset->SortByScore();
	
	STRING RecordSyntax = SutrsRecordSyntax;
	pdb->BeginRsetPresent(RecordSyntax);
	
	if(!TerseFlag) {
		printf("\n%i document(s) matched your query, ", n);
	}
	
	// Display only documents in -startdoc/-enddoc range
	INT x1, x2;
	x1 = StartDoc.GetInt();
	if(x1 <= 1)
		x1 = 1;
	x2 = EndDoc.GetInt();
	
	if ( (x1 != 1) || (x2 != 0) ) {
		//    printf("X1 %d X2 %d\n",x1,x2);
		if (x2 == 0)
			x2 = n;
		
		PRSET NewPrset;
		NewPrset=pirset->GetRset(x1-1,x2); 
		pirset->Fill(x1-1,x2,NewPrset);
		NewPrset->SetScoreRange(pirset->GetMaxScore(),
					pirset->GetMinScore());
		delete prset;
		prset = NewPrset;
	} else {			// display all of them
		//    x2=pirset->GetTotalEntries();
		//    n=x2;
		//    x2=n;
		prset=pirset->GetRset(0,n);
		pirset->Fill(0,n,prset);
		prset->SetScoreRange(pirset->GetMaxScore(),
				     pirset->GetMinScore());
	}

#ifdef DEBUG  
	if (n>0) {
		printf(" unscaled scores from %i to %i\n",
		       pirset->GetMinScore(), pirset->GetMaxScore());
	} else
		printf("\n");
#endif

	n = prset->GetTotalEntries();
	if(!TerseFlag) {
		printf("%i document(s) displayed.\n\n", n);
	}
	
	CHR Selection[80];
	CHR s[256];
	INT FileNum;
	STRING BriefString;
	STRING Element, TempElementSet;
	STRLIST BriefList;
	STRING TotalBrief;
	
	INT MajorCount=0;
	INT LoadPos=1;
	do {
		if ((n != 0) && (!TerseFlag)) {
			printf("      Score   File\n");
		}
    
		for (t=1; t<=n; t++) {
			if(MajorCount%20==0) {
				LoadPos=1;
			} else {
				LoadPos++;
			}
			
			//      prset->GetEntry(LoadPos, &result);
			prset->GetEntry(t, &result);
			++MajorCount;
			
			if(!TerseFlag) {
				printf("%4i.", t);
				printf("%6i   ", prset->GetScaledScore(result.GetScore(), 100));
			} else {
				printf("%i | ", prset->GetScaledScore(result.GetScore(), 100));
			}
			
			result.GetPathName(&PathName);
			result.GetFileName(&FileName);
			PathName.Print();
			FileName.Print();
			
			if(TerseFlag) {
				printf(" | ");
			} else {
				printf("\n");
			}
				
			if (ByteRangeFlag) {
				printf("              [ %i - %i ]   length = %i\n",
				       result.GlobalFileStart,
				       result.GlobalFileEnd,
				       result.RecordEnd);
				
				if (TerseFlag)
				  printf(" [ %i - %i ] l= %i",
					 result.GlobalFileStart,
					 result.GlobalFileEnd,
					 result.RecordEnd);
			}

#if 0			
			if ( !TerseFlag ) {
			  char *buffer;
			  int length;

			  length = result.RecordEnd;
			  buffer = (char *) malloc(length +1);
			  pdb->GetMainIndex()->GetIndirectBuffer(result.GlobalFileStart, 
					       (unsigned char *) buffer, 0, 
					       length);
			  buffer[length] = '\0';
			  printf("%s",buffer);
			  free(buffer);
			  printf("\n");
			}
#endif

			if (TerseFlag)
			  printf("\n");

		}
		pdb->EndRsetPresent(RecordSyntax);
		if ( (QuitFlag) || (n == 0) ) {
			FileNum = 0;
		} else {
			printf("\nSelect file #: ");
			gets(Selection);
			FileNum = atoi(Selection);
		}
		if ( (FileNum > n) || (FileNum < 0) ) {
			printf("\nSelect a number between 1 and %i.\n", n);
		}
		if ( (FileNum != 0) && (FileNum <= n) && (FileNum >= 1) ) {
			prset->GetEntry(FileNum, &result);

			char *buffer;
			int length;
			
			length = result.RecordEnd;
			buffer = (char *) malloc(length +1);
			pdb->GetMainIndex()->GetIndirectBuffer(result.GlobalFileStart, 
							       (unsigned char *) buffer, 0, 
							       length);
			buffer[length] = '\0';
			printf("%s\n",buffer);
			free(buffer);			
			
			printf("Press <Return> to select another file: ");
			gets(s);
			printf("\n");
			LoadPos=0;
			MajorCount=0;
			
		}
	} while (FileNum != 0);
	
	delete [] WordList;
	delete pirset;
	delete prset;
	delete pdb;
	
	return 0;
}
