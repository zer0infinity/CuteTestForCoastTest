/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SysLog.h"
//#define LOG_TRACE 1
//#define LOG_HIGH_TRACE
//#define LOG_LOW_TRACE
#if defined(WIN32)
#define IOSTREAM_IS_THREADSAFE
#endif

//--- standard modules used ----------------------------------------------------
#include "System.h"

//--- c-library modules used ---------------------------------------------------
#include <errno.h>
#include <stdio.h>
#if !defined(WIN32)
#include <unistd.h> /* for ::write() */
#include <string.h>
#endif

#if defined(__sun) ||defined(__linux__) || defined(__aix__)
#include <syslog.h>
#endif

#if defined(WIN32)
enum {
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARNING,
	LOG_ERR,
	LOG_ALERT
};
#endif

#ifdef __370__
enum {
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARNING,
	LOG_ERR,
	LOG_ALERT
};
#endif

//--- SysLog ----------------------------------------------------------
SysLog	*SysLog::fgSysLog = 0;
bool	SysLog::fgDoSystemLevelLog = false;
bool	SysLog::fgDoLogOnCerr = false;

//--- module initialization/termination
void SysLog::Init(const char *appId)
{
	// beware this is not thread safe
	// the syslog channel is initialized
	// by the main thread in the server constructor
	// but since logging takes place before the
	// server object exists we have to take care

	if ( appId ) {
		Terminate();    // close existing syslog channel with default Coast name
	} else {
		appId = "Coast";
	}

	if (System::EnvGet("WD_DOLOG") == "1") {
		fgDoSystemLevelLog = true;
	}

	if (System::EnvGet("WD_LOGONCERR") == "1") {
		fgDoLogOnCerr = true;
	}

	// initialize the syslog channel
	if ( (fgDoSystemLevelLog || fgDoLogOnCerr) && !fgSysLog ) {
		// open the syslog channel for this application
		// there is always only one syslog channel per application
#if defined(__sun) || defined(__linux__) || defined(__aix__)
		fgSysLog = new UnixSysLog(appId);
#endif
#if defined(WIN32)
		fgSysLog = new Win32SysLog(appId);
#endif
#if defined(__370__)
		fgSysLog = new S370SysLog();
#endif
	}
}

void SysLog::Terminate()
{
	if ( fgSysLog ) {
		delete fgSysLog;
		fgSysLog = 0;
	}
}

//--- logging API -------------------------------------

// severity LOG_DEBUG for tracing server
// activity during development and
// deployment
void SysLog::Debug(const char *msg)
{
	Log(LOG_DEBUG, msg);
}

// severity LOG_INFO for general information
// log general useful information about server
// activity
void SysLog::Info(const char *msg)
{
	Log(LOG_INFO, msg);
}

// severity LOG_WARNING	for information about
// inconsistent state or potential dangerous
// situation
void SysLog::Warning(const char *msg)
{
	Log(LOG_WARNING, msg);
}

// severity LOG_ERR for outright errors
// during operation of the server without
// fatal results
void SysLog::Error(const char *msg)
{
	Log(LOG_ERR, msg);
}

// severity LOG_ALERT for fatal errors
// this call triggers also an alert on
// the operator console
void SysLog::Alert(const char *msg)
{
	Log(LOG_ALERT, msg);
}

// bottleneck routine used by others
// here you can use severity levels directly
void SysLog::Log(long level, const char *msg)
{
	if ((fgDoLogOnCerr || fgDoSystemLevelLog || InitOnce()) && fgSysLog) {
		fgSysLog->DoLog(level, msg);
	}
}

bool SysLog::InitOnce()
{
	static bool once = false;
	if (!once) {
		once = true;
		Init(0);
		return (fgDoLogOnCerr || fgDoSystemLevelLog);
	}
	return false;
}

//--- utilities ------------------------------------

// const char *SysErrorMsg(long errnum) maps the error number in errnum
// to an error  message  string,  and returns a pointer to that string.
// SysErrorMsg(long errnum) uses the same set of error messages as perror().
// The returned string should not be overwritten.
String SysLog::SysErrorMsg(long errnum)
{
#if defined(WIN32)
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		errnum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL
	);

	String msg((const char *)lpMsgBuf);
	// Free the buffer.
	LocalFree( lpMsgBuf );
	return msg;
#else
	String theError;
	if (errnum == EMFILE) {
		theError = "[No more filehandles in Process.] ";
	}
	if (errnum == ENFILE) {
		theError = "[No more filehandles in System.] ";
	}
	theError << (long) errnum << " ";
	theError << strerror(errnum);
	return theError;
