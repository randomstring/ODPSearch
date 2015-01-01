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
#include "colondoc.h"


COLONDOC::COLONDOC (PIDBOBJ DbParent): DOCTYPE (DbParent) {
  SetNumBuckets(38); 
}


GPTYPE 
COLONDOC::ParseWords(unsigned char* DataBuffer, // Pointer to document text buffer.
		    int DataLength,            // Length of document text buffer in # of characters.
		    int DataOffset,            // Offset that must be added to all GP positions 
		                               // because GP space is shared with other documents. 
		    unsigned int FileId,       // file identification number
		    int *fdList)               // list of open files to write data to
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
    
    // unsigned char tag;
    // tag = DataBuffer[Position];

    // get the colon delimited field (an ascii character)
    if (Position < DataLength)
        field = DataBuffer[Position];

    // get past the colon tag 
    while ((Position < DataLength) && (DataBuffer[Position] != ':'))
	Position++;
    
    if (Position < DataLength)
	Position++;
    
    while (Position < DataLength) {
	// scan to the begining of the word
	
	break_after_this_utf_char = 0;
	
	while ((Position < DataLength) &&
	       (!IsAlnum(DataBuffer[Position])) &&
	       (!IsUTF8_Start_Char(DataBuffer[Position]))) {
	    
	    if (DataBuffer[Position] == '\n') {
		linenum++;

		// get the colon delimited field (an ascii character)
		while ((Position < DataLength) && (DataBuffer[Position] == '\n'))
		  Position++;

		if (Position < DataLength)
		  field = DataBuffer[Position];

		// get past the tag 
		while ((Position < DataLength) && (DataBuffer[Position] != ':'))
		    Position++;
	    }
	    
	    Position++;
	}
	
	// remember this offset as the beginning of a word
	if (Position < DataLength) {

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
	
	    // Skip over UTF-8 continuation characters
	    if (IsUTF8_Start_Char(DataBuffer[Position])) {
	      
		i = 0;
		utf8_char[i++] = DataBuffer[Position];
		
		Position++;
		
		while ((Position < DataLength) &&  (i < 6) && 
		       IsUTF8_Continuation_Char(DataBuffer[Position])) {
		    // fprintf(stderr,"%c", DataBuffer[Position]);
		    
		    utf8_char[i++] = DataBuffer[Position];
		    
		    Position++;
		}
		
		utf8_char[i] = 0;
		
		// if this is a japanese/chinese/korean character then break after 
		// this UTF-8 char
		if (IsUTF8_CJK_Char(utf8_char)) {
		    break_after_this_utf_char = 1;
		}
	    }
	    else 
		Position++;
	}
	
	// scan to the end of the word, delimited by a non-alphacharacter 
	while ((Position < DataLength) && 
	       IsAlnum(DataBuffer[Position]) && 
	       !break_after_this_utf_char) {
	    Position++;
	}
	
    }
    return numAdded;
} 

float 
COLONDOC::ScoreHit(IRESULT result, float termweight) {

    float Weight = 1.0;
    float LengthWeight = 1.0;

    if (result.HitCount < 1)
        result.HitCount = 1;

    // adjust score for the number of hits in this record, range [1.0, 2.0)
    Weight *= ((float)2.0 - ((float)1.0 / ((float)result.HitCount)));

    // adjust hit for term weighting
    return(termweight * Weight * LengthWeight);
}  

void COLONDOC::ScoreHits(IRESULT *results, 
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

unsigned int 
COLONDOC::TopLevelCat(char *q) {
  
  /* by default use the first three characters of the category as
   * a hash
   */
  if (q)
      return((q[0] << 16) + (q[1] << 8) + q[2]);
  else
      return(TOP_CAT_Other);
}



void 
COLONDOC::GetMdtData(unsigned char* DataBuffer, 
		     INT4 DataLength, 
		     GPTYPE DataOffset,
		     long pathName,
		     long fileName,
		     char *path,
		     char *filename,
		     MDTREC *mdtrec) {
  
  static long prevPath = -1;
  static unsigned int prevCatDepth = 0;
  static unsigned short prevTopCat = 0;

  unsigned short CatDepth = 0;
  unsigned short TopCat = 0;

  if (pathName == prevPath) {
      CatDepth = prevCatDepth;
      TopCat   = prevTopCat;
  }
  else {
	
      char *r,*q;

      // calculate the category depth and 
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
  

  mdtrec->Fields = CatDepth | TopCat;

  prevPath = pathName;
  prevCatDepth = CatDepth;
  prevTopCat   = TopCat;

  return;

}


COLONDOC::~COLONDOC () {}

