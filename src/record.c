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

#include "defs.h"
#include "stringx.h"
#include "common.h"
#include "record.h"
#include "mdthashtable.h"

RECORD::RECORD() {
	pathName=0;
	fileName=0;
	RecordStart=0;
	RecordEnd=0;
}

RECORD& RECORD::operator=(const RECORD& OtherRecord) {
	pathName = OtherRecord.pathName;
	fileName = OtherRecord.fileName;
	RecordStart = OtherRecord.RecordStart;
	RecordEnd = OtherRecord.RecordEnd;
	return *this;
}

void RECORD::SetRecordStart(const GPTYPE NewRecordStart) {
	RecordStart = NewRecordStart;
}

GPTYPE RECORD::GetRecordStart() const {
	return RecordStart;
}

void RECORD::SetRecordEnd(const GPTYPE NewRecordEnd) {
	RecordEnd = NewRecordEnd;
}

GPTYPE RECORD::GetRecordEnd() const {
	return RecordEnd;
}

void RECORD::Write(PFILE fp) const {
	fprintf(fp, "%d\n", pathName);
	fprintf(fp, "%d\n", fileName);
	fprintf(fp, "%d\n", RecordStart);
	fprintf(fp, "%d\n", RecordEnd);
}

void RECORD::Read(PFILE fp) {
	STRING s;
	s.FGet(fp, 16);
	pathName = s.GetInt();
	s.FGet(fp, 16);
	fileName = s.GetInt();
	s.FGet(fp, 16);
	RecordStart = s.GetInt();
	s.FGet(fp, 16);
	RecordEnd = s.GetInt();
}

RECORD::~RECORD() {
}
