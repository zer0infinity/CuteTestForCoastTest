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

	//!setup blocking and calls DoGlobalReinit
	int GlobalReinit();

	//!reintialization of the servers Thread Pool for request processing (RequestThreadsManager) and Acceptors (ListenerPool)
	virtual int ReInit(const ROAnything config);
	//!service handling in its own thread
	//! \param reply stream to generate the requests output on
	//! \param ctx the context of this request, containing the request and all necessary configurable objects
	virtual void ProcessRequest(ostream &reply, Context &ctx);

	//---- admin API -------------------
	void PrepareShutdown(int retCode = 0);

	virtual int BlockRequests();
	virtual int UnblockRequests();
	virtual int QuitRunLoop();

	//---- registry api
	RegCacheDef(Server);	// FindServer()

	//! Register a StatObserver on the WorkerPoolManager of this server
	void RegisterServerStatObserver(StatObserver *observer);

	//! Register a StatObserver on the WorkerPoolManager of this server
	void AddStatGatherer2Observe(StatGatherer *sg);

	//! factory method to create a custom request processor that processes events
	RequestProcessor *MakeProcessor();

	//!check if server is ready and running
	bool IsReady(bool ready, long timeout);

	//! Check if server termination is requested by signal (SIGINT)
	// \Needed to distinguish between server reset and server termination.
	// \Returns true if termination was requested
	// \Returns false in all other cases, including server reset (SIGHUP)
	virtual bool MustTerminate();

	//! Helper method to set uid, only done when no MasterServer configured
	int SetUid();

	//!returns the pid for this server
	int GetPid();

	static bool IsInReInit();

protected:
	//!intialization of the servers Thread Pool for request processing (RequestThreadsManager) and Acceptors (ListenerPool)
	virtual int DoInit();

	//!starts the session cleaner thread and the ListenerPool, waits for termination
	virtual int DoRun();

	//!initialization of the Server and its modules
	virtual int DoGlobalInit(int argc, char *argv[], const ROAnything config);

	//!inner method doing the reinit
	virtual int DoGlobalReinit();

	//!starts up the server; an InterruptHandler is set up to catch signals for shutdown, reset etc.
	virtual int DoGlobalRun();

	//!stops the ListenerPool and waits for requests to terminate; server is shutdown
	virtual int DoTerminate(int val);

	//! overridable hook which gets called by the signal handler to initiate shutdown
	virtual int DoPrepareShutdown(int retCode);

	friend class InterruptHandler;
	friend class InterruptHandlerTest;
	friend class ServerTest;

	//!implementation of the LookupInterface
	virtual bool DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const;

	//!initialisation of the dispatcher
	virtual int SetupDispatcher();

	//!writes pid file if configured to use pid information to configured location
	int WritePIDFile(pid_t lPid = (pid_t) - 1);
	//!removes pid file when server is shutdown
	int RemovePIDFile();

	//!writes pid information to file; contains platform dependent code
	virtual int DoWritePIDFile(const String &pidFilePath, pid_t lPid);
	//!removes pid information from file; contains platform dependent code
	virtual int DoDeletePIDFile(const String &pidFilePath);
	//!generates configured filename for pid information file
	void PIDFileName(String &pidFileName);

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

//! <B>Manages several servers as a composite server</B>
/*!
<B>Configuration:</B><PRE>
{
	/ServerModules {
		{
			/ServerName				MyRegisteredServer	# mandatory, name of server instance to run
			/UsePoolStorage			1					# optional, [0|1], default 0, use preallocated memory pool for storage of Queue elements
			/PoolStorageSize		22001				# optional, [kB], default 10240, pool storage size in kbytes
			/NumOfPoolBucketSizes	16					# optional, default 10, number of different allocation units within PoolStorage, starts at 16 bytes and always doubles the size so 16 << 10 will give a max usable size of 8192 bytes
		}
		...
	}
}</PRE>
The MasterServer manages several servers as configured within ServerModules. Each server has its own thread of control. This allows having an own memory pool for the server instance to optimize for performance.
The Server methods Init() and Terminate() will be called in DoStartedHook and DoTerminatedRunMethodHook respectively which allows usage of pool memory from within Init().
*/
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
	virtual int ReInit(const ROAnything config);
	virtual int BlockRequests();
	virtual int UnblockRequests();

	bool StartServers();

	//!check if server is ready and running
	bool IsReady(bool ready, long timeout);

protected:
	//! life-cycle of the server init run terminate
	//!: intialization of the Server and its modules
	virtual int DoInit();
	//! accepting requests
	virtual int DoRun();

	//! termination of the Server modules
	virtual int DoTerminate(int val);

	virtual int DoPrepareShutdown(int retCode = 0);

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

	void Run();

	void PrepareShutdown(long retCode);
	int BlockRequests();
	int UnblockRequests();
	int ReInit(const ROAnything config);

	bool IsInitialized() {
		return fbInitialized;
	}

	//!check if server is ready and running
	bool IsReady(bool ready, long timeout);

private:
	void DoStartedHook(ROAnything config);
	void DoTerminatedRunMethodHook();

	Server *fServer;
	bool fbInitialized;
	SimpleMutex fTerminationMutex;
};

#endif
