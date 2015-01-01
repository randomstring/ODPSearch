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
 * catlookup.c  - lookup the aol shopping category id given the 
 *                category string
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include "prettymap.h"

typedef struct CATMAPTABLE {
  char *cat;
  unsigned int id;
} CatMapTable;

#define CATMAP_FILE "/aolshop/meta-data/CatMap.txt"
#define CATMAP_MAX_SIZE 5000

static CatMapTable CatMap[CATMAP_MAX_SIZE];
static FILE *CatMapFd = NULL;
static int NumCats = 0;

int isAdultCat(int catid) {

  // 251789312 is Movies & Videos > Adult 
  // 67174912 is Books > Adult 
  // 67240192 is Catalogs > Adult 
  // 67305728 is Magazines > Adult 
  // 167837952 is Food/Drink/Gourmet > Beverages > Beer 
  // 167838464 is Food/Drink/Gourmet > Beverages > Liqueurs & Spirits 
  // 167839232 is Food/Drink/Gourmet > Beverages > Wine 
  // minorcategoryrestrictor=251789312,67240192,67240192,67305728,167837952,167838464,167839232 

  switch(catid) {
  case 67174912:   // Books > Adult
  case 67240192:   // Catalogs > Adult
  case 67305728:   // Magazines > Adult 
  case 167837952:  // Food/Drink/Gourmet > Beverages > Beer
  case 167838464:  // Food/Drink/Gourmet > Beverages > Liqueurs & Spirits 
  case 167839232:  // Food/Drink/Gourmet > Beverages > Wine 
  case 251789312:  // Movies & Videos > Adult
    return 1;
  }

  return 0;

}

int 
CatComp(const void* x, const void* y) {

  return(strcmp(((CatMapTable *)x)->cat,((CatMapTable *)y)->cat));

}

int 
CatIdComp(const void* x, const void* y) {

  return(((PrettyMap *)x)->id - ((PrettyMap *)y)->id);

}

void ReadCatMapTable() {

  int i,j,done;
  char *cat;
  char cat_id_str[64];
  unsigned int cat_id;

  if (CatMapFd == NULL) {
    CatMapFd = fopen(CATMAP_FILE,"r");
    if (CatMapFd == NULL) {
      perror(CATMAP_FILE);
      printf("ERROR: can't open catmap file %s\n",CATMAP_FILE);
      exit(44);
    }
  }

  i = 0;
  done = 0;
  while((i < CATMAP_MAX_SIZE) && !done) {
    cat = (char *) malloc(1024);
    j = 0;
    cat[j] = getc(CatMapFd);
    
    if (cat[j] == EOF) 
      done = 1;
    else {
      
      while (cat[j] != '\t') {
	j++;
	cat[j] = getc(CatMapFd);
      }
      cat[j] = '\0';
      if ((j > 0) && (cat[j-1] == '/')) 
	cat[j-1] = '\0';

      
      j = 0;
      cat_id_str[j] = getc(CatMapFd);
      while ((j < 63) && cat_id_str[j] != '\n') {
	j++;
	cat_id_str[j] = getc(CatMapFd);
      }
      cat_id_str[j] = '\0';
      
      CatMap[i].cat = cat;
      CatMap[i].id  = atoi(cat_id_str);
      
      // printf("%s  ->  %d\n",CatMap[i].cat,CatMap[i].id);

      i++;
    }
  }

  NumCats = i;

  qsort((void *)CatMap, 
	NumCats, sizeof(CatMapTable), 
	CatComp);

}

void CloseCatMapTable() {

  fclose(CatMapFd);

  CatMapFd = NULL;
}


unsigned int ShoppingCatLookup(char *cat) {

  CatMapTable *c;
  CatMapTable x;

  x.id = 0;
  x.cat = cat;

  if (NumCats == 0) {
    ReadCatMapTable();
    CloseCatMapTable();
    if (NumCats == 0) 
      return 0;
  }

  c = (CatMapTable *)bsearch(&x,(void *)CatMap, 
			     NumCats, sizeof(CatMapTable), 
			     CatComp);

  if (c)
    return c->id;

  return 0;
  
}

const char *ShoppingCatIdLookup(int id) {

  PrettyMap *c;
  PrettyMap x;

  x.id = id;
  x.shortname = NULL;

  c = (PrettyMap *)bsearch(&x, (void *)prettyMap,
		           NUM_PRETTY_NAMES, sizeof(PrettyMap), 
			   CatIdComp);

  if (c)
    return c->shortname;

  return 0;
  
}

#if 0

int main() {

  unsigned int id;
  char s[1024];
  int i,c;

  ReadCatMapTable();
  CloseCatMapTable();
  
  while (1) {
    i = 0;
    while((c = getchar()) != '\n') {
      s[i++] = c;
    }
    s[i] = '\0';

    id = ShoppingCatLookup(s);
    printf("Id = %d\n",id);

    if (id == 0) {
      printf("Done.\n");
      exit(0);
    }

  }
}

#endif

