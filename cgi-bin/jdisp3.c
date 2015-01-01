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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <locale.h>
#include <signal.h>
#include "idb.h"
#include "newhoo.h"
#include "jdisp3.h"

#define MAX_DESCRIPTION_SIZE  2048
#define DEBUG_CAT_SCORE_1 1
#define DEBUG_CAT_SCORE_2 0
#define DEBUG_SITE_SCORE  0

extern char *prog_name, *c_url_query_str;
extern struct cat_node *cats[];
extern int cats_top;
extern int num_leftovers;
extern int sites_per_page;

void print_nice(char *s);
int rank_comp(const void *a, const void *b);

extern struct site_node *sites[];
extern int sites_top;

extern int restrict;

extern char *category;
extern char **phrase_tbl;             /* array of terms, used for bolding text */
extern IDB  *site_db;      
extern STRING clean_query_str;
extern MDTHASHTABLE ht;

extern int utf8;
extern char *Locale;
extern unsigned short Kids;
extern cat_node *display[];
extern int display_top;
extern struct cat_node *second[];
extern int second_top;

int first_match_flag = 1;

char *bold(char *s)
{
	extern char *c_clean_query_str;
	static char buf[MAX_DESCRIPTION_SIZE + 64];
	char bold[MAX_DESCRIPTION_SIZE];
	char *p, *q;
	int len,slen;
	int i;
	int quote = 0;
	int or_or_and;
	int bold_count = 0;
	char *sp;

	slen = strlen(s);
	memset((void *) bold, 0, MAX_DESCRIPTION_SIZE);

	q = c_clean_query_str;

	do {
		len = 0;

		/* advance to next alphanumeric */

		while (*q && !isalnum(*q)) {
			if (*q == '"')
				quote = !quote;
			q++;
		}

		for (p = q; *p; p++, len++) {
			if (*p == '"') {
				quote = !quote;
				break;
			}

			if (!quote && !isalnum(*p))
				break;
		}

		if (len > 0) {
			if ((strncasecmp(q, "or", 2) == 0 && !isalnum(q[2])) ||
			    (strncasecmp(q, "and", 3) == 0 && !isalnum(q[3])))
				or_or_and = 1;
			else
				or_or_and = 0;

			if (!or_or_and) {
			    for (sp = s; *sp; sp++)
				if ((strncasecmp(q, sp, len) == 0) &&
				    ((sp == s) || !isalnum(*(sp-1))) &&
				    ((*(sp+len) == 0) || !isalnum(*(sp+len)))) {
				        bold_count++;
					bold[sp-s] = 1;
					bold[sp-s+len] = 2;
				}
			}
		}

		if (*p)
			q = p+1;
	} while (*p);

	slen = slen + 1 + 7*bold_count;
	if (slen > MAX_DESCRIPTION_SIZE)
	    return s;

	int bold_flag = 0;

	for (p = s, q = buf; *p; p++) {
		if (q - buf >=  MAX_DESCRIPTION_SIZE)
			break;
		if (bold[p-s] == 1) {
			*q++ = '<';
			*q++ = 'b';
			*q++ = '>';
			bold_flag = 1;
		} else if (bold[p-s] == 2) {
			*q++ = '<';
			*q++ = '/';
			*q++ = 'b';
			*q++ = '>';
			bold_flag = 0;
		}
		*q++ = *p;
	}

	if (bold_flag) {
		*q++ = '<';
		*q++ = '/';
		*q++ = 'b';
		*q++ = '>';
	}
	*q++ = '\0';

	return buf;
}

