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

/*--------------------------------------------------------------------
 *
 * example-cgi.c - an example CGI for searching a database
 *
 *--------------------------------------------------------------------
 */

#include <iostream.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>
#include <locale.h>
#include <signal.h>

#include "gdt.h"
#include "common.h"
#include "dtreg.h"
#include "index.h"
#include "idb.h"
#include "tokengen.h"
#include "cgi-util.h"

// what is truth anyway?
#define FALSE 0
#define TRUE  1

#define DATABASE_DIR  "/gh/searchDB/example"
#define DB_NAME  "site"

#define MAXHIT_DEFAULT     5000
#define MAX_RESULTS_PER_PAGE 20
#define MAX_SEARCH_TERMS   32

int ParseQueryString(STRING *query, search_query *Query);
PIRSET Search(IDB *db, search_query *And, int term_count, char *category);
void  PutHTTPHeader(void);
void  PutHTMLHead(int start,int end ,int total);
void  PutHTMLBodyStart();
void  PutHTMLBodyEnd(int valid_search_string);
void  NoMatches(char *mesg);
void print_escaped_url(char *url);
IDB *OpenIsearchDB(char * db_path, char *db_name);

/* =========================== GLOBALS =====================================*/
char *category;
int restrict = FALSE;
time_t t;
char *time_str;

extern MDTHASHTABLE ht;

/* raw query string */
STRING raw_query_str = "";

/* parsed query string, run through the tokeniser */
STRING clean_query_str;
STRING url_query_str;

IDB  *search_db;    // pointer to search DB


/*==========================================================================
 *  main() - you know what this is...
 *==========================================================================
 */

int main(int argc, char **argv)
{
  int  Start = 1;                // start displaying with this result number
  int  SiteStart;                // start displaying with this Site result number
  int  MaxHits;                  // maximum number of results displayed per page 
  int  hits = 0;                 // number of category search results presented
  int  term_count = 0;           // number of search terms
  int  first;
  search_query SearchQuery[2*MAX_SEARCH_TERMS + 4];

  PIRSET results = NULL;
  char *tmp_str = NULL;
  CGIAPP *cgidata;

  PutHTTPHeader();
  PutHTMLHead(0,0,0);
  
  cgidata = new CGIAPP();
    
  if (argc >= 2) {
    // Allow the program to be run from the command line
    // only one argument, so complex searches must be quoted
    raw_query_str = argv[1];

    unescape_string((char *)raw_query_str.Buffer);

    if (argc > 2) 
      Start = strtol(argv[2], NULL, 10);
    else
      Start = 1;
    if (Start <= 0)
      Start = 0;
    category = NULL;
  }
  else {
    //================= Read CGI parameters ==================
    cgidata->GetInput();

    // Which result set record number should be displayed first?      
    Start = (tmp_str = cgidata->GetValueByName("start")) ? strtol(tmp_str,NULL,10) : 1;
    if (Start <= 0)
      Start = 1;
    
    // if set then restrict to the following category
    category = cgidata->GetValueByName("cat");
    if (category && (strncmp("Top/",category,4) == 0)) {
	// chop off uneeded Top string
	category = category + 4;
    }

    restrict = FALSE;
    tmp_str = cgidata->GetValueByName("all");
    if (tmp_str && category) {
      if(strcasecmp(tmp_str,"no") == 0) {
	restrict = TRUE;
      }
    }

    tmp_str = cgidata->GetValueByName("search");    
    if (tmp_str){
      raw_query_str = tmp_str;
    }
    else {
      raw_query_str = "";
    }

  }

  /* check for null search */
  if ((raw_query_str.Buffer == NULL) || (raw_query_str == "")) {
    /* text_random_cats(); */
    PutHTMLBodyEnd(0);
    exit (0);
  }  

  // How many result set records should be displayed?
  MaxHits = MAXHIT_DEFAULT;

  // parse the raw query sting and get the SQUERY class instance
  term_count = ParseQueryString(&raw_query_str, SearchQuery);


  // sanitize the query string for use in URLs
  url_query_str = clean_query_str;


  int TotalHits = 0;

  if (((search_db = OpenIsearchDB(DATABASE_DIR, DB_NAME)) != 0) ||
      ((search_db = OpenIsearchDB("example", DB_NAME)) != 0)) {

    results = Search(search_db, SearchQuery, term_count, category);
  
    if (results) {
      TotalHits = results->GetTotalEntries();
    }

  }
  else {
    t = time(NULL);
    time_str = ctime(&t);
    time_str[strlen(time_str) -1] = '\0';   // strip trailing '\n'
    cerr << "[" << time_str 
	 << "] ERROR: Failed to open database [" 
	 << DATABASE_DIR << "/"
	 << DB_NAME << "]\n";
  }

  if (TotalHits == 0) {
      NoMatches(NULL);
      PutHTMLBodyEnd(0);
      exit(0);
  }
  else {
      /* Display results */

      if (Start >= TotalHits)
	Start = TotalHits - MAX_RESULTS_PER_PAGE;

      if (Start <= 0)
	Start = 1;

      char *Url, *Title, *Desc, *full_str;
      IRESULT Record(search_db->GetMainMdt()->MdtTable);
      int len,j;
      int i = Start;
      int num_results = 0;
      first = 1;

      while ((i <= TotalHits) && (num_results < MAX_RESULTS_PER_PAGE)) {
      
	  // Fetch the first hit
	  results->GetEntry(i, &Record);
      
	  if (first) {
	      cout  << "<h2>Search Results:</h2><UL>";	  
	      first = 0;
	  }
	
	  len = Record.GetRecordLength();
	  
	  // Get the file contents directly from our mmap'ed file
	  full_str = (char *) malloc(len +1);
	  search_db->GetMainIndex()->GetIndirectBuffer(Record.GetGlobalFileStart(),
						       (unsigned char *) full_str, 0, 
						       len);
	
	  // now parse out the seperate strings
	  Url   = "";
	  Title = "";
	  Desc  = "";
	  
	  for(j=0; j < len ; j++) 
	      if ((j == 0) || (full_str[j-1] == '\n')) {
		if (j) full_str[j-1] = '\0';
		switch (full_str[j]) {
		case 'u':
		  Url= full_str + j + 2;
		  break;
		case 't':
		  Title = full_str + j + 2;
		  break;
		case 'd':
		  Desc = full_str + j + 2;
		  break;
		default:
		  break;
		}
	      }
	  
	  // display
	  cout << "<LI><a href=\"";
	  print_escaped_url(Url);
	  cout << "\">" << Title << "</a> " << Desc << "\n";
	
	  free(full_str);
	  num_results++;

	  i++;
		
      }
      
  }
  
  if (!first) {
      cout << "</UL>\n";
  }
    
  PutHTMLBodyEnd(0);

  // close IO to flush connection
  fclose(stderr);
  fclose(stdout);    

  exit(0);
}


