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

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "Server.h"
#include "StringStream.h"

#if defined(ONLY_STD_IOSTREAM)
using namespace std;
#endif

//--- c-modules used -----------------------------------------------------------

//---- QueueWorkingModule ---------------------------------------------------------------
QueueWorkingModule::QueueWorkingModule(const char *name)
	: WDModule(name)
	, fpQueue(NULL)
	, fpQAllocator(NULL)
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
	Log(strMessage, fErrorLogName, AppLogModule::eERROR);
}

void QueueWorkingModule::LogWarning(String strMessage)
{
	StartTrace(QueueWorkingModule.LogWarning);
	SYSWARNING(strMessage);
	Log(strMessage, fWarningLogName, AppLogModule::eWARNING);
}

void QueueWorkingModule::LogInfo(String strMessage)
{
	StartTrace(QueueWorkingModule.LogInfo);
	Log(strMessage, fInfoLogName, AppLogModule::eINFO);
}

void QueueWorkingModule::Log(String strMessage, const char *channel, AppLogModule::eLogLevel iLevel)
{
	StartTrace(QueueWorkingModule.Log);
	Trace(strMessage);
	if ( IsAlive() && fpContext ) {
		LockUnlockEntry me(fContextLock);
		if ( IsAlive() && fpContext ) {
			fpContext->GetTmpStore()["LogMessage"] = strMessage;
			AppLogModule::Log(*fpContext, channel, iLevel);
		}
	}
}

void QueueWorkingModule::Log(Anything &anyStatus, const char *channel, AppLogModule::eLogLevel iLevel)
{
	StartTrace(QueueWorkingModule.Log);
	TraceAny(anyStatus, "content to log");
	if ( IsAlive() && fpContext ) {
		LockUnlockEntry me(fContextLock);
		if ( IsAlive() && fpContext ) {
			fpContext->GetTmpStore()["QueueWorkingStatus"] = anyStatus;
			AppLogModule::Log(*fpContext, channel, iLevel);
		}
	}
}

bool QueueWorkingModule::Init(const ROAnything config)
{
	StartTrace(QueueWorkingModule.Init);
	SubTraceAny(FullConfig, config, "Config: ");
	ROAnything roaConfig;
	if ( config.LookupPath(roaConfig, fName) ) {
		fConfig = roaConfig.DeepClone();
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
		anyStat.PrintOn(aStream, true) << "\n" << flush;
		SysLog::WriteToStderr(aStream.str());
	}

	// delete Queue, also wakes up blocked threads, threads MUST be in termination sequence!!
	IntCleanupQueue();

	{
		LockUnlockEntry me(fContextLock);
		delete fpContext;
		fpContext = NULL;
	}

	return true;
}

bool QueueWorkingModule::ResetInit(const ROAnything config)
{
	StartTrace(QueueWorkingModule.ResetInit);
	// calls Init
	return WDModule::ResetInit(config);
}

bool QueueWorkingModule::ResetFinis(const ROAnything config)
{
	StartTrace(QueueWorkingModule.ResetFinis);
	// calls Finis
	return WDModule::ResetFinis(config);
}

void QueueWorkingModule::IntInitQueue(const ROAnything roaConfig)
{
	StartTrace(QueueWorkingModule.IntInitQueue);
	long lQueueSize = roaConfig["QueueSize"].AsLong(100L);
	Allocator *pAlloc = Storage::Global();

	if ( roaConfig["UsePoolStorage"].AsLong(0) == 1 ) {
		// create unique allocator id based on a pointer value
		long lAllocatorId = (((long)this) & 0x00007FFF);
		pAlloc = MT_Storage::MakePoolAllocator(roaConfig["PoolStorageSize"].AsLong(10240), roaConfig["NumOfPoolBucketSizes"].AsLong(10), lAllocatorId);
		if ( pAlloc == NULL ) {
			SYSERROR("was not able to create PoolAllocator with Id:" << lAllocatorId << " for [" << GetName() << "], check config!");
		} else {
			// store allocator pointer for later deletion
			MT_Storage::RefAllocator(pAlloc);
			fpQAllocator = pAlloc;
		}
	}
	fpQueue = new Queue(GetName(), lQueueSize, pAlloc);
	fFailedPutbackMessages = Anything();
}