void lookup_site_info( site_node *site) {
  
    int len, j;
    char *Url_str, *Title_str, *Desc_str, *full_str;

  
    len = site->length;
  
    /* Get the file contents directly from our mmap'ed file */
    full_str = (char *) malloc(site->length +1); 
    site_db->GetMainIndex()->GetIndirectBuffer(site->globalStart, 
					       (unsigned char *) full_str, 0, 
					       len);
    /* convert \n to \0 */
    len = strlen(full_str);
    for(j = len - 1 ; 0 <= j ; j--)
      if (full_str[j] == '\n') 
	full_str[j] = '\0';
    
    /* now parse out the seperate strings */
    Url_str = "";
    Title_str = "";
    Desc_str= "";

    for(j=0; j < len ; j++) 
        if ((j == 0) || (full_str[j-1] == '\0')) {
	    switch (full_str[j]) {
	    case 'u':
	      Url_str = full_str + j + 2;
	      break;
	    case 't':
	      Title_str = full_str + j + 2;
	      break;
	    case 'd':
	      Desc_str = full_str + j + 2;
	      break;
	    default:
	      /* cerr << "Unexpected field: " << full_str[j] << "\n"; */
	      break;
	    }
	}
    
#if 1
    site->url = Url_str;
    site->title = Title_str;
    site->desc = Desc_str;
#else
    site->url = str_save(Url_str);
    site->title = str_save(Title_str);
    site->desc = str_save(Desc_str);
#endif
    
    return;
}

/*
 * Prints out the html code for a new search href. This does not close the href
 * allowing the caller to add some more CGI arguments.
 *
 * WARNING: excessive use of global variables. Geck!
 */
void print_search_href() {

    cout << "<a href=\"" << prog_name << "?search=";
    print_extra_escaped_url(c_url_query_str);  
    
    if (category) {
	if (restrict)
	    cout << "&all=no";
	if (category)
	    cout << "&cat=" << category;
    }
    if (utf8)
	cout <<"&utf8=1";
    
    if (Kids) {
	if (Kids | KID_SITE)
	    cout <<"&Kids=1";
	if (Kids | TEEN_SITE)
	    cout <<"&Teens=1";
	if (Kids | MTEEN_SITE)
	    cout <<"&Mteens=1";
    }

    if (Locale) {
	cout << "&locale=" << Locale;
    }

}
    
void show_sites(int start, int last, int total)
{
    int i;
    struct site_node *p;
    int first = 1;

    for (i = 0; (i < sites_top) && (i <= (last - start +1)); i++) {

	if (first) {
	    cout << "<font size=\"+1\"><b>Open Directory " <<
		"Sites</b></font> (" << start << "-" <<
		last << " of " << total << ")<p>\n";
	    
	    cout << "<ol start=" << start << ">\n";
	    
	    first = 0;
	}
	
	cout << "<li>";
	
	if (sites[i]->url == NULL) { 
	    lookup_site_info(sites[i]);
	}

	
	cout << "<a href=\"" << sites[i]->url;
	/* print_extra_escaped_url(sites[i]->url); */
	cout << "\">";
	
	cout << bold(sites[i]->title) << "</a>";
	if (sites[i]->cool) {
	    cout << "<img src=\"http://dmoz.org/img/star.gif\" width=15 height=16 alt=\"Editor's Choice\"> &nbsp; ";
	}
	
	if (sites[i]->desc && *(sites[i]->desc)) {
	    cout << " - " << bold(sites[i]->desc);
	}
	
	cout << "<br><small><i>-- " <<
	    bold(sites[i]->url);
	
	cout << " &nbsp; ";
	
	print_nice(sites[i]->cat->path);
	
	cout << "</i>";
	
	if (sites[i]->cat->n_sites_here + sites[i]->cat->n_subcat_sites_here > 0) {
	    cout << " &nbsp; <i>(";

	    /*
	     * for sites in the second[] array we use the jsites, the others
	     * use category restrict.
	     */
	    char *tmp= category;
	    int  tmp2 = restrict;
	    category = sites[i]->cat->path;
	    restrict = TRUE;
	    
	    print_search_href();
	    
	    category = tmp;
	    restrict = tmp2;
	    
	    cout << "\">" << sites[i]->cat->n_sites_here + sites[i]->cat->n_subcat_sites_here
		 << "</a>";

	    if (first_match_flag) {
		if (sites[i]->cat->n_sites_here + sites[i]->cat->n_subcat_sites_here == 1)
		    cout << " match";
		else
		    cout << " matches";
		first_match_flag = 0;
	    }
	    cout << ")</i>";

	}
	
	cout << "</small>";
	cout << "<p>\n";
    }
    
    if (!first) {
	cout << "</ol><p>\n";
    }
}

