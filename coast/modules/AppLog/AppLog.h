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
class LogRotator;

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
	/RotateTime				String				optional, default "24:00", time of rotation, rotation will be done daily at the given time
	/Servers {				Anything			mandatory, list of registered servernames to have logging for, the ServersModule <b>must</b> be initialized before AppLogModule
		/ServerName	{		Anything			mandatory, name of the registered server to create AppLogChannels for \note If the channel list is empty and the ServerName has a superclass Server with logging config, both servers will log into the same logfiles
			/ChannelName {	Anything			optional (see above), name of the named AppLogChannel to create
				/FileName	String				mandatory, filename of the logfile including extension
				/Format		Rendererspec		mandatory, Renderer specification for logging output, rendering will be done for each line of logging, can be a time consuming operation if a complex script is given
				/Header		Anything or String	optional, single string or list of strings which get printed first in the newly created logfile
				/SuppressEmptyLines	long		optional, default 0, set to 1 if you want to suppress logging of empty rendered log messages
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
	friend class LogRotator;
	friend class AppLogTest;
public:
	AppLogModule(const char *name);
	~AppLogModule();

	virtual bool Init(const Anything &config);
	virtual bool Finis();

	//!bottleneck routine for logging
	static bool Log(Context &ctx, const char *logChannel);

	//!bottleneck routine for logging
	static bool Log(Context &ctx, const char *logChannel, const ROAnything &config);

protected:
	static AppLogChannel *FindLogger(Context &ctx, const char *logChannel);
	AppLogChannel *GetLogChannel(const char *servername, const char *logChannel, bool &canRotate);

	//!Opens the log streams for one server
	bool MakeChannels(const char *servername, const Anything &config);

	// log rotation
	bool StartLogRotator(const char *waittime); // in hour:minutes
	bool TerminateLogRotator();
	bool DoRotateLogs();
	static bool RotateLogs();

	Anything fLogConnections;
	ROAnything fROLogConnections;

	LogRotator *fRotator;

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

	bool Rotate();

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
