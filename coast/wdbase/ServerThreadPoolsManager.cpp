/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ServerThreadPoolsManager.h"

//--- standard modules used ----------------------------------------------------
#include "Timers.h"
#include "Server.h"
#include "RequestListener.h"
#include "ServerUtils.h"
#include "SysLog.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

class EXPORTDECL_WDBASE WorkerPoolCallBackFactory : public CallBackFactory
{
public:
	WorkerPoolCallBackFactory(RequestThreadsManager *workerPool);
	virtual ~WorkerPoolCallBackFactory() {};
	virtual AcceptorCallBack *MakeCallBack();
protected:
	RequestThreadsManager *fWorkerPool;
};

//---- ServerThreadPoolsManager -----------------------------------------------------------
RegisterServerPoolsManagerInterface(ServerThreadPoolsManager);

ServerThreadPoolsManager::ServerThreadPoolsManager(const char *ServerThreadPoolsManagerName)
	: ServerPoolsManagerInterface(ServerThreadPoolsManagerName)
	, fActiveRequests(0)
	, fAcceptors(0)
{
	StartTrace1(ServerThreadPoolsManager.Ctor, "[" << ServerThreadPoolsManagerName << "]");
}

ServerThreadPoolsManager::~ServerThreadPoolsManager()
{
	StartTrace1(ServerThreadPoolsManager.Dtor, "[" << fName << "]");
	Terminate();
}

int ServerThreadPoolsManager::Init(Server *server)
{
	StartTrace(ServerThreadPoolsManager.Init);
	// allocate the thread pool and prepare the listening socket
	if ( (SetupThreadPool(false, server) == 0) && SetupSocket(server) ) {
		Trace("returning 0");
		return 0;
	}
	Trace("returning -1");
	return -1;
}

bool ServerThreadPoolsManager::SetupSocket(Server *server)
{
	StartTrace(ServerThreadPoolsManager.SetupSocket);

	Context ctx;
	ctx.SetServer(server);
	ctx.Push("ServerThreadPoolsManager", this);

	fAcceptors =
		new ListenerPool(
		new WorkerPoolCallBackFactory(
			fActiveRequests
		)
	);
	if (!fAcceptors) {
		const char *msg = "ListenerPool allocation failed";
		SYSERROR(msg);
		Trace(msg);
		return false;
	}

	ROAnything listenerPoolConfig;
	if (!ctx.Lookup("ListenerPool", listenerPoolConfig)) {
		const char *msg = "ListenerPool configuration not found";
		SYSERROR(msg);
		Trace(msg);
		return false;
	}
	return fAcceptors->Init(listenerPoolConfig.GetSize(), listenerPoolConfig);
}

int ServerThreadPoolsManager::SetupThreadPool(bool reinit, Server *server)
{
	StartTrace(ServerThreadPoolsManager.SetupThreadPool);
	Trace("server at " << long(server) << (reinit ? " for reinit" : " for Init"));
	Context ctx;
	ctx.SetServer(server);
	ctx.Push("ServerThreadPoolsManager", this);
	//--- Active Sessions
	long actSessions = ctx.Lookup("ThreadPoolSize", 25L);
	long usePoolStorage = ctx.Lookup("UsePoolStorage", 0L);
	u_long poolStorageSize = (u_long)ctx.Lookup("PoolStorageSize", 1000L);
	u_long numOfPoolBucketSizes = (u_long)ctx.Lookup("NumOfPoolBucketSizes", 20L);

	Anything args;
	args["processor"] = (IFAObject *)server->DoMakeProcessor();

	if ( fActiveRequests && reinit ) {
		Trace("re-initializing existing pool");
		return fActiveRequests->Init(actSessions, usePoolStorage, poolStorageSize, numOfPoolBucketSizes, args);
	}
	if (fActiveRequests) {
		Trace("deleting current RequestThreadsManager");
		delete fActiveRequests;
		fActiveRequests = 0;
	}
	Trace("creating new RequestThreadsManager");
	fActiveRequests = new RequestThreadsManager(fName);
	if ( fActiveRequests ) {
		// Register the WorkerPoolManager with the serverStatisticObserver
		server->AddStatGatherer2Observe(fActiveRequests);
		long ret = fActiveRequests->Init(actSessions, usePoolStorage, poolStorageSize, numOfPoolBucketSizes, args);
		Trace("Init code of RequestThreadsManager:" << ret);
		return ret;
	}
	return -1;
}

