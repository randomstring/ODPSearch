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

/* This class accesses the index. */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "defs.h"
#include "stringx.h"
#include "vlist.h"
#include "strlist.h"
#include "common.h"
#include "record.h"
#include "mdtrec.h"
#include "mdt.h"
#include "idbobj.h"
#include "iresult.h"
#include "irset.h"
#include "dtreg.h"
#include "idb.h"
#include "catids.h"
#include "heapsort.h"
#include "index.h"

static char *index_version = "$Header: /m/webapps/master/root/gh/ODPsearch/src/index.c,v 1.4 2002/04/26 01:06:09 dole Exp $";

unsigned char *my_BFB_handel = NULL;

INDEX::INDEX(IDB *DbParent, const STRING& NewFileName) 
{
  Parent = DbParent;
  IndexFileName = NewFileName;
  BFB = NULL;
  BFBLength = 0;
}

/*
 * Compare two index_entry's by comparing the word the the
 * global possition index points to
 */

#if 0
/* use this to selectively turn on/off the WordCompare debug */
int wordcompare_debug = 1;
#endif

int 
WordCompare(const void* x, const void* y) {

  /*inline the most frequently called function
   * return StrNCaseCmp( my_BFB_handel + (*((PGPTYPE)x)),
   *		      my_BFB_handel + (*((PGPTYPE)y)), StringCompLength);
   */

  size_t n = StringCompLength;
  unsigned char *s1,*s2;
  s1 = my_BFB_handel + ((index_entry *)x)->gpindex;
  s2 = my_BFB_handel + ((index_entry *)y)->gpindex;

  int diff = 0;
  while ( n-- && ((diff= tolower(*s1) - tolower(*s2)) == 0) && *s1) {
      s1++;
      s2++;
  }

#if 0
if (wordcompare_debug){
    /* for debuging the compare function */
  s1 = my_BFB_handel + ((index_entry *)x)->gpindex;
  s2 = my_BFB_handel + ((index_entry *)y)->gpindex;
  int i; 
  for(i=0; i < 6 ; i++) {
      if (s1[i] < ' ') {
	if (s2[i] == '\0')
	  printf("\\0");
	else if (s2[i] == '\n')
	  printf("\\n");
	else
	  printf("X");
      }
      else
	  printf("%c",s1[i]);
  }

A  if (diff == 0) 
    printf(" = ");
  else if (diff < 0) 
    printf(" < ");
  else
    printf(" > ");

  for(i=0; i < 6 ; i++) {
      if (s2[i] < ' ') {
	if (s2[i] == '\0')
	  printf("\\0");
	else if (s2[i] == '\n')
	  printf("\\n");
	else
	  printf("X");
      }
      else
	  printf("%c",s2[i]);
  }


  printf(" x[byte =%8d,doc = %6d, field = %c]  y[byte =%8d,doc = %6d, field = %c]",
	 ((index_entry *)x)->gpindex,
	 ((index_entry *)x)->mdtindex & FILEID_MASK ,
	 (char)((((index_entry *)x)->mdtindex & FIELD_MASK) >> 24),
	 ((index_entry *)y)->gpindex,
	 ((index_entry *)y)->mdtindex & FILEID_MASK , 
	 (char)((((index_entry *)y)->mdtindex & FIELD_MASK) >> 24)
	 );

  printf("\n");
}
#endif

  return diff;
}


#define MAX_FILENAME_LENGTH     8000

