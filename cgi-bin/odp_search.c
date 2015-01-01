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
 * odp_search.cxx  - Open Directory Search
 *
 * A total rewrite of the total rewrite of the Open Directory Search.
 * This version uses jsearch technology.
 *
 * Bryn Dole (dole@atomicwidget.com)
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
#include <locale.h>
#include <signal.h>
#include <errno.h>
#include <iconv.h> 

#ifdef SUNOS
#include <sys/loadavg.h>
#endif

#include "gdt.h"
#include "isearch.h"
#include "common.h"
#include "dtreg.h"
#include "index.h"
#include "idb.h"
#include "tokengen.h"
#include "cgi-util.h"
#include "newhoo.h"
#include "jdisp3.h"
#include "demote_url.h"

/* maximum allowable record length */
#define MAX_RECORD_LENGTH   800

#define SITE_DB_NAME   "example"
#define CATS_DB_NAME   "example_categories"

void disp(int, int, int, char*, int);
void NoMatches(char *mesg);
int  CountSites(IDB *db, PIRSET prset, int Start, int MaxHits, int CatHits);
int  CountCategories(PIRSET prset, int Start, int MaxHits);
int  PresentCategories(PIRSET prset, int Start, int MaxHits);

/*=========================== GLOBALS =====================================*/
CGIAPP *cgidata;
char *category, *prog_name;
int restrict = FALSE;
int CategoriesOnly = FALSE;
int SitesOnly = FALSE;
int PrevCatHits = 0;
int TotalCatHits = 0;
int max_cat = MAX_CAT;
int maxhit_default = MAXHIT_DEFAULT;
int sites_per_page = SITES_PER_PAGE;

char * template_base = TEMPLATE_DEFAULT_BASE;

int max_sites_displayed = 20;
int max_cats_displayed = 10;

int jsearch_morecat = 0;
int jsearch_start = 1;
int jsearch_jsites = -1;
int sites_or_search = 0;
int category_added = 0;
char *jsearch_catrestrict = 0;
int search_advanced = 0;

int smart_browsing_fallthrough = 0;

struct CAT_HASH_TABLE **cat_hash = NULL;

bias_list_struct *bias_list = NULL;

extern struct cat_node *cats[];
extern int cats_top;
extern int max_site_score;

extern int num_leftovers;
extern struct site_node *Leftover_Sites;
extern struct site_node *Leftover_last;

int editor_buttons = 0;

int jj = 1;    /* this is jjsearch = alters the HTML output */

/* raw query string */
STRING raw_query_str;

/* parsed query string, run through the tokeniser */
STRING clean_query_str;
STRING url_query_str;

char *c_clean_query_str;
char *c_url_query_str;
char *c_simple_query_str;

/* simple query string, sutable for other search engines */
STRING simple_query_str;

/* list of ANDNOT terms for Altavista and "-keyword" enabled searches*/
STRING av_query_str;

/* deja news has a different syntax for ANDNOT, "&!" */
STRING deja_query_str;

/* AOL has a different syntax for ANDNOT, "AND NOT" */
STRING aol_query_str;

char **phrase_tbl;             // array of terms, used for bolding text

IDB  *site_db;                 // site search database
IDB  *cat_db;                  // category search database

int globalBias = 0;
int utf8 = TRUE;
char *Locale = NULL;
char *charset = NULL;
int xml  = FALSE;
unsigned short Kids = 0;       /* only search for kid/teen/mteen sites? */

char *convertToUTF8(iconv_t cd, char *input);


/*==========================================================================
 *  main() - you know what this is...
 *==========================================================================
 */
