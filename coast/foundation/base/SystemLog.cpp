/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SystemLog.h"
#if defined(WIN32)
#define IOSTREAM_IS_THREADSAFE
#endif

//--- standard modules used ----------------------------------------------------
#include "System.h"

//--- c-library modules used ---------------------------------------------------
#include <errno.h>
#include <cstdio>
#include <cstring>
#if !defined(WIN32)
#include <syslog.h>
#endif
#if defined(__SUNPRO_CC)
#include <strings.h>
#include <stdio.h>
#endif

//--- SystemLog ----------------------------------------------------------
SystemLog *SystemLog::fgSysLog = 0;
SystemLog::eLogLevel SystemLog::fgDoSystemLevelLog = SystemLog::eALERT;
SystemLog::eLogLevel SystemLog::fgDoLogOnCerr = SystemLog::eERR;

//--- module initialization/termination
void SystemLog::Init(const char *appId)
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

	String strValue = System::EnvGet("COAST_DOLOG");
	int iValue = (int)strValue.AsLong(eNone);
	if ( ( eNone < iValue ) && ( iValue < eLast ) ) {
		fgDoSystemLevelLog = (eLogLevel)iValue;
	}
	strValue = System::EnvGet("COAST_LOGONCERR");
	iValue = (int)strValue.AsLong(eNone);
	if ( ( eNone < iValue ) && ( iValue < eLast ) ) {
		fgDoLogOnCerr = (eLogLevel)iValue;
	}

	// initialize the syslog channel
	if ( !fgSysLog ) {
		// open the syslog channel for this application
		// there is always only one syslog channel per application
#if defined(WIN32)
		fgSysLog = new Win32SysLog(appId);
#elif defined(__370__)
		fgSysLog = new S370SysLog();
#else
		fgSysLog = new UnixSysLog(appId);
#endif
	}
}

void SystemLog::Terminate()
{
	if ( fgSysLog ) {
		delete fgSysLog;
		fgSysLog = 0;
	}
}

//--- logging API -------------------------------------

// severity eDEBUG for tracing server
// activity during development and
// deployment
void SystemLog::Debug(const char *msg)
{
	Log(eDEBUG, msg);
}

// severity eINFO for general information
// log general useful information about server
// activity
void SystemLog::Info(const char *msg)
{
	Log(eINFO, msg);
}

// severity eWARNING	for information about
// inconsistent state or potential dangerous
// situation
void SystemLog::Warning(const char *msg)
{
	Log(eWARNING, msg);
}

// severity eERR for outright errors
// during operation of the server without
// fatal results
void SystemLog::Error(const char *msg)
{
	Log(eERR, msg);
}

// severity eALERT for fatal errors
// this call triggers also an alert on
// the operator console
void SystemLog::Alert(const char *msg)
{
	Log(eALERT, msg);
}

// bottleneck routine used by others
// here you can use severity levels directly
void SystemLog::Log(eLogLevel level, const char *msg)
{
	if ( InitOnce() ) {
		fgSysLog->DoLog(level, msg);
	}
}

bool SystemLog::InitOnce()
{
	static bool once = false;
	if (!once) {
		once = true;
		Init(0);
	}
	return (fgSysLog != NULL);
}

//--- utilities ------------------------------------

// const char *SysErrorMsg(long errnum) maps the error number in errnum
// to an error  message  string,  and returns a pointer to that string.
// SysErrorMsg(long errnum) uses the same set of error messages as perror().
// The returned string should not be overwritten.
String SystemLog::SysErrorMsg(long errnum)
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

	String msg((const char *)lpMsgBuf, strlen(lpMsgBuf), Storage::Global());
	// Free the buffer.
	LocalFree( lpMsgBuf );
	return msg;
#else
	String theError(Storage::Global());
	if (errnum == EMFILE) {
		theError = "[No more filehandles in Process.] ";
	}
	if (errnum == ENFILE) {
		theError = "[No more filehandles in System.] ";
	}
	theError.Append((long)errnum).Append(' ').Append(strerror(errnum));
	return theError;
#endif
}

// const char *LastSysError() returns the error message retrieved with errno
// from SysErrorMsg(errno)
String SystemLog::LastSysError()
{
	int iError( System::GetSystemError() );
	if ( iError != 0 ) {
		return SysErrorMsg( iError );
	}
	return String().Append((long)iError).Append(": no system-error");
}

//magic function to be used by Assert macro to avoid dependency to class SystemLog and SystemLog.h
int syslog_assert(const char *file, long line, const char *assertion)
{
	return SystemLog::LogAssert(file, line, assertion);
}