void QueueWorkingModule::IntCleanupQueue()
{
	StartTrace(QueueWorkingModule.IntCleanupQueue);
	// we could do something here to persist the content of the queue and the putback message buffer
	delete fpQueue;
	fpQueue = NULL;
	if ( fpQAllocator ) {
		MT_Storage::UnrefAllocator(fpQAllocator);
		fpQAllocator = NULL;
	}
	fFailedPutbackMessages = Anything();
}

bool QueueWorkingModule::IsBlocked(Queue::BlockingSide aSide)
{
	StartTrace(QueueWorkingModule.IsBlocked);
	bool bRet = false;
	if ( fpQueue && fpQueue->IsAlive() && IsAlive() ) {
		bRet = fpQueue->IsBlocked(aSide);
	}
	return bRet;
}

void QueueWorkingModule::Block(Queue::BlockingSide aSide)
{
	StartTrace(QueueWorkingModule.Block);
	if ( fpQueue && fpQueue->IsAlive() && IsAlive() ) {
		fpQueue->Block(aSide);
	}
}

void QueueWorkingModule::UnBlock(Queue::BlockingSide aSide)
{
	StartTrace(QueueWorkingModule.UnBlock);
	if ( fpQueue && fpQueue->IsAlive() && IsAlive() ) {
		fpQueue->UnBlock(aSide);
	}
}

Queue::StatusCode QueueWorkingModule::PutElement(Anything &anyELement, bool bTryLock)
{
	StartTrace(QueueWorkingModule.PutElement);
	Queue::StatusCode eRet = Queue::eDead;
	if ( fpQueue && fpQueue->IsAlive() && IsAlive() ) {
		eRet = fpQueue->Put(anyELement, bTryLock);
		if ( eRet != Queue::eSuccess ) {
			SYSWARNING("Queue->Put failed, Queue::StatusCode:" << eRet << " !");
		}
	}
	return eRet;
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

Queue::StatusCode QueueWorkingModule::GetElement(Anything &anyValues, bool bTryLock)
{
	StartTrace(QueueWorkingModule.GetElement);
	Queue::StatusCode eRet = Queue::eDead;
	if ( fpQueue && fpQueue->IsAlive() && IsAlive() ) {
		Trace("Queue still alive");
		// try to get a failed message first
		if ( fFailedPutbackMessages.GetSize() > 0 ) {
			Trace("getting failed message 1 of " << fFailedPutbackMessages.GetSize() );
			anyValues = fFailedPutbackMessages[0L];
			fFailedPutbackMessages.Remove(0L);
			eRet = Queue::eSuccess;
		} else {
			// Default is blocking get to save cpu time
			eRet = fpQueue->Get(anyValues, bTryLock);
			if ( ( eRet != Queue::eSuccess ) && ( eRet != Queue::eEmpty ) ) {
				SYSWARNING("Queue->Get failed, Queue::StatusCode:" << eRet << " !");
			}
		}
	}
	return eRet;
}

void QueueWorkingModule::PutBackElement(Anything &anyValues)
{
	StartTrace(QueueWorkingModule.PutBackElement);
	// put message back to the queue (Appends!) if possible
	// take care not to lock ourselves up here, thus we MUST use a trylock here!
	Queue::StatusCode eRet = PutElement(anyValues, true);
	if ( eRet != Queue::eSuccess && ( eRet & Queue::eFull ) ) {
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

long QueueWorkingModule::GetCurrentSize()
{
	StartTrace(QueueWorkingModule.GetCurrentSize);
	long lSize = 0L;
	if ( fpQueue ) {
		lSize = fpQueue->GetSize();
	}
	return lSize;
}