int main(int argc, char **argv)
{
  char *tmp_str;
  int  Start;                    // start displaying with this result number
  int  SiteStart;                // start displaying with this Site result number
  int  MaxHits;                  // maximum number of results displayed per page 
  int  cat_hits = 0;             // number of category search results presented
  int  site_hits = 0;            // number of site search results presented
  int  term_count = 0;           // number of search terms
  int  AdultSearch = FALSE;      // do we include adult categories?

  search_query CatAndQuery[2*MAX_SEARCH_TERMS + 4];  /* exclude all ORs */
  search_query AndQuery[2*MAX_SEARCH_TERMS + 4];
  search_query KeywordQuery[MAX_SEARCH_TERMS + 4];
  search_query FallBackQuery[2*MAX_SEARCH_TERMS + 4];

  PIRSET site_results = NULL;    
  PIRSET category_results = NULL;
  int special_flag = 0;

#ifdef SUNOS
  /* This only works for Solaris 2.7 or higher */
  /* check the load, bail if we're maxed out. */
  double load[3];
  getloadavg(load, 1);

  if (load[0] > 25.0) {
      NoMatches("Search is currently under a heavy load. Please try back later.");
      exit(0);
  }
#endif

  /* first set an alarm to wake us in a few seconds, in case we spin out of control */
  signal(SIGALRM, handle_alarm);
  alarm(600);

  cgidata = new CGIAPP();
    
  /* set the name of this program (allows easy program renaming) */
  prog_name = argv[0];

  if (argc >= 2) {
    alarm(600);
    /* Allow the program to be run from the command line
     * only one argument, so compex searches must be quoted
     */
    raw_query_str = argv[argc-1];
    if (argc >= 2) {
      int i = 1;
      while (i < argc) {
	if (strcmp(argv[i],"-a") == 0) {
	    AdultSearch = TRUE;
	    i += 1;
	}
	else if (strcmp(argv[i],"-A") == 0) {
	    AdultSearch = FALSE;
	    i += 1;
	}
	else if (strcmp(argv[i],"-kid") == 0) {
	    Kids |= KID_SITE;
	    i += 1;
	}
	else if (strcmp(argv[i],"-teen") == 0) {
	    Kids |= TEEN_SITE;
	    i += 1;
	}
	else if (strcmp(argv[i],"-mteen") == 0) {
	    Kids |= MTEEN_SITE;
	    i += 1;
	}
	else if (strcmp(argv[i],"-x") == 0) {
	    /* display XML */
	    xml=1;
	    i += 1;
	}	
	else if (strcmp(argv[i],"-u") == 0) {
	    utf8=1;
	    i += 1;
	}
	else if (strcmp(argv[i],"-h") == 0) {
	    /* display Help */
	    cerr << "Usage: odp_search\n";
	    cerr << "\t-x              enable XML output\n";
	    cerr << "\t-u              enable UTF-8 output\n";
	    cerr << "\t-a              enable adult results\n";
	    cerr << "\t-A              disable adult results\n";
	    cerr << "\t-r              restict to category\n";
	    cerr << "\t-s num          Start of results (offset)\n";
	    cerr << "\t-t num          max number of sites per page\n";
	    cerr << "\t-i num          max number of sites displayed\n";
	    cerr << "\t-c num          max number of categories displayed\n";
	    cerr << "\t-m num          start of more categories\n";
	    cerr << "\t-C category     base category search originates from\n";
	    cerr << "\t-L locale       set locale\n";
	    cerr << "\t-S charset      set input charset\n";
	    cerr << "\t-T template     set template name\n";
	    cerr << "\t-h              Program usage/help\n";
	    exit(255);
	}
	else if (strcmp(argv[i],"-s") == 0) {
	    /* starting result number */
	    jsearch_start = strtol(argv[i+1], NULL, 10);
	    if (jsearch_start <= 0)
		jsearch_start = 0;
	    i += 2;
	}
	else if (strcmp(argv[i],"-t") == 0) {
	    /* max sites per page */
	    sites_per_page = strtol(argv[i+1], NULL, 10);
	    if (sites_per_page < 5 )
		sites_per_page = 5;
	    if (sites_per_page > MAX_SITES )
		sites_per_page = MAX_SITES;
	    i += 2;
	}
	else if (strcmp(argv[i],"-i") == 0) {
	    if (argv[i+1]) {
		max_sites_displayed = atoi(argv[i+1]);
	    }
	    i += 2;
	}
	else if (strcmp(argv[i],"-c") == 0) {
	    if (argv[i+1]) {
		max_cats_displayed = atoi(argv[i+1]);
	    }
	    i += 2;
	}
	else if (strcmp(argv[i],"-m") == 0) {
	    if (argv[i+1]) {
		jsearch_morecat = atoi(argv[i+1]);
	    }
	    i += 2;
	}
	else if (strcmp(argv[i],"-r") == 0) {
	    restrict = TRUE;
	    i += 1;
	}
	else if (strcmp(argv[i],"-C") == 0) {
	    if (argv[i+1]) {
		category = strdup(argv[i+1]);
	    }
	    i += 2;
	}
	else if (strcmp(argv[i],"-L") == 0) {
	    if (argv[i+1]) {
		Locale = strdup(argv[i+1]);
	    }
	    i += 2;
	}
	else if (strcmp(argv[i],"-S") == 0) {
	    if (argv[i+1]) {
		charset = strdup(argv[i+1]);
	    }
	    i += 2;
	}
	else if (strcmp(argv[i],"-T") == 0) {
	    if (argv[i+1]) {
		template_base = strdup(argv[i+1]);
	    }
	    i += 2;
	}

	else if (strcmp(argv[i],"-D") == 0) {
	    /* dumb down the search (faster) */
	    maxhit_default = maxhit_default / 2;
	    max_cat = max_cat / 2;
	    i += 1;
	}
	else {
	    if (i < (argc - 1)) 
		cerr << "Unknown option: " << argv[i] << "\n";
	    i++;
	}
      }
    }
  }
  else {
      /*================= Read CGI parameters ================== */
    cgidata->GetInput();

    /* Good for debugging form values */
    /* cout << "Content-type: text/html\n\n";
     * cgidata->Display();
     */

    tmp_str = cgidata->GetValueByName("utf8");
    if (tmp_str != NULL) {
      if(tmp_str[0] == '1') {
	utf8 = TRUE;
      }
      else {
	utf8 = FALSE;
      }
    }

    /* if set then restrict to the following category */
    category = cgidata->GetValueByName("cat");
    restrict = FALSE;
    tmp_str = cgidata->GetValueByName("all");
    if (tmp_str && category) {
      if(strcasecmp(tmp_str,"no") == 0) {
	restrict = TRUE;
	jsearch_catrestrict = category;
      }
    }

    tmp_str = cgidata->GetValueByName("template_base");
    if (tmp_str) {
	/* make sure there are no funny characters in the tmplate name */
	if (strlen(template_base) < 32) {
	    for (int i=0; template_base[i] ; i++)
		if (!isalpha(template_base[i])) 
		    tmp_str = NULL;
	}
	if (tmp_str)
	    template_base = tmp_str;
    }


    if (tmp_str = cgidata->GetValueByName("cs")) 
	charset = tmp_str;

    tmp_str = cgidata->GetValueByName("locale");
    if (tmp_str) {
	Locale = tmp_str;
    }
    else {
	tmp_str = getenv("HTTP_ACCEPT_LANGUAGE");
	if (tmp_str) {
	    int i;
	    for (i = 0; tmp_str[i] && (isalpha(tmp_str[i]) || tmp_str[i] == '_' || tmp_str[i] == '-'); i++) {
		if (tmp_str[i] == '-') 
		    tmp_str[i] = '_';
		else 
		    tmp_str[i] = tolower(tmp_str[i]);
	    }
	    tmp_str[i] = '\0';
	    Locale = tmp_str;
	}
    }

    /* check for sites/categories only flags */

    if (cgidata->GetValueByName("so"))
	SitesOnly = TRUE;
    else {
	SitesOnly = FALSE;
	if (cgidata->GetValueByName("co"))
	    CategoriesOnly = TRUE;
	else
	    CategoriesOnly = FALSE;
    }

#if 0
    if (tmp_str = cgidata->GetValueByName("t")) {
	if (tmp_str[0] == 'c') {
	    SitesOnly = FALSE;
	    CategoriesOnly = TRUE;
	}
	else if (tmp_str[0] == 's') {
	    SitesOnly = TRUE;
	    CategoriesOnly = FALSE;
	}
    }
#endif

    tmp_str = cgidata->GetValueByName("search");    
    if (tmp_str)
	raw_query_str = tmp_str;
    else 
	raw_query_str = "";


    if (tmp_str = cgidata->GetValueByName("spp")) {
	sites_per_page = atoi(tmp_str);
	if (sites_per_page < 5 )
	    sites_per_page = 5;
	if (sites_per_page > MAX_SITES )
	    sites_per_page = MAX_SITES;
    }

    if (tmp_str = cgidata->GetValueByName("morecat"))
	jsearch_morecat = atoi(tmp_str);

    if (tmp_str = cgidata->GetValueByName("start")) {
	jsearch_start = atoi(tmp_str);

#if 0
	/* commented out, doesn't help enough to make a difference */
	/* this is a robot, only a robot would add start=1, give them less complete results */
	if (jsearch_start == 1) {
	    maxhit_default = maxhit_default / 2;
	    max_cat = max_cat / 2;
	}
#endif
    }


#if 1
    /* no longer used, redirect to advanced search page */
    if (tmp_str = cgidata->GetValueByName("jsites"))
	jsearch_jsites = 1;
    if (tmp_str = cgidata->GetValueByName("jstart"))
	jsearch_jsites = 1;
#endif

    if (cgidata->GetValueByName("a.x") || cgidata->GetValueByName("adv"))
	search_advanced = 1;

    tmp_str = cgidata->GetValueByName("ebuttons");
    if (tmp_str && (strcmp(tmp_str,"1") == 0)) {
	editor_buttons = 1;
    }

    if (cgidata->GetValueByName("Kids")) 
	Kids |= KID_SITE;

    if (cgidata->GetValueByName("Teens")) 
	Kids |= TEEN_SITE;

    if (cgidata->GetValueByName("Mteens")) 
	Kids |= MTEEN_SITE;

  }

  if (category && (strncmp(category,"Adult",5) == 0)) {
      AdultSearch = TRUE;
  }

  if (Kids)
      AdultSearch=0;

  if ((search_advanced) || (jsearch_jsites > 0)) {
      AdvancedSearch("advanced_search.html");
      exit(0);
  }

  if (restrict && category) {
     	char *r = category;

	/* calculate the restricted category depth */
	globalBias = 0;

	while(*r) {
	    if (*r++ == '/') {
		globalBias++;
	    }
	}
  }
  
  /* How many result set records should be displayed?  */
  MaxHits = maxhit_default;

  /*
   * check for a null search
   */
  if (raw_query_str.Equals("")) {
    PutHTTPHeader();
    PutHTMLHead(0,0,0);
    PutHTMLBodyStart();

    cout << "<P><CENTER><B>";
    cout << "Please specify a query string.";
    cout << "</B></CENTER><P>";

    /* random_cats(); */

    PutHTMLBodyEnd(0);
    exit(0);
  }

/* define USE_INCONV to use the iconv libraries */
#ifdef USE_ICONV

   /* current list of charsets in use in World (as of 4/4/2002)
      ArmSCII-8
      BIG5
      Big5
      EUC-JP
      EUC-KR
      GB2312
      ISO-8859-1
      ISO-8859-2
      ISO-8859-3
      ISO-8859-8
      ISO-8859-9
      KOI8-F
      KOI8-R
      TIS-620
      UTF-8
      WINDOWS-1250
      WINDOWS-1251
      WINDOWS-1253
      WINDOWS-1256
      WINDOWS-1257
   */

  int iconvSearchQuery = FALSE;
  int iconvCategory = FALSE;
  if (!asciiClean(raw_query_str.Buffer) &&
      !validUTF8(raw_query_str.Buffer))
      iconvSearchQuery = TRUE;
  if (category &&
      !asciiClean((unsigned char *)category) &&
      !validUTF8((unsigned char *)category))
      iconvCategory = TRUE;

  
  /* check and convert query and category strings to UTF-8 */
  if ((iconvCategory || iconvSearchQuery) &&
      (!charset || (strcasecmp("UTF-8",charset) != 0))) {
	  
      iconv_t cd;
      size_t  ret;
      char *cs;
      int success = FALSE;

#define DEFAULT_INPUT_CHARSET "ISO-8859-1"

      if (charset)
	  cs = charset;
      else
	  cs = DEFAULT_INPUT_CHARSET;
      
      while (cs) {
	  cd = iconv_open("UTF-8", cs);
	  if (cd == (iconv_t)-1) {
	      /* iconv_open failed */
	  }
	  else {
	      /* attempt the conversion(s) */
	      if (iconvSearchQuery) {
		  char *utf8Query = convertToUTF8(cd,(char *)raw_query_str.Buffer);
	      
		  if (utf8Query) {
		      raw_query_str = utf8Query;
		      success = TRUE;
		  }
	      }
	      
	      if (iconvCategory) {
		  char *utf8cat = convertToUTF8(cd,category);

		  if (utf8cat) {
		      category = utf8cat;
		      success = TRUE;
		  }
	      }
	      
	      iconv_close(cd);
	  }
	  
	  if ((cs == charset) && !success) {
	      /* try Latin-1, the most likely charset */
	      cs = DEFAULT_INPUT_CHARSET;
	  }
	  else {
	      /* give up */
	      cs = NULL;
	  }
	  
      }
  }
#endif

  /* Parse the query string */
  term_count = NewParseQueryString(&raw_query_str, 
				   AndQuery, 
				   FallBackQuery, 
				   KeywordQuery, 
				   &AdultSearch);
  
  bias_list = SetBiasedList(Locale, category, AdultSearch);

  int NoNeedToFallBack = TRUE;
  int j = 0;
  for(int i = 0 ; (i <= term_count) ; i++) {
      if (AndQuery[i].type != DOMAIN_TERM) {

	  /* cerr << "CATANDQUERY[" << j << "] = " << AndQuery[i].Term << "\n"; */

	  CatAndQuery[j].first       = AndQuery[i].first;
	  CatAndQuery[j].last        = AndQuery[i].last;
	  CatAndQuery[j].result_size = AndQuery[i].result_size;
	  CatAndQuery[j].result_set  = AndQuery[i].result_set; 
	  CatAndQuery[j].Term        = AndQuery[i].Term; 
	  CatAndQuery[j].field       = 0;
	  CatAndQuery[j].docfield    = 0;
	  
	  if (AndQuery[i].type != FallBackQuery[i].type)
	      NoNeedToFallBack = FALSE;

	  if (AndQuery[i].type == OR_TERM)
	      CatAndQuery[j].type = AND_TERM;
	  else
	      CatAndQuery[j].type = AndQuery[i].type;

	  j++;

      }
      else {
	  AndQuery[i].type = OR_TERM;
	  FallBackQuery[i].type = OR_TERM;
      }
  }

  AndQuery[j].first = -1;
  AndQuery[j].last = -1;
  AndQuery[j].result_size = 0;
  AndQuery[j].result_set  = NULL;
  AndQuery[j].Term = ""; 
  AndQuery[j].type = NULL_TERM;
  AndQuery[j].field = 0;
  AndQuery[j].docfield = 0;


  if (term_count == 1) {
      STRING DomainTerm;

      if ((AndQuery[0].type != ANDNOT_TERM) &&
	  (strstr((char *)AndQuery[0].Term.Buffer, ".com") == NULL) &&
	  (strstr((char *)AndQuery[0].Term.Buffer, "www.") == NULL)){
	    
	  AndQuery[1].first = -1;
	  AndQuery[1].last = -1;
	  AndQuery[1].result_size = 0;
	  AndQuery[1].result_set  = NULL;
	  AndQuery[1].type = OR_TERM;
	  AndQuery[1].field ='u';
	    
	  DomainTerm = (AndQuery[0].Term);
	  DomainTerm.Cat((unsigned char *)".com/");
	  
	  AndQuery[1].Term      = DomainTerm; 

	  AndQuery[2].first = -1;
	  AndQuery[2].last = -1;
	  AndQuery[2].result_size = 0;
	  AndQuery[2].result_set  = NULL;
	  AndQuery[2].Term = ""; 
	  AndQuery[2].type = NULL_TERM;
	  AndQuery[2].field = 0;
	  
	  /* cerr << "DOMAIN TERM = " << DomainTerm << "\n"; */
	}
  }

  /* don't do fallback search if there is no difference between AndQuery and FallBackQuery*/
  if (NoNeedToFallBack == TRUE)
    special_flag = NOFALLBACK;

  url_query_str = clean_query_str;
  c_clean_query_str = clean_query_str.NewCString();
  c_url_query_str = url_query_str.NewCString();
  c_simple_query_str = simple_query_str.NewCString();

  SiteStart = 1;
  if (jsearch_start > maxhit_default) {
      /* we grab search results in MAXHIT_DEFAULT chunks */
      SiteStart = jsearch_start - (jsearch_start % maxhit_default);
  }

  if ((!SitesOnly) && (cat_db = OpenIsearchDB(SEARCH_DB_DIR, CATS_DB_NAME)) != 0) {
      /* do category search */
      if (AdultSearch == TRUE)
	  /* disable the exclusion of Adult categories */
	  cat_db->SetRestrictAdult(0);
      
      if (Kids)
	  cat_db->SetRestrictKids(KID_SITE | TEEN_SITE | MTEEN_SITE);
      
      if (restrict && (category != NULL)) {
	  cat_db->SetRestrictPathname(category);
      }

      category_results = Search(cat_db, CatAndQuery, AndQuery, term_count,  NOFALLBACK, 0);
      
      if (category_results) {
	  TotalCatHits = category_results->GetTotalEntries();
	  
	  /* Only need the first MAX_CAT */
	  cat_hits = PresentCategories(category_results, 1, max_cat / 2);
      }
  }

#if 0
  cout << "<br> site start = " << SiteStart << "<br>\n";
  cout << "<br> total cat count = " << TotalCatHits << "<br>\n";
  cout << "<br> cat hits = " << cat_hits << "<br>\n";
#endif
  
  if (!CategoriesOnly) {
      /* Do site search*/

    if ((site_db = OpenIsearchDB(SEARCH_DB_DIR, SITE_DB_NAME)) == 0) {
	/*
	 * something is wrong with the site database
	 */
	if (cat_hits == 0) {
	    /* we need to fix up some search strings, these are raw and unparsed. */
	    clean_query_str = raw_query_str;
	    simple_query_str = raw_query_str;
	    NoMatches("Search is temporarily unavailable. Please try back later.");
	}
    }
    else {
      if (AdultSearch == TRUE)
	  site_db->SetRestrictAdult(0);

      if (Kids)
	  site_db->SetRestrictKids(Kids);

      /* ignore all records longer than this (most likely spam) */
      site_db->SetMaxRecordLength(MAX_RECORD_LENGTH);
  
      if (restrict && category) {
	  site_db->SetRestrictPathname(category);
      }

      site_results = Search(site_db, AndQuery, FallBackQuery, term_count, 
			    NOFALLBACK , 0);
      
      if (site_results) {
	site_hits = CountSites(site_db, site_results, 1, maxhit_default, cat_hits);
      }
    }
  }


  if ((cat_hits == 0) && (site_hits == 0)) {
      /*  Simply return no results, but add the metasearch bar */
      NoMatches(NULL);
  }
  else {
       /* allow time for results to be printed */
       alarm(600);
       disp(jsearch_morecat, jsearch_start, jsearch_jsites,
	    jsearch_catrestrict, site_hits);

       PutHTMLBodyEnd(1);
  }

  /* close IO to flush connection */
  fclose(stderr);
  fclose(stdout);    

  exit(0);
}


