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

/* hashtable.cxx
 *
 * by: Bryn Dole   Jan 5, 1999
 *
 * A simple hash table for storing indexed strings
 *
 
 *
 * 
 * Note: It is possible to mix Add() and Get() calls,
 *       but interleaving Add() and Get() calls may
 *       result in some inefficency.
 *
 */

#include "hashtable.h"
#include <stdlib.h>
#include <string.h>
#include <iostream.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define TRUE  1
#define FALSE 0

HASHTABLE::HASHTABLE() {
} 


HASHTABLE::~HASHTABLE() {
  int i;
  str_list *node, *tmp;

  if(fd >= 0)
    close(fd);

  /* XXX - freeing memory takes time */
  return;

  /* free memory */
  if (Table) {
    for(i = 0; i < tableSize; i++) {
      node = Table[i];
      while(node) {
	if (node->str)
	  free(node->str);
	tmp = node->next;
	free(node);
	node = tmp;
      }
    }
    free(Table);
  }

}

void
HASHTABLE::Init(char *fn, int size) {

  int i;

  filename = strdup(fn);
  fd = -1;
  OnDiskTable = NULL;
  mmapped = FALSE;

  /* some useful primes:
   * tableSize = 101, 1009, 10039, 100003, 500009, 1000003
   * cerr << "setting filename = [" <<filename << "]\n";
   */
  tableSize = size;

  /* allow for zero length table to disable hash table function */
  if (tableSize > 0) {
    Table = (str_list **) malloc(tableSize * sizeof(str_list *));

    /* zero the table */
    memset((void *)Table, 0, tableSize * sizeof(str_list *));
  }

}

int
HASHTABLE::ReadOnDiskHashTable() {

    /* Read a pre-existing on-disk hashtable into memory.
     * This is needed if you need to do Add()'s to an
     * existing hashtable.
     */

    /* return 0 on failure, 1 on success. */

  int str_len;
  char *temp_str;
  unsigned int hash;
  long newoffset = 0;
  str_list *node = NULL;

  if (fd < 0) {
    /* need to open file */
    fd = open(filename,O_RDONLY);
    if (fd < 0) {
        /* perror("HASHTABLE::ReadOnDiskHashTable() can't open file");
         * fprintf(stderr, "filename = [%s]\n", filename);
         */
	return 0;
    }
  }
  file_length = lseek(fd, 0, SEEK_END);
  if (file_length < 0) {
    perror("HASHTABLE:ReadOnDiskHashTable() can't lseek");
    return 0;
  }
  if (file_length <= 4) {
    /* nothing to read */
    close(fd);
    return 0;
  }
    

  OnDiskTable = (char *)mmap(NULL, file_length, PROT_READ, MAP_SHARED, fd, 0);
    
  if (OnDiskTable < 0) {
    /* mmap() failed! */
    perror("HASHTABLE:ReadOnDiskHashTable() mmap failed. ");
    return 0;
  }
  mmapped = TRUE;
  
  newoffset = sizeof(int);                   /* first few bytes are left empty */

  while (newoffset < file_length) {
    memcpy((void *) &str_len, (void *) (OnDiskTable + newoffset), sizeof(unsigned int));
    temp_str = (char*) malloc(str_len);
    if (!temp_str) {
      cerr << "HASHTABLE:ReadOnDiskHashTable() malloc failed ";
      cerr << "Error: index = " << newoffset << " string length =  " << str_len << "\n";
      return 0;
    }
    memcpy((void *) temp_str, (void *) (OnDiskTable + newoffset + sizeof(unsigned int)) , str_len);

    node = (str_list *)malloc(sizeof(str_list));
    node->str = temp_str;
    node->offset = newoffset;
    
    /* insert into memory hashtable */
    hash = Hash(temp_str);
    node->next = Table[hash];
    Table[hash] = node;

    newoffset += str_len + sizeof(int);
  }

  return (1);
}

