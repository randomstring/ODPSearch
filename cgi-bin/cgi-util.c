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
 * Description:    CGI utilities 
 * Comments:       Reads Forms data from standard input, parses out arguments.
 */

#include <string.h>
#include <ctype.h>
#include "cgi-util.h"

/* 512 + 64*/
#define INPUT_BUF_LEN 576

void CGIAPP::GetInput() {
  INT ContentLen=0, x, y, z, len, nn;
  CHR temp1[INPUT_BUF_LEN], temp2[INPUT_BUF_LEN], temp3[INPUT_BUF_LEN];
  CHR *meth, *p, *query=(CHR*)NULL;

  entry_count=0;

  if ((meth = (char *)getenv("REQUEST_METHOD"))==NULL) {
    return;
  }

  if (!strcmp(meth,"POST")) {
    if ((p=(char *)getenv("CONTENT_LENGTH")))
      ContentLen = atoi(p);
    Method=POST;
  } 
  else if (!strcmp(meth,"GET")) {
    if ((query = (char *)getenv("QUERY_STRING")) == NULL)
      query = "";
    Method=GET;
  } 
  else {
      /* No method specified. */
    return;
  }

  /* Build the list of cgi entries */
  if (Method == POST) {
    entry_count=0;
    for (x = 0; (x < CGI_MAXENTRIES) && (ContentLen>0); x++) {
      cin.getline(temp1,INPUT_BUF_LEN,'\n');
      entry_count++;
      temp1[INPUT_BUF_LEN-1] = '\0';
      len=strlen(temp1);
      ContentLen=ContentLen-(len+1);
      for (y=0; (temp1[y]!='=') && (y<len) && (y < INPUT_BUF_LEN-1);y++) {
        temp2[y]=temp1[y];
      }
      temp2[y]='\0';
      y++;
      for (z=0;(y<len) && (y < INPUT_BUF_LEN-1);y++) {
        temp3[z]=temp1[y];
        z++;
      }
      temp3[z]='\0';
      plustospace(temp3);
      unescape_string(temp3);
      name[x]=(char *) malloc(strlen(temp2)+1);
      strcpy(name[x],temp2);

      /* Trim trailing blanks off the string before we stick it into Value */
      for (nn=strlen(temp3)-1; nn >= 0 ;nn--) {
	if(temp3[nn] == ' ')
	  temp3[nn]='\0';
	else
	  break;
      }

      value[x]=(char *)malloc(strlen(temp3)+1);
      strcpy(value[x],temp3);
    }
  } else {  /* Get */
    entry_count=0;
    y=0;
    z=0;
    plustospace(query);
    /* unescape_url(query); */
    len=strlen(query);
    for (x=0;(x < CGI_MAXENTRIES ) && (y<len) && (y < INPUT_BUF_LEN-1);x++) {
      while ((query[y]!='=') && (query[y]!='&') && (y< len)) {
        temp1[z]=query[y];
        z++;
        y++;
      }
      temp1[z]='\0';
      unescape_string(temp1);
      z=0;
      if (query[y]=='=') {
        y++;
        while ((query[y]!='&') && (y<len) && (y < INPUT_BUF_LEN-1)) {
          temp2[z]=query[y];
          z++;
          y++;
        }
      }
      y++;
      temp2[z]='\0';
      unescape_string(temp2);
      z = 0; 
      if (temp2[0]=='\0') {
        name[x]=(char *)malloc(1);
        strcpy(name[x],"");

	/* Trim trailing blanks off the string before we stick it into Value */
	for (nn=strlen(temp1)-1;nn >= 0;nn--) {
	  if(temp1[nn] == ' ')
	    temp1[nn]='\0';
	  else
	    break;
	}

        value[x]=(char *)malloc(strlen(temp1)+1);
        strcpy(value[x],temp1);
      } else {
        name[x]=(char *)malloc(strlen(temp1)+1);
        strcpy(name[x],temp1);

	/* Trim trailing blanks off the string before we stick it into Value */
	for (nn=strlen(temp2)-1;nn==0;nn--) {
	  if(temp2[nn] == ' ')
	    temp2[nn]='\0';
	  else
	    break;
	}

        value[x]=(char *)malloc(strlen(temp2)+1);
        strcpy(value[x],temp2);
      }
      entry_count++;
      z=0;
    }
    entry_count = x;  
  }
}

CGIAPP::CGIAPP() {
}

void CGIAPP::Display() {
  INT x;
  for (x=0;x<entry_count;x++) {
    cout << name[x] << " = " << value[x] << "<br>\n";
  }
}

PCHR CGIAPP::GetName(INT4 i) {
  return name[i];
}

PCHR CGIAPP::GetValue(INT4 i) {
  return value[i];
}

PCHR CGIAPP::GetValueByName(const CHR *field) {
  if ((field==NULL) || (field[0]=='\0'))
    return NULL;
  INT i;
  for (i=0;i<entry_count;i++) {
    if ((name[i]==NULL)||(value[i]==NULL))
      return NULL;
    if (!strcmp(name[i], field)) {
      if (value[i] != NULL) {
        return value[i];
      }
      return NULL;
    }
  }
  return NULL;

}

