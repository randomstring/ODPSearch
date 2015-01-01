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
 * demote_url.h
 *
 * This is a list of URLS that have the score's demoted due to there
 * relative lack of usefullness in search results.
 */

typedef struct BIAS_LIST {
  int  length;      /* length of the string, for faster strncmp() */
  char *path;       /* the category name to be demoted */
  int level;        /* how much the score should be decreased */
} bias_list_struct;

typedef struct LOCALE_BIAS_LIST {
    char *locale_name;
    bias_list_struct *bias_list;
} locale_bias_list_struct;



bias_list_struct* SetBiasedList(char *locale, char * cat, int adult_ok);
