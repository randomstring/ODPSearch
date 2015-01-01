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

/* Generic doctype class */

/* Note: Fields are not used in this generic class, the whole file is
 * parsed as a single document without any seperate fields. All field
 * ids are set to zero (0).
 */

#include <string.h>
#include <ctype.h>
#include "doctype.h"

DOCTYPE::DOCTYPE(IDBOBJ* DbParent) 
{
    Db = DbParent;
    SetNumBuckets(0);
}


void
DOCTYPE::SetNumBuckets(int num) 
{
    NumBuckets = num;
    if ((NumBuckets == 2) ||
	(NumBuckets == 4) ||
	(NumBuckets == 15) ||
	(NumBuckets == 28))
	// XXX only works if numbuckets is 2, 4, 15, or 28
	BucketSize = (26 / (NumBuckets - 2));
    else if (NumBuckets == 38) {
      BucketSize = 1;
    }
    else {
      if (NumBuckets != 0) {
	cerr << "ERROR: bad number of buckets[" 
	     << BucketSize
	     << "]. Must be 0, 2, 4, 15, 28, or 38.\n";
      }
      BucketSize = 0;
    }
}


int
DOCTYPE::GetBucket(unsigned char c) 
{
    unsigned char lc;
    int bucket;

    lc = tolower(c);

    if (NumBuckets < 2) 
       return 0;

    if (NumBuckets == 2) {
	if (lc < 'm')
	    return 0;
	else
	    return 1;
    }

    if (lc < '0') {
	return 0;
    }

    if (lc <= '9') {
      return (lc - '0' + 1);
    }

    if (lc < 'a') {
	return 10;
    }

    if (lc > 'z')
	return NumBuckets - 1;

    // rest are in the range a-z inclusive
    bucket = ((lc - 'a') / BucketSize) + 11;

    if ((bucket < 1) || (NumBuckets - 2 < bucket )) {
	fprintf(stderr,"ERROR: bucket out of range [%c] bucket = %d size = %d\n",
	       lc, bucket, BucketSize);
	bucket = NumBuckets - 2; 
    }
	
    return bucket;
}


GPTYPE 
DOCTYPE::ParseWords(unsigned char* DataBuffer, // Pointer to document text buffer.
		    int DataLength,            // Length of document text buffer in # of characters.
		    int DataOffset,            // Offset that must be added to all GP positions 
		                               // because GP space is shared with other documents. 
		    unsigned int FileId,       // file identification number
		    int *fdList)               // list of open files to write data to
{     

    index_entry ie;
    int fn;
    int numAdded = 0;
    int Position = 0;
    unsigned char utf8_char[7];
    int i;
    int break_after_this_utf_char;
    
#if 0
    printf("%s",DataBuffer);
    printf("PW word location = ");
#endif
    
    while (Position < DataLength) {
	// scan to the begining of the word
	
	break_after_this_utf_char = 0;
	
	while ((Position < DataLength) &&
	       (!IsAlnum(DataBuffer[Position])) &&
	       (!IsUTF8_Start_Char(DataBuffer[Position]))) {
	    Position++;
	}
	
	// record the index of the start of the word
	if (Position < DataLength) {
	    
	    fn = GetBucket(DataBuffer[Position]);

	    if ( FileId > FILEID_MASK ) {
	        fprintf(stderr,"ERROR: FileId %d has exeeded the maximum allowable value %d.\n",FileId, FILEID_MASK);
	    }

	    ie.gpindex = DataOffset + Position;
	    ie.mdtindex = FileId;

	    if ( sizeof(index_entry) != write(fdList[fn], (void *)&ie, sizeof(index_entry))) {
		fprintf(stderr,"ERROR: can't write to fd %d\n");
		exit(88);
	    }
	    numAdded++;

#if 0
	    printf("%2d  %c%c%c%c\n",fn,DataBuffer[Position],
		   DataBuffer[Position+1],
		   DataBuffer[Position+2],
		   DataBuffer[Position+3]
		   );
#endif
			   
#if 0
	    printf("%d ",DataOffset + Position);
#endif
			   
	    // Skip over UTF-8 continuation characters
	    if (IsUTF8_Start_Char(DataBuffer[Position])) {
		
		i = 0;
		utf8_char[i++] = DataBuffer[Position];
		
		Position++;
		
		while ((Position < DataLength) && 
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
	
	// Scan for to the end of the word
	while ((Position < DataLength) &&
	       IsAlnum(DataBuffer[Position]) &&
	       !break_after_this_utf_char) {
	    Position++;
	}
    }

#if 0
    printf("\n");
#endif

    // Return # of GP's added to GpBuffer
    return numAdded;
}     

void
DOCTYPE::ScoreHits(IRESULT *results, 
		   float TermWeight, 
		   int ResultSetSize, 
		   int AbsoluteMaxEntries, 
		   float *MaxScore, 
		   float *MinScore) {

    float ResultSetWeight;
    float SumSqScores = 0;
    float SqrtSum;
    float Score;
    unsigned char field;
    int    x;
    

    //
    // Set the Term's weight of the term inversely proportional to the 
    // number of hits
    //
    // Weight = [1/AbsoluteMaxEntries, 1 ]  = (0,1]
    if (ResultSetSize < 0) 
	return;
    if (ResultSetSize > AbsoluteMaxEntries) 
	ResultSetSize = AbsoluteMaxEntries;

    ResultSetWeight = ( 1.0 + (AbsoluteMaxEntries - ResultSetSize)) / (float)AbsoluteMaxEntries;

    TermWeight *= ResultSetWeight;
    
    for (x=0; x < ResultSetSize; x++) {
	// IncDocFreq measures the inverse frequency of the term in the entire DB

	float Weight = 1.0;

	if (results[x].HitCount < 1)
	    results[x].HitCount = 1;

	// adjust score for the number of hits in this record, range [1.0, 2.0)
	// this defeats spamming a word by putting 100 occurances in a single document.
	Weight *= ((float)2.0 - ((float)1.0 / ((float)results[x].HitCount)));

	// adjust hit for term weighting
	results[x].Score = TermWeight * Weight;

	if(results[x].Score > *MaxScore)
	    *MaxScore= results[x].Score;
	if(results[x].Score < *MinScore)
	    *MinScore= results[x].Score;

    }
}

float
DOCTYPE::ScoreHit(IRESULT result, float weight) {

    // To be implemented, moving the hit scoring from index.html into 
    // the document type where it belongs...

    //cerr << "DEFAULT Score = " << weight<< "\n";

    return(1.0 * weight);
}  

unsigned int 
DOCTYPE::TopLevelCat(char *q) {
  
  /* by default use the first three characters of the category as
   * a hash
   */
  if (q)
    return((q[0] << 16) + (q[1] << 8) + q[2]);
  else
    return(0);
}

unsigned char
DOCTYPE::NewField(unsigned char current, unsigned char n) {

  return current;

}

void 
DOCTYPE::GetMdtData(unsigned char* DataBuffer, 
		    INT4 DataLength, 
		    GPTYPE DataOffset,
		    unsigned int pathName,
		    unsigned int fileName,
		    char *path,
		    char *filename,
		    MDTREC *mdtrec) {

    return;

}


DOCTYPE::~DOCTYPE() {
}
