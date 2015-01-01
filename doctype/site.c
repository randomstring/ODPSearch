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

/* Parses fields based on colon delimeted lines. <TAG>:<TEXT> */

#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "site.h"


SITEDOC::SITEDOC (PIDBOBJ DbParent): DOCTYPE (DbParent) {
    SetNumBuckets(38);
}


GPTYPE 
SITEDOC::ParseWords(unsigned char* DataBuffer, /* Pointer to document text buffer. */
		    int DataLength,            /* Length of document text buffer in # of characters.*/
		    int DataOffset,            /* Offset that must be added to all GP positions */
		                               /* because GP space is shared with other documents. */
		    unsigned int FileId,       /* file identification number */
		    int *fdList)               /* list of open files to write data to */
{     

    unsigned int field = 0;
    unsigned int f = 0;
    unsigned int fid = 0;
    index_entry ie;
    int fn;
    int numAdded = 0;
    int scanning_for_tag = 0;
    int Position = 0;
    int linenum = 0;
    unsigned char utf8_char[8];
    int i;
    int break_after_this_utf_char;
    
    /* get the colon delimited field (an ascii character) */
    if (Position < DataLength)
        field = DataBuffer[Position];

    /* get past the colon tag */
    while ((Position < DataLength) && (DataBuffer[Position] != ':'))
	Position++;

    if ((Position < DataLength) && (DataBuffer[Position] != '\n'))
	Position++;
    
    while (Position < DataLength) {
	/* scan to the begining of the word */
	
	break_after_this_utf_char = 0;
	
	while ((Position < DataLength) &&
	       (!IsAlnum(DataBuffer[Position])) &&
	       (!IsUTF8_Start_Char(DataBuffer[Position]))) {
	    
	    if (DataBuffer[Position] == '\n') {
		linenum++;

		/* get the colon delimited field (an ascii character) */
		while ((Position < DataLength) && (DataBuffer[Position] == '\n'))
		  Position++;

		if (Position < DataLength)
		  field = DataBuffer[Position];

		/* get past the tag */
		while ((Position < DataLength) && (DataBuffer[Position] != ':'))
		    Position++;
	    }
	    
	    if (DataBuffer[Position] != '\n') 
	      Position++;
	}
	
	/* remember this offset as the beginning of a word */
	if (Position < DataLength) {

	    if ((field == 'u') &&
		((strncasecmp("html\n",(char *)DataBuffer + Position,5) == 0) ||
		 (strncasecmp("htm\n", (char *)DataBuffer + Position,4) == 0) ||
		 (strncasecmp("http:", (char *)DataBuffer + Position,5) == 0) ||
		 (strncasecmp("ftp:",  (char *)DataBuffer + Position,4) == 0) ||
		 (strncasecmp("asp\n", (char *)DataBuffer + Position,4) == 0) ||
		 (strncasecmp("cgi\n", (char *)DataBuffer + Position,4) == 0) ||
		 (strncasecmp("www.",  (char *)DataBuffer + Position,4) == 0) ||
		 (strncasecmp(".com/", (char *)DataBuffer + Position - 1,5) == 0) ||
		 (strncasecmp(".com\n",(char *)DataBuffer + Position - 1,5) == 0) ||
		 (strncasecmp(".net/", (char *)DataBuffer + Position - 1,5) == 0) ||
		 (strncasecmp(".net\n",(char *)DataBuffer + Position - 1,5) == 0) ||
		 (strncasecmp(".org/", (char *)DataBuffer + Position - 1,5) == 0) ||
		 (strncasecmp(".org\n",(char *)DataBuffer + Position - 1,5) == 0) ||
		 (strncasecmp(".edu/", (char *)DataBuffer + Position - 1,5) == 0) ||
		 (strncasecmp(".edu\n",(char *)DataBuffer + Position - 1,5) == 0) ||
		 (strncasecmp(".uk/",  (char *)DataBuffer + Position - 1,4) == 0) ||
		 (strncasecmp(".uk\n", (char *)DataBuffer + Position - 1,4) == 0) ||
		 (strncasecmp(".to/",  (char *)DataBuffer + Position - 1,4) == 0) ||
		 (strncasecmp(".to\n", (char *)DataBuffer + Position - 1,4) == 0) ||
		 (strncasecmp(".tv/",  (char *)DataBuffer + Position - 1,4) == 0) ||
		 (strncasecmp(".tv\n", (char *)DataBuffer + Position - 1,4) == 0) ||
		 (strncasecmp(".de/",  (char *)DataBuffer + Position - 1,4) == 0) ||
		 (strncasecmp(".de\n", (char *)DataBuffer + Position - 1,4) == 0))) {
		/* skip it */

#if 0
		cerr << "Skipping: ";
		for (int ii = 0; ii < 5; ii++) {
		    cerr << DataBuffer[Position + ii];
		}
		cerr << "\n";
#endif
	    }
	    else {

		fn = GetBucket(DataBuffer[Position]);
	  
		f = FIELD_MASK & (field << 24);
		if ( FileId > FILEID_MASK ) {
		    fprintf(stderr,"ERROR: FileId %d has exeeded the maximum allowable value %d.\n",FileId, FILEID_MASK);
		    fid = FileId;
		}
		else {
		    fid = FileId | f;
		}
		
		ie.gpindex = DataOffset + Position;
		ie.mdtindex = fid;
		
		if ( sizeof(index_entry) != write(fdList[fn], (void *)&ie, sizeof(index_entry))) {
		    fprintf(stderr,"ERROR: can't write to fd %d\n");
		    exit(88);
		}
		numAdded++;
		
		/* Skip over UTF-8 continuation characters */
		if (IsUTF8_Start_Char(DataBuffer[Position])) {
		    
		    i = 0;
		    utf8_char[i++] = DataBuffer[Position];
		    
		    Position++;
		    
		    while ((Position < DataLength) &&  (i < 6) && 
			   IsUTF8_Continuation_Char(DataBuffer[Position])) {
			/* fprintf(stderr,"%c", DataBuffer[Position]); */
			
			utf8_char[i++] = DataBuffer[Position];
			
			Position++;
		    }
		    
		    utf8_char[i] = 0;
		    
		    /* if this is a japanese/chinese/korean character then break after 
		     * this UTF-8 char
		     */
		    if (IsUTF8_CJK_Char(utf8_char)) {
			break_after_this_utf_char = 1;
		    }
		}
		else 
		    Position++;
	    }
	}
	    
	/* scan to the end of the word, delimited by a non-alphacharacter */
	while ((Position < DataLength) && 
	       IsAlnum(DataBuffer[Position]) && 
	       !break_after_this_utf_char) {
	    Position++;
	}
	
    }
    return numAdded;
} 

