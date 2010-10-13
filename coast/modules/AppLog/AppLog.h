/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef AppLog_H
#define AppLog_H

#include "config_applog.h"
#include "WDModule.h"
#include "Threads.h"

class Context;
class AppLogChannel;

//---- AppLogModule -----------------------------------------------------------
//! Provide an API for application logging
/*!
AppLog defines calling API (AppLogClient) for logging items directly
and a subclass API (FDLogHandler) for logging important elements
conveniently. The method of this class are called by Coast
\par Configuration
\code
{
	/LogDir					String				optional, default "log", relative - to COAST_ROOT - or absolute path of logfile location
	/RotateDir				String				optional, default <LogDir>/rotate, relative - to COAST_ROOT - or absolute path to store away logfiles during rotate or module startup
	/RotateTime				String				optional, default "24:00", time of rotation hh:mm[:ss], rotation will be done daily at the given time, seconds can be optionally specified
	/RotateSecond			long				optional, default 0, second in day when rotation takes place, takes precedence in case RotateTime is also given
	/RotateTimeIsGmTime		bool				optional, default 0, use GMT time to determine log rotation time. Default is local time.
	/RotateEveryNSecond		long				optional, default 0, rotates the logs every N seconds, if given, takes priority over /RotateSecond
	/RotateEveryNSecondTime	long				optional, default "00:00:00", rotates the logs every N seconds, if given, takes priority over /RotateSecond
	/Servers {				Anything			mandatory, list of registered servernames to have logging for, the ServersModule <b>must</b> be initialized before AppLogModule
		/ServerName	{		Anything			mandatory, name of the registered server to create AppLogChannels for \note If the channel list is empty and the ServerName has a superclass Server with logging config, both servers will log into the same logfiles
			/ChannelName {	Anything			optional (see above), name of the named AppLogChannel to create
				/FileName	String				mandatory, filename of the logfile including extension
				/Format		Rendererspec		mandatory, Renderer specification for logging output, rendering will be done for each line of logging, can be a time consuming operation if a complex script is given
				/Header		Anything or String	optional, single string or list of strings which get printed first in the newly created logfile
				/SuppressEmptyLines	long		optional, default 0, set to 1 if you want to suppress logging of empty rendered log messages
				/DoNotRotate	long			optional, default 0 (false), if set to 1, this log-channel will not rotate its logfile at the specified time
				/Rendering		long			optional, default 1. If not set, a slot having the ChannelName in tmpStore is evaluated as String to extract
												the log message. eg. ctx.GetTmpStore()["ChannelName"] = "my log message". A "\n" will be added after each messge line.
				/LogMsgSizeHint	long			optional, reserve LogMsgSizeHint bytes for the internal string holding the message to be logged.
				/BufferItems	long			optional, default 0, (no buffering) buffer <n> items before writing them to the log stream
				/Severity		long			optional, default AppLogModule::eALL, Severity [CRITICAL=1, FATAL=2, ERROR=4, WARN=8, INFO=16, OK=32, MAINT=64, DEBUG=128], all levels lower_equal (<=) the specified value will get logged
			}
			...
		}
		...
	}
}
\endcode

*/
class EXPORTDECL_APPLOG AppLogModule : public WDModule
{
	friend class AppLogChannel;
	friend class AppLogTest;
public:
	AppLogModule(const char *name);
	virtual ~AppLogModule();

	virtual bool Init(const ROAnything config);
	virtual bool Finis();

	/*! define importancy levels in increasing order for easier */
	enum eLogLevel {
		eNone,
		eCRITICAL = 1,
		eFATAL = 2,
		eERROR = 4,
		eWARNING = 8,
		eINFO = 16,
		eOK = 32,
		eMAINT = 64,
		eDEBUG = 128,
		eALL = (eCRITICAL | eFATAL | eERROR | eWARNING | eINFO | eOK | eMAINT | eDEBUG),
		eLast
	};

	//!bottleneck routine for logging
	static bool Log(Context &ctx, const char *logChannel, eLogLevel iLevel);

	//!bottleneck routine for logging
	static bool Log(Context &ctx, const char *logChannel, const ROAnything &config, eLogLevel iLevel);

	//!bottleneck routine for logging
	static bool Log(Context &ctx, const char *logChannel, const String &strMessage, eLogLevel iLevel);

	//!Trigger immediate log rotation. Does not interfere with LogRotator thread because
	//!Rotate() uses Mutex.
	static bool RotateSpecificLog(Context &ctx, const char *logchannel);

	static String GetSeverityText(eLogLevel iLevel);

protected:
	static AppLogChannel *FindLogger(Context &ctx, const char *logChannel);
	AppLogChannel *GetLogChannel(const char *servername, const char *logChannel, bool &canRotate);

	//!Opens the log streams for one server
	bool MakeChannels(const char *servername, const Anything &config);

	/*! Start thread to rotate log files
		\param rotateTime hour:minute to rotate logfiles at
		\param lRotateSecond second in day when to rotate the log files
		\return true in case the rotator thread could be initialized and started */
	bool StartLogRotator(const char *rotateTime, long lRotateSecond, const char *lEveryNSecondsTime, long leveryNSeconds, bool isGmTime);
	bool TerminateLogRotator();
	bool DoRotateLogs();
	static bool RotateLogs();

