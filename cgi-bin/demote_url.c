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
 * demote_url.c  - build a table of category promotions/demotions that
 *                 allow teh overall category scores to be adjusted
 *                 based on global preferences, adult material, and/or
 *                 localized content.
 *
 *  This file contains defaults used by the Open Directory Project.
 * Search looks in the directory set by BIASLIST_PATH for files with
 * the files base set by BIASLIST_PREFIX to override these internal 
 * defaults.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <ctype.h>

#include "demote_url.h"
#include "newhoo.h"

#define MAX_BIASED_CATS   100
#define MAX_LOCALES       100

#define DEBUG_BIAS 0

bias_list_struct* load_category_biaslist(char *bias_name,char *locale);

/* The two bias lists are additive. */

bias_list_struct default_bias_list[] = {
      {  5, "Adult",						+10},
      { 17, "Society/Sexuality",				+5},
      { 0, 0, 0}
};


bias_list_struct lbias_list[10];

/*    Country/Languages:  U.K., France, Germany, Australia, Japan, Italy, Netherlands,
 *                        Sweden, Denmark, Spain, Latin America, Brazil / Portugal,
 *                        Korea 
 */

/* default english*/
bias_list_struct en_list[] = {
      {  5, "World",						+5},
      {  0, 0, 0}
};

/* un-bias list for adult categories */
bias_list_struct adult_list[] = {
      {  5, "Adult",						-10},
      { 17, "Society/Sexuality",				-5},
      {  5, "World",						+1},
      {  0, 0, 0}
};

/* U.K. */
bias_list_struct uk_list[] = {
      { 30, "Regional/Europe/United_Kingdom",			-3},      
      {  5, "World",						+2},
      {  0, 0, 0}
};

/* France */
bias_list_struct fr_list[] = {
      { 10, "World/Fran",					-3},
      { 22, "Regional/Europe/France",				-2},
      {  5, "World",						+2},
      {  0, 0, 0}
};

/* Germany */
bias_list_struct de_list[] = {
      { 13, "World/Deutsch",					-3},
      { 23, "Regional/Europe/Germany",				-2},
      {  5, "World",						+2},
      { 0, 0, 0}
};

/* Austrailia */
bias_list_struct au_list[] = {
      { 26, "Regional/Oceania/Australia",			-2},
      {  5, "World",						+2},
      { 0, 0, 0}
};

/* Canada */
bias_list_struct ca_list[] = {
      { 26, "Regional/North_America/Canada",  			-2},
      {  5, "World",						+2},
      { 0, 0, 0}
};

/* Japan */
bias_list_struct ja_list[] = {
      { 14, "World/Japanese",					-3},
      { 19, "Regional/Asia/Japan",				-2},
      {  5, "World",						+2},
      {  0, 0, 0}
};

/* Italy */
bias_list_struct it_list[] = {
      { 14, "World/Italiano",					-3},
      { 21, "Regional/Europe/Italy",				-2},
      {  5, "World",						+2},
      {  0, 0, 0}
};

/* Netherlands */
bias_list_struct nl_list[] = {
      { 17, "World/Netherlands",				-3},
      { 27, "Regional/Europe/Netherlands",			-2},
      {  5, "World",						+2},
      {  0, 0, 0}
};

/* Sweden */
bias_list_struct se_list[] = {
      { 13, "World/Svenska",					-3},
      { 22, "Regional/Europe/Sweden",				-2},
      {  5, "World",						+2},
      {  0, 0, 0}
};

/* Denmark */
bias_list_struct dk_list[] = {
      { 11, "World/Dansk",					-3},
      { 23, "Regional/Europe/Denmark",				-2},
      {  5, "World",						+2},
      {  0, 0, 0}
};

/* Spain*/
bias_list_struct es_list[] = {
      { 10, "World/Espa",					-3},
      { 23, "Regional/Europe/Spain",				-2},
      {  5, "World",						+2},
      {  0, 0, 0}
};

/* Latin America*/
bias_list_struct latin_list[] = {
      { 24, "Regional/Central_America",				-2},
      { 22, "Regional/South_America",				-2},
      { 29, "Regional/North_America/Mexico",			-3},
      { 10, "World/Espa",       				-1},
      {  5, "World",						+2},
      {  0, 0, 0}
};