int ServerThreadPoolsManager::ReInit(Server *server)
{
	StartTrace(ServerThreadPoolsManager.ReInit);
	return SetupThreadPool(true, server);
}

int ServerThreadPoolsManager::Run(Server *server)
{
	StartTrace(ServerThreadPoolsManager.Run);
	Assert(fAcceptors);
	long retVal = fAcceptors->Start(false, 0, 0); // do not use pool storage
	if ( retVal == -1 ) {
		SYSERROR("server (" << fName << ") start accept loops failed");
		return retVal;
	}
	SetReady(true);
	// wait on Join forever
	long ret = fAcceptors->Join(0);
	SetReady(false);
	return ret;
}

bool ServerThreadPoolsManager::BlockRequests(Server *server)
{
	StartTrace(ServerThreadPoolsManager.BlockRequests);

	Context ctx;
	ctx.SetServer(server);
	ctx.Push("ServerThreadPoolsManager", this);
	fActiveRequests->BlockRequests();
	String m(" done\n");
	SysLog::WriteToStderr(m);
	Trace("done");

	m = "Waiting for requests to terminate \n";
	SysLog::WriteToStderr(m);
	Trace("Waiting for requests to terminate ");
	return fActiveRequests->AwaitEmpty(ctx.Lookup("AwaitResetEmpty", 120L));
}

void ServerThreadPoolsManager::UnblockRequests()
{
	StartTrace(ServerThreadPoolsManager.UnblockRequests);
	fActiveRequests->UnblockRequests();
}

int ServerThreadPoolsManager::RequestTermination()
{
	StartTrace(ServerThreadPoolsManager.RequestTermination);
	if ( fAcceptors && (fAcceptors->Terminate(1, 20) != 0) ) {
		return -1;
	}
	return 0;
}

void ServerThreadPoolsManager::Terminate()
{
	StartTrace(ServerThreadPoolsManager.Terminate);

	if ( fActiveRequests ) {
		String m("Waiting for requests to terminate \n");
		SysLog::WriteToStderr(m);
		fActiveRequests->AwaitEmpty(Lookup("AwaitResetEmpty", 120L));			// wait for the last request to terminate
		bool reallyterminated = fActiveRequests->Terminate();	// AwaitEmpty() may be obsolete
		m = (reallyterminated ? "done\n" : "OOPS threads not correctly terminated\n");
		SysLog::WriteToStderr(m);
		Assert(reallyterminated); // SOP should we kill the program?
		delete fActiveRequests;
		fActiveRequests = 0;
	}
	SetReady(false);
	if ( fAcceptors ) {
		delete fAcceptors;
		fAcceptors = 0;
	}
}

long ServerThreadPoolsManager::GetThreadPoolSize()
{
	if ( fActiveRequests ) {
		return fActiveRequests->GetPoolSize();
	}
	return 25;
}

//---- ServerCallBack ------------------------------------------------------------------
class EXPORTDECL_WDBASE ServerCallBack : public AcceptorCallBack
{
public:
	ServerCallBack(RequestThreadsManager *poolManager): fPoolManager(poolManager), fMutex("ServerCallBack"), fRequests(0) { }
	~ServerCallBack() { }
	void CallBack(Socket *s);

	void Lock() {
		fMutex.Lock();
	}
	void Unlock() {
		fMutex.Unlock();
	}
	void Wait() {
		fCond.Wait(fMutex);
	}
	void Signal() {
		MutexEntry me(fMutex);
		me.Use();
		fCond.Signal();
	}

private:
	// block the following default elements of this class
	// because they're not allowed to be used
	ServerCallBack();
	ServerCallBack(const ServerCallBack &);
	ServerCallBack &operator=(const ServerCallBack &);

	RequestThreadsManager *fPoolManager;		// the thread pool manager i'm working with
	Mutex fMutex;
	Condition fCond;
	long fRequests;
};

void ServerCallBack::CallBack(Socket *s)
{
	StartTrace(ServerCallBack.CallBack);
	if (fPoolManager) {
		Anything work;
		work["socket"] = (IFAObject *)s;
		work["request"] = fRequests++;
		TraceAny(work, "work:");
		fPoolManager->Enter(work);
	}
}

WorkerPoolCallBackFactory::WorkerPoolCallBackFactory(RequestThreadsManager *workerPool)
	: fWorkerPool(workerPool)
{
}

AcceptorCallBack *WorkerPoolCallBackFactory::MakeCallBack()
{
	return new ServerCallBack(fWorkerPool);
}
