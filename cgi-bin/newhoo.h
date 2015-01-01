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
 * common define for search binaries.
 */

#include "idb.h"

/*====================== NEWHOO defines ========================*/
/* NewHoo hostnames */
#define NH_SEARCH_BASE_URL    "http://search.dmoz.org/"
#define NH_MAIN_BASE_URL      "http://dmoz.org/"

/* uncomment the path names to use something other than the local directory */
/* #define SEARCH_DB_DIR            "/gh/searchDB/current/" */
#define FRESHNESS_DATE_FILENAME  "gen_date"

/* #define BIASLIST_PATH            "/gh/search/catbias/" */
#define BIASLIST_PREFIX          "biaslist-"
#define BIASLIST_POSTFIX         ".cfg"

/* #define TEMPLATE_DIR             "/gh/search/templates/" */
#define TEMPLATE_DEFAULT_BASE    "example"
#define TEMPLATE_POSTFIX         ".tpl"

#define MIDDLE_COL_WIDTH        437       

/* search result types */
#define SEARCH_CATS            0x01
#define SEARCH_SITES           0x02
#define SEARCH_NEWS            0x04

/* limit the number of search terms, this lessens any kind of denial
 * of service attack by using up precious CPU time doing really long
 * and complex searches. Altavista tops out at about 65 terms.
 */
#define MAX_SEARCH_TERMS 12

/* what is truth anyway? */
#define FALSE 0
#define TRUE  1

#define CAT_HASHTABLE_SIZE    10039

/* SearchUrl() flags */
#define SITES_ONLY                 0x01
#define CATS_ONLY                  0x02
#define FALLBACK_TO_OR             0x04
#define FAIL_OVER_TO_OTHER_SEARCH  0x08 

/*==================== begin Isearch defines ======================*/

#define SEARCH_TYPE(n)		((n) & SEARCH_TYPE_MASK)
#define SEARCH_TYPE_MASK	0xf0
#define SIMPLE 			0x10
#define ADVANCED 		0x20
#define NOFALLBACK 		0x40
#define FALLBACK 		0x00

/*==================== typedefs ======================*/

typedef struct PAGE_LINK_LIST {
  char* u;
  char* t;
  char* d;
  unsigned char cool;
  float score;
  float rank;
  int   depth;
  int   globalStart;
  int   length;
  struct PAGE_LINK_LIST *next;
} page_link_list;

typedef struct PAGE_CAT_LIST {
  int    cat_id;
  char*  cat_url;
  float maxscore;
  struct PAGE_LINK_LIST *links;
} page_cat_list;

typedef struct CAT_HASH_LIST {
  int index;
  int pathName;
  struct CAT_HASH_LIST *next;
} cat_hash_list;

/*==================== function declarations ======================*/

void  SearchUrl(int Cats, int Start, int Count, char* Title, int flags);
PIDB  OpenIsearchDB(char *db_path, char *db_name);
int   NewParseQueryString(STRING *query_str, search_query *And, search_query *FallBack,
			  search_query *Keywords, int *AdultSearch);
PIRSET Search(IDB *db, search_query *And, search_query *FallBack, int term_count, int TYPE, int AbsMax);
int   CatCompare(const void *a,const void *b);
void  sanitize_cgi_arg(STRING& qs);
void  Bold(CHR **qs, char *Text,char *Bolded);
void  FailoverSearch(STRING s);
void  NoMatches(char *mesg);
void  PutSearchNavBar();
void  AdvancedSearch(char *location);
void  PutHTTPHeader(void);
void  PutHTMLHead(int start,int end ,int total);
void  PutHTMLBodyStart();
void  PutHTMLBodyEnd(int valid_search_string);
void  PutXMLHeader(void);
void  PutXMLHead(int start,int end ,int total);
void  PutXMLBodyStart();
void  PutXMLBodyEnd(int valid_search_string);
void  PutCopyright(void);
void  PutFreshnessDate();
void  random_cats();
void  print_nice(char *s);
void  handle_alarm(int x);
void  print_escaped_url(char *url);
void  print_extra_escaped_url(char *url);
int   validUTF8(unsigned char *s);
int   asciiClean(unsigned char *s);
