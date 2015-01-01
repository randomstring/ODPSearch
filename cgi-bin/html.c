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
 * HTML generation functions common to all search binaries.
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
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#include "newhoo.h"

/* 
 * Extern Globals
 */
extern char *category;
extern char *prog_name;
extern int restrict;
extern int warned;
extern char *warning_string;
extern STRING raw_query_str;
extern STRING clean_query_str;
extern STRING url_query_str;
extern STRING simple_query_str;
extern STRING av_query_str;
extern STRING deja_query_str;
extern int maxhit_default;
extern char **phrase_tbl;             /* array of terms, used for bolding text */
extern int utf8;
extern int editor_buttons;
extern char *template_base;

int no_jj_footer = 0;

/*-----------------------------------------------------------------
 *  FailoverSearch() - called if we don't have any results
 *-----------------------------------------------------------------
 */

void FailoverSearch(STRING s) {

    /* redirect to google */
    cout << "Location: http://www.google.com/search?q="
	 << s << "\n\n";
    
    cout.flush();
}


/*-----------------------------------------------------------------
 *  NoMatches()  - display a blank page with an optional message
 *-----------------------------------------------------------------
 */

void NoMatches(char *mesg) {
    /* simply return no results, but with a metasearch bar */
    
    PutHTTPHeader();
    PutHTMLHead(0,0,0);
    PutHTMLBodyStart();

    if (mesg) {
	cout << "<CENTER><I>" << mesg << "</I></CENTER>\n";
    }
    
    STRING qs = clean_query_str;

    cout << "<center>";
    cout << "No results found</center><p>";
    
    cout << "<center>";
    cout << "<table cellpadding=0 cellspacing=0 border=0><tr><td><table cellpadding=5 cellspacing=0 border=0>";
    cout << "<tr bgcolor=\"#669933\"><td align=center>&nbsp; &nbsp; <font color=\"#FFFFFF\">Try your search on:</font>&nbsp; &nbsp; </td></tr>\n";
    

    /*
     * Google need the following args to use UTF-8:
     *      &ie=utf8&oe=utf8  (ie = input encoding, oe = output encoding )
     */

    cout << "<tr bgcolor=\"#cccccc\"><td align=center><a href=\"";
    cout << "http://www.google.com/search?q=";
    print_extra_escaped_url((char *)qs.Buffer);
    cout << "&ie=utf8\">Google</a></td></tr>\n";

    cout << "<tr bgcolor=\"#cccccc\"><td align=center><a href=\"";
    cout << "http://www.google.com/search?q=";
    print_extra_escaped_url((char *)qs.Buffer);
    cout << "&cat=gwd%2FTop";
    cout << "&ie=utf8\">Google Directory</a></td></tr>\n";
    
    cout << "<tr bgcolor=\"#cccccc\"><td align=center><a href=\"";
    cout << "http://groups.google.com/groups?q=";
    print_extra_escaped_url((char *)qs.Buffer);
    cout << "&ie=utf8\">Google Groups</a></td></tr>\n";
    
    cout << "<tr bgcolor=\"#cccccc\"><td align=center><a href=\"";
    cout << "http://images.google.com/images?q=";
    print_extra_escaped_url((char *)qs.Buffer);
    cout << "&ie=utf8\">Google Image</a></td></tr>\n";

#if 0
    cout << "<tr bgcolor=\"#cccccc\"><td align=center><a href=\"";
    cout << "http://www.altavista.digital.com/cgi-bin/query?pg=q&user=netscape.com&q=";
    print_extra_escaped_url((char *)qs.Buffer);
    cout << "\">AltaVista</a></td></tr>\n";
	
    cout << "<tr bgcolor=\"#cccccc\"><td align=center><a href=\"";
    cout << "http://service.bfast.com/bfast/click/goto?bfsiteid=11405412&bfpage=search8&Keywords=";
    print_extra_escaped_url((char *)qs.Buffer);
    cout << "\">GoTo</a></td></tr>\n";
    
    cout << "<tr bgcolor=\"#cccccc\"><td align=center><a href=\"";
    cout << "http://search.yahoo.com/bin/search?p=";
    print_extra_escaped_url((char *)qs.Buffer);
    cout << "\">Yahoo</a></td></tr>\n";

    cout << "<tr bgcolor=\"#cccccc\"><td align=center>"
	 << "<A href=\"http://search.aol.com/dirsearch.adp?query=";
    print_extra_escaped_url((char *)qs.Buffer);
    cout << "\">AOL</a></td></tr>\n"
	 << "<tr bgcolor=\"#cccccc\"><td align=center><a href=\"http://www.lycos.com/srch/?query=";
    print_extra_escaped_url((char *)qs.Buffer);
    cout << "\">Lycos</a></td></tr>\n";
#endif

    cout << "</table></td></tr><tr bgcolor=\"#669933\" height=1><td height=1>"
	 << "<img height=1 width=1 src=\"http://dmoz.org/img/pixel.gif\"></td></tr>"
	 << "</table>";
    
    cout << "</CENTER><P>\n";
    PutHTMLBodyEnd(1);
    exit(0);
}

