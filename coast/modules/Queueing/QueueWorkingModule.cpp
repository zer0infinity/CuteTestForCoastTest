/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "QueueWorkingModule.h"

//--- project modules used -----------------------------------------------------
#include "Queue.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "SysLog.h"
#include "Server.h"
#include "AppLog.h"
#include "StringStream.h"

//--- c-modules used -----------------------------------------------------------

//---- QueueWorkingModule ---------------------------------------------------------------
QueueWorkingModule::QueueWorkingModule(const char *name)
	: WDModule(name)
	, fpQueue(NULL)
	, fpContext(NULL)
	, fContextLock("QueueWorkingModuleContextLock")
	, fFailedPutbackMessages(Storage::Global())
	, fAlive(0UL)
{
	StartTrace(QueueWorkingModule.QueueWorkingModule);
}

QueueWorkingModule::~QueueWorkingModule()
{
}

void QueueWorkingModule::LogError(String strMessage)
{
	StartTrace(QueueWorkingModule.LogError);
	SYSERROR(strMessage);
	Log(strMessage, fErrorLogName);
}

void QueueWorkingModule::LogWarning(String strMessage)
{
	StartTrace(QueueWorkingModule.LogWarning);
	SYSWARNING(strMessage);
	Log(strMessage, fWarningLogName);
}

void QueueWorkingModule::LogInfo(String strMessage)
{
	StartTrace(QueueWorkingModule.LogInfo);
	Log(strMessage, fInfoLogName);
}

void QueueWorkingModule::Log(String strMessage, const char *channel)
{
	StartTrace(QueueWorkingModule.Log);
	Trace(strMessage);
	if ( IsAlive() && fpContext ) {
		MutexEntry me(fContextLock);
		me.Use();
		if ( IsAlive() && fpContext ) {
			fpContext->GetTmpStore()["LogMessage"] = strMessage;
			AppLogModule::Log(*fpContext, channel);
		}
	}
}

void QueueWorkingModule::Log(Anything &anyStatus, const char *channel)
{
	StartTrace(QueueWorkingModule.Log);
	TraceAny(anyStatus, "content to log");
	if ( IsAlive() && fpContext ) {
		MutexEntry me(fContextLock);
		me.Use();
		if ( IsAlive() && fpContext ) {
			fpContext->GetTmpStore()["QueueWorkingStatus"] = anyStatus;
			AppLogModule::Log(*fpContext, channel);
		}
	}
}

bool QueueWorkingModule::Init(const Anything &config)
{
	StartTrace(QueueWorkingModule.Init);
	SubTraceAny(FullConfig, config, "Config: ");
	if ( config.LookupPath(fConfig, fName) ) {
		TraceAny(fConfig, "Module config");
		fErrorLogName = GetNamedConfig("Logging")["ErrorChannel"].AsCharPtr("ErrorLog");
		fWarningLogName = GetNamedConfig("Logging")["WarningChannel"].AsCharPtr("WarningLog");
		fInfoLogName = GetNamedConfig("Logging")["InfoChannel"].AsCharPtr("AccessLog");
		const char *pServerName = GetNamedConfig("Logging")["Servername"].AsCharPtr("Server");
		Server *pServer = Server::FindServer(pServerName);
		Anything dummy;
		fpContext = new Context(fConfig, dummy, pServer, 0, 0);
		IntInitQueue(GetConfig());
		fAlive = 0xf007f007;
		return true;
	}
	return false;
}

bool QueueWorkingModule::Finis()
{
	StartTrace(QueueWorkingModule.Finis);

	SetDead();

	Anything anyStat;
	if ( GetQueueStatistics(anyStat) ) {
		StringStream aStream;
		aStream << "Statistics for [" << GetName() << "]\n";
		anyStat.PrintOn(aStream, true) << flush;
		SysLog::WriteToStderr(aStream.str());
	}

	// delete Queue, also wakes up blocked threads, threads MUST be in termination sequence!!
	IntCleanupQueue();

	{
		MutexEntry me(fContextLock);
		me.Use();
		delete fpContext;
		fpContext = NULL;
	}

	return true;
}

