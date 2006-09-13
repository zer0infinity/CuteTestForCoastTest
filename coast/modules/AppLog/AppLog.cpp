/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "AppLog.h"

//--- project modules used -----------------------------------------------------
#include "Server.h"
#include "Renderer.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------
#if defined(WIN32)
#include <stdio.h>
#endif

//---- AppLogModule -----------------------------------------------------------
RegisterModule(AppLogModule);

AppLogModule *AppLogModule::fgAppLogModule = 0;

AppLogModule::AppLogModule(const char *name)
	: WDModule(name)
	, fRotator(0)
{
}

AppLogModule::~AppLogModule()
{
}

bool AppLogModule::Init(const ROAnything config)
{
	// initialization of application logging tries a socket connection
	// defined in the configuration file
	SysLog::WriteToStderr("\tStarting Application Logging");
	bool retCode = true;
	ROAnything appLogConfig;
	StartTrace(AppLogModule.Init);

	if ( config.LookupPath(appLogConfig, "AppLogModule") && appLogConfig["Servers"].GetSize() ) {
		TraceAny(appLogConfig, "AppLogConfig:");
		ROAnything servers(appLogConfig["Servers"]);
		long numOfServers = servers.GetSize();
		String strLogDir, strRotateDir;
		strLogDir = appLogConfig["LogDir"].AsCharPtr("log");
		strRotateDir = strLogDir;
		strRotateDir << System::Sep() << "rotate";
		if ( appLogConfig.IsDefined("RotateDir") ) {
			strRotateDir = appLogConfig["RotateDir"].AsCharPtr();
		}
		Trace("raw LogDir [" << strLogDir << "]");
		Trace("raw RotateDir [" << strRotateDir << "]");
		// generate for each server its respective logchannels
		for (long i = 0; i < numOfServers; ++i) {
			const char *servername = servers.SlotName(i);
			Server *s;
			if ( servername && (s = Server::FindServer(servername)) ) {
				Anything cfg;
				cfg["Channels"] = servers[i].DeepClone();
				cfg["LogDir"] = strLogDir;
				cfg["RotateDir"] = strRotateDir;
				retCode = retCode && MakeChannels(servername, cfg);
			}
			SysLog::WriteToStderr(".", 1);
		}
		TraceAny(fLogConnections, "LogConnections: ");
		if (retCode && StartLogRotator(appLogConfig["RotateTime"].AsCharPtr("24:00"), appLogConfig["RotateSecond"].AsLong(0L)) ) {
			fgAppLogModule = this;
			fROLogConnections = fLogConnections;
			SysLog::WriteToStderr(" done\n");
		} else {
			fgAppLogModule = 0L;
			SysLog::WriteToStderr(" failed\n");
		}
	} else {
		SysLog::WriteToStderr(" done\n");
	}
	return retCode;
}

bool AppLogModule::Finis()
{
	StartTrace(AppLogModule.Finis);
	SysLog::WriteToStderr("\tTerminating Application Logging");
	TerminateLogRotator();
	String strDoNotRotate("DoNotRotate");
	for (long i = 0, sz = fLogConnections.GetSize(); i < sz; ++i) {
		TraceAny(fLogConnections[i], "config of server [" << fLogConnections.SlotName(i) << "]");
		// ServerConfigs with DoNotRotate slot are duplicates, so do not delete their channels!
		if ( !fLogConnections[i].IsDefined("DoNotRotate") ) {
			for (long j = 0, szj = fLogConnections[i].GetSize(); j < szj; ++j) {
				Trace("finishing channel [" << fLogConnections[i].SlotName(j) << "]");
				IFAObject *logchannel = fLogConnections[i][j][0L].AsIFAObject(0);
				if (logchannel) {
					delete logchannel;
				}
				fLogConnections[i][j] = 0L;
			}
		}
	}
	fLogConnections = MetaThing();
	fgAppLogModule = 0L;
	SysLog::WriteToStderr(" done\n");
	return true;
}

