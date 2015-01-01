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

#include <iostream.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/time.h>

#ifdef LINUX
#include <time.h>
#endif

#include "gdt.h"
#include "search.h"
#include "common.h"
#include "dtreg.h"
#include "index.h"
#include "idb.h"
#include "tokengen.h"
#include "newhoo.h"

extern char *prog_name;

extern char *warning_string;
extern STRING raw_query_str;
extern STRING clean_query_str;
extern STRING url_query_str;
extern STRING simple_query_str;
extern STRING av_query_str;
extern STRING deja_query_str;
extern STRING aol_query_str;
extern char **phrase_tbl;

time_t t;
char *time_str;
int warned;

/*---------------------------------------------------------------------------
 *  Search()
 *
 *  db         = pointer to database to use
 *  query_str  = term(s) for which to search.
 *  term_count = the number of terms searched on 
 *               (used to determining if we should do a fallback to OR search) 
 *  type       = type of search NOFALLBACK
 *  AbsMax     = absolute limit of result set size
 *
 *  Returns the PRSET with the results on success and NULL on failure.
 *---------------------------------------------------------------------------
 */
PIRSET Search(IDB *db, search_query *And, search_query *FallBack, int term_count, int type, int AbsMax)
{
  PIRSET pirset;
  int HitCount; 

  /* Execute the search */
  if (AbsMax)
    pirset=db->Search(And,AbsMax);
  else
    pirset=db->Search(And);

  /* How many hits? */
  if (pirset == NULL) {
    HitCount = 0;
  }
  else {
    pirset->SortByScore();
    HitCount = pirset->GetTotalEntries();
  }

  /*
   * if AND search fails try with OR as the default operator
   */
  if( FallBack && !(type & NOFALLBACK) && 
      (HitCount == 0) && (term_count > 1)) {

    if (AbsMax)
      pirset=db->Search(FallBack,AbsMax);
    else
      pirset=db->Search(FallBack);
    
    if (pirset == NULL) {
      return NULL;
    }
    
    pirset->SortByScore();
    HitCount = pirset->GetTotalEntries();

    if (HitCount == 0) {
	return NULL;
    }
    else {
	/* set the "falling back to OR search" flag */
	warned |= 0x4;
    }
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

  /*  Open Site database */
  if (!db_path || !db_name || ((db = new IDB(db_path, db_name)) == NULL)) {
    t = time(NULL);
    time_str = ctime(&t);
    time_str[strlen(time_str) -1] = '\0';   /* strip trailing '\n' */
    cerr << "[" << time_str 
	 << "] ERROR: Failed to open database [" 
	 << db_path << db_name << "]\n";

    return NULL;
  }

  /* Is the database valid? */
  if (db->GetTotalRecords() <= 0) {
    t = time(NULL);
    time_str = ctime(&t);
    time_str[strlen(time_str) -1] = '\0';   /* strip trailing '\n' */
    cerr << "[" << time_str 
	 << "] ERROR: Database " << db_path << db_name    
	 << " does not exist or is corrupted\n";
    return NULL;
  }

  return db;

}

/*-----------------------------------------------------------------
 *  ParseQueryString() - take the user specified search string and
 *                      parse into terms and create a Isearch 
 *                      squery instance 
 *-----------------------------------------------------------------
 */
int NewParseQueryString(STRING *query,  
			search_query *AndQuery, 
			search_query *FallBackQuery, 
			search_query *KeywordQuery, 
			int *AdultSearch) { 

  STRING ProcessedQuery;
  STRING keyword_str;
  unsigned char field = 0;
  char *tmp;
  int i,k;
  int a,b;

  int t = 0;

  int BooleanOp = NULL_TERM;

  query->Replace(","," ");
	    
  int TotalPhrases = 0;

  /* break the query string into tokens. */
  STRING   StrTerm;
  TOKENGEN TokenGen(*query);
  
  /* initialize various search strings
   * XXX - these are all GLOBALS, yuck.
   */
  clean_query_str = "";
  simple_query_str = "";
  av_query_str    = "";
  deja_query_str  = "";
  keyword_str     = "";
  
  /* default to boolean search */
  char *tmp_string;
  int  other_query_word_count =0;
  
  TotalPhrases = TokenGen.GetTotalEntries();
  phrase_tbl = (char **) malloc(TotalPhrases * sizeof(char *) + 1);
  k = 0;
  phrase_tbl[k] = NULL;
  
  /* limit the number of search */
  if (TotalPhrases > (2 * MAX_SEARCH_TERMS)) { 
    warned = 1; 
    TotalPhrases = 2 * MAX_SEARCH_TERMS;
  }
  
  for (i=1; (t < MAX_SEARCH_TERMS) && (i <= TotalPhrases); i++) {

    TokenGen.GetEntry(i, &StrTerm);

    field = 0;

    /* check for prepended "+" or "-"*/
    if (StrTerm.GetLength() > 1) {
      if (StrTerm.GetChr(1) == '+') {
	  /* treat + terms as forcing "AND" between all terms
	   * remove the leading '+'
	   */
	StrTerm.EraseBefore(2);
	
	BooleanOp = AND_TERM;
      }
      else if (StrTerm.GetChr(1) == '-') {
	  /* treat - terms as forcing "ANDNOT" before this term
	   * remove the leading '-'
	   */
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
    
    /* convert to a canonical form for boolean expressions */
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

    /* check for single * character */
    if (StrTerm == "*") {
      StrTerm = "";
    }

    /* rebuild the query */
    if (StrTerm != "") {

        AndQuery[t].first = -1;
        AndQuery[t].last = -1;
        AndQuery[t].result_size = 0;
        AndQuery[t].result_set  = NULL;
	AndQuery[t].field = field;
	AndQuery[t].docfield = 0;
        FallBackQuery[t].first = -1;
        FallBackQuery[t].last = -1;
        FallBackQuery[t].result_size = 0;
        FallBackQuery[t].result_set  = NULL;
	FallBackQuery[t].field = field;
	FallBackQuery[t].docfield = 0;

	AndQuery[t].Term      = StrTerm; 
	FallBackQuery[t].Term = StrTerm; 


	if (StrTerm.GetChr(1) == '"') {
	    /* strip off quotes */
	    AndQuery[t].Term.Replace("\"","");
	    FallBackQuery[t].Term.Replace("\"","");
	}

        /* prepend boolean operator */
        if (BooleanOp == AND_TERM) {
	    clean_query_str += '+';
	    AndQuery[t].type = AND_TERM;
	    FallBackQuery[t].type = AND_TERM;
	    /* cerr << t << " AND Term = " << StrTerm << "\n"; */
	}
	else if (BooleanOp == ANDNOT_TERM) {
	    clean_query_str += '-';
	    AndQuery[t].type = ANDNOT_TERM;
	    FallBackQuery[t].type = ANDNOT_TERM;

	    /*cerr << t << " ANDNOT Term = " << StrTerm << "\n";*/
	}
	else if (BooleanOp == OR_TERM) {
	    clean_query_str += "or ";
	    AndQuery[t].type = OR_TERM;
	    FallBackQuery[t].type = OR_TERM;

	    /*cerr << t << " OR Term = " << StrTerm << "\n"; */
	}
	else {
	    /* default action */
	    AndQuery[t].type = AND_TERM;
	    FallBackQuery[t].type = OR_TERM;

	    /* cerr << t << " DEFAULT Term = " << StrTerm << "\n"; */
	}

	clean_query_str += StrTerm;

	if (i < TotalPhrases)
	    clean_query_str += ' ';

#if 1
	/* check for sexual keywords to enable Adult categories*/
	if ((StrTerm ^= "sex")   || 
	    (StrTerm ^= "xxx")   ||
	    (StrTerm ^= "fuck") ||
	    (StrTerm ^= "pussy") ||
	    (StrTerm ^= "porn")  ||
	    (StrTerm ^= "porno")  ||
	    (StrTerm ^= "erotic")  ||
	    (StrTerm ^= "naked")  ||
	    (StrTerm ^= "nude")){
	    /* enable adult search results */
	    *AdultSearch = TRUE;
	}
#endif
	
	if (BooleanOp == ANDNOT_TERM) {
	    /* don't use ANDNOT terms for keyword search not other simple
	     * searches, but include for AltaVista style "-keyword" searches 
	     */
	  av_query_str += " -";
	  av_query_str += StrTerm;
	  deja_query_str += " &! ";
	  deja_query_str += StrTerm;
	  aol_query_str += " AND NOT ";
	  aol_query_str += StrTerm;
	}
	else {
	  /*simple keyword search string */
	  if (other_query_word_count > 0)
	    simple_query_str += ' ';
	  simple_query_str += StrTerm;
	  keyword_str += StrTerm;
	  other_query_word_count++;
	  
	  /* list of keyword for bolding, after striping out / and " */
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
	  phrase_tbl[k++] = tmp;
	  phrase_tbl[k] = NULL;
	  
	}

	t++;

#if 0	
	STRING DomainTerm;


	if ((BooleanOp != ANDNOT_TERM) &&
	    (strstr((char *)StrTerm.Buffer, ".com") == NULL) &&
	    (strstr((char *)StrTerm.Buffer, "www.") == NULL)){
	    
	    AndQuery[t].first = -1;
	    AndQuery[t].last = -1;
	    AndQuery[t].result_size = 0;
	    AndQuery[t].result_set  = NULL;
	    AndQuery[t].type = DOMAIN_TERM;
	    AndQuery[t].field ='u';
	    AndQuery[t].docfield = 0;
	    FallBackQuery[t].first = -1;
	    FallBackQuery[t].last = -1;
	    FallBackQuery[t].result_size = 0;
	    FallBackQuery[t].result_set  = NULL;	
	    FallBackQuery[t].type = DOMAIN_TERM;
	    FallBackQuery[t].field ='u';
	    FallBackQuery[t].docfield = 0;
	    
	    DomainTerm = "www.";
	    DomainTerm.Cat(StrTerm.Buffer);
	    DomainTerm.Cat((unsigned char *)".com/");
	    
	    AndQuery[t].Term      = DomainTerm; 
	    FallBackQuery[t].Term = DomainTerm; 

	    /* cerr << "DOMAIN TERM = " << DomainTerm << "\n"; */

	    t++;

	}
#endif

	BooleanOp = NULL_TERM;

    }
  }

  AndQuery[t].first = -1;
  AndQuery[t].last = -1;
  AndQuery[t].result_size = 0;
  AndQuery[t].result_set  = NULL;
  AndQuery[t].Term = ""; 
  AndQuery[t].type = NULL_TERM;
  AndQuery[t].field = 0;
  AndQuery[t].docfield = 0;

  FallBackQuery[t].first = -1;
  FallBackQuery[t].last = -1;
  FallBackQuery[t].result_size = 0;
  FallBackQuery[t].result_set  = NULL;
  FallBackQuery[t].Term = ""; 
  FallBackQuery[t].type = NULL_TERM;
  FallBackQuery[t].field = 0;
  FallBackQuery[t].docfield = 0;
  
  keyword_str.Replace("*","");
#if 0
  keyword_str.Replace(".","");
  keyword_str.Replace("!","");
  keyword_str.Replace("-","");
  keyword_str.Replace("&","");
  keyword_str.Replace(",","");
#endif

  KeywordQuery[0].Term = keyword_str;
  KeywordQuery[0].type = AND_TERM;
  KeywordQuery[0].first = -1;
  KeywordQuery[0].last = -1;
  KeywordQuery[0].result_size = 0;
  KeywordQuery[0].result_set  = NULL;
  KeywordQuery[0].field  = 'k';
  KeywordQuery[0].docfield  = 0;

  KeywordQuery[1].Term = "";
  KeywordQuery[1].type = NULL_TERM;
  KeywordQuery[1].first = -1;
  KeywordQuery[1].last = -1;
  KeywordQuery[1].result_size = 0;
  KeywordQuery[1].result_set  = NULL;
  KeywordQuery[1].field  = 0;

  return(t);
}