bool QueueWorkingModule::ResetInit(const Anything &config)
{
	StartTrace(QueueWorkingModule.ResetInit);
	// calls Init
	return WDModule::ResetInit(config);
}

bool QueueWorkingModule::ResetFinis(const Anything &config)
{
	StartTrace(QueueWorkingModule.ResetFinis);
	// calls Finis
	return WDModule::ResetFinis(config);
}

void QueueWorkingModule::IntInitQueue(ROAnything roaConfig)
{
	StartTrace(QueueWorkingModule.IntInitQueue);
	long lQueueSize = roaConfig["QueueSize"].AsLong(100L);
	fpQueue = new Queue(lQueueSize);
	fFailedPutbackMessages = Anything();
}

void QueueWorkingModule::IntCleanupQueue()
{
	StartTrace(QueueWorkingModule.IntCleanupQueue);
	// we could do something here to persist the content of the queue and the putback message buffer
	delete fpQueue;
	fpQueue = NULL;
	fFailedPutbackMessages = Anything();
}

bool QueueWorkingModule::PutElement(Anything &anyELement, bool bTryLock)
{
	StartTrace(QueueWorkingModule.PutElement);
	if ( fpQueue && fpQueue->IsAlive() && IsAlive() ) {
		return fpQueue->Put(anyELement, bTryLock);
	}
	return false;
}

ROAnything QueueWorkingModule::GetConfig()
{
	StartTrace(QueueWorkingModule.GetConfig);
	return ((ROAnything)fConfig);
}

ROAnything QueueWorkingModule::GetNamedConfig(const char *name)
{
	StartTrace(QueueWorkingModule.GetNamedConfig);
	return ((ROAnything)fConfig)[name];
}

bool QueueWorkingModule::GetElement(Anything &anyValues)
{
	StartTrace(QueueWorkingModule.GetElement);
	bool bRet = false;
	if ( fpQueue && fpQueue->IsAlive() && IsAlive() ) {
		Trace("Queue still alive");
		// try to get a failed message first
		if ( fFailedPutbackMessages.GetSize() > 0 ) {
			Trace("getting failed message 1 of " << fFailedPutbackMessages.GetSize() );
			anyValues = fFailedPutbackMessages[0L];
			fFailedPutbackMessages.Remove(0L);
			bRet = true;
		} else {
			// blocking get to save cpu time
			if ( fpQueue->Get(anyValues) ) {
				Trace("got values from Queue");
				bRet = true;
			} else {
				SYSWARNING("Queue->Get failed!");
			}
		}
	}
	return bRet;
}

void QueueWorkingModule::PutBackElement(Anything &anyValues)
{
	StartTrace(QueueWorkingModule.PutBackElement);
	// put message back to the queue (Appends!) if possible
	// take care not to lock ourselves up here, thus we MUST use a trylock here!
	if ( !PutElement(anyValues, true) ) {
		// no more room in Queue, need to store this message internally for later put back
		fFailedPutbackMessages.Append(anyValues);
	}
}

long QueueWorkingModule::FlushQueue(Anything &anyElements)
{
	StartTrace(QueueWorkingModule.FlushQueue);
	long lElements = 0L;
	if ( fpQueue && fpQueue->IsAlive() && IsAlive() ) {
		fpQueue->EmptyQueue(anyElements);
		lElements = anyElements.GetSize();
		LogInfo(String(GetName()) << ": flushed " << lElements << " elements");
	}
	return lElements;
}

bool QueueWorkingModule::GetQueueStatistics(Anything &anyStat)
{
	StartTrace(QueueWorkingModule.GetQueueStatistics);
	if ( fpQueue ) {
		fpQueue->GetStatistics(anyStat);
		return true;
	}
	return false;
}