	Anything fLogConnections;
	ROAnything fROLogConnections;

	class EXPORTDECL_APPLOG LogRotator : public Thread
	{
		friend class AppLogTest;
	public:
		LogRotator(const char *rotateTime, const char *everyNSecondsTime, long lRotateSecond = 0L, long lEveryNSeconds = 0L, bool isGmTime = false);

	protected:
		long GetSecondsToWait();
		long ParseTimeString( const char *time);
		void Run();
		//! when to rotate
		long fRotateSecond;
		long fEveryNSeconds;
		bool fIsGmTime;
	} *fRotator;
	// gcc 2.95.x fix: friend declaration must be after nested class declaration
	friend class LogRotator;

	static AppLogModule *fgAppLogModule;
};

//---- AppLogChannel -----------------------------------------------------------
//! Handles logging into a specific channel
/*!
\par Configuration
\code
{
	/ChannelName {	Anything			optional (see above), name of the named AppLogChannel to create
		/FileName	String				mandatory, filename of the logfile including extension
		/Format		Rendererspec		mandatory, Renderer specification for logging output, rendering will be done for each line of logging, can be a time consuming operation if a complex script is given
		/Header		Anything or String	optional, single string or list of strings which get printed first in the newly created logfile
		/SuppressEmptyLines	long		optional, default 0, set to 1 if you want to suppress logging of empty rendered log messages
		/DoNotRotate	long			optional, default 0 (false), if set to 1, this log-channel will not rotate its logfile at the specified time
		/Rendering		long			optional, default 1. If not set, a slot having the ChannelName in tmpStore is evaluated as String to extract
										the log message. eg. ctx.GetTmpStore()["ChannelName"] = "my log message". A "\n" will be added after each messge line.
		/LogMsgSizeHint	long			optional, reserve LogMsgSizeHint bytes for the internal string holding the message to be logged.
		/BufferItems	long			optional, default 0, (no buffering) buffer <n> items before writing them to the log stream
		/Severity		long			optional, default AppLogModule::eALL, Severity [CRITICAL=1, FATAL=2, ERROR=4, WARN=8, INFO=16, OK=32, MAINT=64, DEBUG=128], all levels lower_equal (<=) the specified value will get logged
	}
}
\endcode

During the rendering process of Format, the following fields exist in the Context for lookup:
\par Temporary slots in Context:
\code
{
	/LogSeverity		long			value of severity level when Log() was called
	/LogSeverityText	String			text of severity level, eg. [CRITICAL|FATAL...] see above
}
\endcode

*/
class EXPORTDECL_APPLOG AppLogChannel : public RegisterableObject
{
	friend class AppLogTest;
	// use careful, you inhibit subclass use
	//--- private class api
	// block the following default elements of this class
	// because they're not allowed to be used
	AppLogChannel();
	AppLogChannel(const AppLogChannel &);
	AppLogChannel &operator=(const AppLogChannel &);
public:
	AppLogChannel(const char *name);
	virtual ~AppLogChannel();

	bool InitClone(const char *name, const Anything &channel);

	bool Log(Context &ctx, AppLogModule::eLogLevel iLevel);
	bool LogAll(Context &ctx, AppLogModule::eLogLevel iLevel, const ROAnything &config);

	bool Rotate(bool overrideDoNotRotateLogs = false);
	ROAnything GetChannelInfo() {
		return fChannelInfo;
	}

	static String GenTimeStamp(const String &format);

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) AppLogChannel("ClonedAppLogChannel");
	}

	RegCacheDef(AppLogChannel);

protected:
	virtual bool DoInitClone(const char *name, const Anything &channel);
	bool GetLogDirectories(ROAnything channel, String &logdir, String &rotatedir);
	bool RotateLog(const String &logdirName, const String &rotatedirName, String &logfileName);

	ostream *OpenLogStream(ROAnything channel, String &logfileName);

	void WriteHeader(ostream &os);

	virtual void DoCreateLogMsg(Context &ctx, AppLogModule::eLogLevel iLevel, String &logMsg, const ROAnything &config);

	AppLogModule::eLogLevel GetSeverity() const {
		return fSeverity;
	}

private:
	//! stream where logs are written to
	ostream *fLogStream;
	//! information about the log stream
	Anything fChannelInfo;
	//! Don't write empty lines
	bool fSuppressEmptyLines;
	//! Don't render log message, spool it directly
	bool fRendering;
	//! Size of buffer the log messge is rendered into
	long fLogMsgSizeHint;
	//! Don't rotate this channel
	bool fDoNotRotate;
	//! Don't write log entries (header is written)
	bool fNoLogItemsWrite;
	Anything fFormat;
	//! guard of stream
	SimpleMutex fChannelMutex;
	long fBufferItems;
	String fBuffer;
	long fItemsWritten;
	AppLogModule::eLogLevel fSeverity;
};

#endif