long
HASHTABLE::Add(char *str) {

  /* Add a string to our hashtable and return the unique id. (really the
   * index to the strings location in the on disk table). We keep a
   * in memory hashtable to do fast lookups for repeated Add()'s of the
   * same string.
   */
  unsigned int hash;
  unsigned int str_len; 
  long newoffset = 0;
  str_list *node = NULL;

  if (tableSize) {
      /* check to see if this str/offet pair already exists in the hash table */
      hash = Hash(str);
      node = Table[hash];
      while (node && strcmp(str,node->str)) 
	  node = node->next;
  }

  if (!node) {
      /* check to see if we had and Get() operations that would have mmaped the file... */
      if (mmapped) {
	  close(fd);
	  fd = -1;
	  mmapped = FALSE;
      }

      /* we have a new entry, add it. */
      if (fd < 0) {
	  /* need to open file  */
	  fd = open(filename,  O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	  if (fd < 0) {
	      perror("HASHTABLE::Add() can't open file");
	      fprintf(stderr, "filename = [%s]\n", filename);
	      return 0;
	  }
      }

      /* append new string to the end of the file */
      newoffset = lseek(fd, 0, SEEK_END);
      if (newoffset < 0) {
	  /* error */
	  perror("HASHTABLE:Add() can't lseek");
	  return 0;
      }
      else if (newoffset > 0) {
	  /* the disk hash table already exists
	   * we need to load it.
	   */

	  /* XXX - do this ?
	   * ReadOnDiskHashTable();
	   */
    }
    else if (newoffset == 0) {
	/* First time opening this file */
	       
	/* Zero is the error code, so we fill the first few bytes
	 * with zeros, so that the offset zero is never used.
	 */
	str_len = 0;
	write(fd , (void *)&str_len, sizeof(unsigned int));
	
	/* lseek to the new end of file */
	newoffset = lseek(fd, 0, SEEK_END);
	if (newoffset < 0) {
	    perror("HASHTABLE:Add() can't lseek #2");
	    return 0;
	}
    }

    if (tableSize) {
	/* insert the string/offset pair into the hash in front */
	node = (str_list *) malloc(sizeof(str_list));
	node->str = strdup(str);
	node->offset = newoffset;
	node->next = Table[hash];
	Table[hash] = node;
    }

    /* write out the string length (unsigned int) */
    str_len = strlen(str) + 1;
    write(fd , (void *)&str_len, sizeof(unsigned int));
    
    /* write out the string to disk */
    write(fd, (void *)str, str_len);

    /* update current offset value */
    offset = newoffset + str_len + sizeof(int);

    file_length = offset;
  }

  if (node) 
      return (node->offset);
  else
      return (newoffset);
}

char *
HASHTABLE::Get(long i) {

    /* Take the unique id (really the index into the on-disk file) and
     * return the corresponding string.
     */
  
    /* The file has to be mmap()'d for this to work. */

  char *str;
  unsigned int str_len; 

  if ((mmapped == FALSE) && (fd >= 0)) {
      /* the file is opened, but not mmapped. We must close() and re-open() */
      close(fd);
      fd = -1;
  }

  if (fd < 0) {
      /* need to open file */
      fd = open(filename,O_RDONLY);
      if (fd < 0) {
	  perror("HASHTABLE::Get() can't open file");
	  return 0;
      }
      /* mmap file for fast easy access */
      file_length = lseek(fd, 0, SEEK_END);
      OnDiskTable = (char *)mmap(NULL, file_length, PROT_READ, MAP_SHARED, fd, 0);
      
      if (OnDiskTable < 0) {
	  /* mmap() failed! */
	  perror("HASHTABLE:Get() mmap failed.");
	  return 0;
      }
      mmapped = TRUE;
  }

  if (file_length < (i + sizeof(unsigned int))) {
      cerr << "HASHTABLE:Get() can't lookup index ";
      cerr << "Error: index " << i << " is out of bounds of mmap'ed file " << filename << "\n";
      /* cerr << "Error: file length " << file_length << " sizeof(int) " << sizeof(unsigned int) << "\n"; */
      return 0;
  }


  memcpy((void *) &str_len, (void *) (OnDiskTable + i), sizeof(unsigned int));

  if ((i + str_len) > file_length) {
      cerr << "HASHTABLE:Get() can't lookup string ";
      cerr << "Error: index = " << i << " string length " << str_len 
	   << " is out of bounds of mmap'ed file " << filename << "\n";
      return 0;
  }    

  str = (char*) malloc(str_len);

  if (!str) {
      cerr << "HASHTABLE:Get() malloc failed ";
      cerr << "Error: index = " << i << " string length =  " << str_len << "\n";
      return 0;
  }

  memcpy((void *) str, (void *)(OnDiskTable + i + sizeof(unsigned int)) , str_len);

  /* cerr << "GOT: str =[" << str <<"] len=[" << str_len << "] offset=[" << i << "]\n"; */

  return str;
}

unsigned int
HASHTABLE::Hash(char *str) {

  unsigned int hash;
  int start,stop,i;

  stop = strlen(str);
  start = 0;

  /* Only calculate hash on the last 30 characters
   * for speed. Some exerimentation showed that this was
   * enough.
   */
  start = ( ( start < (stop - 30)) ? (stop - 30) : start);

  /* seed hash with strlen()  */
  hash = stop;  
  for(i = start; i < stop; i++)
      hash = ((hash << 7) + ((unsigned int) str[i])) % tableSize;

  return hash;

}