bool AppLogModule::MakeChannels(const char *servername, const Anything &config)
{
	StartTrace1(AppLogModule.MakeChannels, "Server [" << servername << "]");
	if ( !servername ) {
		servername = "Server";
	}

	Anything anyChannels = config["Channels"];
	long numOfChannels = anyChannels.GetSize();

	TraceAny(config, "ServerConfig");

	if ( numOfChannels > 0 ) {
		// has its own definition of logging
		for (long i = 0; i < numOfChannels; ++i) {
			String strChannelName = anyChannels.SlotName(i);
			if ( strChannelName.Length() ) {
				Anything channel = anyChannels[i];
				TraceAny(channel, "config of channel [" << strChannelName << "]");
				if ( !channel.IsDefined("LogDir") ) {
					channel["LogDir"] = config["LogDir"];
				}
				if ( !channel.IsDefined("RotateDir") ) {
					channel["RotateDir"] = config["RotateDir"];
				}
				AppLogChannel *pChannel = new AppLogChannel(strChannelName, channel);
				fLogConnections[servername][strChannelName] = pChannel;
				if ( !pChannel || !pChannel->Rotate() ) {
					SYSERROR("LogChannel [" << strChannelName << "] failed to open logfile");
					return false;
				}
			}
		}
	} else {
		Trace("might use the superservers logs if they exist");
		String serverLeafName(servername);
		bool configured = false;

		Server *server = Server::FindServer(servername);
		while ( server && !configured ) {
			String serverName;
			server->GetName(serverName);
			Trace("testing <" << serverName << "> for connections");
			Anything connections;
			if ( fLogConnections.LookupPath(connections, serverName) ) {
				Trace("config for <" << serverName << "> found");
				TraceAny(connections, "Connections");
				// must break reference semantics otherways the log does not get rotated!
				fLogConnections[serverLeafName] = connections.DeepClone();
				fLogConnections[serverLeafName]["DoNotRotate"] = true;
				configured = true;
			} else {
				Trace("no config for <" << serverName << ">, using super");
				server = (Server *)server->GetSuper();
			}
		}
	}
	return true;
}

bool AppLogModule::RotateLogs()
{
	return (fgAppLogModule && fgAppLogModule->DoRotateLogs());
}

bool AppLogModule::DoRotateLogs()
{
	StartTrace(AppLogModule.DoRotateLogs);
	long configSz = fROLogConnections.GetSize();
	for (long i = 0; i < configSz; ++i) {
		const char *servername = fROLogConnections.SlotName(i);
		ROAnything roaLogChannels = fROLogConnections[i];
		long roaLogChannelsSz = roaLogChannels.GetSize();
		for (long j = 0; j < roaLogChannelsSz; ++j) {
			const char *channelname = roaLogChannels.SlotName(j);
			if ( channelname && servername ) {
				bool doNotRotate = false;
				AppLogChannel *applog = GetLogChannel(servername, channelname, doNotRotate);
				if (applog && !doNotRotate) {
					applog->Rotate();
				}
			}
		}
	}
	return true;
}

bool AppLogModule::StartLogRotator(const char *rotateTime, long lRotateSecond)
{
	fRotator = new LogRotator(rotateTime, lRotateSecond);

	if (fRotator) {
		return fRotator->Start();
	}

	return false;
}

bool AppLogModule::TerminateLogRotator()
{
	if (fRotator) {
		fRotator->Terminate();
		delete fRotator;
		fRotator = 0;
		return true;
	}
	return false;
}

AppLogChannel *AppLogModule::GetLogChannel(const char *servername, const char *logChannel, bool &doNotRotate)
{
	StartTrace1(AppLogModule.GetLogChannel, "server [" << servername << "] LogChannel [" << NotNull(logChannel) << "]");

	AppLogChannel *logger = NULL;
	doNotRotate = false;
	ROAnything loggerConfig;
	if ( servername && logChannel && fROLogConnections.LookupPath(loggerConfig, servername) && loggerConfig.IsDefined(logChannel) ) {
		doNotRotate = loggerConfig["DoNotRotate"].AsBool(false);
		TraceAny(loggerConfig, "logs for server [" << servername << "] must " << (doNotRotate ? "not " : "") << "be rotated");
		logger = SafeCast(loggerConfig[logChannel].AsIFAObject(0), AppLogChannel);
	} else {
		SYSERROR("log channel [" << servername << '.' << NotNull(logChannel) << "] not found");
	}
	return logger;
}

