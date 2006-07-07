/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SysLog_H
#define _SysLog_H

#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
#include "ITOString.h"

//--- SysLog ----------------------------------------------------------
/*! <b>API for syslog access</b>
This is the Coast system logging API. It is used for system-level and application-level logging.
The values of <tt>WD_LOGONCERR</tt> and <tt>WD_DOLOG</tt> control the level of severities shown either on the console or in the syslog. Possible values are:
<pre>
1: DEBUG
2: INFO
3: WARNING
4: ERROR
5: ALERT
</pre>
All messages with a severity above or equal the specified value will log on the appropriate channel.<br>
The loggers behavior is to write ALERT messages into syslog and to log ERROR and ALERT messages to the console by default.

The flag <tt>TRACE_STATICALLOC</tt> shows you the allocation and deletion of all statically allocated objects used in Coast.
*/
class EXPORTDECL_FOUNDATION SysLog
{
public:
	//! module initialization
	static void Init(const char *appId);

	//! ensure early initialization but do it only once
	static bool InitOnce();

	//! module termination
	static void Terminate();

	/*--- logging API -------------------------------------*/

	/*! define importancy levels in increasing order for easier */
	enum eLogLevel {
		eNone,
		eDEBUG,
		eINFO,
		eWARNING,
		eERR,
		eALERT,
		eLast
	};

	//! severity eDEBUG for tracing server activity during development and deployment
	static void Debug(const char *msg);

	//! severity eINFO for general information log general useful information about server activity
	static void Info(const char *msg);

	//! severity eWARNING	for information about inconsistent state or potential dangerous situation
	static void Warning(const char *msg);

	//! severity eERR for outright errors during operation of the server without fatal results
	static void Error(const char *msg);

	//! severity eALERT for fatal errors this call triggers also an alert on  the operator console
	static void Alert(const char *msg);

	//! for use by the Assert() macro defined in config.h
	static int LogAssert(const char *file, long line, const char *assertion);
	/*--- utilities ------------------------------------*/

	//! provides error messages as strings for errno values
	//! const char *SysErrorMsg(long errnum) maps the error number in errnum
	//! to an error  message  string,  and returns a pointer to that string.
	//! SysErrorMsg(long errnum) uses the same set of error messages as perror().
	//! The returned string should not be overwritten.
	static String SysErrorMsg(long errnum);

	// provides an error message for the last call that has set errno
	// const char *LastSysError() returns the error message retrieved with errno
	// from SysErrorMsg(errno)
	static String LastSysError();

	//!wrapper to ::write(2,...) on Unix systems to get things written to stderr
	//!needed, because some iostream implementations aren't thread safe
	static void WriteToStderr(const char *msg, long length);
	static void WriteToStderr(const String &msg);

	//!wrapper to ::write(1,...) on Unix systems to get things written to stdout
	//!needed, because some iostream implementations aren't thread safe
	static void WriteToStdout(const char *msg, long length);
	static void WriteToStdout(const String &msg);

	friend class SysLogTest;

protected:
	/*---- object api for different platforms ---*/
	SysLog();
	virtual ~SysLog();

	//!template method
	virtual void DoLog(eLogLevel level, const char *msg);

	//!do a system dependent log call
	virtual void DoSystemLevelLog(eLogLevel level, const char *msg) = 0;

	//!write log messages to cerr with a level a string indicating the level if preprocessor flags are set accordingly
	virtual void DoLogTrace(eLogLevel level, const char *msg);

	//!write log messages to cerr if preprocessor flags are set accordingly
	virtual void DoTraceLevel(const char *level, const char *msg);

	//! bottleneck routine used by the other methods, you can use severity levels directly
	static void Log(eLogLevel level, const char *msg);

private:
	//! fgSysLog is the system dependent variable that calls a system's log api
	//! since SysLog messages might be generated from everywhere we don't
	//! know when this is happening for the first time<P>
	//! Init is called normally in the bootstrap process once but then the syslog
	//! might already be in use
	static SysLog *fgSysLog;
	static eLogLevel fgDoSystemLevelLog;
	static eLogLevel fgDoLogOnCerr;
};

#define	SYSDEBUG(msg) \
	{ String strFileLineMsg(Storage::Global());\
	strFileLineMsg << "( " << __FILE__ << ":" << (long)__LINE__ << " ) " << msg;\
	SysLog::Debug(strFileLineMsg); }
#define	SYSINFO(msg) \
	{ String strFileLineMsg(Storage::Global());\
	strFileLineMsg << "( " << __FILE__ << ":" << (long)__LINE__ << " ) " << msg;\
	SysLog::Info(strFileLineMsg); }
#define	SYSWARNING(msg) \
	{ String strFileLineMsg(Storage::Global());\
	strFileLineMsg << "( " << __FILE__ << ":" << (long)__LINE__ << " ) " << msg;\
	SysLog::Warning(strFileLineMsg); }
#define	SYSERROR(msg) \
	{ String strFileLineMsg(Storage::Global());\
	strFileLineMsg << "( " << __FILE__ << ":" << (long)__LINE__ << " ) " << msg;\
	SysLog::Error(strFileLineMsg); }
#define	SYSALERT(msg) \
	{ String strFileLineMsg(Storage::Global());\
	strFileLineMsg << "( " << __FILE__ << ":" << (long)__LINE__ << " ) " << msg;\
	SysLog::Alert(strFileLineMsg); }

#if defined(__aix__) || defined(__linux__) || defined(__sun)
//! implementation for Unix syslog api
class UnixSysLog : public SysLog
{
public:
	UnixSysLog(const char *appId);
	~UnixSysLog();

protected:
	virtual void DoSystemLevelLog(eLogLevel level, const char *msg);
};
#endif

#if defined(__370__)
//! implementation of SysLog api for System/370; just uses cerr
class S370SysLog: public SysLog
{
public:
	S370SysLog() { }
	~S370SysLog() { }

protected:
	virtual void DoSystemLevelLog(eLogLevel level, const char *msg);
};
#endif

#if defined(WIN32)
//! implementation of SysLog api for WIN32
class EXPORTDECL_FOUNDATION Win32SysLog : public SysLog
{
public:
	Win32SysLog(const char *appId);
	~Win32SysLog();

protected:
	virtual void DoSystemLevelLog(eLogLevel level, const char *msg);

	HANDLE fLogHandle;
};
#endif

#endif		//not defined _SysLog_H
