/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AppLog.h"
#include "Server.h"
#include "Renderer.h"
#include "Registry.h"
#include "SystemBase.h"
#include "SystemFile.h"
#include <errno.h>
#include <cstdio>
using namespace coast;

RegisterModule(AppLogModule);

AppLogModule *AppLogModule::fgAppLogModule = 0;

AppLogModule::AppLogModule(const char *name)
	: WDModule(name)
	, fRotator(0)
	, fFlusher(0)
{
}

bool AppLogModule::Init(const ROAnything config)
{
	// initialization of application logging tries a socket connection
	// defined in the configuration file
	SystemLog::WriteToStderr("\tStarting Application Logging");
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
		strRotateDir << system::Sep() << "rotate";
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
			SystemLog::WriteToStderr(".", 1);
		}
		TraceAny(fLogConnections, "LogConnections: ");
		if (retCode && StartLogRotator(appLogConfig["RotateTime"].AsCharPtr("24:00"),
									   appLogConfig["RotateSecond"].AsLong(0L),
									   appLogConfig["RotateEveryNSecondsTime"].AsCharPtr("00:00:00"),
									   appLogConfig["RotateEveryNSeconds"].AsLong(0L),
									   appLogConfig["RotateTimeIsGmTime"].AsLong(0L)
									  ) && StartLogFlusher(appLogConfig["FlushEveryNSeconds"].AsLong(60L)) ) {
			fgAppLogModule = this;
			fROLogConnections = fLogConnections;
			SystemLog::WriteToStderr(" done\n");
		} else {
			fgAppLogModule = 0L;
			SystemLog::WriteToStderr(" failed\n");
		}
	} else {
		SystemLog::WriteToStderr(" done\n");
	}
	return retCode;
}

bool AppLogModule::Finis()
{
	StartTrace(AppLogModule.Finis);
	TerminateLogRotator();
	TerminateLogFlusher();
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
	fLogConnections = Anything(Anything::ArrayMarker());
	fgAppLogModule = 0L;
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
				AppLogChannel *pChannel = NULL, *pCloneChannel = AppLogChannel::FindAppLogChannel(channel["ChannelClass"].AsString("AppLogChannel"));
				if ( pCloneChannel ) {
					pChannel = SafeCast(pCloneChannel->Clone(coast::storage::Global()), AppLogChannel);
					if ( pChannel ) {
						pChannel->InitClone(strChannelName, channel);
					}
				}
				if ( !pChannel || !pChannel->Rotate() ) {
					SYSERROR("LogChannel [" << strChannelName << "] failed to open logfile");
					return false;
				}
				fLogConnections[servername][strChannelName] = pChannel;
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

bool AppLogModule::FlushLogs()
{
	return (fgAppLogModule && fgAppLogModule->DoFlushLogs());
}

bool AppLogModule::RotateLogs()
{
	return (fgAppLogModule && fgAppLogModule->DoRotateLogs());
}

bool AppLogModule::RotateSpecificLog(Context &ctx, const char *channelName)
{
	StartTrace(AppLogModule.RotateSpecificLog);
	AppLogChannel *logChannel = FindLogger(ctx, channelName);
	bool ret = false;
	if (logChannel ) {
		ret = logChannel->Rotate(true);
	}
	return ret;
}

bool AppLogModule::DoFlushLogs()
{
	StartTrace(AppLogModule.DoFlushLogs);
	long configSz = fROLogConnections.GetSize();
	for (long i = 0; i < configSz; ++i) {
		const char *servername = fROLogConnections.SlotName(i);
		ROAnything roaLogChannels = fROLogConnections[i];
		long roaLogChannelsSz = roaLogChannels.GetSize();
		for (long j = 0; j < roaLogChannelsSz; ++j) {
			const char *channelname = roaLogChannels.SlotName(j);
			if ( channelname && servername ) {
				AppLogChannel *logChannel = GetLogChannel(servername, channelname);
				if (logChannel) {
					logChannel->FlushItems();
				}
			}
		}
	}
	return true;
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
				AppLogChannel *logChannel = GetLogChannel(servername, channelname);
				if (logChannel) {
					logChannel->Rotate();
				}
			}
		}
	}
	return true;
}

bool AppLogModule::StartLogRotator(const char *rotateTime, long lRotateSecond,  const char *everyNSecondsTime, long lEveryNSeconds, bool isGmTime)
{
	fRotator = new (coast::storage::Global()) LogRotator(rotateTime, everyNSecondsTime,  lRotateSecond, lEveryNSeconds, isGmTime);
	if (fRotator) {
		return fRotator->Start();
	}
	return false;
}