void 
INDEX::AddRecordList(PFILE RecordListFp) 
{
  int MemoryIndexLength = 0;
  STRING PathName, FileName;
  MDTREC mdtrec;
  UINT4 DataFileSize = 0;
  unsigned int numWords = 0;
  GPTYPE TrueGlobalStart = 0;
  GPTYPE TrueGlobalEnd   = 0;
  unsigned char *q, *r;
  int i;
  int numFiles = 38;    /* (26) + 2 */
  int fdList[40];
  PDOCTYPE DocTypePtr = NULL;
  int filecount = 1;
  CHR f[MAX_FILENAME_LENGTH];  /* filename */

  /* Open the Big Frigg'n Buffer (BFB) to avoid having to read
   * the files in individually
   */
  if (!BFB) {
    if (!OpenBFB()) {
      printf("ERROR: can't open BFB\n");
      exit(2);
    }
  }

  umask(000);

  /* 
   * open up some buckets for sorting the word entries into
   */
  for(i = 0; i < numFiles; i++) {
      sprintf(f,"%s.bucket.%d",(char *)IndexFileName.Buffer,i);
      fdList[i] = open(f,O_WRONLY | O_CREAT | O_TRUNC,0644);
      if (fdList[i] < 0 ) {
	  perror("can't open file.");
	  fprintf(stderr,"can't open file %s\n",f);
      }
  }

  DocTypePtr = Parent->GetDocTypePtr(Parent->GetGlobalDocType());
  DocTypePtr->SetNumBuckets(numFiles);

  MDT *mdt = Parent->GetMainMdt();
  
  unsigned char doctype = Parent->GetGlobalDocType();

  int parsedFiles = 0;

  while (fgets(f,MAX_FILENAME_LENGTH -1 , RecordListFp) != NULL) {
      if (f[strlen(f)-1] != '\n') {
	  f[MAX_FILENAME_LENGTH - 1] = '\0';
	  printf("ERROR: filename too long [%s]\n",f);
      }
      else {
	  parsedFiles++;

	  f[strlen(f)-1] = '\0';
	  
	  /* each file is '\0' terminated */
	  DataFileSize = strlen((char *)BFB + TrueGlobalStart) + 1;
	  
	  TrueGlobalEnd = TrueGlobalStart + DataFileSize - 1;
	  
	  PathName = f;
	  FileName = f;
	  RemovePath(&FileName);
	  RemoveFileName(&PathName);


	  numWords += Parent->ParseWords(doctype, 
					 BFB + TrueGlobalStart,
					 DataFileSize,
					 TrueGlobalStart,
					 filecount,
					 fdList);

	  mdtrec.pathName = Parent->ht.AddPathName(PathName);
	  mdtrec.fileName = Parent->ht.AddFileName(FileName);
	  mdtrec.GlobalFileStart = TrueGlobalStart;
	  mdtrec.FileLength      = TrueGlobalEnd - TrueGlobalStart;

          DocTypePtr->GetMdtData(BFB,
                                 DataFileSize,
                                 TrueGlobalStart,
                                 mdtrec.pathName,
                                 mdtrec.fileName,
                                 (char *) PathName.Buffer,
                                 (char *) FileName.Buffer,
                                 &mdtrec);

	  mdt->AddEntry(mdtrec);

#if 0
          /* debugging for file parsing */
	  printf("filesize = %d\n",DataFileSize);
	  printf("DATA = %s",(char *)BFB + TrueGlobalStart);
	  printf("File: %d %s Start= %d  End= %d length= %d\n", oldpath, f,
		 TrueGlobalStart, TrueGlobalEnd, DataFileSize);
	  printf("-----------------------------------------------------\n");
	  sleep(1);
#endif

	  /* We're in a new document file, so update global pointers */
	  TrueGlobalStart = TrueGlobalEnd + 1;
      }
      
      filecount++;
  }
  
  Parent->IndexingStatus(IndexingStatusIndexing, 0, numWords);

  for(i = 0; i < numFiles; i++) {
      close(fdList[i]);
  }

  /*
   * open files one at a time and sort them
   */
  sprintf(f,"%s",(char *)IndexFileName.Buffer,i);
  int mainfd = open(f,O_WRONLY | O_CREAT | O_TRUNC,0644);
  int fd = -1;
  int file_length = 0;
  index_entry *bucket_file;

  printf("Sorting %d buckets, bucket: ", numFiles);
  fflush(stdout);

  for(i = 0; i < numFiles; i++) {
      sprintf(f,"%s.bucket.%d",(char *)IndexFileName.Buffer,i);
      fd = open(f,O_RDWR);
      if (fd < 0 ) {
	  perror("can't re-open file.");
	  fprintf(stderr,"can't re-open file %s\n",f);
      }

      /* now read/mmap the file */
      file_length = lseek(fd,0L, SEEK_END);

      bucket_file = (index_entry *) mmap(NULL, file_length ,PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);

      printf("%d ",i + 1);
      fflush(stdout);

#if 0
      qsort(bucket_file, file_length / sizeof(index_entry), sizeof(index_entry), WordCompare);
#else
      heapsort(bucket_file, (size_t)(file_length / sizeof(index_entry)));
#endif

      if (file_length != write(mainfd,(void *)bucket_file, file_length)) {
	  printf("ERROR: failed to write out bucket %d.\n", i);
      }

      munmap((caddr_t) bucket_file, file_length);

      close(fd);
      unlink(f);
  }
  close(mainfd);

  printf("\nStats: Total Records = %d      Total Words = %d\n",parsedFiles,numWords);

  printf("Done sorting.\n");

}

