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

/* Class RECORD - Database Record */

#ifndef RECORD_H
#define RECORD_H

class RECORD {
public:
	RECORD();
	RECORD(STRING& NewPathName, STRING& NewFileName);
	RECORD& operator=(const RECORD& OtherRecord);
	void SetPathName(const STRING& NewPathName);
	void GetPathName(PSTRING StringBuffer) const;
	void SetFileName(const STRING& NewFileName);
	void GetFileName(PSTRING StringBuffer) const;
	void GetFullFileName(PSTRING StringBuffer) const;
	void SetRecordStart(const GPTYPE NewRecordStart);
	GPTYPE GetRecordStart() const;
	void SetRecordEnd(const GPTYPE NewRecordEnd);
	GPTYPE GetRecordEnd() const;
	void Write(PFILE fp) const;
	void Read(PFILE fp);
	~RECORD();

	long   pathName;
        long   fileName;
	GPTYPE RecordStart;
	GPTYPE RecordEnd;
};

typedef RECORD* PRECORD;

#endif