/*-----------------------------------------------------------------
 *  PutHTTPHeader() - print the HTTP header
 *-----------------------------------------------------------------
 */
void  PutHTTPHeader()
{
  static int first_time  = 1;

  if (first_time) {
    if (utf8) 
      cout << "Content-type: text/html; charset=UTF-8\n\n";
    else
      cout << "Content-type: text/html\n\n";
    first_time = 0;
  }
}

/*-----------------------------------------------------------------
 *  PutHTMLHead()   - print <HEAD> and <TITLE> tags
 *-----------------------------------------------------------------
 */

void  PutHTMLHead(int start, int end, int total)
{

  static int first_time  = 1;

  if (first_time) {
    
    if (utf8) 
      cout << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n";

    cout << "<HTML>\n<HEAD>\n<TITLE>Open Directory Search - Search Results ";
    

#if 0
    // if we know the range, and its a valid one display the range
    if ((start) && (start <= end))
      cout << "(" << start << "-" << end << " of " << total << ")";
#endif

    cout << "</TITLE>\n";

    if (utf8) 
      cout << "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">\n";

    cout << "<BASE target=\"_top\">\n"
	 << "</HEAD>\n";  
    
    first_time = 0;
  }

}

void AdvancedSearch(char *location)
{

    cout << "Location: " << location << "\n\n";

}



/*-----------------------------------------------------------------
 *  PutTemplate()   - print a template file
 *-----------------------------------------------------------------
 */

int PutTemplate(char* template_name) {

    char *filename;
    int fd = -1;
    int len;
    int total_len = 0;
    char buffer[516];
    
    filename = (char *)malloc(strlen(TEMPLATE_DIR) +
			      strlen(template_base) + 
			      strlen(template_name) + 
			      strlen(TEMPLATE_POSTFIX) + 3);
    
    if (filename) {
	
	sprintf(filename,"%s%s-%s%s",TEMPLATE_DIR, template_base, template_name, TEMPLATE_POSTFIX);
	
	if ((fd = open(filename,O_RDONLY)) > 0) {
	    free(filename);
	    
	    while (len = read(fd,buffer,512)) {
		total_len += len;
		buffer[len] = '\0';
		cout << buffer;
	    }
	    close(fd);
	}
    }

    return total_len;
}


/*-----------------------------------------------------------------
 *  PutHTMLBodyStart()   - print <BODY> tags and top of page
 *-----------------------------------------------------------------
 */

void  PutHTMLBodyStart()
{

  static int first_time  = 1;

  if (first_time) {
   
      if (PutTemplate("header")) {
	  first_time = 0;
      }

      /* 
       * Plain and simple Mozilla version
       */  

      if (first_time) {
	  first_time = 0;
	  cout << "<body bgcolor=\"#ffffff\" text=\"#000000\" link=\"#3300cc\" "
	       << "vlink=\"#660066\" alink=\"#FF0000\"><p>\n"
	       << "<h2>Example ODP Search CGI</h2>\n<p>\n";
      }
    
      /* Show the Search string  */
      cout << "<CENTER>";

      if (clean_query_str != "") {
	  STRING qs = clean_query_str;
	  qs.Replace("&","&amp;");
	  qs.Replace("\"","&quot;");
	  qs.Replace("<","&lt;");
	  qs.Replace(">","&gt;");
	  cout << "Search: <B>" << qs << "</B><br>\n";
      }
      
      
      if (restrict && category) {
	  STRING qs =  category ;
	  qs.Replace("&","&amp;");
	  qs.Replace("\"","&quot;");
	  qs.Replace("<","&lt;");
	  qs.Replace(">","&gt;");
	  cout << "Restricting Search to Category: <B>" << qs << "</B>";
      }
      cout << "</CENTER>\n";
  }
}