GPTYPE 
INDEX::BuildGpList(unsigned char Doctype,
		   int DataOffset,            /* Index offset into the text buffer 
					       * where the document starts. 
					       */
		   unsigned char *DataBuffer, /*  Pointer to beginning of big text buffer. */
		   int DataLength,            /* Length of big text buffer.*/
		   unsigned int FileId,       /* file identification number */
		   int *fdList)               /* list of open files to write data to */
{
    return ( Parent->ParseWords(Doctype, 
				DataBuffer + DataOffset,
				DataLength,
				DataOffset, 
				FileId,
				fdList));

}

/*
 * Sorting the query terms for speed will of course alter
 * the result set of some search queries, if a true logical
 * boolean query is desired. However, for internet seach 
 * engine style searching, each term is independent of each 
 * other, so this is OK. Users have come to expect this
 * behaviour in search engines, and true boolean searches
 * just confuse them, and is rarely desirable anyway.
 */
static int CompareTerms(const void* x, const void* y) {

  search_query *qx,*qy;

  qx = (search_query *) x;
  qy = (search_query *) y;

  /* sort first by type of boolean And, Or, AndNot */
  if ( qx->type != qy->type )
    return (qx->type - qy->type);
  
  return(qx->result_size - qy->result_size);
}


/* 
 * This takes a list of search terms, finds the number of hits per term,
 * sorts the terms for faster set intersection (AND) and union (OR) 
 * computation, then builds the result set.
 */