AppLogChannel *AppLogModule::FindLogger(Context &ctx, const char *logChannel)
{
	StartTrace1(AppLogModule.FindLogger, "LogChannel [" << NotNull(logChannel) << "]");
	AppLogChannel *logger = NULL;
	if ( fgAppLogModule ) {
		Server *s = ctx.GetServer();
		String servername;
		if ( s && s->GetName(servername) ) {
			bool dummy;
			logger = fgAppLogModule->GetLogChannel(servername, logChannel, dummy);
			if ( !logger ) {
				SYSERROR("log channel [" << servername << "." << NotNull(logChannel) << "] returned no logger object");
			}
		} else {
			SYSERROR("Server [" << servername << "] not valid");
		}
	}
	return logger;
}

bool AppLogModule::Log(Context &ctx, const char *logChannel)
{
	StartTrace1(AppLogModule.Log, "LogChannel [" << NotNull(logChannel) << "]" );
	Server *s = ctx.GetServer();
	if (!s) {
		s = Server::FindServer("Server");
		ctx.SetServer(s);
	}
	AppLogChannel *logger = AppLogModule::FindLogger(ctx, logChannel);
	if ( logger ) {
		Trace("logger found");
		return logger->Log(ctx);
	}
	Trace("logger not found");
	return false;
}

bool AppLogModule::Log(Context &ctx, const char *logChannel, const ROAnything &config)
{
	StartTrace1(AppLogModule.Log, "LogChannel [" << NotNull(logChannel) << "]" );
	TraceAny(config, "Config: ");
	Server *s = ctx.GetServer();
	if (!s) {
		s = Server::FindServer("Server");
		ctx.SetServer(s);
	}
	AppLogChannel *logger = FindLogger(ctx, logChannel);
	if ( logger ) {
		return logger->LogAll(ctx, config);
	}

	return false;
}

//---- AppLogChannel ---------------------------------------------------------------------------------------
AppLogChannel::AppLogChannel(const char *name, const Anything &channel)
	: NotCloned(name)
	, fLogStream(NULL)
	, fChannelInfo(channel)
	, fChannelMutex(name)
{
}

AppLogChannel::~AppLogChannel()
{
	if (fLogStream) {
		delete fLogStream;
	}
	fLogStream = 0;
}

bool AppLogChannel::Log(Context &ctx)
{
	StartTrace(AppLogChannel.Log);
	return LogAll(ctx, GetChannelInfo()["Format"]);
}

bool AppLogChannel::LogAll(Context &ctx, const ROAnything &config)
{
	StartTrace(AppLogChannel.LogAll);
	bool suppressEmptyLines = GetChannelInfo()["SuppressEmptyLines"].AsBool(false);
	if ( fLogStream && fLogStream->good() ) {
		TraceAny(config, "config: ");
		String logMsg(128);
		Renderer::RenderOnString(logMsg, ctx, config);

		if (!suppressEmptyLines || logMsg.Length()) {
			MutexEntry me(fChannelMutex);
			me.Use();
			Trace("fLogStream state before logging: " << (long)fLogStream->rdstate());
			(*fLogStream) << logMsg << endl;
			Trace("fLogStream state after logging: " << (long)fLogStream->rdstate());
			return (!!(*fLogStream));
		}
	}
	return suppressEmptyLines ? true : false;
}