/*-----------------------------------------------------------------
 *  PutHTMLBodyEnd()  -  print out the footer: search, metasearch,
 *                       copyright, and feedback.
 *-----------------------------------------------------------------
 */

void  PutHTMLBodyEnd(int valid_search_string)
{

  /* search bar */

  if (!no_jj_footer) {

      no_jj_footer = 0;

      cout << "<p><CENTER>\n";
      cout << "<TABLE cellpadding=0 cellspacing=0 border=0><TR><TD>\n";
      cout << "<FORM METHOD=GET ACTION=\"";
      cout << prog_name << "\">";
      cout << "<INPUT SIZE=\"30\" MAXLENGTH=\"512\" NAME=\"search\" VALUE=\"";


      STRING qs = raw_query_str;
      qs.Replace("&","&amp;");
      qs.Replace("\"","&quot;");
      qs.Replace("<","&lt;");
      qs.Replace(">","&gt;");
      
      cout << qs;

      cout << "\">";
      cout << " &nbsp; ";
      cout << "<INPUT TYPE=\"submit\" VALUE=\"New Search\"> &nbsp;";
      cout << "<A HREF=\"/advanced_search.html\">Advanced Search</A>&nbsp;&nbsp;";
      cout << "<A HREF=\"/searchguide.html\">Help on Search</A>";
      if (category) {
	  cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"cat\" VALUE=\"";
	  print_extra_escaped_url(category);
	  cout << "\">";
      }
      if (editor_buttons)
	  cout << "<INPUT TYPE=\"HIDDEN\" NAME=\"ebuttons\" VALUE=\"1\">\n";
      cout << "</FORM>";
      cout << "</TD></TR></TABLE>";
      cout << "</CENTER>\n";
  }
  

  /* build a table of alternate search engines */
  cout << "<P><CENTER><HR>\n";
  cout << "<TABLE><TR><TD VALIGN=\"top\">";
  if (valid_search_string) {
    STRING qs = clean_query_str;
    qs.Replace("&","&amp;");
    qs.Replace("\"","&quot;");
    qs.Replace("<","&lt;");
    qs.Replace(">","&gt;");
    cout << "\"<B>" << qs << "</B>\"&nbsp;";
  }
  cout << "search&nbsp;on:</TD><TD VALIGN=\"top\"><SMALL>\n";
  
  /*=========================== META-Search Bar Start Here ====================*/
     
  if (valid_search_string) {
    
    cout << "<A HREF=\"http://www.alltheweb.com/cgi-bin/search?type=all&query=";
    print_extra_escaped_url((char*)simple_query_str.Buffer);
    cout << "\">All&nbsp;the&nbsp;Web</a>&nbsp;- \n"; 
    
    cout << "<A HREF=\"http://www.altavista.com/cgi-bin/query?q=";
    print_extra_escaped_url((char*)simple_query_str.Buffer);
    print_extra_escaped_url((char*)av_query_str.Buffer);
    cout << "\">AltaVista</A>&nbsp;- \n";   
    
    cout << "<A HREF=\"http://www.google.com/search?num=10&query=";
    print_extra_escaped_url((char*)simple_query_str.Buffer);
    cout << "&ie=utf8\">Google</A>&nbsp;- \n";
    
    cout << "<A HREF=\"http://hotbot.lycos.com/?MT=";
    print_extra_escaped_url((char*)simple_query_str.Buffer);
    cout << "&SM=MC&DV=0&LG=any&DC=10&DE=2\">HotBot</A>&nbsp;- \n";
    
    cout << "<A HREF=\"http://search.netscape.com/search.psp?search=";
    print_extra_escaped_url((char*)simple_query_str.Buffer);
    print_extra_escaped_url((char*)av_query_str.Buffer);
    cout << "\">Netscape</A>&nbsp;- \n";
    
    cout << "<A HREF=\"http://www.northernlight.com/nlquery.fcg?qr=";
    print_extra_escaped_url((char*)simple_query_str.Buffer);
    cout << "\">Northern&nbsp;Light</a>&nbsp;- \n";
    
    cout << "<A HREF=\"http://search.yahoo.com/bin/search?p=";
    print_extra_escaped_url((char*)simple_query_str.Buffer);
    print_extra_escaped_url((char*)av_query_str.Buffer);
    cout << "\">Yahoo</A>\n";
  }
  else {
      /* we have a null search string, so give a generic version of the Meta-Search bar */
      cout << "<A HREF=\"http://www.alltheweb.com/\">All&nbsp;the&nbsp;Web</A>&nbsp;- \n";
      cout << "<A HREF=\"http://www.altavista.com/\">AltaVista</A>&nbsp;- \n";
      cout << "<A HREF=\"http://www.google.com/\">Google</A>&nbsp;- \n";
      cout << "<A HREF=\"http://hotbot.lycos.com/\">HotBot</A>&nbsp;- \n";
      cout << "<A HREF=\"http://www.northernlight.com/\">Northern&nbsp;Light</a>&nbsp;- \n"; 
      cout << "<A HREF=\"http://search.netscape.com/\">Netscape</A>&nbsp;- \n";
      cout << "<A HREF=\"http://www.yahoo.com/\">Yahoo</A>\n";
  }

  /*=========================== META-Search Bar Ends Here ====================*/
    

  cout << "</SMALL></TD></TR></TABLE></CENTER>\n";
  cout << "<HR>\n";
  
  /* copyright and feedback bar  */
  PutCopyright();

  PutFreshnessDate();

  cout << "</BODY></HTML>\n";
}


