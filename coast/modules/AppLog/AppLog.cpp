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
#include "SysLog.h"
#include "System.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------
#if defined(WIN32)
#include <stdio.h>
#endif

class EXPORTDECL_APPLOG LogRotator : public Thread
{
public:
	LogRotator(const char *rotateTime);

protected:
	long GetSecondsToWait();
	void Run();
	//! when to rotate
	long fHour, fMinute;
};

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

bool AppLogModule::Init(const Anything &config)
{
	// initialization of application logging tries a socket connection
	// defined in the configuration file
	SysLog::WriteToStderr("\tStarting Application Logging");
	bool retCode = true;
	Anything appLogConfig;
	StartTrace(AppLogModule.Init);

	if ( config.LookupPath(appLogConfig, "AppLogModule") && appLogConfig.IsDefined("Servers") && appLogConfig["Servers"].GetSize() ) {
		TraceAny(appLogConfig, "AppLogConfig:");
		Anything servers(appLogConfig["Servers"]);
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
		for (long i = 0; i < numOfServers; i++) {
			const char *servername = servers.SlotName(i);
			Server *s;
			if ( servername && (s = Server::FindServer(servername)) ) {
				Anything config;
				config["Channels"] = servers[i];
				config["LogDir"] = strLogDir;
				config["RotateDir"] = strRotateDir;
				retCode = retCode && MakeChannels(servername, config);
			}
			SysLog::WriteToStderr(".", 1);
		}
		TraceAny(fLogConnections, "LogConnections: ");
		if (retCode && StartLogRotator(appLogConfig["RotateTime"].AsCharPtr("24:00"))) {
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
	for (long i = 0; i < fLogConnections.GetSize(); i++) {
		TraceAny(fLogConnections[i], "config of server [" << fLogConnections.SlotName(i) << "]");
		// ServerConfigs with DoNotRotate slot are duplicates, so do not delete their channels!
		if ( !fLogConnections[i].IsDefined("DoNotRotate") ) {
			for (long j = 0; j < fLogConnections[i].GetSize(); j++) {
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
		for (long i = 0; i < numOfChannels; i++) {
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
	for (long i = 0; i < configSz; i++) {
		const char *servername = fROLogConnections.SlotName(i);
		ROAnything roaLogChannels = fROLogConnections[i];
		long roaLogChannelsSz = roaLogChannels.GetSize();
		for (long j = 0; j < roaLogChannelsSz; j++) {
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

bool AppLogModule::StartLogRotator(const char *waittime)
{
	fRotator = new LogRotator(waittime);

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
		TraceAny(loggerConfig, "logger config for server [" << servername << "]");
		doNotRotate = loggerConfig["DoNotRotate"].AsBool(false);
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
	return LogAll(ctx, fChannelInfo["Format"]);
}

bool AppLogChannel::LogAll(Context &ctx, const ROAnything &config)
{
	StartTrace(AppLogChannel.LogAll);
	if ( fLogStream && fLogStream->good() ) {
		TraceAny(config, "config: ");
		String logMsg;
		Renderer::RenderOnString(logMsg, ctx, config);

		if (!fChannelInfo["SuppressEmptyLines"].AsBool(false) || logMsg.Length()) {
			MutexEntry me(fChannelMutex);
			me.Use();
			Trace("fLogStream state before logging: " << (long)fLogStream->rdstate());
			(*fLogStream) << logMsg << endl;
			Trace("fLogStream state after logging: " << (long)fLogStream->rdstate());
			return (!!(*fLogStream));
		}
	}
	return false;
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
	MutexEntry me(fChannelMutex);
	me.Use();
	if ( fLogStream ) {
		delete fLogStream;
		fLogStream = NULL;
	}
	String logfileName;
	fLogStream = OpenLogStream(fChannelInfo, logfileName);
	if ( fLogStream ) {
		WriteHeader(*fLogStream);
	} else {
		SYSERROR("Rotation of [" << logfileName << "] failed");
	}
	return ( fLogStream != NULL );
}

void AppLogChannel::WriteHeader(ostream &os)
{
	StartTrace(AppLogChannel.WriteHeader);
	ROAnything header = fChannelInfo["Header"];

	Trace("os state before writing header: " << (long)os.rdstate());
	for (long i = 0; i < header.GetSize(); i++) {
		os << header[i].AsCharPtr("");
		if (!os) {
			SYSERROR("Write Header to logfile failed");
			return;
		}
	}
	os << endl;
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
LogRotator::LogRotator(const char *rotateTime)
	: Thread("LogRotator")
	, fHour(0)
	, fMinute(0)
{
	StartTrace1(LogRotator.LogRotator, "Rotate time: " << NotNull(rotateTime));
	StringTokenizer st(rotateTime, ':');
	String hour;
	String minute;

	if (!(st.NextToken(hour) && st.NextToken(minute))) {
		SYSERROR("wrong time format [" << rotateTime << "]");
	} else {
		fHour = hour.AsLong(0) % 24;
		fMinute = minute.AsLong(0) % 60;
	}
}

long LogRotator::GetSecondsToWait()
{
	StartTrace(LogRotator.GetSecondsToWait);
	time_t now = time(0);
	struct tm res, *tt;
	tt = System::LocalTime(&now, &res);

	long deltamin = (fMinute - tt->tm_min);
	long deltahour = (fHour - tt->tm_hour);

	if ( deltamin < 0 ) {
		deltahour--;
		deltamin += 60;
	}
	if ( deltahour < 0 ) {
		deltahour += 24;
	}

	long lSecondsToWait = (deltahour * 60 * 60) + deltamin * 60;
	if ( lSecondsToWait == 0 ) {
		lSecondsToWait = 24 * 60 * 60;
	}

	Assert(lSecondsToWait > 0);
	Trace("SecondsToWait: " << lSecondsToWait << "s");
	return lSecondsToWait;
}

void LogRotator::Run()
{
	StartTrace1(LogRotator.Run, "starting...");
	while ( IsRunning() ) {
		CheckRunningState(eWorking, GetSecondsToWait());

		// check the alive flag
		if (!IsRunning()) {
			break;
		}

		// rotate the log files
		AppLogModule::RotateLogs();
	}
	Trace("terminating...");
}
