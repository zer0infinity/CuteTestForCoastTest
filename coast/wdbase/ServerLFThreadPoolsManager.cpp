/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ServerLFThreadPoolsManager.h"

//--- standard modules used ----------------------------------------------------
#include "Registry.h"
#include "Timers.h"
#include "Server.h"
#include "SysLog.h"
#include "LFListenerPool.h"
#include "WPMStatHandler.h"
#include "RequestProcessor.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

RegisterServerPoolsManagerInterface(ServerLFThreadPoolsManager);

//---- ServerLFThreadPoolsManager -----------------------------------------------------------
ServerLFThreadPoolsManager::ServerLFThreadPoolsManager(const char *ServerThreadPoolsManagerName)
	: 	ServerPoolsManagerInterface(ServerThreadPoolsManagerName),
		fLFPool(0),
		fThreadPoolSz(25)
{
	StartTrace(ServerLFThreadPoolsManager.Ctor);
}

ServerLFThreadPoolsManager::~ServerLFThreadPoolsManager()
{
	StartTrace(ServerLFThreadPoolsManager.Dtor);
	if ( fLFPool ) {
		delete fLFPool;
	}
}

int ServerLFThreadPoolsManager::Init(Server *server)
{
	StartTrace(ServerLFThreadPoolsManager.Init);
	// return 0 in case of success
	return ( SetupLFPool(server) ? 0 : 1 );
}

bool ServerLFThreadPoolsManager::SetupLFPool(Server *server)
{
	StartTrace(ServerLFThreadPoolsManager.SetupLFPool);
	Context ctx;
	ctx.SetServer(server);
	ctx.Push(this);
	fThreadPoolSz = ctx.Lookup("ThreadPoolSize", fThreadPoolSz);
	ROAnything listenerPoolConfig;
	if (!ctx.Lookup("ListenerPool", listenerPoolConfig)) {
		const char *msg = "ListenerPool configuration not found";
		SYSERROR(msg);
		Trace(msg);
		return false;
	}
	TraceAny(listenerPoolConfig, "ListenerPool config:");
	RequestReactor *rr = new RequestReactor(server->DoMakeProcessor(), new WPMStatHandler(fThreadPoolSz));
	server->AddStatGatherer2Observe(rr);
	fLFPool = new LFListenerPool(rr);
	bool usePoolStorage = (ctx.Lookup("UsePoolStorage", 0L) != 0L);

	return fLFPool->Init(fThreadPoolSz, listenerPoolConfig, usePoolStorage);
}

int ServerLFThreadPoolsManager::ReInit(Server *server)
{
	StartTrace(ServerLFThreadPoolsManager.ReInit);
	return 0;
}

int ServerLFThreadPoolsManager::Run(Server *server)
{
	StartTrace(ServerLFThreadPoolsManager.Run);
	Assert(fLFPool);

	Context ctx;
	ctx.SetServer(server);
	ctx.Push(this);
	bool usePoolStorage = (ctx.Lookup("UsePoolStorage", 0L) != 0L);
	u_long poolStorageSize = (u_long)ctx.Lookup("PoolStorageSize", 1000L);
	u_long numOfPoolBucketSizes = (u_long)ctx.Lookup("NumOfPoolBucketSizes", 20L);

	int retVal = fLFPool->Start(usePoolStorage, poolStorageSize, numOfPoolBucketSizes);
	if ( retVal != 0 ) {
		SYSERROR("server (" << fName << ") start accept loops failed");
		return retVal;
	}
	SetReady(true);
	// wait on Join forever
	retVal = fLFPool->Join(0);
	SetReady(false);
	return retVal;
}

bool ServerLFThreadPoolsManager::BlockRequests(Server *server)
{
	StartTrace(ServerLFThreadPoolsManager.BlockRequests);
	fLFPool->BlockRequests();

	String m(" done\n");
	SysLog::WriteToStderr(m);
	Trace("done");

	m = "Waiting for requests to terminate \n";
	SysLog::WriteToStderr(m);
	Trace(m);
	Context ctx;
	ctx.SetServer(server);
	ctx.Push(this);
	return fLFPool->AwaitEmpty(ctx.Lookup("AwaitResetEmpty", 120L));
}

void ServerLFThreadPoolsManager::UnblockRequests()
{
	StartTrace(ServerLFThreadPoolsManager.UnblockRequests);
	fLFPool->UnblockRequests();
}

int ServerLFThreadPoolsManager::RequestTermination()
{
	StartTrace(ServerLFThreadPoolsManager.RequestTermination);

	if (fLFPool) {
		fLFPool->RequestTermination();
	}
	return 0;
}

void ServerLFThreadPoolsManager::Terminate()
{
	StartTrace(ServerLFThreadPoolsManager.Terminate);
	Trace("ServerLFThreadPoolsManager->Terminate");
	if (fLFPool) {
		fLFPool->Join(20);
	}
	Trace("ServerLFThreadPoolsManager->Terminate ready false");

	SetReady(false);
	Trace("ServerLFThreadPoolsManager->Terminate done");
}

long ServerLFThreadPoolsManager::GetThreadPoolSize()
{
	return fThreadPoolSz;
}
