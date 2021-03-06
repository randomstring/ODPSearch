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

/* Document Type configuration utility for Isearch */

#include <iostream.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAXDT 500
#define MAXSTR 80

static char DtName[MAXDT][MAXSTR];
static char DtFn[MAXDT][MAXSTR];

main() {
  cout << endl;
  cout << "Configuring Isearch for the following document types (see dtconf.inf):" << endl;

  // Read configuration
  int x;
#ifndef DEV_STUDIO
  int y;
#endif
  int TotalDt = 0;
  char s[MAXSTR], t[MAXSTR], u[MAXSTR], v[MAXSTR];
  char* p;
  char* pp;
  FILE* fp;
  FILE* fpi;
  fp = fopen("dtconf.inf", "r");
  if (!fp) {
    cout << "You need to create a doctype configuration file: dtconf.inf";
    cout << endl;
    exit(1);
  }
  else {
    while ( fgets(s, MAXSTR, fp) ) {
      p = s;
      while (isalnum(*p)) {	// truncate after the first word
	p++;
      }
      *p = '\0';
      if (*s != '\0') {
	strcpy(DtFn[TotalDt], s);
	sprintf(t, "%s.h", DtFn[TotalDt]);
	fpi = fopen(t, "r");
	if (fpi) {
	  x = 0;
	  while ( (fgets(u, MAXSTR, fpi)) && (!x) ) {
	    if (!strncmp(u, "class ", 6)) {
	      x = 1;
	      strcpy(v, u);
	      p = v + 6;
	      while (*p == ' ') {
		p++;
	      }
	      pp = p;
	      while (isalnum(*pp)) {
		pp++;
	      }
	      *pp = '\0';
	      strcpy(DtName[TotalDt], p);
	    }
	  }
	  fclose(fpi);
	  printf(" %s", DtName[TotalDt]);
	  fpi = fopen(t, "r");
	  if (fpi) {
	    x = 0;
	    while ( (fgets(u, MAXSTR, fpi)) && (!x) ) {
	      if (!strncmp(u, "Description:", 12)) {
		x = 1;
		if ( (p=strchr(u, '-')) ) {
		  printf(" %s", p);
		}
	      }
	    }
	    fclose(fpi);
	  } else {
	    printf("\n");
	  }
	  TotalDt++;
	} else {
	  printf(" (File %s not found.)\n", t);
	}
      }
    }
    fclose(fp);
  }

  /* Generate dtreg.h */
  cout << endl << "Creating ../src/dtreg.h" << endl;
  fp = fopen("../src/dtreg.h", "w");
  if (!fp) {
    perror("src/dtreg.h");
    exit(1);
  }

  fprintf(fp, "/* dtreg.h - Document Type registry */\n");
  fprintf(fp, "\n/* WARNING - this file is automaticly generated by ../doctypes/dtconf.c */\n");
  fprintf(fp, "\n");
  fprintf(fp, "#ifndef DTREG_H\n");
  fprintf(fp, "#define DTREG_H\n");
  fprintf(fp, "\n");
  fprintf(fp, "#include \"../doctype/doctype.h\"\n");


  // print out the #includes
  for (x=0; x<TotalDt; x++) {
    fprintf(fp, "#include \"../doctype/%s.h\"\n", DtFn[x]);
  }

  // print out the defines
  fprintf(fp, "\n");
  fprintf(fp, "#define DOC_TYPE_INVALID    \t255\n");
  for (x=0; x<TotalDt; x++) {
    fprintf(fp, "#define DOC_TYPE_%s    \t%d\n", DtName[x], x+1);
  }

  fprintf(fp, "\n");
  fprintf(fp, "class DTREG {\n");
  fprintf(fp, "public:\n");
  fprintf(fp, "\tDTREG(PIDBOBJ DbParent);\n");
  fprintf(fp, "\tPDOCTYPE GetDocTypePtr(unsigned char DocType);\n");
  fprintf(fp, "\tunsigned char GetDocTypeFromString(STRING DocType);\n");
  fprintf(fp, "\tchar * GetStringFromDocType(unsigned char dt);\n");
  fprintf(fp, "\tchar *GetDocTypeList() const;\n");
  fprintf(fp, "\t~DTREG();\n");
  fprintf(fp, "private:\n");
  fprintf(fp, "\tPIDBOBJ Db;\n");
  fprintf(fp, "\tPDOCTYPE DtDocType;\n");
  for (x=0; x<TotalDt; x++) {
    fprintf(fp, "\tP%s Dt%s;\n", DtName[x], DtName[x]);
  }
  fprintf(fp, "};\n");
  fprintf(fp, "\n");
  fprintf(fp, "typedef DTREG* PDTREG;\n");
  fprintf(fp, "\n");
  fprintf(fp, "#endif\n");
  fclose(fp);
  
  // Generate dtreg.c
  cout << "Creating ../src/dtreg.c" << endl;
  fp = fopen("../src/dtreg.c", "w");
  if (!fp) {
    perror("src/dtreg.c");
    exit(1);
  }

  fprintf(fp, "/* dtreg.h - Document Type registry */\n");
  fprintf(fp, "\n/* WARNING - this file is automaticly generated by ../doctypes/dtconf.c */\n");
  fprintf(fp, "\n");
  fprintf(fp, "#include \"../doctype/doctype.h\"\n");

  fprintf(fp, "\n");
  fprintf(fp, "#include <stdlib.h>\n");
  fprintf(fp, "#include \"defs.h\"\n");
  fprintf(fp, "#include \"stringx.h\"\n");
  fprintf(fp, "#include \"mdtrec.h\"\n");
  fprintf(fp, "#include \"mdt.h\"\n");
  fprintf(fp, "#include \"idbobj.h\"\n");
  fprintf(fp, "#include \"iresult.h\"\n");
  fprintf(fp, "#include \"irset.h\"\n");
  fprintf(fp, "#include \"dtreg.h\"\n");
  fprintf(fp, "\n");
  fprintf(fp, "DTREG::DTREG(PIDBOBJ DbParent) {\n");
  fprintf(fp, "\tDb = DbParent;\n");
  fprintf(fp, "\tDtDocType = new DOCTYPE(Db);\n");
  for (x=0; x<TotalDt; x++) {
    fprintf(fp, "\tDt%s = 0;\n", DtName[x]);
  }
  fprintf(fp, "}\n");
  fprintf(fp, "\n");
  fprintf(fp, "PDOCTYPE DTREG::GetDocTypePtr(unsigned char DocType) {\n");
  fprintf(fp, "\tif (DocType == 0) {\n");
  fprintf(fp, "\t\treturn DtDocType;\n");
  fprintf(fp, "\t}\n");

  fprintf(fp, "\tswitch(DocType) {\n");
  for (x=0; x<TotalDt; x++) {
    fprintf(fp, "\tcase DOC_TYPE_%s:\n", DtName[x]);
    fprintf(fp, "\t\tif (!Dt%s) {\n", DtName[x]);
    fprintf(fp, "\t\t\tDt%s = new %s(Db);\n", DtName[x], DtName[x]);
    fprintf(fp, "\t\t}\n");
    fprintf(fp, "\t\treturn Dt%s;\n", DtName[x]);
    fprintf(fp, "\t\tbreak;\n");
  }
  // Return default doctype if not recognized
  fprintf(fp, "\tdefault:\n");
  fprintf(fp, "\t\treturn DtDocType;\n");
  fprintf(fp, "\t\tbreak;\n");
  fprintf(fp, "\t}\n");
  fprintf(fp, "}\n\n");

#if 0
  for (x=0; x<TotalDt; x++) {
    fprintf(fp, "\tif (DocType == DOC_TYPE_%s) {\n", DtName[x]);
    fprintf(fp, "\t\tif (!Dt%s) {\n", DtName[x]);
    fprintf(fp, "\t\t\tDt%s = new %s(Db);\n", DtName[x], DtName[x]);
    fprintf(fp, "\t\t}\n");
    fprintf(fp, "\t\treturn Dt%s;\n", DtName[x]);
    fprintf(fp, "\t}\n");
  }
  // Return default doctype if not recognized
  fprintf(fp, "\treturn DtDocType;\n");
  fprintf(fp, "}\n\n");

#endif

  fprintf(fp, "unsigned char DTREG::GetDocTypeFromString(STRING DocType) {\n");
  for (x=0; x<TotalDt; x++) {
    fprintf(fp, "\tif (DocType == \"%s\") {\n", DtName[x]);
    fprintf(fp, "\t\treturn DOC_TYPE_%s;\n", DtName[x]);
    fprintf(fp, "\t}\n");
  }
  fprintf(fp, "\treturn DOC_TYPE_INVALID;\n");
  fprintf(fp, "}\n\n");

  fprintf(fp, "char * DTREG::GetStringFromDocType(unsigned char dt) {\n");
  for (x=0; x<TotalDt; x++) {
    fprintf(fp, "\tif (dt == DOC_TYPE_%s) {\n", DtName[x]);
    fprintf(fp, "\t\treturn \"%s\";\n", DtName[x]);
    fprintf(fp, "\t}\n");
  }
  fprintf(fp, "\treturn \"INVALID\";\n");
  fprintf(fp, "}\n\n");

  fprintf(fp, "char * DTREG::GetDocTypeList() const {\n");
  fprintf(fp, "\treturn(\"");
  for (x=0; x<TotalDt; x++) {
      if (x > 0)
	  fprintf(fp," ");
      fprintf(fp, "%s", DtName[x]);
  }
  fprintf(fp, "\");\n");
  fprintf(fp, "}\n");
  fprintf(fp, "\n");
  fprintf(fp, "DTREG::~DTREG() {\n");
  fprintf(fp, "\tdelete DtDocType;\n");
  for (x=0; x<TotalDt; x++) {
    fprintf(fp, "\tif (Dt%s) {\n", DtName[x]);
    fprintf(fp, "\t\tdelete Dt%s;\n", DtName[x]);
    fprintf(fp, "\t}\n");
  }
  fprintf(fp, "}\n");
  fclose(fp);
  
  // Generate Makefile
  cout << "Creating ../src/Makefile" << endl;
  fpi = fopen("../src/Makefile.000", "r");
  if (!fpi) {
    perror("../src/Makefile.000");
    exit(1);
  }
  fp = fopen("../src/Makefile", "w");
  if (!fp) {
    perror("../src/Makefile");
    exit(1);
  }
  fprintf(fp, "#############################################################################\n");
  fprintf(fp, "#############################################################################\n");
  fprintf(fp, "#############################################################################\n");
  fprintf(fp, "#####                                                                   #####\n");
  fprintf(fp, "##### NOTE: This Makefile was generated by dtconf            To make    #####\n");
  fprintf(fp, "#####       changes to the Makefile, modify the file Makefile.000.in    #####\n");
  fprintf(fp, "#####       instead of this file.  The dtconf utility uses Makefile.000 #####\n");
  fprintf(fp, "#####       to generate this file, and Makefile.000 is generated from   #####\n");
  fprintf(fp, "#####       Makefile.000.in by Gnu Autoconf.                            #####\n");
  fprintf(fp, "#####                                                                   #####\n");
  fprintf(fp, "#############################################################################\n");
  fprintf(fp, "#############################################################################\n");
  fprintf(fp, "#############################################################################\n\n");
  if (fpi) {
    while ( fgets(s, MAXSTR, fpi) ) {
      y = 0;
      if (strstr(s, "###DTOBJ###")) {
	y = 1;
	fprintf(fp, "\tdoctype.o");
	for (x=0; x<TotalDt; x++) {
	  fprintf(fp, " \\\n\t%s.o", DtFn[x]);
	}
	fprintf(fp, "\n\n");
      }
      if (strstr(s, "###DTHXX###")) {
	y = 1;
	fprintf(fp, "\t$(DOCTYPE_DIR)/doctype.h");
	for (x=0; x<TotalDt; x++) {
	  fprintf(fp, " \\\n\t$(DOCTYPE_DIR)/%s.h", DtFn[x]);
	}
	fprintf(fp, "\n\n");
      }
      if (strstr(s, "###DTMAKE###")) {
	fprintf(fp, "%s.o:$(H) $(DOCTYPE_DIR)/%s.c\n", "doctype", "doctype");
	fprintf(fp, "\t$(CC) $(CFLAGS) $(INC) -o $@ -c $(DOCTYPE_DIR)/%s.c\n", "doctype");
	fprintf(fp, "\n");
	y = 1;
	for (x=0; x<TotalDt; x++) {
	  fprintf(fp, "%s.o:$(H) $(DOCTYPE_DIR)/%s.c\n", DtFn[x], DtFn[x]);
	  fprintf(fp, "\t$(CC) $(CFLAGS) $(INC) -o $@ -c $(DOCTYPE_DIR)/%s.c\n", DtFn[x]);
	  fprintf(fp, "\n");
	}
      }
      if (!y) {
	fprintf(fp, "%s", s);
      }
    }
    fclose(fpi);
  }
  fclose(fp);
  printf("\n");

  return 0;
}