bool AppLogModule::TerminateLogRotator()
{
	if (fRotator) {
		fRotator->Terminate(10);
		delete fRotator;
		fRotator = 0;
		return true;
	}
	return false;
}

bool AppLogModule::StartLogFlusher(long lEveryNSeconds)
{
	fFlusher = new (coast::storage::Global()) LogFlusher(lEveryNSeconds);
	if (fFlusher) {
		return fFlusher->Start();
	}
	return false;
}

bool AppLogModule::TerminateLogFlusher()
{
	if (fFlusher) {
		fFlusher->Terminate(10);
		delete fFlusher;
		fFlusher = 0;
		return true;
	}
	return false;
}

AppLogChannel *AppLogModule::GetLogChannel(const char *servername, const char *logChannel)
{
	StartTrace1(AppLogModule.GetLogChannel, "server [" << servername << "] LogChannel [" << NotNull(logChannel) << "]");

	AppLogChannel *logger = NULL;
	ROAnything loggerConfig;
	if ( servername && logChannel && fROLogConnections.LookupPath(loggerConfig, servername) && loggerConfig.IsDefined(logChannel) ) {
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
			logger = fgAppLogModule->GetLogChannel(servername, logChannel);
			if ( !logger ) {
				SYSERROR("log channel [" << servername << "." << NotNull(logChannel) << "] returned no logger object");
			}
		} else {
			SYSERROR("Server [" << servername << "] not valid");
		}
	}
	return logger;
}

bool AppLogModule::Log(Context &ctx, const char *logChannel, eLogLevel iLevel)
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
		return logger->Log(ctx, iLevel);
	}
	Trace("logger not found");
	return false;
}

bool AppLogModule::Log(Context &ctx, const char *logChannel, const ROAnything &config, eLogLevel iLevel)
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
		Trace("logger found");
		return logger->LogAll(ctx, iLevel, config);
	}
	return false;
}

bool AppLogModule::Log(Context &ctx, const char *logChannel, const String &strMessage, eLogLevel iLevel)
{
	StartTrace1(AppLogModule.Log, "LogChannel [" << NotNull(logChannel) << "], Severity:" << (long)iLevel );
	Server *s = ctx.GetServer();
	if (!s) {
		s = Server::FindServer("Server");
		ctx.SetServer(s);
	}
	AppLogChannel *logger = FindLogger(ctx, logChannel);
	if ( logger ) {
		Trace("logger found");
		Anything anyMessage(strMessage);
		return logger->LogAll(ctx, iLevel, anyMessage);
	} else {
		Trace("logger:" << logChannel << " not found, message:" << strMessage);
	}
	return false;
}

String AppLogModule::GetSeverityText(eLogLevel iLevel)
{
	StartTrace(AppLogModule.GetSeverityText);
	switch (iLevel) {
		case AppLogModule::eCRITICAL:
			return "CRITICAL";
			break;

		case AppLogModule::eFATAL:
			return "FATAL";
			break;

		case AppLogModule::eERROR:
			return "ERROR";
			break;

		case AppLogModule::eWARNING:
			return "WARNING";
			break;

		case AppLogModule::eINFO:
			return "INFO";
			break;

		case AppLogModule::eOK:
			return "OK";
			break;

		case AppLogModule::eMAINT:
			return "MAINT";
			break;

		case AppLogModule::eDEBUG:
			return "DEBUG";
			break;

		default:
			break;
	}
	return "unknown";
}

//---- AppLogChannel ---------------------------------------------------------------------------------------
RegisterObject(AppLogChannel, AppLogChannel);
RegCacheImpl(AppLogChannel);

AppLogChannel::AppLogChannel(const char *name)
	: RegisterableObject(name)
	, fLogStream(NULL)
	, fChannelInfo( coast::storage::Global() )
	, fSuppressEmptyLines(false)
	, fRendering(true)
	, fLogMsgSizeHint(128L)
	, fDoNotRotate(false)
	, fNoLogItemsWrite(false)
	, fFormat( coast::storage::Global() )
	, fChannelMutex(name, coast::storage::Global())
	, fItemsToBuffer(1L)
	, fBuffer( coast::storage::Global() )
	, fItemsInBuffer(0L)
	, fSeverity(AppLogModule::eINFO)
{
	StartTrace(AppLogChannel.AppLogChannel);
}

AppLogChannel::~AppLogChannel()
{
	StartTrace(AppLogChannel.~AppLogChannel);
	if (fLogStream) {
		FlushItems();
		LockUnlockEntry me(fChannelMutex);
		delete fLogStream;
		fLogStream = 0;
	}
}