/*-----------------------------------------------------------------
 *  PutFreshnessDate()   - show when the searchDB was updated
 *-----------------------------------------------------------------
 */

#define MAX_DATE_LENGTH  80

void PutFreshnessDate() {

    int fd, len;
    char buf[MAX_DATE_LENGTH];
    char *filename;

    filename = (char *)malloc(strlen(SEARCH_DB_DIR) + 
			      strlen(FRESHNESS_DATE_FILENAME) + 3);

    sprintf(filename,"%s%s",SEARCH_DB_DIR,FRESHNESS_DATE_FILENAME);

    if (filename) {
	if ((fd = open(filename,O_RDONLY)) > 0) {
	    free(filename);
	    if ((len = read(fd, buf, MAX_DATE_LENGTH)) > 0) {
		buf[len] = '\0';
		cout << "<center><small>Search database last updated on: " 
		     << buf << "</small></center><br>";
	    }
	    close(fd);
	}
    }
}


/*-----------------------------------------------------------------
 *  PutCopyright() - print mozilla version of the copyright
 *-----------------------------------------------------------------
 */
void PutCopyright() {


    if (PutTemplate("copyright") == 0) {
	cout << "<p><table border=0 bgcolor=\"#669933\" cellpadding=2 cellspacing=0 ";
	cout << "width=\"100%\"><tr><td valign=middle> <font color=\"#ffffff\" ";
	cout << "face=\"sans-serif, Arial, Helvetica\" size=\"-2\"> &nbsp; &nbsp; ";
	cout << "Copyright &copy; 1999-2002 Netscape </font> </td></tr></table>";
    }
}


void print_nice(char *s)
{
	char buf[2048];
	char *p, *q;

	for (p = s, q = buf; *p; p++) {
		if (*p == '/') {
			*q++ = ':';
			*q++ = ' ';
		} else if (*p == '_') {
			*q++ = ' ';
		} else
			*q++ = *p;
	}

	*q++ = '\0';

	cout << "<a href=\"http://dmoz.org/";
	print_extra_escaped_url(s); 
	cout << "/\">" << buf << "</a>\n";
}