#endif
}

// const char *LastSysError() returns the error message retrieved with errno
// from SysErrorMsg(errno)
String SysLog::LastSysError()
{
	if (System::GetSystemError()) {
		return SysErrorMsg(System::GetSystemError());
	}
	return "no error";
}

//magic function to be used by Assert macro to avoid dependency to class SysLog and SysLog.h
int syslog_assert(const char *file, long line, const char *assertion)
{
	return SysLog::LogAssert(file, line, assertion);
}

int SysLog::LogAssert(const char *file, long line, const char *assertion)
{
	// implement brain dead with C style things to avoid problems of recursion
	char asrt_buf[2048];
	int asrt_buf_used = sprintf(asrt_buf, "%s:%ld\n Assert(%s) failed\n", file, line, assertion);
	\
	SysLog::WriteToStderr(asrt_buf, asrt_buf_used);
	return 0;
}

void SysLog::WriteToStderr(const String &msg)
{
	SysLog::WriteToStderr((const char *)msg, msg.Length());
}

void SysLog::WriteToStderr(const char *msg, long length)
{
#ifdef IOSTREAM_IS_THREADSAFE
	cerr.write(msg, length).flush();
#else
	::write(2, msg, length);
#endif
}

void SysLog::WriteToStdout(const String &msg)
{
	SysLog::WriteToStdout((const char *)msg, msg.Length());
}

void SysLog::WriteToStdout(const char *msg, long length)
{
#ifdef IOSTREAM_IS_THREADSAFE
	cout.write(msg, length).flush();
#else
	::write(1, msg, length);
#endif
}

SysLog::SysLog()
{
}

SysLog::~SysLog()
{
}

void SysLog::DoLog(long level, const char *msg)
{
	if (fgDoSystemLevelLog) {
		DoSystemLevelLog(level, msg);
	}
	if (fgDoLogOnCerr) {
		DoLogTrace(level, msg);
	}
}

void SysLog::DoTraceLevel(const char *level, const char *msg)
{
	String finalMessage(level);
	finalMessage.Append(msg);
	finalMessage.Append('\n');
	SysLog::WriteToStderr(finalMessage, finalMessage.Length());
}

void SysLog::DoLogTrace(long level, const char *logMsg)
{
	switch (level) {
		case LOG_DEBUG:
			DoTraceLevel("DEBUG: ", logMsg);
			break;
		case LOG_INFO:
			DoTraceLevel("INFO: ", logMsg);
			break;
		case LOG_WARNING:
			DoTraceLevel("WARNING: ", logMsg);
			break;
		case LOG_ERR:
			DoTraceLevel("ERROR: ", logMsg);
			break;
		case LOG_ALERT:
			DoTraceLevel("ALERT: ", logMsg);
			break;

		default:
			break;
	}
}

//---- UnixSysLog ----------------------------------------------------------------
#if defined(__sun) || defined(__linux__) || defined(__aix__)

UnixSysLog::UnixSysLog(const char *appId)
{
	// initialize the syslog channel
	::openlog(appId, LOG_PID, LOG_USER);
}

UnixSysLog::~UnixSysLog()
{
	// close the syslog channel for this application
	::closelog();
}

void UnixSysLog::DoSystemLevelLog(long level, const char *logMsg)
{
	::syslog(level, "%s", (const char *)logMsg);
}

#endif

#if defined(__370__)

void S370SysLog::DoSystemLevelLog(long level, const char *msg)
{
	cerr << "level " << level << ": " << logMsg << endl;
}

void SysLog::DoLogTrace(long level, const char *msg)
{
	// do nothing we already logged it on cerr
}

#endif

#if defined(WIN32)
Win32SysLog::Win32SysLog(const char *appId)
{
	fLogHandle =	::RegisterEventSource(0, appId);
}

Win32SysLog::~Win32SysLog()
{
	::DeregisterEventSource( fLogHandle );
}

void Win32SysLog::DoSystemLevelLog(long level, const char *logMsg)
{
	WORD evtType = EVENTLOG_INFORMATION_TYPE;
	LPCTSTR str[1];
	str[0] = logMsg;

	switch (level) {
		case LOG_WARNING:
			evtType = EVENTLOG_WARNING_TYPE;
			break;
		case LOG_ERR:
		case LOG_ALERT:
			evtType = EVENTLOG_ERROR_TYPE;
			break;
		default:
			break;
	}
	::ReportEvent(fLogHandle, evtType, 0, 0, 0, 2, 0, str, 0);
}
#endif