void AppLogChannel::FlushItems()
{
	StartTrace(AppLogChannel.FlushItems);
	if ( fItemsToBuffer >= 1L ) {
		LockUnlockEntry me(fChannelMutex);
		if ( fItemsInBuffer > 0 ) {
			String msg;
			msg << " AppLogChannel: [" << fName << "] flushing [" << fItemsInBuffer << "] " <<
				"buffered messages to log [" << fChannelInfo["FileName"].AsString() << "]\n";
			SystemLog::Info(msg);
			DoFlushItems();
		}
	}
}

void AppLogChannel::DoFlushItems()
{
	StartTrace(AppLogChannel.DoFlushItems);
	if ( fLogStream && (fItemsToBuffer >= 1L) && (fItemsInBuffer > 0L) ) {
		Trace("fLogStream state before logging: " << (long)fLogStream->rdstate());
		(*fLogStream) << fBuffer << std::flush;
		Trace("fLogStream state after logging: " << (long)fLogStream->rdstate());
		fBuffer.Trim(0L);
		fItemsInBuffer = 0L;
	}
}

bool AppLogChannel::InitClone(const char *name, const Anything &channel)
{
	StartTrace(AppLogChannel.InitClone);
	return DoInitClone(name, channel);
}

bool AppLogChannel::DoInitClone(const char *name, const Anything &channel)
{
	StartTrace(AppLogChannel.DoInitClone);
	SetName(name);
	fChannelInfo = channel;
	fSuppressEmptyLines	= fChannelInfo["SuppressEmptyLines"].AsBool(false);
	fRendering			= fChannelInfo["Rendering"].AsBool(true);
	fLogMsgSizeHint		= fChannelInfo["LogMsgSizeHint"].AsLong(128L);
	fDoNotRotate		= fChannelInfo["DoNotRotate"].AsBool(false);
	fNoLogItemsWrite	= fChannelInfo["NoLogItemsWrite"].AsBool(false);
	fFormat				= fChannelInfo["Format"];
	fItemsToBuffer		= fChannelInfo["BufferItems"].AsLong(1L);
	fSeverity			= (AppLogModule::eLogLevel)fChannelInfo["Severity"].AsLong((long)AppLogModule::eALL);
	if ( fItemsToBuffer <= 0L ) {
		// Avoid zero division in mod operation
		fItemsToBuffer=1L;
	}
	fBuffer.Reserve(fItemsToBuffer * fLogMsgSizeHint);
	return true;
}

bool AppLogChannel::Log(Context &ctx, AppLogModule::eLogLevel iLevel)
{
	StartTrace(AppLogChannel.Log);
	return LogAll(ctx, iLevel, fFormat);
}

bool AppLogChannel::LogAll(Context &ctx, AppLogModule::eLogLevel iLevel, const ROAnything &config)
{
	StartTrace(AppLogChannel.LogAll);
	if ( fLogStream && fLogStream->good() ) {
		if ( fNoLogItemsWrite ) {
			return true;
		}
		if ( ( iLevel & fSeverity ) > 0 ) {
			SubTraceAny(TraceConfig, config, "config: ");
			Anything anyLogSev;
			anyLogSev["LogSeverity"] = (long)iLevel;
			anyLogSev["LogSeverityText"] = AppLogModule::GetSeverityText(iLevel);
			Context::PushPopEntry<Anything> aEntry(ctx, "LogSeverity", anyLogSev);
			String logMsg(fLogMsgSizeHint);
			DoCreateLogMsg(ctx, iLevel, logMsg, config);
			if (!fSuppressEmptyLines || logMsg.Length()) {
				logMsg << "\n";
				LockUnlockEntry me(fChannelMutex);
				{
					fBuffer.Append(logMsg);
					++fItemsInBuffer;
					if ( ( fItemsInBuffer % fItemsToBuffer ) ) return true;
					DoFlushItems();
					return (!!(*fLogStream));
				}
			}
		}
	}
	return fSuppressEmptyLines ? true : false;
}

void AppLogChannel::DoCreateLogMsg(Context &ctx, AppLogModule::eLogLevel iLevel, String &logMsg, const ROAnything &config)
{
	StartTrace(AppLogChannel.DoCreateLogMsg);
	if ( fRendering ) {
		Trace("rendered log part");
		Renderer::RenderOnString(logMsg, ctx, config);
	} else {
		Trace("looking up name [" << fName << "] in context");
		logMsg = ctx.Lookup(fName).AsString();
	}
	Trace("message to log [" << logMsg << "]");
}

