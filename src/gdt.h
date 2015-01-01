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

/* Description:	Generic Data Type definitions */

#ifndef GDT_H
#define GDT_H

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Why change all the names? Are you mad? Just more evidence that the
 * origional Isearch coders are on crack.  One day These should be
 * fixed globally... -Bryn 
 */

typedef size_t        SIZE_T;
typedef int           INT;
typedef unsigned int  UINT;
typedef UINT*         PUINT;
typedef float         FLOAT;
typedef FLOAT*        PFLOAT;
typedef double        DOUBLE;
typedef char          CHR;
typedef CHR*          PCHR;
typedef CHR**         PPCHR;
typedef unsigned char UCHR;
typedef UCHR*         PUCHR;
typedef UCHR**        PPUCHR;
typedef FILE*         PFILE;

#if !defined(_MSDOS) || !defined(WINAPI)
  typedef long          LONG;
  typedef unsigned long ULONG;
#endif

/* what is truth anyway? */
#define TRUE  1
#define FALSE 0

#ifdef NO_BOOL_TYPE  /* Define as an enum if user asks */
  enum GDT_BOOLEAN { GDT_FALSE = 0, GDT_TRUE = 1 };
#else
  typedef bool GDT_BOOLEAN;
  const GDT_BOOLEAN GDT_TRUE  = true;
  const GDT_BOOLEAN GDT_FALSE = false;
#endif

#include "conf.h"

/* System-specific typedefs */

/* INT2 */
#if (SIZEOF_INT == 2)
	typedef int INT2;
	typedef unsigned int UINT2;
#else
#if (SIZEOF_SHORT_INT == 2)
	typedef short int INT2;
	typedef unsigned short int UINT2;
#endif
#endif

/* INT4 */
#if (SIZEOF_INT == 4)
	typedef int INT4;
	typedef unsigned int UINT4;
#else
#if (SIZEOF_LONG_INT == 4)
	typedef long int INT4;
	typedef unsigned long int UINT4;
#endif
#endif

/* INT8 */
#ifndef _MSC_VER
#if (SIZEOF_INT == 8)
	typedef int INT8;
	typedef unsigned int UINT8;
#else
#if (SIZEOF_LONG_INT == 8)
	typedef long int INT8;
	typedef unsigned long int UINT8;
#else
#if (SIZEOF_LONG_LONG_INT == 8)
	typedef long long int INT8;
	typedef unsigned long long int UINT8;
#endif
#endif
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* GDT_H */
