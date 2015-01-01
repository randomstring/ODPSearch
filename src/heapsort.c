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
 * Implementation of heapsort.
 * 
 * Compared to qsort():
 *   Pros: guaranteed n*log(n) sort time
 *   Cons: ave. time is worse than qsort() 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "defs.h"
#include "common.h"
#include "dtreg.h"
#include "iresult.h"
#include "index.h"

#define PARENT(x) (x>>1)
#define LEFT(x)   (x<<1)
#define RIGHT(x)  ((x<<1)+1)

int WordCompare(const void* x, const void* y);

int heapify(index_entry *base, int i, size_t heap_size)
{  
  int l,r,largest,heapmore;
  index_entry ltmp;

  heapmore = TRUE;
  
  while (heapmore) {
      heapmore = FALSE;

      l = LEFT(i);
      r = RIGHT(i);
      
      //printf("HEAPIFY: %3d   -> l(%d) r(%d)\n",i,l,r);
      
      if ((l <= heap_size) && 
	  (0 < WordCompare(&(base[l-1]), &(base[i-1])))) {
	largest = l;
	// printf("Largest is LEFT:  %3d  :  l(%d) < i(%d)\n",largest,l,i);
      }
      else {
	largest = i;
	// printf("Largest is I:     %3d  :  l(%d) >= i(%d)\n",largest,l,i);
      }
      
      if ((r <= heap_size) && 
	  (0 < WordCompare(&(base[r-1]),&(base[largest-1])))) {
	largest = r;
	// printf("Largest is RIGHT: %3d  :  r(%d) > l(%d)\n",largest,r,largest);
      }
      
      if (largest != i) {
	/* swap record 0 and i (heap_exchange(i,largest)) */
	ltmp = base[i-1];
	base[i-1] = base[largest-1];
	base[largest-1] =ltmp;
	
	// heapify(base, largest, heap_size);
	i = largest;
	heapmore = TRUE;
      }
      
  }

  return 1;
}


int heapsort(index_entry *base, size_t heap_size)
{

  int i;
  index_entry ltmp;
  size_t records = heap_size;

  // buildHeap(base,records,size);
  for(i = (heap_size >> 1); i > 0; i--) {
      heapify(base,i,heap_size);
  }

  // printf("HEAPSORT START\n");

  for(i = heap_size; i > 1; i--) {
      /* swap record 0 and i */
      ltmp = base[0];
      base[0] = base[i-1];
      base[i-1] = ltmp;

      records--;
      heapify(base, 1, records);
  }

  return 1;
}

