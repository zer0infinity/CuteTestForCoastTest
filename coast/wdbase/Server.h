/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SERVER_H
#define _SERVER_H

//--- superclass interface ---
#include "Application.h"
#include "WDModule.h"

#if defined(WIN32)
#define InterruptHandler WIN32InterruptHandler
#endif

//---- forward declaration -----------------------------------------------
class RequestProcessor;
class Context;
class ServiceDispatcher;
class StatObserver;
class StatGatherer;
class ServerPoolsManagerInterface;

//---- ServersModule -----------------------------------------------------------
class EXPORTDECL_WDBASE ServersModule : public WDModule
{
public:
	ServersModule(const char *);
	~ServersModule();

	virtual bool Init(const ROAnything config);
	virtual bool Finis();
	virtual bool ResetInit(const ROAnything );
	virtual bool ResetFinis(const ROAnything );
	// access implicitely protected by Server::fgReInitMutex
	static	Server *GetServerForReInit() {
		return fgServerForReInit;
	}
	static 	void SetServerForReInit(Server *runningGlobalReInit) {
		fgServerForReInit = runningGlobalReInit;
	}

protected:
	static Server *fgServerForReInit;
};

//---- Server -----------------------------------------------------------
//!manages the components of the process
//!manages the life-cycle of the server: init run terminate
//!the server initializes SysLog, sets RootDir and Path and initializes the global modules using WDModule::Install(fgConfig)<p>
//!handles service requests as a session based service handler<p>
//!filters and verifies requests
class EXPORTDECL_WDBASE Server : public Application
{
public:
	//!support for named object
	//! \param serverName name of the server
	Server(const char *serverName);
	~Server();

	//!support for prototype pattern
	IFAObject *Clone() const {
		return new Server(fName);
	}
	//!initialization of the Server and its modules
	virtual int GlobalInit(int argc, char *argv[], const ROAnything config);
	//!starts up the server; an InterruptHandler is set up to catch signals for shutdown, reset etc.
	virtual int GlobalRun();

	//!setup blocking and calls DoGlobalReinit
	virtual int GlobalReinit();
	//!inner method doing the reinit
	virtual int DoGlobalReinit();

	//!intialization of the servers Thread Pool for request processing (RequestThreadsManager) and Acceptors (ListenerPool)
	virtual int Init();

	//!reintialization of the servers Thread Pool for request processing (RequestThreadsManager) and Acceptors (ListenerPool)
	virtual int ReInit(const ROAnything config);
	//!starts the session cleaner thread and the ListenerPool, waits for termination
	virtual int Run();
	//!stops the ListenerPool and waits for requests to terminate; server is shutdown
	virtual int Terminate(int val);

	//!service handling in its own thread
	//! \param reply stream to generate the requests output on
	//! \param ctx the context of this request, containing the request and all necessary configurable objects
	virtual void ProcessRequest(ostream &reply, Context &ctx);

	//---- admin API -------------------
	virtual void PrepareShutdown(long retCode = 0);
	virtual int  BlockRequests();
	virtual int  UnblockRequests();
	virtual void QuitRunLoop();

	//---- registry api
	RegCacheDef(Server);	// FindServer()

	//! Register a StatObserver on the WorkerPoolManager of this server
	void RegisterServerStatObserver(StatObserver *observer);

	//! Register a StatObserver on the WorkerPoolManager of this server
	void AddStatGatherer2Observe(StatGatherer *sg);

	//!access to configuration data
	virtual long GetThreadPoolSize();

	//! factory method to create a custom request processor that processes events
	virtual RequestProcessor *DoMakeProcessor();

	//!check if server is ready and running
	bool IsReady(bool ready, long timeout);

	//! Check if server termination is requested by signal (SIGINT)
	// \Needed to distinguish between server reset and server termination.
	// \Returns true if termination was requested
	// \Returns false in all other cases, including server reset (SIGHUP)
	virtual bool MustTerminate();

	//! Helper method to set uid, only done when no MasterServer configured
	virtual int  SetUid();

	//!returns the pid for this server
	virtual int GetPid();

	static bool IsInReInit();

protected:
	friend class InterruptHandler;
	friend class InterruptHandlerTest;
	friend class ServerTest;

	//!implementation of the LookupInterface
	virtual bool DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const;

	//!initialisation of the dispatcher
	virtual int SetupDispatcher();

	//!writes pid file if configured to use pid information to configured location
	virtual int WritePIDFile();
	//!removes pid file when server is shutdown
	virtual int RemovePIDFile();

	//!writes pid information to file; contains platform dependent code
	virtual int DoWritePIDFile(const String &pidFilePath);
	//!removes pid information from file; contains platform dependent code
	virtual int DoDeletePIDFile(const String &pidFilePath);
	//!generates configured filename for pid information file
	virtual void PIDFileName(String &pidFileName);

private:
	// block the following default constructor; it should not be used
	Server();
	// block the following default constructor; it should not be used
	Server(const Server &);
	// block the following default operator; it should not be used
	Server &operator=(const Server &);

protected:
	//!stores the return value set in prepare shutdown
	long fRetVal;

	//!manager of thread pools
	ServerPoolsManagerInterface *fPoolManager;

	//! guard for fInReInit
	static Mutex fgReInitMutex;
	//!state to flag reinit is in process
	static bool fgInReInit;

	//!guard access to fPidFileName and fPid
	Mutex fPidFileNameMutex;
	//! name of the pid file if any is written
	String fPidFileName;
	//! pid of this server
	int fPid;

	Mutex fStoreMutex;				// the guard for fStore
	Anything fStore;				// the server's store

	//!dispatcher for this server
	ServiceDispatcher *fDispatcher;

	//!statistic observer
	StatObserver *fStatisticObserver;
};

#define RegisterServer(name) RegisterApplication(name)

//---- MasterServer -------------------------------------------------------------------
class ServerThread;

//!manages several servers as a composite server
//!manages several servers; each server has its own thread of control
class EXPORTDECL_WDBASE MasterServer : public Server
{
public:
	MasterServer(const char *name);
	~MasterServer();

	//!support for prototype pattern
	IFAObject *Clone() const {
		return new MasterServer(fName);
	}

	//! life-cycle of the server init run terminate
	//!: intialization of the Server and its modules
	virtual int Init();
	//! life-cycle of the server init run terminate
	//!: intialization of the Server and its modules
	virtual int ReInit(const ROAnything config);
	//! accepting requests
	virtual int Run();
	//! termination of the Server modules
	virtual int Terminate(int val);
	virtual void PrepareShutdown(long retCode = 0);
	virtual bool StartServers();

	virtual int BlockRequests();
	virtual int UnblockRequests();

	//!check if server is ready and running
	bool IsReady(bool ready, long timeout);
protected:
	long fNumServers;
	ServerThread *fServerThreads;
};

//---- ServerThread ------------------------------------------------------------
//!thread wrapper for a server started by the MasterServer
class EXPORTDECL_WDBASE ServerThread: public Thread
{
public:
	ServerThread();
	ServerThread(Server *aServer);
	virtual ~ServerThread();

	virtual int Init(ROAnything config);

	virtual void Run();

	virtual void PrepareShutdown(long retCode);
	virtual int BlockRequests();
	virtual int  UnblockRequests();
	virtual int ReInit(const ROAnything config);

	//!check if server is ready and running
	bool IsReady(bool ready, long timeout);

protected:
	Server *fServer;
};

#endif