void PutHTTPHeader() 
{
    static int first = 1;

    if (first) 
      cout << "Content-type: text/html\n\n";

    first = 0;
}

void PutHTMLHead(int start,int end ,int total)
{
    static int first = 1;

    if (first) 
      cout << "<BODY BGCOLOR=WHITE>\n";

    first = 0;

}

void PutHTMLBodyStart() {

}


void PutHTMLBodyEnd(int valid_search_string) 
{
  cout << "</BODY>\n";
}

void NoMatches(char *mesg){
  printf("There where no matches to your query.\n");
}

void print_nice(char *s)
{
  cout << s << " ";
}

/*---------------------------------------------------------------------------
 *  Search()
 *
 *  db         = pointer to database to use
 *  Query      = term(s) for which to search.
 *  term_count = the number of terms searched on 
 *  category   = the category to restrict the search to, if any
 *
 *  Returns the PRSET with the results on success and NULL on failure.
 *---------------------------------------------------------------------------
 */

PIRSET Search(IDB *db, search_query *And, int term_count, char *category)
{
  PIRSET pirset;
  int HitCount; 

  /* restrict to the chosen category, if we are to do so */
  if (category) {
    if (restrict == TRUE) {
      /* restrict to a single set of sub categories */
      db->SetRestrictPathname(category);
    }
  }

  /* Execute the search */
  pirset=db->Search(And);


  if (pirset != NULL) {
    /* sort the results by score */
    pirset->SortByScore();
  }

  return pirset;

}


/*-----------------------------------------------------------------
 *  OpenIsearchDB()  - open an Isearch database, with error checking
 *                     return 1 on success, 0 on failure 
 *-----------------------------------------------------------------
 */

IDB *OpenIsearchDB(char * db_path, char *db_name) {

  IDB *db = NULL;

  // Open Site database
  if (!db_path || !db_name || ((db = new IDB(db_path, db_name)) == NULL)) {
    return NULL;
  }

  // Is the database valid?
  if (db->GetTotalRecords() <= 0) {
    t = time(NULL);
    time_str = ctime(&t);
    time_str[strlen(time_str) -1] = '\0';   // strip trailing '\n'
    cerr << "[" << time_str 
	 << "] ERROR: Database " << db_path << "/" << db_name    
	 << " does not exist or is corrupted\n";
    return NULL;
  }

  return db;

}

/*-----------------------------------------------------------------
 *  ParseQueryString() - take the user specified search string and
 *                       parse into terms and create a Isearch 
 *                       squery instance 
 *-----------------------------------------------------------------
 */
