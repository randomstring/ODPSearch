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
 * Copyright (c) 1992 The Regents of the University of California.
 * All rights reserved.
 *
 * Author:	Ray Larson, ray@sherlock.berkeley.edu
 *		School of Library and Information Studies, UC Berkeley
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND THE AUTHOR ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _marc_
#define _marc_

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************/
/* special char definitions  for MARC records                        */
/*********************************************************************/
#define SUBFDELIM '\037'
#define FIELDTERM '\036'
#define RECTERM   '\035'

/*********************************************************************/
/* Structures for internal processing format of MARC records         */
/*********************************************************************/
typedef struct marc_leader_over /* overlay for marc leader */
 { char LRECL[5];    /* Logical Record Length            */
   char RecStatus;   /* Record Status                    */
   char RecType;     /* Legend - type of record          */
   char BibLevel;    /* Legend - Bibliographic Level     */
   char blanks1[2];
   char IndCount;    /* Indicator count                  */
   char SubFCount;   /* Subfield code count              */
   char BaseAddr[5]; /* Base Address of data             */
   char EncLevel;    /* encoding level                   */
   char DesCatForm;  /* Descriptive Cataloging Form      */
   char blank2;      
   /* Entry Map - description of directory fields */
   char LenLenF;     /* length of length of field        */
   char LenStartF;   /* length of Starting char position */
   char UnDef[2];    /* undefined chars                  */
  } MARC_LEADER_OVER;

typedef struct marc_direntry_over  /* overlay for directory entries */
  { char tag[3];     /* field tag           */
    char flen[4];    /* field length        */
    char fstart[5];  /* field start position*/
  } MARC_DIRENTRY_OVER;

typedef struct marc_subfield  /* processing structure for subfields */
  { char *data;                  /* pointer to data */
    struct marc_subfield  *next; /* pointer to next subfield */
    char code;                   /* subfield code char */
  } MARC_SUBFIELD;
   
typedef struct marc_field  /* field linked list node */
  { char *data;                /* pointer to start of field data */
    MARC_SUBFIELD *subfield;   /* head of linked list of subfields */
    MARC_SUBFIELD *lastsub;    /* tail of linked list of subfields */
    struct marc_field *next;   /* next field in list */
    char tag[4];               /* 3 char tag + null */
    char indicator1;           /* first indicator   */
    char indicator2;           /* second indicator  */
    char subfcodes[21];        /* string of subfield codes            */
                               /* element [0] is a count of subfields */
    int  length; /* full field length */
  } MARC_FIELD;
    
typedef struct marc_rec  /* marc record processing format */
  { INT4 length;
    char *record,
         *BaseAddr;
    MARC_LEADER_OVER *leader;
    char *fixed_fields;
    MARC_FIELD *fields;    /* head of linked list */
    MARC_FIELD *lastfield; /* tail of linked list */
    int nfields;
   } MARC_REC;

#ifdef __cplusplus
}
#endif

#endif

