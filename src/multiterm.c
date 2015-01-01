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

/* Do multi-term search. */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/resource.h>    

#include "defs.h"
#include "stringx.h"
#include "common.h"
#include "record.h"
#include "mdtrec.h"
#include "mdt.h"
#include "idbobj.h"
#include "iresult.h"
#include "irset.h"
#include "dtreg.h"
#include "index.h"
#include "idb.h"
#include "catids.h"


void INDEX::PrintKeywords(int mincount) {

    char b[512];
    int i, j, len, count, maxip, fd;
    GPTYPE gp, gpx;
    STRING TmpIndexFileName;
    long file_length;
    index_entry *Index_Table;     /* this is the pointer to the mmapped file */

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

    if (!BFB) {
	if (!OpenBFB()) {
	    cerr << "Can't open BFB\n";
	    return;
	}
    }

    maxip = (file_length / sizeof(index_entry)) - 1;
    count = 0;

    for (i = 0; i < maxip; i++) {

	if (count == 0) {
	    gpx = Index_Table[i].gpindex;
	    len = 0;
	    while (isalnum(BFB[gpx + len]) && 
		   (BFB[gpx + len] < 123) &&
		   (BFB[gpx + len] > 47)) { len++; }
	    count = 1;
	}
	else {
	    gp = Index_Table[i].gpindex;
	    
	    if ((strncasecmp((char *)BFB + gpx,(char *)BFB + gp ,len) == 0) &&
		(!isalnum(BFB[gp + len]))) {
		/* same word */
		count++;
	    }
	    else {
		/* new word */
		for( j = 0; (j < len) && (j < 64); j ++) {
		    b[j] = BFB[gpx +j];
		}
		b[j] = '\0';
		
		if (count >= mincount) {
		    printf("%64s   %7d\n",b,count);
		}
		count = 0;
	    }
	}
    }

    close(fd);
}


void INDEX::ResultRange(index_entry *Index_Table,
			int file_length,
			const STRING& Term, 
			int *result_first,
			int *result_last) 
{
  GPTYPE gp;
  int TermLength;
  int ip, maxip, low, high;
  int best_low, best_high;
  int x, z;
  int first, last;
  int match, nomatch;
  int done;
  GDT_BOOLEAN hit;

  *result_first = -1;
  *result_last  = -1;

  /* Set up the searching variables */
  maxip = (file_length / sizeof(index_entry)) - 1;
  low = 0;
  high = maxip;
  best_low = low;
  best_high = high;
  ip = high / 2;
  z = 0;
  TermLength = strlen((char *)Term.Buffer);

  /* Now, do the binary search on the pointers */
  done = 0;    
  hit = GDT_FALSE;
  
  while ( (!done) && (high >= low) ) {
      ip = (low + high) / 2;
      gp = Index_Table[ip].gpindex;

#if 0
      /* for debugging binary search of terms */
      fprintf(stderr,"%7d ",ip);
#endif

      z = Match(Term, TermLength, gp);

      if (z == 0) {
	done = 1;
	hit = GDT_TRUE;
      }
      else if (z < 0) {
	high = ip-1;
	best_high = ip;
      }
      else if (z > 0) {
	low = ip + 1;
	best_low = ip;
      }
    } 
    
    if (!hit) {
	/* no hits - return an empty range */
	return;
    }
    
    /* bracket hits */

    /*  find first matching string */
    low = best_low;
    high = ip;
    match = ip;
    nomatch = best_low;
    
    /*  iterate until we're within 5 records of the first element */
    while ( (match - nomatch) > 5 ) {
      first = (low + high) / 2;
      gp = Index_Table[first].gpindex;
      z = Match(Term, TermLength, gp);
      
      if (z == 0) {
	match = first;
	high = first;
      } else {
	nomatch = first;
	low = first + 1;
      }
    }
    
    first = match;
    
    z = 0; 
    while((z == 0) && (first > nomatch)) {
      first--;
      gp = Index_Table[first].gpindex;
      z = Match(Term, TermLength, gp);
    }
    if (z != 0) {
      first++;
    }
    
    /* find last matching string */
    low = ip;
    high = best_high;
    match = ip;
    nomatch = best_high;
    
    while ( (nomatch - match) > 5 ) {
      last = (high + low) / 2;
      gp = Index_Table[last].gpindex;
      z = Match(Term, TermLength, gp);
      if (z == 0) {
	match = last;
	low = last + 1;
      } else {
	nomatch = last;
	high = last;
      }
    } 
    
    last = match;
    
    z = 0;
    while ((z == 0) && (last < nomatch)) {
      last++;
      gp = Index_Table[last].gpindex;
      z = Match(Term, TermLength, gp);
    } 
    
    if (z != 0) {
      last--;
    }

#if 0
    fprintf(stderr, "##################################################\n");
    for(int i = first - 2; i < last + 2 ; i++) {
      gp = Index_Table[i].gpindex;
      z = Match(Term, TermLength, gp);
      fprintf(stderr, "<%4d>  %d \n", i, z);
    }
    fprintf(stderr, "##################################################\n");
#endif

#if 0
    fprintf(stderr,"Term = %12s   Result Set %8d\n",Term.Buffer,(last-first));
#endif

  *result_first = first;
  *result_last  = last;

}