PIRSET 
INDEX::Search(search_query* QueryTerms, int AbsMax) 
{
    PIRSET NewIrset = NULL;
    PIRSET TmpIrset = NULL;
    STRING TmpIndexFileName;
    STRING LastTerm = "";
    int fd;
    int i;
    int total_hits = 0;
    int first, last;
    long file_length;
    index_entry *Index_Table;     /* this is the pointer to the mmapped file */

    if (AbsMax <= 0) {
      AbsMax = MAX_RESULT_ENTRIES;
    }
    
    /* Open the index file */
    TmpIndexFileName=IndexFileName;
    fd = open(TmpIndexFileName,O_RDONLY);
    if (fd < 0) {
        perror(TmpIndexFileName);
	exit(12);
    }
    /* now mmap the file */
    file_length = lseek(fd,0L, SEEK_END);
    Index_Table = (index_entry *) mmap(NULL, file_length ,PROT_READ, MAP_SHARED, fd, 0);
    
    /* find all the result ranges */
    i = 0;
    while (QueryTerms[i].type != NULL_TERM) {
      
        ResultRange(Index_Table,file_length,QueryTerms[i].Term, &first, &last);

	QueryTerms[i].first = first;
	QueryTerms[i].last = last;
	if (first >= 0) 
	    QueryTerms[i].result_size = last - first + 1;
	else 
	    QueryTerms[i].result_size = 0;

	total_hits += QueryTerms[i].result_size;
	i++;
    }
    
    /* no search terms */
    if (i == 0) 
        return NewIrset;

    /* reorder terms for optimal set union/intersection  */
    if (i > 1)
        qsort(QueryTerms,i,sizeof(search_query), CompareTerms);

    /* Merge results */
    i = 0;
    while (QueryTerms[i].type != NULL_TERM) {

        if ((QueryTerms[i].result_size != 0) && (QueryTerms[i].Term != LastTerm)) {

	    LastTerm = QueryTerms[i].Term;

	    /* build the result set */
 	    if (NewIrset && (NewIrset->GetTotalEntries() > 0)) {

	      if ((QueryTerms[i].last - QueryTerms[i].first) < 100000) {
		  /* only add more terms if the remaining sets are less than the max set size */
		if (NewIrset->GetTotalEntries() <  AbsMax) {
		  
		    if (QueryTerms[i].type & OR_TERM) {
		        /* don't exclude any records from the set */
		        QueryTerms[i].result_set =  MultiTermSearch(QueryTerms[i].Term,
								    QueryTerms[i].first, 
								    QueryTerms[i].last,
								    QueryTerms[i].field,
								    QueryTerms[i].docfield,
								    NULL,
								    AbsMax);
		    }
		    else {

		        /* only add records that are already represented  */
		        QueryTerms[i].result_set =  MultiTermSearch(QueryTerms[i].Term,
								    QueryTerms[i].first, 
								    QueryTerms[i].last,
								    QueryTerms[i].field,
								    QueryTerms[i].docfield,
								    NewIrset,
								    AbsMax);
		    }
		    
		    QueryTerms[i].result_set->ComputeScores(1.0, 
							    QueryTerms[i].last -  
							    QueryTerms[i].first + 1);
		    
		    /* combine the result sets AND/OR/ANDNOT  */
		    if (QueryTerms[i].type & AND_TERM) 
		        NewIrset->And(QueryTerms[i].result_set);
		    else if (QueryTerms[i].type & OR_TERM) {
		        if (NewIrset->GetTotalEntries() < AbsMax )
			    NewIrset->Or(QueryTerms[i].result_set);
		    }
		    else if (QueryTerms[i].type & ANDNOT_TERM) 
		        NewIrset->AndNot(QueryTerms[i].result_set);
		}
	      }
#if 0
	      else {
		cerr << "Too many hits [" << (QueryTerms[i].last - QueryTerms[i].first) 
		     << "] for term: " << QueryTerms[i].Term << "\n";;
	      }
#endif
	    }
	    else {
	        if (QueryTerms[i].type != ANDNOT_TERM) {
#if 0
		    /* don't include result sets greater than this size */
		  if ((QueryTerms[i].last - QueryTerms[i].first) < 100000) {
#endif
		      NewIrset =  MultiTermSearch(QueryTerms[i].Term,
						  QueryTerms[i].first, 
						  QueryTerms[i].last,
						  QueryTerms[i].field,
						  QueryTerms[i].docfield,
						  NULL,
						  AbsMax);
		      NewIrset->ComputeScores(1.0,
					      QueryTerms[i].last - QueryTerms[i].first + 1 );
		      
		      QueryTerms[i].result_set = NewIrset;
#if 0		  
		  }
#endif
		}
	    }


#if 0
	    fprintf(stderr,"%2d ",i);
	    if (QueryTerms[i].type & AND_TERM )
	      fprintf(stderr,"AND    ");
	    if (QueryTerms[i].type & OR_TERM )
	      fprintf(stderr,"OR     ");
	    if (QueryTerms[i].type & ANDNOT_TERM )
	      fprintf(stderr,"ANDNOT ");
	    
	    fprintf(stderr,"--> Lookup TERM = %20s %10d ", 
		   QueryTerms[i].Term.Buffer, 
		   QueryTerms[i].result_size);

	    if (QueryTerms[i].result_set) {
	      fprintf(stderr,"%10d",QueryTerms[i].result_set->GetTotalEntries());
	    }
	    else {
	      fprintf(stderr,"%s","[no result set!]");
	    }
	    fprintf(stderr,"\n");

#endif
	}
#if 0
	else {
	    /* no results for this term */
	  
	    cerr << i << " ";
	    if (QueryTerms[i].type & AND_TERM )
	      cerr << "AND    ";
	    if (QueryTerms[i].type & OR_TERM )
	      cerr << "OR     ";
	    if (QueryTerms[i].type & ANDNOT_TERM )
	      cerr << "ANDNOT ";
	    cerr << "--> Lookup TERM = " << QueryTerms[i].Term 
		 << "   \t"
		 << QueryTerms[i].result_size
		 << "   \t"
		 << "0"
		 << "\n";
	}
#endif
	    
#if 0
	if (NewIrset) 
	    cerr << "(" << IndexFileName 
		 << ")=======> Working set size = " << NewIrset->GetTotalEntries() << "\n";
	else
	    cerr << "(" << IndexFileName 
		 << ")=======> Working set size = 0\n";
#endif

	/* if its an AND term and the result set is zero, return */
	if ( QueryTerms[i].type & AND_TERM ) {
	    if ((NewIrset == NULL) || (NewIrset->GetTotalEntries() == 0)) {
		/* if (NewIrset)
		 *  delete NewIrset; 
		 */
	      return NULL;
	    }
	}

	i++;
    }
  
    return NewIrset;
}