bool AppLogChannel::GetLogDirectories(ROAnything channel, String &logdir, String &rotatedir)
{
	StartTrace(AppLogChannel.GetLogDirectories);
	logdir = channel["LogDir"].AsCharPtr();
	rotatedir = channel["RotateDir"].AsCharPtr();

	if ( !system::IsAbsolutePath(logdir) ) {
		Trace("log path is non-absolute [" << logdir << "]");
		logdir = system::GetRootDir();
		logdir << system::Sep() << channel["LogDir"].AsCharPtr();
	}
	Trace("current logfile-path [" << logdir << "]");

	if ( !system::IsAbsolutePath(rotatedir) ) {
		Trace("rotate path is non-absolute [" << rotatedir << "]");
		rotatedir = system::GetRootDir();
		rotatedir << system::Sep() << channel["RotateDir"].AsCharPtr();
	}
	Trace("current rotate-path [" << rotatedir << "]");
	bool bGoAhead = true;
	if ( !system::IsDirectory(logdir) ) {
		SYSERROR("log directory [" << logdir << "] does not exist! please create it first");
		bGoAhead = false;
	}
	if ( !system::IsDirectory(rotatedir) ) {
		SYSERROR("rotate directory [" << rotatedir << "] does not exist! please create it first");
		bGoAhead = false;
	}
	return bGoAhead;
}

std::ostream *AppLogChannel::OpenLogStream(ROAnything channel, String &logfileName)
{
	StartTrace(AppLogChannel.OpenLogStream);
	String logdir, rotatedir;
	bool bGoAhead;

	bGoAhead = GetLogDirectories(channel, logdir, rotatedir);

	logfileName = channel["FileName"].AsCharPtr();

	Trace("rotating [" << logfileName << "]");
	std::ostream *os = NULL;
	if ( bGoAhead && RotateLog(logdir, rotatedir, logfileName) ) {
		os = system::OpenOStream(logfileName, NULL, std::ios::app, true);
	}
	Trace(((os) ? " succeeded" : " failed"));
	return os;
}

bool AppLogChannel::RotateLog(const String &logdirName, const String &rotatedirName, String &logfileName)
{
	StartTrace(AppLogChannel.RotateLog);
	Trace("LogFileName [" << logfileName << "]");

	String oldLogFileName(logdirName);
	oldLogFileName << system::Sep() << logfileName;
	String newLogFileName(rotatedirName);
	newLogFileName << system::Sep();

	String stampedLogFileName(logfileName);
	stampedLogFileName << "." << GenTimeStamp("%Y%m%d%H%M%S");
	Trace("rotate LogFileName [" << stampedLogFileName << "]");
	newLogFileName << stampedLogFileName;

	system::ResolvePath(oldLogFileName);
	system::ResolvePath(newLogFileName);
	Trace("old [" << oldLogFileName << "]");
	Trace("new [" << newLogFileName << "]");

	int retCode = 0;
	Trace("testing if old logfile exists and must be rotated");
	if ( system::IsRegularFile(oldLogFileName) ) {
		Trace("testing uniqueness of rotate FileName");
		long lNum = 1;
		String strLastRot(newLogFileName);
		// assume not more than 10 rotates of same file per second...
		while ( system::IsRegularFile(newLogFileName) && lNum < 10 ) {
			Trace("not unique! [" << newLogFileName << "]");
			newLogFileName = strLastRot << '.' << lNum++;
			Trace("New name to test [" << newLogFileName << "]");
		}
		retCode = ::rename(oldLogFileName, newLogFileName);
		if ( retCode != 0 ) {
			SYSWARNING("rotate of [" << oldLogFileName << "] with [" << newLogFileName << "] failed! (" << (long)errno << ") " << SystemLog::LastSysError());
		}
	}
	// assign full filename of log to use
	logfileName = oldLogFileName;
	return (retCode == 0);
}

