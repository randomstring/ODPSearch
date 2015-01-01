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

/* General definitions */

#ifndef DEFS_H
#define DEFS_H

#include <unistd.h>
#include "gdt.h"

/* Masks for .inx file (mdt_index | field) */
#define   FIELD_MASK      (0xFF000000)
#define   FILEID_MASK     (0x00FFFFFF)

/* Masks for misc. record data fields */
#define   TOP_CAT_MASK         (0x001F)
#define   TOP_CAT_SHIFT        0

/* Open Directory Category names */
#define   TOP_CAT_Deleted      (0x0000)
#define   TOP_CAT_Adult        (0x0001)
#define   TOP_CAT_Arts         (0x0002)
#define   TOP_CAT_Business     (0x0003)
#define   TOP_CAT_Computers    (0x0004)
#define   TOP_CAT_Games        (0x0005)
#define   TOP_CAT_Health       (0x0006)
#define   TOP_CAT_Home         (0x0007)
#define   TOP_CAT_News         (0x0008)
#define   TOP_CAT_Recreation   (0x0009)
#define   TOP_CAT_Reference    (0x000A)
#define   TOP_CAT_Regional     (0x000B)
#define   TOP_CAT_Science      (0x000C)
#define   TOP_CAT_Shopping     (0x000D)
#define   TOP_CAT_Society      (0x000E)
#define   TOP_CAT_Sports       (0x000F)
#define   TOP_CAT_World        (0x0010)
#define   TOP_CAT_Kids         (0x0011)
#define   TOP_CAT_Bookmarks    (0x0012)
#define   TOP_CAT_Test         (0x0013)
#define   TOP_CAT_Restaurant   (0x0014)
#define   TOP_CAT_Other        (0x0015)
#define   TOP_CAT_None         (0x0015)
#define   TOP_CAT_Private      (0x0016)
#define   TOP_CAT_Netscape     (0x0017) 

#define   TOP_CAT_Unused1      (0x0018) 
#define   TOP_CAT_Unused2      (0x0019) 
#define   TOP_CAT_Unused3      (0x001A) 
#define   TOP_CAT_Unused4      (0x001B) 
#define   TOP_CAT_Unused5      (0x001B) 
#define   TOP_CAT_Unused6      (0x001C) 
#define   TOP_CAT_Unused7      (0x001D) 
#define   TOP_CAT_Unused8      (0x001E) 
#define   TOP_CAT_Unused9      (0x001F) 

/* Kids and Teens sites */
#define   KID_MASK             (0x00E0)
#define   KID_SHIFT            5
#define   KID_SITE             (0x0020)
#define   TEEN_SITE            (0x0040)
#define   MTEEN_SITE           (0x0080)


/* category depth mask*/
#define   CAT_DEPTH_MASK       (0x7F00)
#define   CAT_DEPTH_SHIFT      8

/* cool site mask */
#define   COOL_MASK            (0x8000)
#define   COOL_SHIFT           15

/* the maximum number of results to return */
#define MAX_RESULT_ENTRIES     50000

extern const CHR* IsearchDefaultDbName;
extern const CHR* IsearchVersion;
extern const CHR* DbExtIndex;
extern const CHR* DbExtMdt;

typedef UINT4 GPTYPE;
typedef GPTYPE* PGPTYPE;

/* Idea:
 * combine the gpindex and the mdtindex. Use a relative pointer for gpindex
 * as a delta of record's gpindex in the mdtrecord.
 *   10 bits for the record index (offset is 0-1024) 
 *   22 bits for the mdtrecord index (0-4194304) roughly 4 million records.
 * Create multiple indexes and mdts to keep the total number of mdt records
 * under 4 million.
 */

typedef struct INDEX_ENTRY {
    GPTYPE gpindex;           /* global position index of a word */
    GPTYPE mdtindex;          /* multiple document table index of a word  */
} index_entry;

const int StringCompLength = 32;   /* default=32 how many characters to compare */

const int IndexingStatusParsingDocument = 1;
const int IndexingStatusIndexing = 2;
const int IndexingStatusMerging = 3;
const int IndexingStatusParsingFiles = 4;

// Operand Types
const int TypeTerm = 1;
const int TypeRset = 2;

#define COUT cout

#endif
