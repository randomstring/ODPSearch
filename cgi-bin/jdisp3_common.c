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


#define DEBUG_SUBSUME 0
#define DEBUG_SUBSUME_SCORE 0

void lookup_site_info( site_node *site);


struct cat_node *cats[MAX_CAT+1];
int cats_top = 0;

struct site_node *sites[MAX_SITES];
int sites_top = 0;

int num_leftovers = 0;
struct site_node *Leftover_Sites = NULL;
struct site_node *Leftover_last = NULL;

extern char *category;
extern char **phrase_tbl;             /* array of terms, used for bolding text */
extern IDB  *site_db;      
extern IDB  *nscp_db;
extern PIRSET nscp_results;      
extern STRING clean_query_str;

struct cat_node *display[MAX_DISP];
int display_top = 0;

struct cat_node *second[MAX_DISP];
int second_top = 0;

int max_site_score = 0;

void *my_malloc(unsigned n)
{
	void *ret;

	ret = (void *) malloc(n);
	if (ret == 0) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}
	memset(ret,0, n);

	return ret;
}

char *str_save(char *s)
{
	char *t;

	if (s == 0)
		return 0;

	t = (char *) my_malloc(strlen(s) + 1);
	strcpy(t, s);
	return t;
}

int rank_comp(const void *a, const void *b)
{
 
  int diff = 0;
 
  cat_node **ca = (cat_node **) a;
  cat_node **cb = (cat_node **) b;
 
  diff = (*cb)->cat_score - (*ca)->cat_score;

  return diff;
}
 
#define DEBUG_POPULATE  0

void populate_sites(int start, int last)
{
    int i;
    int count = 0;    /* how many sites we've scanned so far */
    int advanced;
    struct site_node *s = Leftover_Sites;

    for (i = 0; i < second_top; i++)
	second[i]->out_p = second[i]->sites_here;

    /* take 1 or 2 top sites from the second[] (AKA displayed categories) array first */
    for (i = 0; (i < second_top) && (second[i]->cat_score >= CAT_THRESH); i++) {
	if ((sites_top >= MAX_SITES) || (count >= last))
	    return;
	
	if (second[i]->out_p) {
	    if (start -1 <= count) {
		sites[sites_top++] = second[i]->out_p;
#if DEBUG_POPULATE
		cerr << "SITE[" << sites_top << "] " << count << " TOP           " << sites[sites_top-1]->cat->path << "\n";
#endif
	    }
	    count++;
	    second[i]->out_p = second[i]->out_p->next;
	}
	
	/* take 2 if they're cool */
	
	if (second[i]->out_p && second[i]->out_p->cool) {
	    if (start - 1 <= count) {
		sites[sites_top++] = second[i]->out_p;
#if DEBUG_POPULATE
		cerr << "SITE[" << sites_top << "] " << count << " TOP 2nd COOL  " << sites[sites_top-1]->cat->path << "\n";
#endif
	    }
	    count++;
	    second[i]->out_p = second[i]->out_p->next;
	}
	
    }
    
    /* iterate over all the categories until the sites are all gone */
    advanced = 1;
    while(advanced) {
	advanced=0;
	for (i = 0; i < second_top; i++) {
	    if ((sites_top >= MAX_SITES) || (count >= last ))
		return;

	    if (second[i]->out_p) {
		if (start - 1<= count) {
		    sites[sites_top++] = second[i]->out_p;
#if DEBUG_POPULATE
		    cerr << "SITE[" << sites_top << "] " << count << " REST          " << sites[sites_top-1]->cat->path << "\n";
#endif
		}
		advanced =1;
		count++;
		second[i]->out_p = second[i]->out_p->next;
	    }
	    
	    /* take 2 if cool */
	    if (second[i]->out_p && second[i]->out_p->cool) {
		if (start <= count) {
		    sites[sites_top++] = second[i]->out_p;
#if DEBUG_POPULATE
		    cerr << "SITE[" << sites_top << "] " << count << " REST 2nd COOL " << sites[sites_top-1]->cat->path << "\n";
#endif
		}
		advanced =1;
		count++;
		second[i]->out_p = second[i]->out_p->next;
	    }
	}

	/* Take one site from the leftover list */
	if( s && (sites_top < MAX_SITES) && (count < last)) {
	    if (start <= count) {
		sites[sites_top++] = s;
#if DEBUG_POPULATE
		cerr << "SITE[" << sites_top << "] " << count << " LEFTOVER      " << sites[sites_top-1]->cat->path << "\n";
#endif
	    }
	    /* don't set advanced flag, we want to drop out of this slow loop if
	     * there aren't any more second[] sites.
	     */
	    count++;
	    s = s->next;
	}

    }
    
    /* now grab all the leftover sites */
    while (s && (sites_top < MAX_SITES) && (count < last)) {
	if (start <= count) {
	    sites[sites_top++] = s;
#if DEBUG_POPULATE
	    cerr << "SITE[" << sites_top << "] " << count << " LEFTOVER      " << sites[sites_top-1]->cat->path << "\n";
#endif
	}
	count++;
        s = s->next;
    }
    
    return;
}