bool AppLogChannel::Rotate(bool overrideDoNotRotateLogs)
{
	StartTrace(AppLogChannel.Rotate);
	bool bSuccess = true;
	// check if this channel must be rotated
	TraceAny(GetChannelInfo(), "channel info");
	std::ostream *pStream = NULL;
	{
		LockUnlockEntry me(fChannelMutex);
		pStream = fLogStream;
	}
	// must enter here in case the logfile does not exist yet
	if ( !pStream || (overrideDoNotRotateLogs || (fDoNotRotate == false)) ) {
		LockUnlockEntry me(fChannelMutex);
		if ( fLogStream ) {
			DoFlushItems();
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

void AppLogChannel::WriteHeader(std::ostream &os)
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
		os << std::endl;
	}
	Trace("os state " << (long)os.rdstate());
}

String AppLogChannel::GenTimeStamp(const String &format)
{
	StartTrace(AppLogChannel.GenTimeStamp);
	const int dateSz = 40;
	time_t now = time(0);
	struct tm res, *tt;
	tt = system::LocalTime(&now, &res);

	// NOTICE: array is limited to gcMaxDateArraySize chars
	char date[dateSz];
	strftime(date, dateSz, format, tt);
	Trace("generated timestamp [" << date << "]");
	return date;
}

//--- LogRotator ----------------------
AppLogModule::LogRotator::LogRotator(const char *rotateTime, const char *everyNSecondsTime, long lRotateSecond, long leveryNSeconds, bool isGmTime)
	: Thread("LogRotator")
	, fRotateSecond(lRotateSecond)
	, fEveryNSeconds(leveryNSeconds)
	, fIsGmTime(isGmTime)
{
	StartTrace(LogRotator.LogRotator);
	if ( fEveryNSeconds == 0 ) {
		long rotateSecond = ParseTimeString(everyNSecondsTime);
		if ( rotateSecond != 0 ) {
			fEveryNSeconds = rotateSecond;
		}
	}
	if ( fEveryNSeconds == 0 ) {
		if ( fRotateSecond == 0 ) {
			fRotateSecond = ParseTimeString(rotateTime);
		}
	}
	Trace("fRotateSecond: [" << fRotateSecond << "] fEveryNSeconds: [" << fEveryNSeconds << "]");
}

long AppLogModule::LogRotator::ParseTimeString( const char *time)
{
	StartTrace(LogRotator.ParseTimeString);

	StringTokenizer st(time, ':');
	String hour, minute, second;
	long rotateSecond = 0L;

	if ( !(st.NextToken(hour) && st.NextToken(minute)) ) {
		SYSERROR("wrong time format [" << time << "]");
	} else {
		// optional seconds spec
		st.NextToken(second);
		rotateSecond = ( ( hour.AsLong(0) % 24 * 60 ) + minute.AsLong(0) % 60 ) * 60 + second.AsLong(0) % 60;
	}
	Trace("rotateSecond: [" << rotateSecond << "]");
	return rotateSecond;
}

long AppLogModule::LogRotator::GetSecondsToWait()
{
	StartTrace(LogRotator.GetSecondsToWait);

	long lSecondsToWait = 0;
	if ( fEveryNSeconds != 0 ) {
		lSecondsToWait = fEveryNSeconds;
	} else {
		time_t now = time(0);

		struct tm res, *tt;
		tt = ((fIsGmTime == false) ? (system::LocalTime(&now, &res)) :
			  (system::GmTime(&now, &res)));

		long lCurrentSecondInDay = ( ( ( ( tt->tm_hour * 60 ) + tt->tm_min ) * 60 ) + tt->tm_sec ) % 86400;
		Trace("fRotateSecond: [" << fRotateSecond << "] lCurrentSecondInDay: [" <<
			  lCurrentSecondInDay << "] fIsGmTime: [" << fIsGmTime << "]");
		lSecondsToWait = fRotateSecond - lCurrentSecondInDay;
		if ( lSecondsToWait <= 0 ) {
			lSecondsToWait += 86400;
		}
	}
	Assert(lSecondsToWait > 0);
	Trace("SecondsToWait: [" << lSecondsToWait << "] s fIsGmTime: [" << fIsGmTime << "]");
	return lSecondsToWait;
}

void AppLogModule::LogRotator::Run()
{
	StartTrace1(LogRotator.Run, "starting...");
	while ( CheckState( eRunning, 0, 1 ) ) {
		CheckRunningState(eWorking, GetSecondsToWait());

		// rotate only if we are still running and not already in termination sequence
		// never try to call CheckState() without at least a nanosecond to wait
		// -> otherwise we will block until program termination...
		if ( CheckState( eRunning, 0, 1 ) ) {
			// rotate the log files
			AppLogModule::RotateLogs();
		}
	}
	Trace("terminating...");
}

void AppLogModule::LogFlusher::Run()
{
	StartTrace1(LogFlusher.Run, "starting...");
	while ( CheckState( eRunning, 0, 1 ) ) {
		CheckRunningState(eWorking, fEveryNSeconds);
		// flush only if we are still running and not already in termination sequence
		// never try to call CheckState() without at least a nanosecond to wait
		// -> otherwise we will block until program termination...
		if ( CheckState( eRunning, 0, 1 ) ) {
			AppLogModule::FlushLogs();
		}
	}
	Trace("terminating...");
}