int ParseQueryString(STRING *query, search_query *Query) {

  STRING ProcessedQuery;
  STRING keyword_str;
  STRING DomainTerm;
  unsigned char field = 0;
  char *tmp;
  int  i,a,b;

  int t = 0;
  int BooleanOp = NULL_TERM;
  query->Replace(","," ");
	    
  int TotalPhrases = 0;

  // break the query string into tokens.
  STRING   StrTerm;
  TOKENGEN TokenGen(*query);
  
  // default to boolean search
  char *tmp_string;
  int  other_query_word_count =0;
  
  TotalPhrases = TokenGen.GetTotalEntries();
  
  /* limit the number of search */
  if (TotalPhrases > (2 * MAX_SEARCH_TERMS)) { 
    TotalPhrases = 2 * MAX_SEARCH_TERMS;
  }
  
  for (i=1; (t < MAX_SEARCH_TERMS) && (i <= TotalPhrases); i++) {

    TokenGen.GetEntry(i, &StrTerm);

    field = 0;
    
    /* check for prepended "+" or "-"  */
    if (StrTerm.GetLength() > 1) {
      if (StrTerm.GetChr(1) == '+') {
	/* treat + terms as forcing "AND" between all terms, remove the leading '+' */
	StrTerm.EraseBefore(2);
	
	BooleanOp = AND_TERM;
      }
      else if (StrTerm.GetChr(1) == '-') {
	/* treat - terms as forcing "ANDNOT" before this term remove the leading '-' */
	StrTerm.EraseBefore(2);
	
	BooleanOp = ANDNOT_TERM;
      }
    }

    /* look for field modifier "u:test" is only return instances of test in */
    if (StrTerm.GetLength() > 2) {
      if (StrTerm.GetChr(2) == ':') {
	field = StrTerm.GetChr(1);
	StrTerm.EraseBefore(3);
      }
    }
    
    // convert to a canonical form for boolean expressions
    if ((StrTerm ^= "OR") || (StrTerm == "|") || (StrTerm == "||")) {
      BooleanOp = OR_TERM;
      StrTerm = "";
    }
    else if ((StrTerm ^= "AND") || (StrTerm == "&") || (StrTerm == "&&") || (StrTerm == "+")) {
      BooleanOp = AND_TERM;
      StrTerm = "";
    }
    else if ((StrTerm ^= "ANDNOT") || (StrTerm == "&!") || (StrTerm == "-")) {
      BooleanOp = ANDNOT_TERM;
      StrTerm = "";
    }

    // check for single * character
    if (StrTerm == "*") {
      StrTerm = "";
    }

    // rebuild the query
    if (StrTerm != "") {

        Query[t].first       = -1;
        Query[t].last        = -1;
        Query[t].result_size = 0;
        Query[t].result_set  = NULL;
	Query[t].field       = field;
	Query[t].Term        = StrTerm; 

	if (StrTerm.GetChr(1) == '"') {
	  // strip off quotes
	  Query[t].Term.Replace("\"","");
	}

        // prepend boolean operator
        if (BooleanOp == AND_TERM) {
	    clean_query_str += '+';
	    Query[t].type = AND_TERM;
	}
	else if (BooleanOp == ANDNOT_TERM) {
	    clean_query_str += '-';
	    Query[t].type = ANDNOT_TERM;
	}
	else if (BooleanOp == OR_TERM) {
	    clean_query_str += "or ";
	    Query[t].type = OR_TERM;
	}
	else {
	    // default action
	    Query[t].type = AND_TERM;
	}

	clean_query_str += StrTerm;

	if (i < TotalPhrases) 
	    clean_query_str += ' ';


	if (BooleanOp == ANDNOT_TERM) {
	  // don't use ANDNOT terms for keyword search not other simple
	  // searches, but include for AltaVista style "-keyword" searches
	}
	else {
	  other_query_word_count++;
	  
	  // list of keyword for bolding, after striping out / and "
	  tmp = (char *)malloc(strlen((char *)StrTerm.Buffer)+1);
	  a = 0;
	  b = 0;
	  while (StrTerm.Buffer[a]) {
	    if ((StrTerm.Buffer[a] != '"') && (StrTerm.Buffer[a] != '*'))
	      tmp[b++] = StrTerm.Buffer[a++];
	    else
	      a++;
	  }
	  tmp[b] = 0;
	  
	}
	
	t++;
	
	BooleanOp = NULL_TERM;
	
    }
  }
  
  Query[t].result_size = 0;
  Query[t].result_set  = NULL;
  Query[t].first = -1;
  Query[t].last  = -1;
  Query[t].Term  = ""; 
  Query[t].type  = NULL_TERM;
  Query[t].field = 0;

  return(t);
}

