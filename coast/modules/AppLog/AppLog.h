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
//! <b>Provide an API for application logging</b>
/*!
AppLog defines calling API (AppLogClient) for logging items directly
and a subclass API (FDLogHandler) for logging important elements
conveniently. The method of this class are called by Coast
<b>Configuration:</b><pre>
{
	/LogDir					String				optional, default "log", relative - to WD_ROOT - or absolute path of logfile location
	/RotateDir				String				optional, default <LogDir>/rotate, relative - to WD_ROOT - or absolute path to store away logfiles during rotate or module startup
	/RotateTime				String				optional, default "24:00", time of rotation hh:mm[:ss], rotation will be done daily at the given time, seconds can be optionally specified
	/RotateSecond			Long				optional, default 0, second in day when rotation takes place, takes precedence in case RotateTime is also given
	/Servers {				Anything			mandatory, list of registered servernames to have logging for, the ServersModule <b>must</b> be initialized before AppLogModule
		/ServerName	{		Anything			mandatory, name of the registered server to create AppLogChannels for \note If the channel list is empty and the ServerName has a superclass Server with logging config, both servers will log into the same logfiles
			/ChannelName {	Anything			optional (see above), name of the named AppLogChannel to create
				/FileName	String				mandatory, filename of the logfile including extension
				/Format		Rendererspec		mandatory, Renderer specification for logging output, rendering will be done for each line of logging, can be a time consuming operation if a complex script is given
				/Header		Anything or String	optional, single string or list of strings which get printed first in the newly created logfile
				/SuppressEmptyLines	long		optional, default 0, set to 1 if you want to suppress logging of empty rendered log messages
				/DoNotRotate	long			optional, default 0 (false), if set to 1, this log-channel will not rotate its logfile at the specified time
				/Rendering	long				optional, default 1. If not set, a slot having the ChannelName in tmpStore is evaluated as String to extract
												the log message. eg. ctx.GetTmpStore()["ChannelName"] = "my log message". A "\n" will be added after each messge line.
				/LogMsgSizeHint	 long			optional, reserve LogMsgSizeHint bytes for the internal string holding the message to be logged.
			}
			...
		}
		...
	}
}</pre>
*/
class EXPORTDECL_APPLOG AppLogModule : public WDModule
{
	friend class AppLogChannel;
	friend class AppLogTest;
public:
	AppLogModule(const char *name);
	~AppLogModule();

	virtual bool Init(const ROAnything config);
	virtual bool Finis();

	//!bottleneck routine for logging
	static bool Log(Context &ctx, const char *logChannel);

	//!bottleneck routine for logging
	static bool Log(Context &ctx, const char *logChannel, const ROAnything &config);

	//!Trigger immediate log rotation. Does not interfere with LogRotator thread because
	//!Rotate() uses Mutex.
	static bool RotateSpecificLog(Context &ctx, const char *logchannel);

protected:
	static AppLogChannel *FindLogger(Context &ctx, const char *logChannel);
	AppLogChannel *GetLogChannel(const char *servername, const char *logChannel, bool &canRotate);

	//!Opens the log streams for one server
	bool MakeChannels(const char *servername, const Anything &config);

	/*! Start thread to rotate log files
		\param rotateTime hour:minute to rotate logfiles at
		\param lRotateSecond second in day when to rotate the log files
		\return true in case the rotator thread could be initialized and started */
	bool StartLogRotator(const char *rotateTime, long lRotateSecond);
	bool TerminateLogRotator();
	bool DoRotateLogs();
	static bool RotateLogs();

	Anything fLogConnections;
	ROAnything fROLogConnections;

	class EXPORTDECL_APPLOG LogRotator : public Thread
	{
		friend class AppLogTest;
	public:
		LogRotator(const char *rotateTime, long lRotateSecond = 0L);

	protected:
		long GetSecondsToWait();
		void Run();
		//! when to rotate
		long fRotateSecond;
	} *fRotator;
	// gcc 2.95.x fix: friend declaration must be after nested class declaration
	friend class LogRotator;

	static AppLogModule *fgAppLogModule;
};

//---- AppLogChannel -----------------------------------------------------------
//! this class holds parameters identifying a single logstream belonging to a server
class EXPORTDECL_APPLOG AppLogChannel : public NotCloned
{
	friend class AppLogTest;
public:
	AppLogChannel(const char *name, const Anything &channel);
	virtual ~AppLogChannel();

	bool Log(Context &ctx);
	bool LogAll(Context &ctx, const ROAnything &config);

	bool Rotate(bool overrideDoNotRotateLogs = false);
	ROAnything GetChannelInfo() {
		return fChannelInfo;
	}

	static String GenTimeStamp(const String &format);

protected:
	bool GetLogDirectories(ROAnything channel, String &logdir, String &rotatedir);
	bool RotateLog(const String &logdirName, const String &rotatedirName, String &logfileName);

	ostream *OpenLogStream(ROAnything channel, String &logfileName);

	void WriteHeader(ostream &os);

private:
	//! stream where logs are written to
	ostream *fLogStream;
	//! information about the log stream
	Anything fChannelInfo;
	//! guard of stream
	Mutex fChannelMutex;
};

#endif
