/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CONFIG_H
#define _CONFIG_H

#if defined(__GNUG__)
#if defined(__OPTIMIZE__) && defined(DEBUG)
#define WD_OPTFLAG	"OPT_DBG"
#elif defined(__OPTIMIZE__) && !defined(DEBUG)
#define WD_OPTFLAG	"OPT"
#else
#define WD_OPTFLAG	"DBG"
#endif
#else
#if !defined(DEBUG)
#define WD_OPTFLAG	"OPT"
#else
#define WD_OPTFLAG	"DBG"
#endif
#endif

#if defined(ONLY_STD_IOSTREAM)
#define WD_IOVERSION	"_STDIO"
#else
#define WD_IOVERSION	""
#endif
#define WD_BUILDFLAGS		WD_OPTFLAG WD_IOVERSION
#if defined(__GNUG__)
#define WD_COMPILER		"GCC_"  __VERSION__
#elif defined(WIN32) && defined(_MSC_VER)
#define WD_COMPILER		"MSC_" _MSC_VER
#else
#define WD_COMPILER		"CompilerUnknown"
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

// handle legacy compilers that do not have bool as a separate type
#if ((defined(__SUNPRO_CC) && (__SUNPRO_CC <= 0x4ff)) || defined(_ARCH_COM) || defined(__370__))
#define BOOL_NOT_SUPPORTED /* use this define for masking interfaces */
typedef int bool;
#if !defined(true)
#define true ((bool)1)
#define false ((bool)0)
#endif
#endif

// another legacy compiler problem
#if defined(__SUNPRO_CC) && __SUNPRO_CC <0x500
#define OPERATOR_NEW_ARRAY_NOT_SUPPORTED
#endif

// use unsafe_...streams, because they don't use locking
#if defined(__SUNPRO_CC)
#if !defined(__STD_ISTREAM__)
class unsafe_fstream;
#define ios unsafe_ios
#define iostream unsafe_iostream
#define istream unsafe_istream
#define ostream unsafe_ostream
#define fstream unsafe_fstream
#include "UnsafeFStream.h"

// PS: add defines for names for our Streambufs to be unsafe
// buffer base() and end ebuf() are no longer supported by std iostream, so
// we should recognize their use in old code somehow
//#define base() base_unlocked()
//#define ebuf() ebuf_unlocked()
//#define blen() blen_unlocked()
#define pbase()	pbase_unlocked()
#define pptr()	pptr_unlocked()
#define epptr()	epptr_unlocked()
#define eback()	eback_unlocked()
#define gptr()	gptr_unlocked()
#define egptr()   egptr_unlocked()
#define setp(a,b)	setp_unlocked((a), (b))
#define setg(a,b,c)	setg_unlocked((a),(b),(c))
#define pbump(a)	pbump_unlocked(a)
#define gbump(a)	gbump_unlocked(a)
//#define setb(a,b,c)	setb_unlocked((a),(b),(c) )
//#define setb(a,b)    setb_unlocked((a),(b))
// unbuffered might be obsolete, allocate might not be used
#define unbuffered(a)	unbuffered_unlocked(a)
#define unbuffered()	unbuffered_unlocked()
#define allocate()	allocate_unlocked()
#else
// deal with std iostream and std namespace for SunCC 5.0
using std::streampos;
using std::streamoff;
using std::ws;
#endif
#endif

// general settings
typedef unsigned long u_long;

// handle l_long and ul_long as type
// also define macros to correctly declare literals
#ifdef __370__
typedef long l_long;
typedef unsigned long ul_long;
#define UINT64_LITERAL(n) n ## ul
#define INT64_LITERAL(n) n ## l
#elif defined(WIN32)		// already defined above!
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