PCHR CGIAPP::GetEnvByName(const char *field) {
  int   i;
  char *e;

  if ((field==NULL) || (field[0]=='\0'))
    return NULL;

  e = (char *)getenv(field);

  return (e);
}

CGIAPP::~CGIAPP() {
  int i;
  for (i=0;i<entry_count;i++) {
    if (name[i])
      free(name[i]);
    if (value[i])
      free(value[i]);
  }
}


int x2c(char* what) {
    unsigned int digit, i;
    unsigned char c; 

    digit = 0;
    for (i=0; i < 2; i++) {
      digit *= 16;
      c = (unsigned char) what[i];
      if (isxdigit(c)) {
	c = tolower(c);
	if (c >= 'a')
	  digit += c - 'a' + 10;
	else
	  digit += c - '0';
      }
      else
	  /* error, not a valid escaped character */
	  return(256);
    }

    return(digit);
}


void unescape_string(char *url) {
  INT x,y,i,l;
  l = strlen(url);
  for(x=0,y=0; (y < l) ;++x,++y) {
    if ((url[x] = url[y]) == '%') {
      i = x2c(&url[y+1]);
      if (i < 256) {
	url[x] = (unsigned char ) i;
        y+=2;
      }
    }
  }
  url[x] = '\0';
}

void unescape_url(char *url) {
  unescape_string(url);
}

void plustospace(PCHR str) {
  int x;
  for(x=0;str[x];x++)
    if(str[x] == '+')
      str[x] = ' ';
}

void spacetoplus(char *str) {
  int x;
  for(x=0;str[x];x++)
    if(str[x] == ' ')
      str[x] = '+';
}

unsigned char *c2x(unsigned char what) {
  unsigned char *out= (unsigned char *)malloc(4);
  sprintf((char *)out, "%%%2x", what);
  return out;
}

void escape_url(unsigned char *url, unsigned char *out) {
  out[0] = '\0';
  int x, y;
  unsigned char *esc;
  int plus;
  for(x=0,y=0;url[x];++x,++y) {
    if (isalnum(url[x]) || (url[x] == ' ')) {
      out[y] = url[x];
    } else {
      esc = c2x(url[x]);
      plus = strlen((char *)esc);
      out[y] = '\0';
      strcat((char *)out, (char *)esc);
      y += (plus - 1);
    }
  }
  out[y] = '\0';
  spacetoplus((char *)out);
}

/*-----------------------------------------------------------------
 * print_escaped_url()      - take a string of unsigned characters
 *                            and print them in a HTML safe format.
 *-----------------------------------------------------------------
 */ 
void print_escaped_url(char *url) {
  unsigned char buf[4]; 
  int x = 0;  

  if (url == NULL) 
    return;

  while(url[x]) {
    if ( (url[x] <= 32) || 
	 (url[x] == '<') || 
	 (url[x] == '>') ||
	 (url[x] >= 123)) {
      sprintf((char *)buf, "%%%2x", (unsigned char) url[x]);
      cout << buf;
    }
    else {
      cout << url[x];
    }
    x++;
  }
}

/*-----------------------------------------------------------------
 * print_extra_escaped_url() - take a string of unsigned characters
 *                             and print them in a HTML safe format.
 *-----------------------------------------------------------------
 */ 
void print_extra_escaped_url(char *url) {
  unsigned char buf[4]; 
  int x = 0;  
  while(url[x]) {
    if ( ((url[x] >= 'a') && (url[x] <= 'z')) || 
	 ((url[x] >= 'A') && (url[x] <= 'Z')) || 
	 ((url[x] >= '0') && (url[x] <= '9')) || 
	 (url[x] == '/') || 
	 (url[x] == '-') ||
	 (url[x] == '_') ||
	 (url[x] == ',') ||
	 (url[x] == '.')) {
      cout << url[x];
    }
    else {
      sprintf((char *)buf, "%%%2x", (unsigned char) url[x]);
      cout << buf;
    }
    x++;
  }
}


/*-----------------------------------------------------------------
 * printXMLescaped() - take a string of characters
 *                     and print them in an XML safe format.
 *-----------------------------------------------------------------
 */ 
void printXMLescaped(char *text) {
    int x = 0;  
    while(text[x]) {
	if (text[x] == '<') 
	    cout << "&lt;";
	else if (text[x] == '>') 
	    cout << "&gt;";
	else if (text[x] == '"') 
	    cout << "&quote;";
	else if (text[x] == '&') { 
	    /* do not double encode */
	    if ((strcasecmp(text+x+1,"lt;") != 0) &&
		(strcasecmp(text+x+1,"gt;") != 0) &&
		(strcasecmp(text+x+1,"quote;") != 0) &&
		(strcasecmp(text+x+1,"amp;") != 0)) {
		cout << "&amp;";
	    }
	}
	else
	    cout << text[x];
	x++;
    }
}