void more_cats_search(int more)
{
	cout << "<blockquote>[ ";
	
	print_search_href();

	cout << "&morecat=" << more;

	cout << "\">more...</a> ]</blockquote>\n";
}

void show_jsites(int jsites)
{
	struct site_node *p;
	char *path, *npath;
	int others;

	cout << "<font size=\"+1\"><b>";
	path = cats[jsites]->path;
	print_nice(cats[jsites]->path);
	cout << "</b></font> &nbsp; ";

	cout << "(" << cats[jsites]->n_sites_here + cats[jsites]->n_subcat_sites_here;
	if ((cats[jsites]->n_sites_here + cats[jsites]->n_subcat_sites_here)== 1)
		cout << " match";
	else
		cout << " matches";

	cout << ")<p><ul>\n";

	for (p = cats[jsites]->sites_here; p; p = p->next) {

	        npath = p->cat->path;

		if (npath != path) {
		    path = npath;

		    cout << "</ul><p>\n";

		    cout << "<font size=\"+1\"><b>";
		    print_nice(path);
		    cout << "</b></font><p><ul>\n";
		    
		}
	    

		if (p->url == NULL) { 
		    lookup_site_info(p);
		}

		cout << "<li>";

		cout << "<a href=\"" << p->url;
		/* print_extra_escaped_url(p->url);  */
		cout << "\">" << bold(p->title) << "</a>\n";

		if (p->cool) {
		  cout << "<img src=\"/img/star.gif\" width=15 height=16 alt=\"\"> &nbsp; ";
		}

		if (p->desc && *(p->desc)) {
			cout << " - " << bold(p->desc);
		}

	}

	cout << "</ul><p>\n";

	cout << "<b>Go to the <a href=\"http://dmoz.org/";
	print_extra_escaped_url(cats[jsites]->path);
	cout <<	"/\">category page</a></b>";

}