float 
SITEDOC::ScoreHit(IRESULT result, float termweight) {

    float Weight = 1.0;
    float RecordLengthWeight; 
    int RecordLength;

    /* alter base Weight based on field
     * demote URl "u:" (
     */
    if (result.Field == 'u') {
        Weight *= 0.5;
    }
	
    if (result.HitCount < 1)
        result.HitCount = 1;

    /* adjust score for the number of hits in this record, range [1.0, 2.0)*/
    Weight *= ((float)2.0 - ((float)1.0 / ((float)result.HitCount)));

    /* cerr << "Site Score = " << termweight * Weight << "\n"; */

    /* adjust hit for term weighting */
    return(termweight * Weight);
}  


void
SITEDOC::ScoreHits(IRESULT *results, 
		   float TermWeight, 
		   int ResultSetSize, 
		   int AbsoluteMaxEntries, 
		   float *MaxScore, 
		   float *MinScore) {

    float ResultSetWeight;
    float Score;
    unsigned char field;
    int    x;
    
    /*
     * Set the Term's weight of the term inversely proportional to the 
     * number of hits
     *
     * Weight = [1/AbsoluteMaxEntries, 1 ]  = (0,1]
     */
    if (ResultSetSize < 0) 
	return;
    if (ResultSetSize > AbsoluteMaxEntries) 
	ResultSetSize = AbsoluteMaxEntries;

    ResultSetWeight = ( 1.0 + (AbsoluteMaxEntries - ResultSetSize)) / (float)AbsoluteMaxEntries;

    TermWeight *= ResultSetWeight;
    
    for (x=0; x < ResultSetSize; x++) {
	/* IncDocFreq measures the inverse frequency of the term in the entire DB */

	float Weight = 1.0;
	field = results[x].Field;

	switch(field) {
	case 'u':
	    Weight *= 0.5;
	    break;
	}

	if (results[x].HitCount < 1)
	    results[x].HitCount = 1;

	/* adjust hit for term weighting */
	results[x].Score = TermWeight * Weight;

	if(results[x].Score > *MaxScore)
	    *MaxScore= results[x].Score;
	if(results[x].Score < *MinScore)
	    *MinScore= results[x].Score;

    }

}

/*
 * TopLevelCat() - given a category name, determin the numerical
 *                 category value. This makes category comparisons
 *                 much faster
 */

