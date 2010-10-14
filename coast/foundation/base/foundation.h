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
#elif defined(WIN32) && defined(_MSC_VER)
#define COAST_COMPILER		"MSC_" _MSC_VER
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
#include "config_foundation.h"
EXPORTDECL_FOUNDATION int syslog_assert(const char *, long, const char *);
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
#if defined(WIN32)		// already defined above!
typedef __int64 l_long;
typedef unsigned __int64 ul_long;
#define UINT64_LITERAL(n) n ## ui64
#define INT64_LITERAL(n) n ## i64
#else /* assume everybody else supports long long */
typedef long long l_long;
typedef unsigned long long ul_long;
#define UINT64_LITERAL(n) n ## ull
#define INT64_LITERAL(n) n ## ll
#endif
#if defined(WIN32) || ( defined(__linux__) && defined(__GNUG__) && __GNUG__ < 4 )
#define LLONG_MAX	INT64_LITERAL(9223372036854775807)
#define LLONG_MIN	(-LLONG_MAX-1)
#define ULLONG_MAX	UINT64_LITERAL(18446744073709551615)
#endif

// write correct end of line sequence for http
// without flushing the stream
// use it instead of endl especially in case of socket streams
#define ENDL "\r\n"

#define ZAPEOF(x)       (x < 0 ? -x : x)   /* for MSC70 == zapeof(x) */

// Macro for safer use of typecasts
#define SafeCast(objname, objtype) (dynamic_cast<objtype*>(objname))

// min and max macros
#define itoMIN(a,b) (a<b?a:b)
#define itoMAX(a,b) (a>b?a:b)

#endif