void disp(int morecat, int start, int jsites, char *cat, int site_hits)
{
	int i;
	int tm = 0;
	int first = 1;
	int last;
	int limit;
	extern char *c_clean_query_str;

	if ((cats_top == 0) && (num_leftovers == 0)) {
	    NoMatches(NULL);
	    return;
	}
	
        PutHTTPHeader();
	PutHTMLHead(0,0,0);
	PutHTMLBodyStart();

	/* set the category's score */
	for (i = 0; i < cats_top; i++) {
	  int d = cats[i]->depth + cats[i]->anti;

	  cats[i]->cat_score = (cats[i]->rank + cats[i]->site_rank ) / 
	      max(1, d);

	}

	qsort(cats, cats_top, sizeof(struct cat_node *), rank_comp);

	for (i = 0; i < cats_top; i++) {
	    cats[i]->catid = i;
	    second_insert(cats[i]);
	    if (second_top >= MAX_DISP)
		break;
	}

	qsort(second, second_top, sizeof(struct cat_node *), rank_comp);
	
	if (jsites >= 0) {
	    if (jsites < cats_top) {
		show_jsites(jsites);
	    }
	    return;
	}

	if (start == 1) {

	    int startcat = morecat - 1;
	    int max_show_count = CATS_PER_PAGE;
	    int show_cat_count;

	    if (startcat < 0)
		startcat = 0;

	    /* show at least 5 categories, start cuting after sites
	     * fall below the set cat rank.
	     */ 
	    show_cat_count=0;
	    while ((show_cat_count < second_top) && 
		   ((show_cat_count < 5) || (second[show_cat_count]->cat_score > CAT_THRESH))) {
		show_cat_count++;
	    }

	    if (morecat)
		limit = second_top;
	    else if (show_cat_count > 0 && show_cat_count < max_show_count )
		limit = show_cat_count;
	    else
		limit = max_show_count;
	    
	    if (limit > second_top)
		limit = second_top;

	    if (limit > startcat + MORECATS_PER_PAGE)
		limit = startcat + MORECATS_PER_PAGE;
	    
	    for (i = startcat; (i < show_cat_count) && (i < limit); i++) {

		if (first) {
		    first = 0;
		    cout << "<font size =\"+1\"><b>Open Directory " <<
			"Categories</b></font>\n";
		    
		    cout << " (" << startcat + 1 << "-" << limit <<
			" of " << show_cat_count << ")\n";
		    cout << "<ol start=" << startcat+1 << ">\n";
		}
		
		if ((i > 0) && (i % 5) == 0) {
		    cout << "<p>\n";
		}
		
		cout << "<li>";

		cout << "<b>";
		print_nice(second[i]->path);
		cout << "</b>";
		
		if (second[i]->n_sites_here + second[i]->n_subcat_sites_here> 0) {

		    cout << " &nbsp; <i>(";

#if 1
		    /*
		     * for sites in the second[] array we use the jsites, the others
		     * use category restrict.
		     */
		    char *tmp= category;
		    int  tmp2 = restrict;
		    category = second[i]->path;
		    restrict = TRUE;
		    
		    print_search_href();
		    
		    category = tmp;
		    restrict = tmp2;
		    
		    cout << "\">" << second[i]->n_sites_here + second[i]->n_subcat_sites_here
			 << "</a>";
		    
		    if (first_match_flag) {
			if (second[i]->n_sites_here + second[i]->n_subcat_sites_here == 1)
			    cout << " match";
			else
			    cout << " matches";
			first_match_flag = 0;
		    }
		    cout << ")</i>";
		    
#else
		    print_search_href();

		    cout << "&jsites=" << second[i]->catid;
		    cout << "\">" <<
			second[i]->n_sites_here + second[i]->n_subcat_sites_here << "</a>";
		    
		    if (first_match_flag) {
			if (second[i]->n_sites_here + second[i]->n_subcat_sites_here == 1)
			    cout << " match";
			else
			    cout << " matches";
			first_match_flag = 0;
		    }
		    
		    cout << ")</i>";
#endif
		}
	
		cout << "<br>\n";

	    }

	    if (!first) {
		cout << "</ol><p>\n";
	    }

	    if (i < show_cat_count) {
		if (i == 5)
		    more_cats_search(1);
		else
		    more_cats_search(i+1);
	    }
	    
	}

	if (morecat == 0) {
	    /*
	     * we're not listing just categories 
	     */

	    if (start > site_hits)
		start = site_hits;

	    if (start < 1)
		start = 1;
	    
	    last = site_hits;
	    
	    if (last > start + sites_per_page - 1)
		last = start + sites_per_page - 1;
	    
	    populate_sites(start, last);
	    
	    show_sites(start, last, site_hits);

	    if ((start > 1) || (start+ SITES_PER_PAGE <= site_hits)) {
		cout << "<center>";
		
		if (start > 1) {
		    cout << "&nbsp; ";

		    print_search_href();

		    if (morecat)
			cout << "&morecat=1";

		    int newstart = start - SITES_PER_PAGE;
		    if (newstart > 1)
			cout << "&start=" << newstart;
		    cout << "\">Previous</a> &nbsp\n";
		}
	      
		if (start+SITES_PER_PAGE <= site_hits) {
		    cout << "&nbsp; ";

		    print_search_href();
		    
		    cout << "&start=" << start+SITES_PER_PAGE;

		    if (morecat)
			cout << "&morecat=1";

		    cout << "\">Next</a> &nbsp;\n";
		}
		
		cout << "</center><p>\n";
	    }
	}
}


