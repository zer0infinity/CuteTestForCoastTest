/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FOUNDATION_H
#define _FOUNDATION_H

#if defined(COAST_OPT)
#define STR_MODE	"OPT"
#else
#define STR_MODE	"DBG"
#endif
#if defined(COAST_TRACE)
#define STR_TRACE	"_TRACE"
#else
#define STR_TRACE	""
#endif
#define COAST_BUILDFLAGS	STR_MODE STR_TRACE

#if defined(__GNUG__)
#define COAST_COMPILER		"GCC_"  __VERSION__
#else
#define COAST_COMPILER		"CompilerUnknown"
#endif

// symbol concatenation operator
#define _NAME1_(name) name
#define _NAME2_(name1,name2) name1##name2
#define _NAME3_(name1,name2,name3) name1##name2##name3

// stringizing
#define _QUOTE_(name) #name

#ifdef DEBUG
#if defined(WIN32)
int syslog_assert(const char *, long, const char *);
#else
extern int syslog_assert(const char *, long, const char *);
#endif
#define Assert(expr)	\
	((void)((expr) || syslog_assert(__FILE__, __LINE__, _QUOTE_(expr))))
#else
#define Assert(expr)
#endif

// general settings
typedef unsigned long u_long;

// handle l_long and ul_long as type
// also define macros to correctly declare literals
typedef long long l_long;
typedef unsigned long long ul_long;
#define UINT64_LITERAL(n) n ## ull
#define INT64_LITERAL(n) n ## ll

// write correct end of line sequence for http
// without flushing the stream
// use it instead of endl especially in case of socket streams
#define ENDL "\r\n"

#define ZAPEOF(x)       (x < 0 ? -x : x)

// Macro for safer use of typecasts
#define SafeCast(objname, objtype) (dynamic_cast<objtype*>(objname))

// min and max macros
#define itoMIN(a,b) (a<b?a:b)
#define itoMAX(a,b) (a>b?a:b)

#endif