int CountSites(IDB *db, PIRSET prset, int Start, int MaxHits, int CatHits) {
 
  /* used for parsing category headings */
  struct CAT_HASH_TABLE *e;
  int FetchCount = 0;
  int HitCount = 0;
  struct site_node *link;
  struct cat_node *newp;

  /* the number of site hits */
  HitCount = prset->GetTotalEntries();

  if (HitCount <= 0) {
      /* somehow we lost some search matches.. */
      return 0;
  } 
  
  /* how many sites to fetch this time */
  FetchCount = HitCount > MaxHits ? MaxHits : HitCount;

  if (FetchCount <= 0) {
    return 0;
  } 

  /* Build the list of Categories and Links */
  IRESULT Record(db->GetMainMdt()->MdtTable);
  int Score;
  int i, j, h, k, found;
  char *path;
  long pathHash;

  /* initialize the cat hash table */
  if (cat_hash == NULL) {
    cat_hash = (struct CAT_HASH_TABLE **) malloc( CAT_HASHTABLE_SIZE * sizeof(struct CAT_HASH_TABLE *));
    memset((void *) cat_hash,  0, CAT_HASHTABLE_SIZE * sizeof(struct CAT_HASH_TABLE *));
  }

  for (i=Start;i <= FetchCount; i++) {
     
      /* Fetch the next result from the hash */
      prset->GetEntry(i, &Record);

      Score = (int) prset->GetScaledScore(Record.GetScore(),SITE_MAX_SCORE);

      if (Score > SITE_MAX_SCORE) {
	  /* cerr << "Bogus score: " << Score << "\n"; */
	  Score = SITE_MAX_SCORE;
      }
      
      if (Score > max_site_score) {
	    max_site_score = Score;
      }

      pathHash = Record.GetPathNameHash();
      h = pathHash % CAT_HASHTABLE_SIZE;
      e = cat_hash[h];
      while(e && (e->pathName != pathHash )) 
	  e = e->next;
	
      if (e)
	    j = e->index;
      else
	    j = cats_top;

      if (j < cats_top) {
 	    /* there is an existing category entry */
	    link = (struct site_node *) malloc(sizeof(struct site_node));
	    memset((void *) link,  0, sizeof(struct site_node));
	    link->cool = Record.IsCool();
	    link->depth = Record.GetCatDepth() - globalBias;
	    link->score = Score;
	    link->globalStart = Record.GetGlobalFileStart();
	    link->length = Record.GetRecordLength();
	    link->cat = cats[j];

	    if (Record.GetRecordLength() > MAX_RECORD_LENGTH) {

		/* cerr << "MAX_RECORD_LENGTH exceeded: " << cats[j]->path << "\n"; */

	        /* the record exceeds the max length */
	        num_leftovers++;
		link->next = NULL;
		if (Leftover_Sites) {
		  Leftover_last->next = link;
		  Leftover_last = link;
		}
		else {
		  Leftover_Sites = link;
		  Leftover_last = link;
		}
	    }
	    else {
	        if (cats[j]->maxscore < Score) {
		    cats[j]->maxscore = Score;
		}

	        link->next = NULL;
		cats[j]->site_rank += max(1,(int) ((WEIGHT_SITE_MATCH * Score) / SITE_MAX_SCORE));
		if (cats[j]->lastp)
		  cats[j]->lastp->next = link;
		cats[j]->lastp = link;
		if (cats[j]->sites_here == NULL) 
		  cats[j]->sites_here = link;
		cats[j]->n_sites_here++;
	    }
      }
      else {
	  /* This is a new category  */
	    pathHash = Record.GetPathNameHash();
	    path = db->ht.GetPathName(pathHash);
	  
	    if (path) {
		/* new category entry */
	        newp = (struct cat_node *) malloc(sizeof(struct cat_node));
		memset((void *) newp,  0, sizeof(struct cat_node));

		/* chop off the trailing "/" */
		/* path[strlen(path) - 1] = 0; */
		newp->path = path;
		newp->pathName = pathHash;
		newp->n_sites_here = 0;
		newp->n_subcat_sites_here = 0;
		newp->site_rank = 0;
		newp->rank = 0;
		newp->anti = 0;
		newp->cat_match = 0;  
		newp->lastp = NULL;
		newp->sites_here = NULL;
		newp->subcats = NULL;
		newp->depth = Record.GetCatDepth() -globalBias;
		
		/* Adjust category ranks based on the biased list */
		if (bias_list) {
		    found = 0;
		    for (k = 0; !found && bias_list[k].path; k++)
			if (strncmp(path, bias_list[k].path, bias_list[k].length) == 0) {
			    newp->anti = bias_list[k].level;
			    found = 1;
			}
		}
		
		link = (struct site_node *) malloc(sizeof(struct site_node));
		memset((void *) link,  0, sizeof(struct site_node));
		link->cool = Record.IsCool();
		link->score = Score;
		link->depth = Record.GetCatDepth() - globalBias;
		link->globalStart = Record.GetGlobalFileStart();
		link->length = Record.GetRecordLength();
		link->next = NULL;
		link->cat = newp;;

		newp->maxscore = Score;
		newp->site_rank += max(1,((WEIGHT_SITE_MATCH * Score) / SITE_MAX_SCORE));
		newp->n_sites_here++;
		newp->sites_here = link;
		newp->lastp = link;
		
		if ((cats_top >= max_cat) ||
		    (Record.GetRecordLength() > MAX_RECORD_LENGTH)) {
		    /* no more categories, we're at the max OR the record exceeds the max length */

		    /* cerr << "MAX_CAT exceeded: " << newp->path << "\n"; */

		    num_leftovers++;
		    link->next = NULL;
		    if (Leftover_Sites) {
			Leftover_last->next = link;
			Leftover_last = link;
		    }
		    else {
			Leftover_Sites = link;
			Leftover_last = link;
		    }
		}
		else {
		    /* update the cat_hash*/
		    e = (struct CAT_HASH_TABLE *) malloc(sizeof(struct CAT_HASH_TABLE));
		    e->index = j;
		    e->pathName = pathHash;
		    e->next = cat_hash[h];
		    cat_hash[h] = e;

		    cats[cats_top++] = newp;
		}
	    }
      }
  }

  return FetchCount;
}