int SystemLog::LogAssert(const char *file, long line, const char *assertion)
{
	// implement brain dead with C style things to avoid problems of recursion
	char asrt_buf[2048];
	int asrt_buf_used = sprintf(asrt_buf, "%s:%ld\n Assert(%s) failed\n", file, line, assertion);
	\
	SystemLog::WriteToStderr(asrt_buf, asrt_buf_used);
	return 0;
}

void SystemLog::WriteToStderr(const String &msg)
{
	SystemLog::WriteToStderr((const char *)msg, msg.Length());
}

void SystemLog::WriteToStderr(char *msg, long length)
{
	SystemLog::WriteToStderr((const char *)msg, length);
}

void SystemLog::WriteToStderr(const char *msg, long length)
{
	if ( msg ) {
		long sLen = length;
		if ( sLen < 0 ) {
			sLen = strlen(msg);
		}
#ifdef IOSTREAM_IS_THREADSAFE
		cerr.write(msg, sLen).flush();
#else
		::write(2, msg, sLen);
#endif
	}
}

void SystemLog::WriteToStdout(const String &msg)
{
	SystemLog::WriteToStdout((const char *)msg, msg.Length());
}

void SystemLog::WriteToStdout(char *msg, long length)
{
	SystemLog::WriteToStdout((const char *)msg, length);
}

void SystemLog::WriteToStdout(const char *msg, long length)
{
	if ( msg ) {
		long sLen = length;
		if ( sLen < 0 ) {
			sLen = strlen(msg);
		}
#ifdef IOSTREAM_IS_THREADSAFE
		cout.write(msg, sLen).flush();
#else
		::write(1, msg, sLen);
#endif
	}
}

SystemLog::SystemLog()
{
}

SystemLog::~SystemLog()
{
}

void SystemLog::DoLog(eLogLevel level, const char *msg)
{
	// override logging parameter if it is an alert message
	if ( level >= fgDoSystemLevelLog ) {
		DoSystemLevelLog(level, msg);
	}
	// override logging parameter if it is an error or alert message
	if ( level >= fgDoLogOnCerr ) {
		DoLogTrace(level, msg);
	}
}

void SystemLog::DoTraceLevel(const char *level, const char *msg)
{
	String finalMessage(level, strlen(level), Storage::Global());
	finalMessage.Append(msg);
	finalMessage.Append('\n');
	SystemLog::WriteToStderr(finalMessage, finalMessage.Length());
}

void SystemLog::DoLogTrace(eLogLevel level, const char *logMsg)
{
	switch (level) {
		case eDEBUG:
			DoTraceLevel("DEBUG: ", logMsg);
			break;
		case eINFO:
			DoTraceLevel("INFO: ", logMsg);
			break;
		case eWARNING:
			DoTraceLevel("WARNING: ", logMsg);
			break;
		case eERR:
			DoTraceLevel("ERROR: ", logMsg);
			break;
		case eALERT:
			DoTraceLevel("ALERT: ", logMsg);
			break;

		default:
			break;
	}
}

//---- System specific logger ----------------------------------------------------------------
#if defined(WIN32)
Win32SysLog::Win32SysLog(const char *appId)
{
	fLogHandle =	::RegisterEventSource(0, appId);
}

Win32SysLog::~Win32SysLog()
{
	::DeregisterEventSource( fLogHandle );
}

void Win32SysLog::DoSystemLevelLog(eLogLevel level, const char *logMsg)
{
	WORD evtType = EVENTLOG_INFORMATION_TYPE;
	LPCTSTR str[1];
	str[0] = logMsg;

	switch (level) {
		case eWARNING:
			evtType = EVENTLOG_WARNING_TYPE;
			break;
		case eERR:
		case eALERT:
			evtType = EVENTLOG_ERROR_TYPE;
			break;
		default:
			break;
	}
	::ReportEvent(fLogHandle, evtType, 0, 0, 0, 2, 0, str, 0);
}
#elif defined(__370__)
void S370SysLog::DoSystemLevelLog(eLogLevel level, const char *msg)
{
	cerr << "level " << level << ": " << logMsg << endl;
}
#else
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

void UnixSysLog::DoSystemLevelLog(eLogLevel level, const char *logMsg)
{
	// need to do a remapping of internal code to unix specific code now
	long lLevel = LOG_DEBUG;
	switch (level) {
		case eINFO:
			lLevel = LOG_INFO;
			break;
		case eWARNING:
			lLevel = LOG_WARNING;
			break;
		case eERR:
			lLevel = LOG_ERR;
			break;
		case eALERT:
			lLevel = LOG_ALERT;
			break;
		default:
			break;
	};
	::syslog(lLevel, "%s", (const char *)logMsg);
}
#endif