bool AppLogChannel::GetLogDirectories(ROAnything channel, String &logdir, String &rotatedir)
{
	StartTrace(AppLogChannel.GetLogDirectories);
	logdir = channel["LogDir"].AsCharPtr();
	rotatedir = channel["RotateDir"].AsCharPtr();

	if ( !System::IsAbsolutePath(logdir) ) {
		Trace("log path is non-absolute [" << logdir << "]");
		logdir = System::GetRootDir();
		logdir << System::Sep() << channel["LogDir"].AsCharPtr();
	}
	Trace("current logfile-path [" << logdir << "]");

	if ( !System::IsAbsolutePath(rotatedir) ) {
		Trace("rotate path is non-absolute [" << rotatedir << "]");
		rotatedir = System::GetRootDir();
		rotatedir << System::Sep() << channel["RotateDir"].AsCharPtr();
	}
	Trace("current rotate-path [" << rotatedir << "]");
	bool bGoAhead = true;
	if ( !System::IsDirectory(logdir) ) {
		SYSERROR("log directory [" << logdir << "] does not exist! please create it first");
		bGoAhead = false;
	}
	if ( !System::IsDirectory(rotatedir) ) {
		SYSERROR("rotate directory [" << rotatedir << "] does not exist! please create it first");
		bGoAhead = false;
	}
	return bGoAhead;
}

ostream *AppLogChannel::OpenLogStream(ROAnything channel, String &logfileName)
{
	StartTrace(AppLogChannel.OpenLogStream);
	String logdir, rotatedir;
	bool bGoAhead;

	bGoAhead = GetLogDirectories(channel, logdir, rotatedir);

	logfileName = channel["FileName"].AsCharPtr();

	Trace("rotating [" << logfileName << "]");
	ostream *os = NULL;
	if ( bGoAhead && RotateLog(logdir, rotatedir, logfileName) ) {
		os = System::OpenOStream(logfileName, NULL, ios::app, true);
	}
	Trace(((os) ? " suceeded" : " failed"));
	return os;
}

bool AppLogChannel::RotateLog(const String &logdirName, const String &rotatedirName, String &logfileName)
{
	StartTrace(AppLogChannel.RotateLog);
	Trace("LogFileName [" << logfileName << "]");

	String oldLogFileName(logdirName);
	oldLogFileName << System::Sep() << logfileName;
	String newLogFileName(rotatedirName);
	newLogFileName << System::Sep();

	String stampedLogFileName(logfileName);
	stampedLogFileName << "." << GenTimeStamp("%Y%m%d%H%M%S");
	Trace("rotate LogFileName [" << stampedLogFileName << "]");
	newLogFileName << stampedLogFileName;

	System::ResolvePath(oldLogFileName);
	System::ResolvePath(newLogFileName);
	Trace("old [" << oldLogFileName << "]");
	Trace("new [" << newLogFileName << "]");

	int retCode = 0;
	Trace("testing if old logfile exists and must be rotated");
	if ( System::IsRegularFile(oldLogFileName) ) {
		Trace("testing uniqueness of rotate FileName");
		long lNum = 1;
		String strLastRot(newLogFileName);
		// assume not more than 10 rotates of same file per second...
		while ( System::IsRegularFile(newLogFileName) && lNum < 10 ) {
			Trace("not unique! [" << newLogFileName << "]");
			newLogFileName = strLastRot << '.' << lNum++;
			Trace("New name to test [" << newLogFileName << "]");
		}
		retCode = ::rename(oldLogFileName, newLogFileName);
		if ( retCode != 0 ) {
			SYSWARNING("rotate of [" << oldLogFileName << "] with [" << newLogFileName << "] failed! (" << (long)errno << ") " << SysLog::LastSysError());
		}
	}
	// assign full filename of log to use
	logfileName = oldLogFileName;
	return (retCode == 0);
}

