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

/* A simple parser that will find the start of every word. */

#include "simple.h"
#include "defs.h"

SIMPLE::SIMPLE(PIDBOBJ DbParent) : DOCTYPE(DbParent) 
{
    SetNumBuckets(38);
}


float 
SIMPLE::ScoreHit(IRESULT result, float termweight) {
  
    float Weight = 1.0;
    int Depth;

    if (result.HitCount < 1)
        result.HitCount = 1;

    // adjust score for the number of hits in this record, range [1.0, 2.0)
    Weight *= ((float)2.0 - ((float)1.0 / ((float)result.HitCount)));

    return(termweight * Weight);
}


/*
 * TopLevelCat() - given a category name, determin the numerical
 *                 category value. This makes category comparisons
 *                 much faster
 */

unsigned int
SIMPLE::TopLevelCat(char *q) {
    
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
SIMPLE::GetMdtData(unsigned char* DataBuffer,
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

    if (TopCat == TOP_CAT_Kids)
	Kid = KID_MASK;

    /* no kid bits set in the adult tree */ 
    if (TopCat == TOP_CAT_Adult)
	Kid = 0;

    mdtrec->Fields =  Cool | CatDepth | Kid | TopCat;

    prevPath     = pathName;
    prevCatDepth = CatDepth;
    prevTopCat   = TopCat;

  return;

}

SIMPLE::~SIMPLE() 
{
}