/* Brazil */
bias_list_struct br_list[] = {
      { 29, "Regional/South_America/Brazil",			-2},
      {  5, "World",						+2},
      {  0, 0, 0}
};

/* Portugal */
bias_list_struct pt_list[] = {
      { 13, "World/Portugu",					-3},
      { 24, "Regional/Europe/Portugal",				-2},
      {  5, "World",						+2},
      {  0, 0, 0}
};

/* Russia */
bias_list_struct ru_list[] = {
      { 13, "World/Russian",					-3},
      { 22, "Regional/Europe/Russia",				-2},
      {  5, "World",						+2},
      {  0, 0, 0}
};

/* China */
bias_list_struct zh_list[] = {
       /* covers Chinese_Traditional & Chinese_Simplified */
      { 12, "World/Chinese",					-3},  
      { 19, "Regional/Asia/China",				-2},
      {  5, "World",						+2},
      {  0, 0, 0}
};

/* Korea */
bias_list_struct ko_list[] = {
      { 12, "World/Korean",					-3},
      { 25, "Regional/Asia/South_Korea",			-2},
      { 25, "Regional/Asia/North_Korea",			-2},
      {  5, "World",						+2},
      {  0, 0, 0}
};

/* Generic World Promotion */
bias_list_struct world_list[] = {
      {  5, "World",						-2},
      {  0, 0, 0}
};

/*
 * The list of biased lists based on locality, lookup by category
 */
bias_list_struct* locality_bias_lists[] = {
    uk_list,
    au_list,
    fr_list,
    de_list,
    ja_list,
    it_list,
    nl_list,
    se_list,
    dk_list,
    es_list,
    latin_list,
    br_list,
    pt_list,
    ko_list,
    zh_list,
    ru_list,
    world_list,  /* must be last in World category list */
    NULL
};

/*
 * The list of biased lists based on locality, lookup by Local
 */

locale_bias_list_struct locales_bias_list[MAX_LOCALES] = {
    { "en_us", en_list},
    { "en_gb", uk_list},
    { "en_au", au_list},
    { "en_ca", ca_list},
    { "en", en_list},
    { "fr_ca", ca_list},   /* French Canadian = Quebec */
    { "fr", fr_list},
    { "ru", ru_list},
    { "de", de_list},
    { "ja", ja_list},
    { "it", it_list},
    { "nl", nl_list},
    { "se", se_list},
    { "dk", dk_list},
    { "es", es_list},
    { "br", br_list},
    { "pt", pt_list},
    { "zh", zh_list},     /* chinese */   
    { "ko", ko_list},
    { "", world_list},
    { NULL, NULL }
};


/*
 * Copy the bias list from b into a. 
 */
void cpBiasList(bias_list_struct *a, bias_list_struct *b) {
    int i=0;

    while ((b[i].path) && i < MAX_BIASED_CATS) {
	a[i].length = b[i].length;
	a[i].path   = b[i].path;
	a[i].level  = b[i].level;
	i++;
    }

    a[i].length = 0;
    a[i].path   = NULL;
    a[i].level  = 0;
}


/*
 * Join the bias list together, adding the bias values together.
 */
bias_list_struct *joinBiasList(bias_list_struct *a, bias_list_struct *b) {

    int i,j,k;
    int matched;
    bias_list_struct *join;

    if (a == NULL)
	return b;

    if (b == NULL)
	return a;

    join = (bias_list_struct *) malloc ((MAX_BIASED_CATS + 1) * sizeof(bias_list_struct));

    if (join == NULL) 
	return NULL;

    k = 0;
    for (i = 0; (a[i].path != NULL) && (i < MAX_BIASED_CATS); i++) {
	matched = 0;
	for (j = 0; (b[j].path != NULL) && !matched && (j < MAX_BIASED_CATS);) {
	    if ((a[i].length == b[j].length) &&	(strcmp(a[i].path,b[j].path) == 0)) {
		b[j].length = 0;  /* mark the b items as matched */
		matched = 1;
	    }
	    else {
		j++;
	    }
	}
	join[k].level  = a[i].level;
	join[k].path   = a[i].path;
	join[k].length = a[i].length;
	if (matched) {
	    /* join the two level adjustments */
	    join[k].level += b[j].level;
	}
	k++;
    }
    for (j = 0; (b[j].path != NULL) && (j < MAX_BIASED_CATS) && (k < MAX_BIASED_CATS); j++) {
	if (b[j].length > 0) {
	    join[k].level  = b[j].level;
	    join[k].path   = b[j].path;
	    join[k].length = b[j].length;
	    k++;
	}
    }

    join[k].level  = 0;
    join[k].path   = NULL;
    join[k].length = 0;

    return join;
}