bool AppLogChannel::Rotate()
{
	StartTrace(AppLogChannel.Rotate);
	bool bSuccess = true;
	// check if this channel must be rotated
	TraceAny(GetChannelInfo(), "channel info");
	ostream *pStream = NULL;
	{
		MutexEntry me(fChannelMutex);
		me.Use();
		pStream = fLogStream;
	}
	// must enter here in case the logfile does not exist yet
	if ( !pStream || GetChannelInfo()["DoNotRotate"].AsBool(false) == false ) {
		MutexEntry me(fChannelMutex);
		me.Use();
		if ( fLogStream ) {
			delete fLogStream;
			fLogStream = NULL;
		}
		String logfileName;
		fLogStream = OpenLogStream(GetChannelInfo(), logfileName);
		if ( fLogStream ) {
			WriteHeader(*fLogStream);
		} else {
			SYSERROR("Rotation of [" << logfileName << "] failed");
		}
		bSuccess = ( fLogStream != NULL );
	}
	return bSuccess;
}

void AppLogChannel::WriteHeader(ostream &os)
{
	StartTrace(AppLogChannel.WriteHeader);
	ROAnything header = GetChannelInfo()["Header"];

	Trace("os state before writing header: " << (long)os.rdstate());
	long szh = header.GetSize();
	for (long i = 0; i < szh; ++i) {
		os << header[i].AsCharPtr("");
		if (!os) {
			SYSERROR("Write Header to logfile failed");
			return;
		}
	}
	if ( szh > 0 ) {
		os << endl;
	}
	Trace("os state " << (long)os.rdstate());
}

String AppLogChannel::GenTimeStamp(const String &format)
{
	StartTrace(AppLogChannel.GenTimeStamp);
	const int dateSz = 40;
	time_t now = time(0);
	struct tm res, *tt;
	tt = System::LocalTime(&now, &res);

	// NOTICE: array is limited to gcMaxDateArraySize chars
	char date[dateSz];
	strftime(date, dateSz, format, tt);
	Trace("generated timestamp [" << date << "]");
	return date;
}

//--- LogRotator ----------------------
AppLogModule::LogRotator::LogRotator(const char *rotateTime, long lRotateSecond)
	: Thread("LogRotator")
	, fRotateSecond(lRotateSecond)
{
	StartTrace(LogRotator.LogRotator);
	if ( fRotateSecond == 0 ) {
		StringTokenizer st(rotateTime, ':');
		String hour, minute, second;

		if ( !(st.NextToken(hour) && st.NextToken(minute)) ) {
			SYSERROR("wrong time format [" << rotateTime << "]");
		} else {
			// optional seconds spec
			st.NextToken(second);
			fRotateSecond = ( ( hour.AsLong(0) % 24 * 60 ) + minute.AsLong(0) % 60 ) * 60 + second.AsLong(0) % 60;
		}
	}
	Trace("RotateTime [" << rotateTime << "] param lRotateSecond: " << lRotateSecond << " -> fRotateSecond: " << fRotateSecond);
}

long AppLogModule::LogRotator::GetSecondsToWait()
{
	StartTrace(LogRotator.GetSecondsToWait);

	time_t now = time(0);
	struct tm res, *tt;
	tt = System::LocalTime(&now, &res);

	long lCurrentSecondInDay = ( ( ( ( tt->tm_hour * 60 ) + tt->tm_min ) * 60 ) + tt->tm_sec ) % 86400;
	Trace("fRotateSecond: " << fRotateSecond << " lCurrentSecondInDay: " << lCurrentSecondInDay);
	long lSecondsToWait = fRotateSecond - lCurrentSecondInDay;
	if ( lSecondsToWait <= 0 ) {
		lSecondsToWait += 86400;
	}

	Assert(lSecondsToWait > 0);
	Trace("SecondsToWait: " << lSecondsToWait << "s");
	return lSecondsToWait;
}

void AppLogModule::LogRotator::Run()
{
	StartTrace1(LogRotator.Run, "starting...");
	while ( IsRunning() ) {
		CheckRunningState(eWorking, GetSecondsToWait());

		// rotate only if we are still running and not already in termination sequence
		// never try to call CheckState() without at least a nanosecond to wait
		// -> otherwise we will block until program termination...
		if ( IsRunning() && !CheckState(eTerminationRequested, 0, 1) ) {
			// rotate the log files
			AppLogModule::RotateLogs();
		}
	}
	Trace("terminating...");
}
