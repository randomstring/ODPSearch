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

// -*- C++ -*-

#ifndef DebugInfo_h
#define DebugInfo_h

#ifdef __GNUC__
#ifndef HAS__FUNC__
#define HAS__FUNC__ (1)
#endif
#endif

#ifdef HAS__FUNC__
#define __HERE__ __FILE__, __LINE__, __FUNCTION__
#else
#define __HERE__ __FILE__, __LINE__
#endif

#ifndef DEBUGCLASS
class Debug
{
public:
    Debug(const char *) {}
    static bool active() { return false; }
    static void out(const char *, ...) { return; }
};
#else
class Debug
{
public:
    Debug(const char *);

    bool active() const { return _active; }
    void out(const char *file, long line,
#ifdef HAS__FUNC__
             const char *func,
#endif
             const char *, ...) const;

    static void trap();

private:
    const char *_category; // Not a string, so no overhead until used
    bool _active;

    static void _init();
    static bool _isActiveCategory(const char * category);

    static char * _environment_setting;
    static char * _active_categories[1024]; // Avoid using STL, for now
};
#endif


#endif

// END OF FILE