/*
 * What SetBiasdList does:
 *
 * 0.  load default bias list  
 *    a. look for file /gh/search/biaslist_default.cfg
 *    b. use the built in default 
 * 1.  check locale, if not in the "en" locale
 *    a. look for bias list file /gh/search/biaslist_<locale>.cfg
 *    b. if not use built in default
 * 2. if Adult is OK, zero out the /Adult bias
 *
 */


bias_list_struct* SetBiasedList(char *locale, char * cat, int adult_ok) {

    bias_list_struct *bias_list;            /* stock bias list */
    bias_list_struct *locale_bias_list;     /* locale based bias list */
    bias_list_struct *adult_bias_list;      /* adult based bias list */
    bias_list_struct *join;
    bias_list_struct *blist;

    bias_list = NULL;
    locale_bias_list = NULL;
    adult_bias_list = NULL;

    /* steps 0a & 0b */
    if ((bias_list = load_category_biaslist("default","")) == NULL) {
	bias_list = (bias_list_struct *) malloc ((MAX_BIASED_CATS + 1) * sizeof(bias_list_struct));
	cpBiasList(bias_list,default_bias_list);
    }


    if (locale == NULL) {

	/* step 0.5 try to guess the locale from the category */

	/* check for searches from World or Regional categories*/
	if (cat && ((cat[0] == 'W') || ((cat[0] == 'R') && (cat[2] == 'g')))) {
	    int i = 0;
	    while((i < MAX_LOCALES) && (blist = locality_bias_lists[i++])) {
		int j = 0;
		while ((j < MAX_LOCALES) && (blist[j].level < 0)) {
		    if (strncmp(cat, blist[j].path, blist[j].length) == 0) {
			locale_bias_list = (bias_list_struct *) malloc ((MAX_BIASED_CATS + 1) * sizeof(bias_list_struct));
			cpBiasList(locale_bias_list, blist);
#if DEBUG_BIAS
			cerr << "Guessed locale from category [" << cat << "]";

			for(int k=0; (k < MAX_LOCALES) && (locales_bias_list[k].locale_name); k++) { 
			    if (locales_bias_list[k].bias_list == blist) {
				cerr << " locale = [" << locales_bias_list[k].locale_name << "]";
				k = MAX_LOCALES;
			    }
			}
			cerr << "\n"; 
			for(int k=0; (k < MAX_BIASED_CATS) && (locale_bias_list[k].path != NULL); k++) {
			    cerr << locale_bias_list[k].length << ", \"" 
				 << locale_bias_list[k].path << "\", " 
				 << locale_bias_list[k].level << ",\n";
			} 
#endif
			j = MAX_LOCALES;
			i = MAX_LOCALES;
		    }
		    j++;
		}
	    }
	}
    }

    if (locale != NULL) {

	/* step 1a and 1b */

	if ((locale_bias_list = load_category_biaslist("locale-",locale)) == NULL) {
	    /* no locale file, use compiled in default*/
	    int i = 0;
	    while ((i < MAX_LOCALES) && (locales_bias_list[i].locale_name != NULL)) {
		if (strcmp(locales_bias_list[i].locale_name, locale) == 0) {
		    locale_bias_list = (bias_list_struct *) malloc ((MAX_BIASED_CATS + 1) * sizeof(bias_list_struct));
		    cpBiasList(locale_bias_list, locales_bias_list[i].bias_list);
		    i = MAX_LOCALES;
		}		    
		i++;
	    }
	}
    }

    /* step 2, check for searches from Adult */
    if (adult_ok || (cat && (cat[0] == 'A') && (cat[1] == 'd'))) {
	if ((adult_bias_list = load_category_biaslist("adult","")) == NULL) {
	    adult_bias_list = adult_list;
	}
	bias_list = joinBiasList(bias_list,adult_bias_list);
    }

    join = joinBiasList(bias_list,locale_bias_list);

#if DEBUG_BIAS
    cerr << "The joined bias list:\n";
    for(int i = 0; (i < MAX_BIASED_CATS) && (join[i].path != NULL); i++) {
	cerr << join[i].length << ", \"" <<  join[i].path << "\", " << join[i].level << ",\n";
    } 
#endif

    return(join);

}