int subsume(struct cat_node *a, struct cat_node *b)
{
 
        if (b->cat_match && !(a->cat_match))
                return 0;
 
        char *s = a->path;
        char *t = b->path;
 
        while (*s == *t) {
                s++;
                t++;
        }
 
        if (*t == '/' && ((*s == 0) || (*s == '/')))
                return 1;
        return 0;

}

void do_subsume(struct cat_node *parent, struct cat_node *child) 
{

    if (parent->maxscore < child->maxscore) {
	parent->maxscore = child->maxscore ;
    }
    
    if (parent->lastp && parent->sites_here) {
	parent->lastp->next = child->sites_here;
	if (child->lastp) 
	    parent->lastp = child->lastp;
    }
    else {
	parent->sites_here = child->sites_here;
	parent->lastp = child->lastp;
    }

    parent->n_subcat_sites_here += child->n_sites_here + child->n_subcat_sites_here;

    parent->rank += child->rank;
    parent->site_rank += child->site_rank;
    parent->cat_score += child->cat_score;

#if DEBUG_SUBSUME
    cerr << "DOSUBSUME: p=" << parent->path  << " (" << parent->n_sites_here 
	 << "," << parent->n_subcat_sites_here << ")"
	 << "  c=" << child->path << " (" << child->n_sites_here << "," 
	 << child->n_subcat_sites_here << ")\n";
#endif
    
    return;
}


void second_insert(struct cat_node *cat)
{
        int i;
 
        for (i = 0; i < second_top; i++) {
 
	    /* at least check to see if they are in the same top level cat */
	    if (second[i]->path[0] == cat->path[0]) {
	    
		/* we don't subsume matched categories into unmached ones */
		if (!(cat->cat_match) || (second[i]->cat_match))

		    if (subsume(second[i], cat)) {
		    
			/* cerr << "SECOND INSERT [0]\n"; */
			
			do_subsume(second[i],cat);
			return;
		    }
 
		/* we don't subsume matched categories into unmached ones */
		if (!(second[i]->cat_match) || (cat->cat_match))
		    if (subsume(cat, second[i])) {
			
			/* cerr << "SECOND INSERT [1]\n"; */
			
			do_subsume(cat, second[i]);
			second[i] = cat;
			return;
		    }
	    }
        }

	/* Grab at least 25 categories, if possible. 
	 * Then start dropping them if the cat score is less than CAT_ADD_THRESH.
	 * Dropped cats get their sites put in the leftover list.
	 */
	if ((second_top >= MAX_DISP) || 
	    ((second_top > 25) && (cat->cat_score < CAT_ADD_THRESH))) {

	    if (cat->sites_here && cat->lastp) {
		num_leftovers += cat->n_sites_here + cat->n_subcat_sites_here;

		if (Leftover_Sites && Leftover_last) {
		    Leftover_last->next = cat->sites_here;
		    Leftover_last = cat->lastp;
		}
		else {
		    Leftover_Sites = cat->sites_here;
		    Leftover_last = cat->lastp;
		}
	    }
	    
	    return;
	}
  
        second[second_top++] = cat;
}