unsigned int
SITEDOC::TopLevelCat(char *q) {

   unsigned short Cat = TOP_CAT_Other;
    
   if (!q) {
       Cat =  TOP_CAT_Other;
   }  
   else {
       switch (q[0]) {
	   
       case 'A' :
	   if (q[1] == 'r')
	       Cat = TOP_CAT_Arts;
	   else if (q[1] == 'd')
	       Cat = TOP_CAT_Adult;
	   else 
	       Cat = TOP_CAT_Other;
	   break;
       case 'B' :
	   if (q[1] == 'o')
	       Cat = TOP_CAT_Bookmarks;
	   else if (q[1] == 'u')
	       Cat = TOP_CAT_Business;
	   else 
	       Cat = TOP_CAT_Other;
	   break;
       case 'C' :
	   Cat = TOP_CAT_Computers;  
	   break;
       case 'G' :
	   Cat = TOP_CAT_Games;
	   break;
       case 'H' :
	   if (q[1] == 'e')
	       Cat = TOP_CAT_Health;
	   else if (q[1] == 'o')
	       Cat = TOP_CAT_Home;
	   else 
	       Cat = TOP_CAT_Other;  
	   break;
       case 'K' :
	   Cat = TOP_CAT_Kids;
	   break;
       case 'N' :
	   if (q[2] == 't')
	       Cat = TOP_CAT_Netscape;
	   else if (q[2] == 'w')
	       Cat = TOP_CAT_News;
	   else
	       Cat = TOP_CAT_Other;  
	   break;
       case 'R' :
	   if (q[2] == 'g')
	       Cat = TOP_CAT_Regional;
	   else if (q[2] == 'f')
	       Cat = TOP_CAT_Reference;
	   else if (q[2] == 'c')
	       Cat = TOP_CAT_Recreation;
	   else if (q[2] == 's')
	       Cat = TOP_CAT_Restaurant;
	   else 
	       Cat = TOP_CAT_Other;  
	   break;
       case 'S' :
	   if (q[1] == 'p')
	       Cat = TOP_CAT_Sports;
	   else if (q[1] == 'h')
	       Cat = TOP_CAT_Shopping;
	   else if (q[1] == 'o')
	       Cat = TOP_CAT_Society;
	   else if (q[1] == 'c')
	       Cat = TOP_CAT_Science;
	   else 
	       Cat = TOP_CAT_Other;
	   break;
       case 'T' :
	   Cat = TOP_CAT_Test;
	   break;
       case 'W' :
	   Cat = TOP_CAT_World;
	   break;
       default :
	   Cat = TOP_CAT_Other;
	   break;
    }
  }

  return Cat;

}


void
SITEDOC::GetMdtData(unsigned char* DataBuffer,
                    INT4 DataLength,
                    GPTYPE DataOffset,
                    unsigned int pathName,
                    unsigned int fileName,
                    char *path,
                    char *filename,
                    MDTREC *mdtrec) {

   
    static long prevPath = -1;
    static unsigned short prevCatDepth = 0;
    static unsigned short prevTopCat = 0;
    static unsigned short prevKid = 0;
    
    unsigned short TopCat, Kid, CatDepth, Cool;

    CatDepth = 0;
    TopCat = 0;
    Cool = 0;
    Kid = 0;
    
    if (pathName == prevPath) {
	CatDepth = prevCatDepth;
	TopCat   = prevTopCat;
    }
    else {
	
	char *r,*q;

	/* calculate the category depth and */
	r = path;
	CatDepth = 1;
	q = NULL;
	
	q = r;
	while(*r) {
	    if (*r++ == '/') {
		CatDepth++;
	    }
	}
	
	if (CatDepth <= 0)
	    CatDepth = 1;

	if (CatDepth > (CAT_DEPTH_MASK >> CAT_DEPTH_SHIFT))
	    CatDepth = (CAT_DEPTH_MASK >> CAT_DEPTH_SHIFT);

	CatDepth = (CatDepth << CAT_DEPTH_SHIFT) & CAT_DEPTH_MASK;

	TopCat = TopLevelCat(path); 
    }

    if (filename && filename[0] == 'c') 
	Cool = COOL_MASK;

    ParseSiteItem(DataBuffer, DataLength, DataOffset, &Kid);

    /* no kid bits set in the adult tree */ 
    if (TopCat == TOP_CAT_Adult)
	Kid = 0;

    mdtrec->Fields =  Cool | CatDepth | Kid | TopCat;
    
    prevPath     = pathName;
    prevCatDepth = CatDepth;
    prevTopCat   = TopCat;

  return;

}

void
SITEDOC::ParseSiteItem(unsigned char *DataBuffer,
		       INT4 DataLength,
		       GPTYPE DataOffset,
		       unsigned short *Kid) {
    
    int i, j;
    char *buffer = (char *) (DataBuffer + DataOffset);
    
    *Kid = 0;

    for(j=0; j < DataLength ; j++)
        if ((j == 0) || (buffer[j-1] == '\n')) {
            switch (buffer[j]) {
            case 't':  /* title  */
		break;
            case 'd':  /* description */
		break;
            case 'u':  /* URL */
		break;
	    case 'a':  /* Kid & Teen site */
		*Kid = atoi(buffer + j + 2);
		*Kid = (*Kid << KID_SHIFT) & KID_MASK;
		/* cerr << "KID: " << (int) *Kid << " from " << (buffer + j + 2); */
		break;
            default:
		/* cerr << "Unexpected field: " << full_str[j] << "\n"; */
		break;
            }
        }
 
    return;
 
}

SITEDOC::~SITEDOC () {}