PIRSET INDEX::MultiTermSearch(const STRING& QueryTerm,
			      int first, 
			      int last,
			      unsigned char field,
			      unsigned char document_fields,
			      PIRSET WorkingSet,
			      int AbsMax)
{
    static long file_length = 0;
    static index_entry *Index_Table = NULL;     /* this is the pointer to the mmapped file */
    static int fd = -1;
    static STRING TmpIndexFileName;
    
    int y;
    unsigned char f = 0;
    unsigned char OrigTerm[StringCompLength+1], *Term;
    int TermLength, OrigTermLength;
    PIRSET pirset = NULL;
    int ip, w;
    /* unsigned char Buffer[StringCompLength+1];   */
    
    if (first == -1) {
	/* no results */
	return NULL;
    }
    
    if ((Index_Table == NULL) || (TmpIndexFileName != IndexFileName)) {
	/* Open the index file */
	TmpIndexFileName = IndexFileName;
	fd = open(TmpIndexFileName,O_RDONLY);
	if (fd < 0) {
	    perror(TmpIndexFileName);
	    exit(13);
	}
	
	/* now mmap the file */
	file_length = lseek(fd,0L, SEEK_END);

	struct rlimit rp;
	/* increase the max mmap() limit to 12G */
	/* 12G is too big for 32bit pointers!  so no 12884901888UL; 
	 * the max amount that can be mmap()'d is 4G
	 */
	rp.rlim_cur = rp.rlim_max = 4294967295UL; 

#ifdef SUNOS
	/* Solaris */
        if (setrlimit(RLIMIT_VMEM, &rp) == -1) {
#else
	/* Linux */
        if (setrlimit(RLIMIT_AS, &rp) == -1) {
#endif
            perror("setrlimit VMEM failed, not enough memory for mmap()");
	    exit(15);
	}
        if (setrlimit(RLIMIT_DATA, &rp) == -1) {
            perror("setrlimit DATA failed, not enough memory for mmap()");
	    exit(16);
	}

	/*
	 * getrlimit( RLIMIT_VMEM, &rp );
	 * printf( "VMEM Current = %lu\nVMEM Max     = %lu\n", rp.rlim_cur, rp.rlim_max );
	 * getrlimit( RLIMIT_DATA, &rp );
	 * printf( "DATA Current = %lu\nDATA Max     = %lu\n", rp.rlim_cur, rp.rlim_max );
	 */

	Index_Table = (index_entry *) mmap(NULL, file_length ,PROT_READ, MAP_SHARED, fd, 0);

	if (Index_Table == MAP_FAILED) {
	    char msg[1024];
	    sprintf(msg,"mmap() failed for %s",TmpIndexFileName.Buffer);
	    perror(msg);
	    exit(14);
	}

    }
    
    QueryTerm.GetCString((char *)OrigTerm, sizeof(OrigTerm));
    OrigTermLength = strlen((char *)OrigTerm);
    
    if (OrigTermLength < QueryTerm.GetLength()) {
	OrigTerm[OrigTermLength - 1] = '*';
    }
    
    /*
     * cerr << "QueryTermLen = " << OrigTermLength << "\n";
     * cerr << "OrigTermlen  = " << strlen(OrigTerm) << "\n";
     */
    
    int PhraseEnd = OrigTermLength;
    if (OrigTerm[OrigTermLength - 1] == '*')
	PhraseEnd--;
  
    Term = OrigTerm; 
    TermLength = OrigTermLength;
    
    /* Build result set */
    IRESULT iresult(Parent->GetMainMdt()->MdtTable);
    MDTREC *mdtrec;
    GPTYPE GlobalRecEnd;
    int TermLenNoStar;
  
    if (QueryTerm.GetChr(OrigTermLength) == '*') {
	TermLenNoStar = OrigTermLength - 1; /* ignore "*" at end */
    } else {
	TermLenNoStar = OrigTermLength;
    }
    
    /* 
     * Loop through mdt records and add to the irset
     */ 
    
    /* resize ahead of time */
    pirset = new IRSET(Parent, (last - first) + 1, AbsMax);  
    MDT* mainMdt = Parent->GetMainMdt();
    int mdtEntries   = mainMdt->TotalEntries;
    MDTREC *MdtTable = mainMdt->MdtTable;
    unsigned short RestrictTopCat  = Parent->RestrictTopCat;
    unsigned short RestrictKids    = Parent->RestrictKids;
    int            RestrictAdult   = Parent->RestrictAdult;
    int            MaxRecordLength = Parent->MaxRecordLength;
    int            pathlen = 0;
    char *  RestrictPathname       = Parent->RestrictPathname;
#if 0
    char *  RestrictFilename       = Parent->RestrictFilename;
    long    RestrictFilenameHash   = Parent->RestrictFilenameHash;
#endif
    
    if (RestrictPathname) {
	pathlen = strlen(RestrictPathname);
    }

#if 0    
    cerr << "restictPathname = " << RestrictPathname << "\n";
    cerr << "pathlen         = " << pathlen << "\n";
    cerr << "restictTopcat   = " << RestrictTopCat << "\n";
    cerr << "restictAdult    = " << RestrictAdult << "\n";
    cerr << "restictKids     = " << RestrictKids << "\n";
#endif

    for (ip=first; ip <= last; ip++) {

	/* only check last character if its not a wildcard search */
	if (TermLenNoStar == OrigTermLength) {
	    /* make sure the last character is a word seperator */
	    /* GetIndirectBuffer(Index_Table[ip].gpindex, Buffer, 0, StringCompLength);  */
	    
	    GPTYPE bfbindex  = Index_Table[ip].gpindex + TermLenNoStar;

#if DEBUG_TRAILING_CHARS
if (ip == first) {
    cout << "first: [";
    for(int k=0; k < TermLenNoStar + 2 ; k++) {
	if (isalnum(BFB[Index_Table[ip].gpindex +k]))
	    cout << BFB[Index_Table[ip].gpindex +k];
	else
	    cout << " ";
    }
    cout << "]\n";
}
if (ip == last) {
    cout << "last:  [";
    for(int k=0; k < TermLenNoStar +2 ; k++) {
	if (isalnum(BFB[Index_Table[ip].gpindex +k]))
	    cout << BFB[Index_Table[ip].gpindex +k];
	else
	    cout << " ";
    }
    cout << "]\n";
}
#endif
	    if (bfbindex < BFBLength) {
	      if (IsAlnum(BFB[bfbindex])) {
		  /* bfbindex points to a letter IsALnum() charcter */

		  if (bfbindex+1 >= BFBLength)
		      continue;

		  if ((BFB[bfbindex] == 's') && (BFB[bfbindex + 1] == ' ')) {
#if DEBUG_TRAILING_CHARS
 cerr << "trailing s: [";
 for(int k=0; k < TermLenNoStar + 2; k++) {
     cerr << BFB[Index_Table[ip].gpindex +k];
 }
 cerr << "]\n";
#endif
		  }
		  else {
		      /* the trailing character is not a space, or word seperating character */
		      continue;
		  }
	      }
	    }
	    else {
	      cerr << "gpindex out of range for ip = " << ip << "  gp = " << Index_Table[ip].gpindex << "\n";
	      cerr << "BFBLength = " << BFBLength << "\n";
	    }
	}
	
	/* fetch the mdt information for this record */
	w = Index_Table[ip].mdtindex & FILEID_MASK;
	f = (Index_Table[ip].mdtindex & FIELD_MASK) >> 24;

	/* if the field is set, only include records that match the field value */
	if (field && f && (f != field)) {
	    continue;
	}

	/* access the mmap()'d data directly avoiding a memcpy() */
	if ((w > 0) && (w <= mdtEntries)){
	    mdtrec = MdtTable + (w - 1);
	}
	else {
	    cerr << "mdtrec index out of range index = " << w << "  ip = " << ip << "\n";
	    continue;
	}

	if (RestrictTopCat && ((mdtrec->Fields & TOP_CAT_MASK) != RestrictTopCat)) {
	    continue;
	}

	if (RestrictAdult && ((mdtrec->Fields & TOP_CAT_MASK) == TOP_CAT_Adult)) {
	    continue;
	}

	if (RestrictKids && !(mdtrec->Fields & RestrictKids & KID_MASK)) {
	    continue;
	}

	if ((MaxRecordLength > 0) && 
	    (mdtrec->FileLength > MaxRecordLength)) { continue; }

	if (RestrictPathname) {
	    char *pathname = Parent->ht.GetPathName(mdtrec->pathName);

	    if(pathname && strncmp(RestrictPathname,pathname,pathlen) != 0) {
		free(pathname);
		continue;
	    }
	    free(pathname);
        }

#if 0
        /* Exact filename match */
	if (RestrictFilename) {
	  if (RestrictFilenameHash) {
	    if (RestrictFilenameHash != mdtrec->fileName) {
	      continue;
	    }
	  }
	  else {
	    char *filename = Parent->ht.GetFileName(mdtrec->fileName);

	    if (strcmp(RestrictFilename,filename) == 0) {
	      RestrictFilenameHash = mdtrec->fileName;
	      Parent->RestrictFilenameHash = mdtrec->fileName;
	    }
	    else {
	      continue;
	    }
	  }
	}
#endif

#if 0
	/* XXX - currently we don't delete records so why check? */

	/* Skip deleted records */
	if ((mdtrec->Fields & TOP_CAT_MASK) == TOP_CAT_Deleted) {
	    continue;
	}

#endif
	
	/* Check working set to see if we should add this record
	 * we only add new instances if the record (site) is already represented,
	 * this makes merging faster by reducing the size of the sets, and 
	 * avoiding overflow of the IRSETS for comon keywords.
	 */
	if (WorkingSet != NULL) {
	    y = WorkingSet->HashGet(w);
	    
	    if (y < 0) {
		/* this element isn't in the working set, so skip it */
		continue;
	    }
	}
	
	iresult.MdtIndex        = w;
	iresult.HitCount        = 1;
	iresult.Score           = 0;
	iresult.Field           = f;
	
	pirset->AddEntry(iresult, 1);
    }
    
    return pirset;
}