/*----------------------------------------
 * OpenBFB - Open the Big Friggen Buffer
 *----------------------------------------
 */
int
INDEX::OpenBFB() {

  STRING BFBFileName;
  int fd;

  if (BFB) {
    printf("WARNING: BFB file is already open\n");
    return 1;
  }

  Parent->ComposeDbFn(&BFBFileName, ".bfb");

  fd = open(BFBFileName,O_RDONLY);
  if (fd < 0) {
    perror(BFBFileName);
    return 0;
  }

  /* how big is the index file ? */
  BFBLength = lseek(fd,0L, SEEK_END);
   
  BFB = (unsigned char *) mmap(NULL, BFBLength ,PROT_READ, MAP_SHARED, fd, 0);

  my_BFB_handel = BFB;
  
  return 1;
}

int
INDEX::GetIndirectBuffer(const GPTYPE Gp, UCHR *Buffer, 
			 const int Offset,
			 const int BufferLen) {
  int len;

  if (!BFB) {
    if (!OpenBFB()) {
	/* Can't open the BFB, nothing to do! */
	if (Buffer && (BufferLen > 1))
	    Buffer[0] = '\0';
	return 0;
    }
  }

  /* make sure we don't walk off the end of the mmap()'d buffer */
  len = ((BufferLen + Gp) < BFBLength ? BufferLen : (BFBLength - Gp));
  
  memcpy(Buffer, BFB + Gp, len);
  Buffer[len] = '\0';

  /* DEBUG: */
  /* cerr << "[" << Buffer << "]\n"; */

  return(len);

}

GDT_BOOLEAN 
INDEX::GetIndirectBuffer(const GPTYPE Gp, UCHR *Buffer,
			 const INT Offset) {
  INT x;
  x=GetIndirectBuffer(Gp,Buffer,Offset,StringCompLength);
  if (x>0)
    return GDT_TRUE;
  return GDT_FALSE;
}


GDT_BOOLEAN 
INDEX::GetIndirectBuffer(const GPTYPE Gp, UCHR *Buffer) {
  INT x;
  x=GetIndirectBuffer(Gp,Buffer,0,StringCompLength);
  if (x>0)
    return GDT_TRUE;
  return GDT_FALSE;
}


/*
 * This used to be the heart of evil. Now comparisons 
 * are done between to stings in a mmap()'d file.
 */
int
INDEX::Match(const UCHR *QueryTerm, const INT TermLength, 
	     const GPTYPE gp, const INT4 Offset) {
    int z;
  
    /* if (!GetIndirectBuffer(gp, Buffer, Offset, StringCompLength))   return -1; */

    if (!BFB) {
	if (!OpenBFB()) {
	    /* Can't open the BFB, nothing to do! */
	    return 0;
	}
    }
    
    if ( QueryTerm[TermLength - 1] == '*' ) 
	z = StrNCaseCmp(QueryTerm, BFB + gp, TermLength - 1);
    else {
	z = StrNCaseCmp(QueryTerm, BFB + gp, TermLength);
	
	/* this is an ugly hack to ignore all word matches that have
	 * a trailing character greater than the first alphanumeric character
	 * in the ASCII range ('A'). This will ignore words that have a trailing
	 * '\' or '~'
	 */
	/* this is used for binary search, so compares must be be transitive and
	 * symetric.  cmp(a,b) = - cmp(b,a) 
	 * So we cannot simply use isalnum(BFB[gp + TermLength]) to determin
	 * if we're at a word break. That has to be determined in the MultiTermSearch()
	 * function.
	 */
#if 0
	/* optimize by doing a subset of terms < 4 characters in length */
	if ((z == 0) && (TermLength < 4) && (BFB[gp + TermLength] >= 'A') )
	    z = -1;

#endif

    }
    
#if DEBUG_BINARY_SEARCH
    /*
     * For debugging search lookups/compares
     */

    cout << z << " [" << QueryTerm << "]  [";
    for(int i = 0; i < TermLength + 1; i++) {
	if (isalnum(BFB[gp+i]))
	    cout << BFB[gp+i];
	else
	    cout << " ";
    }
    cout << "]    gp=" << gp << "\n";
#endif

  return z;
}

INDEX::~INDEX() { 

    if (BFB)
	munmap((char *)BFB,BFBLength);

}