/*-----------------------------------------------------------------
 *  PresentCategories() - display category results
 *-----------------------------------------------------------------
 */

int PresentCategories(PIRSET prset, int Start, int MaxHits) {
 
    /* used for parsing category headings */
    int FetchCount = 0;
    int HitCount = 0;
    int i, h, j, k, found;
    struct cat_node *newp;
    
    /* the number of site hits */
    HitCount = prset->GetTotalEntries();
    
    /* fetch the lesser of MaxHits and HitCount */
    FetchCount = HitCount > MaxHits ? MaxHits : HitCount;
    
    if (HitCount <= 0) {
	/* somehow we lost some search matches.. */
	return 0;
    } 

    if (FetchCount <= 0) {
	return 0;
    } 
    
    /* insert the categories into our result structure */
    IRESULT Record(cat_db->GetMainMdt()->MdtTable);
    struct CAT_HASH_TABLE *e;
    char *path;
    long pathHash;
    
    /*  initialize the cat hash table */
    if (cat_hash == NULL) {
	cat_hash = (struct CAT_HASH_TABLE **) malloc( CAT_HASHTABLE_SIZE * sizeof(struct CAT_HASH_TABLE *));
	memset((void *) cat_hash,  0, CAT_HASHTABLE_SIZE * sizeof(struct CAT_HASH_TABLE *));
    }
    
    for (i=1; i <=  FetchCount ; i++) {
   
	/* Fetch the first hit */
	prset->GetEntry(i, &Record);
	
	/* Get the name of the file */
	pathHash = Record.GetPathNameHash();
	path = cat_db->ht.GetPathName(pathHash);

	/* chop off the trailing "/"
	 * path[strlen(path) - 1] = 0;
	 */

	/* add_cat(path, Record.pathName, Record.GetCatDepth()); */
	category_added = 1;

	newp = (struct cat_node *) malloc(sizeof(struct cat_node));
	memset((void *) newp,  0, sizeof(struct cat_node));
	newp->path = path;
	newp->pathName = pathHash;
	newp->n_sites_here = 0;
	newp->n_subcat_sites_here = 0;
	newp->site_rank = 0;
	newp->rank = 0;
	newp->anti = 0;
	newp->cat_match = 1;  
	newp->lastp = NULL;
	newp->sites_here = NULL;
	newp->subcats = NULL;
	newp->depth = Record.GetCatDepth() - globalBias;
	
	/* Adjust category ranks based on the biased list */
	if (bias_list) {
	    found = 0;
	    for (k = 0; !found && bias_list[k].path; k++)
		if (strncmp(path, bias_list[k].path, bias_list[k].length) == 0) {
		    newp->anti = bias_list[k].level;
		    found = 1;
		}
	}
		
	/* include category score?? */
	/* Score = prset->GetScaledScore(Record.GetScore(),100); */

	newp->rank += WEIGHT_CAT_MATCH / max(1,newp->depth + newp->anti);

	/* insert into cat hash table */
	
	h = pathHash % CAT_HASHTABLE_SIZE;
	e = cat_hash[h];
	while(e && (e->pathName != pathHash )) 
	    e = e->next;
	
	if (e)
	    j = e->index;
	else
	    j = cats_top;
	
	if (j < cats_top) {
	    /* duplicate cat ids? This should never happen */
	    cerr << "ERROR: duplicate category ids for: " << newp->path << " and " << cats[j]->path << "\n";
	}
	else {
	    /* update the cat_hash */
	    e = (struct CAT_HASH_TABLE *) malloc(sizeof(struct CAT_HASH_TABLE));
	    e->index = j;
	    e->pathName = pathHash;
	    e->next = cat_hash[h];
	    cat_hash[h] = e;
	    
	    cats[cats_top++] = newp;
	}
	
    }

    return FetchCount;
}

#ifdef USE_ICONV

char *convertToUTF8(iconv_t cd, char *input) {

    size_t inlen  = strlen(input);
    size_t outlen = inlen * 8 + 1;
    char *output  = (char *) malloc(outlen);
    char *o = output;
    size_t ret;

#ifdef SUNOS
    ret = iconv(cd, (const char **)&input, &inlen, &o, &outlen);
#else
    ret = iconv(cd, (char **)&input, &inlen, &o, &outlen);
#endif
    if (ret == (size_t)-1) {
	/* error condition */
	return NULL;
    }
    else {
	return output;
    }
}
#endif

void handle_alarm(int x) {

  NoMatches("Search timed out.");

  /* report the error */
  cerr << "Search: ALARM signal caught. query = [" << raw_query_str << "]" << endl;

  exit(0);
}
