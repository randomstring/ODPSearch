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

#define		MAX_BUF		        1024
#define		WEIGHT_CAT_MATCH	2000
#define		WEIGHT_SITE_MATCH	80
#define		RANK_THRESH		20
#define		CAT_THRESH		100   /* display cat score threshold */
#define		CAT_ADD_THRESH		20    /* for adding to second[] */

#define SITES_PER_PAGE      20
#define CATS_PER_PAGE        5
#define MORECATS_PER_PAGE   25
	    
#define		MAX_CAT		        1000

/* MAXHIT_DEFAULT <= MAX_SITES */
#define		MAX_SITES               10000
#define         MAXHIT_DEFAULT          10000

#define		MAX_DISP	         500
#define         SITE_MAX_SCORE          2048

#define         ITEM_RELEVANCE_SORT     1
#define         CATEGORY_SORT           2
#define         ITEM_PRICE_SORT         3
#define         ITEM_ALPHA_SORT         4
#define         ITEM_POPULARITY_SORT    5

#define         DESCENDING_ORDE         1
#define         ASCENDING_ORDER         2

struct match_data {
      char *name;
      char *desc;
      char *brand;
      char *logo_image;
      char *product_id;
      char *merch_name;
      char *format;
      char *upc;
      char *promo_txt;
      char *mpn;
      char *isbn;
      char *aol_image_md5;
      int  shoppingcatid;
      int  quick_checkout;
      int  merch_id;
      int  minprice;
      int  maxprice;
      int  merch_num_items;
};

struct shop_data {
      char *author;
      char *artist;
      char *brand;
      char *buynow;
      char *product_id;
      char *upc;
      char *mpn;
      char *isbn;
      char *price;
      char *format;
      char *aol_uid;
      char *aol_image_md5;
      char status;
      int  cents;
      int  minprice;
      int  maxprice;
      int  merch_num_items;
};

struct cat_node;

struct site_node {
        int score;

	char *url;
	char *title;
	char *desc;

        unsigned char cool;
        int depth;
        int length;
	int globalStart;
        long fileName;

        void *data;

	struct cat_node *cat;

	struct site_node *next;
};

struct cat_node {
        int maxscore;

        int cat_score;
        int site_rank;
	int rank;
	int anti;
        int pathName;
	char *path;

        int shoppingcatid;

	struct site_node *sites_here;
	struct site_node *lastp;
	int n_sites_here;
	int r_sites_here;

        struct cat_node *subcats;
        int n_subcat_sites_here;

        int chop;
	int cat_match;
	int display_count;
	int catid;

	struct site_node *out_p;	/* output site node traversal ptr */
        int depth;
};

struct CAT_HASH_TABLE {
  int index;
  int pathName;
  struct CAT_HASH_TABLE *next;
};

void second_insert(struct cat_node *cat);
void populate_sites(int start, int last);