/*
 * load_category_biaslist()
 *
 * Read a bias list from a file. This allows the category bias to be changed
 * without recompiling.
 *
 * File Format:
 *   <int:BiasLevel>\t<string:CategoryName>\n
 *
 */

bias_list_struct* load_category_biaslist(char *bias_name, char *locale) {

  char *filename;
  int fd = -1;
  int file_length = 0;
  char *buffer;
  bias_list_struct* blist = NULL;
  int i,j,start_pos,bias,slen;
  char *str;

  if (bias_name) {
    filename = (char *)malloc(strlen(BIASLIST_PATH) +
			      strlen(BIASLIST_PREFIX) + 
			      strlen(bias_name) + 
			      strlen(locale) + 
			      strlen(BIASLIST_POSTFIX) + 3);

    if (filename) {

      sprintf(filename,"%s%s%s%s%s",BIASLIST_PATH, BIASLIST_PREFIX, bias_name, locale, BIASLIST_POSTFIX);
      
      if ((fd = open(filename,O_RDONLY)) > 0) {
	free(filename);
	if (file_length = lseek(fd,0L, SEEK_END)) {

#if DEBUG_BIAS
	   cerr << "Using Bias List file: [" << filename << "]\n";
#endif
	   if (buffer = (char *) mmap(NULL, file_length ,PROT_READ, MAP_SHARED, fd, 0)) {
	       /* parse category bias list */
	       if (blist = ( bias_list_struct*) malloc((MAX_BIASED_CATS + 1) * sizeof(bias_list_struct))) {
	      
		  i = 0;
		  j = 0;
		  while (( i < (file_length-1)) && ( j < MAX_BIASED_CATS)) {

		      /* assertion: we're at the beginning of a line, just after a \n char */

		      /* Skip lines that begin with the # character */
		      while ((i < (file_length - 1)) && (buffer[i] == '#')) {
			  i++;
#if DEBUG_BIAS
			  cerr << "#";
#endif
			  while ((i < (file_length - 1)) && (buffer[i++] != '\n')) {
#if DEBUG_BIAS
			      cerr << buffer[i-1];
#endif
			  }
#if DEBUG_BIAS
			  cerr << "\n";
#endif
		      }
		      
		      if (i < (file_length - 1)) {
			  bias = atoi(buffer + i);
			  while ((i < file_length) && (buffer[i] != '\t')) {
			      i++;
			  }
			  i++;
			  /* read string */
			  if (i < file_length) {
			      str = buffer + i;
			      start_pos = i;
			  }
			  else {
			      str = NULL;
			  }

			  /* find the end of the category name */
			  while ((i < (file_length - 1)) && !isspace(buffer[i]) && (buffer[i] != '\n')) {
			      i++;
			  }
			  
			  if (str && isalnum(str[0]) && (bias != 0)) {
			      slen = i - start_pos;
			      blist[j].length = slen;
			      blist[j].level  = bias;
			      blist[j].path   = (char *) malloc(slen + 1);
			      
			      if (blist[j].path) {
				  strncpy(blist[j].path, str, slen);
				  blist[j].path[slen] = '\0';
			      }
			      
#if DEBUG_BIAS
			      cerr << slen << ", \"" <<  blist[j].path << "\", " << bias << ",\n";
#endif
			      
			      j++;
			  }

			  /* find the end of the category line */
			  while ((i < (file_length - 1)) && (buffer[i] != '\n')) {
			      i++;
			  }
			  
			  i++;
		      }
		  }
			  
			  
		  blist[j].length = 0;
		  blist[j].level  = 0;
		  blist[j].path   = NULL;
		  
		  close(fd);
		  return(blist);
	      }
	  }
	}
	close(fd);
      }
    }
  }

  /* We failed, return empty handed. */
  return NULL;
}



