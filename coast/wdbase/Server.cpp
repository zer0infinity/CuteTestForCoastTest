/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Server.h"
#include "SystemBase.h"
#include "SystemFile.h"
#include "ServerPoolsManagerInterface.h"
#include "ServerUtils.h"
#include "RequestProcessor.h"
#include "ServiceDispatcher.h"
#include "AppBooter.h"
#include "RequestBlocker.h"
#include "AnyIterators.h"
#include "Policy.h"
#include "MT_Storage.h"

using namespace coast;

#if !defined(WIN32)
#include <pwd.h>
#else
#include <io.h>
#endif

/*! alias installer installs the same object with different names in the registry */
class ServerReInitInstaller : public InstallerPolicy
{
public:
	ServerReInitInstaller(const char *category)
		: InstallerPolicy(category)
	{}

protected:
	virtual bool DoInstall(const ROAnything installerSpec, Registry *r) {
		StartTrace1(ServerReInitInstaller.DoInstall, "cat <" << GetCategory() << ">");
		// nothing to do, base class does the right thing!
		return true;
	}
};

class ServerReInitTerminator : public TerminationPolicy
{
public:
	ServerReInitTerminator(const char *category)
		: TerminationPolicy(category)
	{}

protected:
	virtual bool DoTerminate(Registry *r) {
		StartTrace1(ServerReInitTerminator.DoTerminate, "cat <" << GetCategory() << ">");
		// nothing to do, base class does the right thing!
		return true;
	}
};

RegisterModule(ServersModule);

Server *ServersModule::fgServerForReInit = 0;

ServersModule::ServersModule(const char *name)
	: WDModule(name)
{
	SetServerForReInit(0);
}

bool ServersModule::Init(const ROAnything config)
{
	StartTrace(ServersModule.Init);
	// special case, servers and applications are treated the same but installed within Application registry
	HierarchyInstaller hi("Application");
	// install anyways, Null-config should not fail the Installer
	return RegisterableObject::Install(config["Servers"], "Application", &hi);
}

bool ServersModule::Finis()
{
	return StdFinis("Application", "Application");
}

bool ServersModule::ResetFinis(const ROAnything )
{
	StartTrace(ServersModule.ResetFinis);
	ServerReInitTerminator at("Application");
	bool bRet = RegisterableObject::ResetTerminate("Application", &at);
	// we need to check if the GetServerForReInit() is registered in the registry
	// if not, we need to re-initialize its config manually because only registered objects will have their config reloaded automatically
	Server *serverToReInit = GetServerForReInit();
	if ( serverToReInit ) {
		String serverName;
		serverToReInit->GetName(serverName);
		Trace("Servername for re-init (master) [" << serverName << "]");
		if ( !Server::FindServer(serverName) ) {
			Trace("manually finalizing server config of [" << serverName << "]");
			bRet = serverToReInit->Finalize() && bRet;
		}
	}
	return bRet;
}

bool ServersModule::ResetInit(const ROAnything config)
{
	StartTrace(ServersModule.ResetInit);
	String serverName;
	Server *serverToReInit = 0;
	serverToReInit = GetServerForReInit();
	if (serverToReInit) {
		serverToReInit->GetName(serverName);
		Trace("Servername for re-init (master) [" << serverName << "]");
	}

	// re-initialize configuration of server(s)
	ServerReInitInstaller hi("Application");
	// install anyways, Null-config should not fail the Installer
	bool bReInitOk = RegisterableObject::Install(config["Servers"], "Application", &hi);

	if ( bReInitOk && serverToReInit ) {
		// we need to check if the serverToReInit is registered in the registry
		// if not, we need to re-initialize its config manually because only registered objects will have their config reloaded automatically
		if ( !Server::FindServer(serverName) ) {
			Trace("manually finalizing server config of [" << serverName << "]");
			bReInitOk = serverToReInit->Initialize("Application") && bReInitOk;
		}

		if ( bReInitOk && ( 0 == serverToReInit->ReInit(config) ) ) {
			SetServerForReInit(0);
			return true;
		}
	}
	String logMsg("ReInit failed for <");
	logMsg << serverName << "> at " << long(serverToReInit);
	SYSWARNING(logMsg);
	Trace(logMsg);
	return false;
}

RegisterServer(Server);

Mutex Server::fgReInitMutex("Reinit");
bool Server::fgInReInit = false;


//RegCacheImpl(Server);	// FindServer()
// implement FindServer by hand since it uses Application's registry entries
// guard against misuse by SafeCast
Server *Server::FindServer(const char *name)
{
	return SafeCast(Application::FindApplication(name), Server);
}

Server::Server(const char *name)
	: Application(name)
	, fRetVal(0)
	, fPoolManager(0)
	, fPidFileNameMutex("PIDFile")
	, fPidFileName("pid")
	, fPid(system::getpid())	// on linux this pid can't be used to stop the server
	, fStoreMutex("Store")
	, fStore(coast::storage::Global())
	, fStatisticObserver(0)
{
	StartTrace1(Server.Server, "<" << GetName() << ">");
}

Server::~Server()
{
	StartTrace1(Server.~Server, "<" << GetName() << ">");
	Assert(this != ServersModule::GetServerForReInit());
	if ( fPoolManager ) {
		delete fPoolManager;
	}
}

// intialization of the Server and its modules
int Server::DoGlobalInit(int argc, const char *argv[], const ROAnything config)
{
	StartTrace(Server.DoGlobalInit);
	int ret(-1);
	if ( ( ret = Application::DoGlobalInit(argc, argv, config) ) >= 0 ) {
		SetUid();
	}
	return (ret);
}

int Server::DoGlobalRun()
{
	StartTrace(Server.DoGlobalRun);
	InterruptHandler ir(this);
	// before continuing, wait for the InterruptHandler to be working, eg. ready to wait on signals
	ir.CheckRunningState(Thread::eWorking);
	return Application::DoGlobalRun();
}

bool Server::IsInReInit()
{
	StartTrace(Server.IsInReInit);
	LockUnlockEntry me(fgReInitMutex);
	return fgInReInit;
}

int Server::GlobalReinit()
{
	StartTrace(Server.GlobalReinit);
	LockUnlockEntry me(fgReInitMutex);
	if ( BlockRequests() != 0 ) {
		UnblockRequests();
		return -1;
	}
	fgInReInit = true;
	ServersModule::SetServerForReInit(this);
	int globalReinitReturnCode = 0;
	// assume there are no threads running with pending requests
	{
		// reinitialization should take place in global storage
		// otherwise configuration Anythings are allocated in thread local storage
		coast::storage::ForceGlobalStorageEntry onlyUseGlobalMemoryHere;
		// block cleaner thread since CheckTimeout of Session
		// makes Lookup calls to shared configurations
		globalReinitReturnCode = DoGlobalReinit();
	}
	ServersModule::SetServerForReInit(0);
	if ( globalReinitReturnCode == 0 ) {
		globalReinitReturnCode = UnblockRequests();
	}
	fgInReInit = false;
	String msg;
	msg << "Global reinit: " << (globalReinitReturnCode == 0 ? "succeeded" : "failed");
	Trace(msg);
	SYSINFO(msg);
	return globalReinitReturnCode;
}

// reintialization of the Server and its modules
int Server::DoGlobalReinit()
{
	StartTrace(Server.DoGlobalReinit);

	Anything config;
	const char *bootfilename = GetConfig()["COAST_BOOTFILE"].AsCharPtr("Config");
	if (AppBooter().ReadFromFile(config, bootfilename)) {
		// config file may redefine root directory
		system::SetRootDir( Lookup("Root", system::GetRootDir()), true);
		// fgConfig file may redefine path list
		system::SetPathList( Lookup("PathList", system::GetPathList()), true);
		SystemLog::WriteToStderr("Environment set\nResetting Components\n");

		TraceAny(GetConfig(), "Old Config");
		TraceAny(config, "New Config");

		int retCode = WDModule::Reset(GetConfig(), config);
		InitializeGlobalConfig(config);
		return retCode;
	}
	return -1;
}

// intialization of the Server and its modules
int Server::DoInit()
{
	StartTrace(Server.DoInit);
	// Although already done on modules init, do it
	// explicitly here for clarity: Unblock requests
	RequestBlocker::RB()->UnBlock();
	TraceAny(fConfig, "Server config");
	String strServerName;
	GetName(strServerName);

	if ( Lookup("NoPoolManager", 0L) == 0L ) {
		String poolManagerName(Lookup("PoolManager", "ServerThreadPoolsManager"));
		fPoolManager = ServerPoolsManagerInterface::FindServerPoolsManagerInterface(poolManagerName);
		Trace("PoolManager <" << poolManagerName << "> " << (long)fPoolManager << ((fPoolManager) ? " found" : " not found"));
		if ( fPoolManager ) {
			// make unique name different of clone-base name to avoid problems
			poolManagerName << "_of_" << strServerName;
			fPoolManager = (ServerPoolsManagerInterface *)fPoolManager->ConfiguredClone("ServerPoolsManagerInterface", poolManagerName, true);
			if ( fPoolManager ) {
				if ( (fPoolManager->Init(this) == 0) && (SetupDispatcher() == 0) ) {
					SystemLog::Info(String("Server init of [") << strServerName << "] OK.");
					return 0;
				}
			}
		}
	} else {
		Trace("not using PoolManager");
		if ( SetupDispatcher() == 0 ) {
			SystemLog::Info(String("Server init with NoPoolManager of [") << strServerName << "] OK.");
			return 0;
		}
	}
	SYSERROR("Server init of [" << strServerName << "] FAILED.");
	return -1;
}

int Server::SetupDispatcher()
{
	StartTrace(Server.SetupDispatcher);

	fDispatcher = ServiceDispatcher::FindServiceDispatcher(Lookup("Dispatcher", "ServiceDispatcher"));
	if ( !fDispatcher ) {
		String msg;
		msg << "ServiceDispatcher not found";
		SYSERROR(msg);
		Trace(msg);
		return -1;
	}
	return 0;
}

int Server::ReInit(const ROAnything )
{
	StartTrace(Server.ReInit);
	// hidden dependency in module init/termination sequence!
	long ret = 0L;
	if ( fDispatcher ) {
		ret = SetupDispatcher();
	}
	if ( ret == 0 && fPoolManager ) {
		// re-initialize configuration of pool
		fPoolManager->Finalize();
		fPoolManager->Initialize("ServerPoolsManagerInterface");
		ret = fPoolManager->ReInit(this);
	}
	return ret;
}

// termination of the Server modules
int Server::DoTerminate(int val)
{
	StartTrace(Server.DoTerminate);
	if ( fPoolManager ) {
		SystemLog::WriteToStderr("\t\tTerminating running requests");
		if ( fPoolManager->IsReady(false, 10) ) {
			fPoolManager->Terminate();
			fPoolManager->Finalize();
			delete fPoolManager;
			fPoolManager = 0;
			SystemLog::WriteToStderr(" done\n");
		} else {
			SystemLog::WriteToStderr(" failed\n");
		}
	}
	return fRetVal;
}

bool Server::DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const
{
	StartTrace1(Server.DoLookup, "key:<" << NotNull(key) << ">");
	// lookup the servers store
	if ( ROAnything(fStore).LookupPath(result, key, delim, indexdelim) ) {
		Trace("found in store");
		return true;
	}

	// have a lookup in the servers config file
	if ( HierarchConfNamed::DoLookup(key, result, delim, indexdelim) ) {
		Trace("found in servers config");
		return true;
	}

	// lookup the main process config
	if (GetConfig().LookupPath(result, key, delim, indexdelim)) {
		Trace("found in Master Config");
		return true;
	}
	Trace("failed");
	return false;
}

int Server::DoRun()
{
	StartTrace(Server.DoRun);
	int iRet = -1;
	if (fPoolManager) {
		Trace("<" << GetName() << "> running PoolManager...");
		iRet = fPoolManager->Run(this);
		Trace("PoolManager::Run of <" << GetName() << "> returned with code " << (long)iRet);
	}
	return iRet;
}

bool Server::IsReady(bool ready, long timeout)
{
	StartTrace1(Server.IsReady, "ready: [" << ready << "] timeout: [" << timeout << "]");
	if ( fPoolManager ) {
		return fPoolManager->IsReady(ready, timeout);
	}
	return false;
}

bool Server::ProcessRequest(std::ostream &reply, Context &ctx)
{
	StartTrace(Server.ProcessRequest);
	if ( fDispatcher ) {
		return fDispatcher->Dispatch2Service(reply, ctx);
	}
	return false;
}

RequestProcessor *Server::MakeProcessor()
{
	StartTrace(Server.MakeProcessor);
	RequestProcessor *rp;
	const char *rpn =  Lookup("RequestProcessor", "RequestProcessor") ;

	Trace("Processor: <" << NotNull(rpn) << ">");
	rp = RequestProcessor::FindRequestProcessor(rpn);
	if (rp == 0) {
		SystemLog::WriteToStderr(String("RequestProcessor ") << rpn << " not found\n");
		// shut down the server
		PrepareShutdown(-1);
	} else {
		// create processor that is connected to this server
		rp = (RequestProcessor *)rp->Clone(coast::storage::Global());
		rp->Init(this);
	}
	return rp;
}

RequestProcessor* Server::GetRequestProcessor() {
	if ( fPoolManager ) {
		return fPoolManager->GetRequestProcessor();
	}
	return 0;
}

void Server::PrepareShutdown(int retCode)
{
	StartTrace(Server.PrepareShutdown);
	fRetVal = DoPrepareShutdown(retCode);
}

int Server::DoPrepareShutdown(int retCode)
{
	StartTrace(Server.DoPrepareShutdown);
	RequestBlocker::RB()->Block();
	retCode = QuitRunLoop();
	return retCode;
}

int Server::QuitRunLoop()
{
	StartTrace(Server.QuitRunLoop);
	int iRetCode = 0L;
	// shutdown the listening socket and terminate the server threads
	if ( fPoolManager ) {
		iRetCode = fPoolManager->RequestTermination();
	}
	return iRetCode;
}

int Server::BlockRequests()
{
	StartTrace1(Server.BlockRequests, "Server [" << fName << "]");
	Trace("Blocking requests");
	RequestBlocker::RB()->Block();
	if ( fPoolManager ) {
		return ( fPoolManager->BlockRequests(this) ? 0 : -1 );
	}
	return  0;
}

int Server::UnblockRequests()
{
	StartTrace1(Server.UnblockRequests, "Server [" << fName << "]");
	// Set to unblocked before the threads accept requests again, otherwise
	// pending requests would be aborted by workerthread implementation which
	// copy requests in chunks and ask for RB()->IsBlocked
	RequestBlocker::RB()->UnBlock();

	if ( fPoolManager ) {
		fPoolManager->UnblockRequests();
	}
	return 0;
}

bool Server::MustTerminate()
{
	StartTrace(Server.MustTerminate);
	if ( IsInReInit() ) {
		Trace("IsInReInit: [" << IsInReInit() << "]");
	} else {
		if (RequestBlocker::RB()->IsBlocked() ) {
			Trace("Server: [" << GetName() << "] IsBlocked: [" << RequestBlocker::RB()->IsBlocked() << "]");
			return true;
		}
	}
	return false;
}

// pidfile handling
int Server::WritePIDFile(pid_t lPid)
{
	if ( Lookup("UsePIDFile", 0L) ) {
		LockUnlockEntry me(fPidFileNameMutex);

		PIDFileName(fPidFileName);
		if ( DoWritePIDFile(fPidFileName, lPid) != 0 ) {
			return -1;
		}
	}
	return 0;
}

int Server::RemovePIDFile()
{
	if ( Lookup("UsePIDFile", 0L) ) {
		LockUnlockEntry me(fPidFileNameMutex);
		return DoDeletePIDFile(fPidFileName); // ignore retCode
	}
	return 0;
}

void Server::PIDFileName(String &pidFilePath)
{
	StartTrace(Server.PIDFileName);

	String pidFileName(Lookup("PIDFileName", "pid"));

	// reset path to root dir
	pidFilePath = system::GetRootDir();

	// add log directory and file name
	pidFilePath << system::Sep() << pidFileName;
	Trace("PID File<" << pidFilePath << ">");
}

int Server::DoWritePIDFile(const String &pidFilePath, pid_t lPid)
{
	StartTrace(Server.WritePIDFile);

	std::ostream *os = system::OpenOStream(pidFilePath, 0);

	if ( os ) {
		Trace("PID File<" << pidFilePath << "> opened");
		if ( lPid != (pid_t) - 1 ) {
			fPid = lPid;
		} else {
			fPid = system::getpid();
		}
		(*os) << fPid;
		os->flush();
		delete os;
	} else {
		SYSWARNING("couldn't write pid file to " << pidFilePath);
		return -1;
	}

	return 0;
}

int Server::DoDeletePIDFile(const String &pidFilePath)
{
	StartTrace(Server.DoDeletePIDFile);

	if ( system::io::unlink(pidFilePath) != 0 ) {
		SYSWARNING("couldn't delete pid file " << pidFilePath << ": " << SystemLog::LastSysError());
		return -1;
	}
	return 0;
}

int Server::GetPid()
{
	LockUnlockEntry me(fPidFileNameMutex);
	return fPid;
}

int Server::SetUid()
{
	ROAnything serverModules;
	int ret = 1;
#if !defined(WIN32)
	String lookupedUser( Lookup("UserName", "") );

	// from uname2id in util.c in apache/src
	struct passwd *ent;

	String m;
	if (lookupedUser != "") {
		const char *username = lookupedUser;
		/* Only try to switch if we're running as root */
		// get real id
		if (!(ent = getpwnam(username))) {
			m << "bad user name %s\n" << username << "\n" ;
			SystemLog::WriteToStderr(m);
		}

		if ((geteuid() != ent->pw_uid) && (setuid(ent->pw_uid) == -1)) {
			m = "";
			m << "WARNING setuid - unable to change uid to " << lookupedUser << "\n";
			SystemLog::WriteToStderr(m);
			ret = 0;
		}
	}

	ent = getpwuid(geteuid());
	m = "";
	m << (Server::Lookup("ServerModules", serverModules) == 0 ? "Server" : "MasterServer") << " handling requests as: " << ent->pw_name << "(";
#if defined(__linux__) || defined(__APPLE__)
	m << long(geteuid());
#else
	m << geteuid();
#endif
	m << "), server-pid: " << GetPid() << "\n";
	SystemLog::WriteToStderr(m);
#endif
	return ret;
}

void Server::RegisterServerStatObserver(StatObserver *observer)
{
	fStatisticObserver = observer;
}

void Server::AddStatGatherer2Observe(StatGatherer *sg)
{
	if (fStatisticObserver) {
		String myServerName;
		GetName(myServerName);
		fStatisticObserver->Register(myServerName, sg);
	}
}

RegisterServer(MasterServer);

int MasterServer::DoInit()
{
	StartTrace(MasterServer.DoInit);
	long retCode = 0;

	if ( !fgInReInit ) {
		retCode = Server::DoInit();
	}

	ROAnything serverModules;
	if ( (retCode == 0) && (Server::Lookup("ServerModules", serverModules)) ) {
		TraceAny(serverModules, "Server Modules");
		fNumServers = serverModules.GetSize();
		if ( fNumServers > 0 ) {
			fServerThreads = new ServerThread[fNumServers];
			AnyExtensions::Iterator<ROAnything> aServersIterator(serverModules);
			ROAnything roaServerConfig;
			long lIdx = 0L;
			bool bStartSuccess = true;
			while ( aServersIterator.Next(roaServerConfig) && bStartSuccess ) {
				TraceAny(roaServerConfig, "initializing server");
				// Start serverthread which internally waits on setting to work
				Allocator *pAlloc = coast::storage::Global();
				if ( !roaServerConfig.IsNull() && ( roaServerConfig["UsePoolStorage"].AsLong(0) == 1 ) ) {
					TraceAny(roaServerConfig, "creating PoolAllocator for server");
					pAlloc = MT_Storage::MakePoolAllocator(roaServerConfig["PoolStorageSize"].AsLong(10240), roaServerConfig["NumOfPoolBucketSizes"].AsLong(10), 0);
					if ( pAlloc == NULL ) {
						SYSERROR("was not able to create PoolAllocator for [" << roaServerConfig["ServerName"].AsString() << "], check config!");
						pAlloc = coast::storage::Global();
					}
				}
				bStartSuccess = fServerThreads[lIdx].Start(pAlloc, roaServerConfig);
				fServerThreads[lIdx].CheckState(Thread::eStarted);
				bStartSuccess = bStartSuccess && fServerThreads[lIdx].serverIsInitialized();
				Trace("Start of ServerThread:" << lIdx << "<" << roaServerConfig["ServerName"].AsString() << "> " << ( bStartSuccess ? "successful" : "unsuccessful" ) );
				if ( !bStartSuccess ) {
					SYSERROR("Start of ServerThread:" << lIdx << "<" << roaServerConfig["ServerName"].AsString() << "> unsuccessful");
				}
				++lIdx;
			}
			retCode = ( bStartSuccess ? 0 : -1 );
		} else {
			retCode = -1;
		}
	}
	return retCode;
}

int MasterServer::ReInit(const ROAnything config)
{
	StartTrace(MasterServer.ReInit);
	// save current configuration for re-initializing
	InitializeGlobalConfig(config.DeepClone());
	long retCode = 0;
	for (long i = 0; (retCode == 0) && (i < fNumServers); ++i) {
		// terminates each server thread
		retCode = fServerThreads[i].ReInit(config);
	}
	if (retCode == 0) {
		return Server::ReInit(config);
	}
	return retCode;
}

int MasterServer::BlockRequests()
{
	StartTrace(MasterServer.BlockRequests);
	// start blocking all requests and terminate master server thread
	long retCode = 0;
	Trace("blocking [" << fNumServers << "] servers");

	for (long i = 0; (retCode == 0) && (i < fNumServers); ++i) {
		// terminates each server thread
		retCode = fServerThreads[i].BlockRequests();
		if ( retCode != 0 ) {
			Trace("blocking failed");
			return retCode;
		}
	}
	return Server::BlockRequests();
}

int MasterServer::UnblockRequests()
{
	StartTrace(MasterServer.UnblockRequests);
	// start blocking all requests and terminate master server thread
	long retCode = Server::UnblockRequests();
	for (long i = 0; (retCode == 0) && (i < fNumServers); ++i) {
		// terminates each server thread
		retCode = fServerThreads[i].UnblockRequests();
		if ( retCode != 0 ) {
			return retCode;
		}
	}
	return retCode;
}

bool MasterServer::IsReady(bool ready, long timeout)
{
	StartTrace1(MasterServer.IsReady, "ready: [" << ready << "] timeout: [" << timeout << "]");

	bool success = true;
	for (long i = 0; i < fNumServers; ++i) {
		success = fServerThreads[i].IsReady(ready, timeout) && success;
	}
	return Server::IsReady(ready, timeout) && success;
}

bool MasterServer::StartServers()
{
	StartTrace(MasterServer.StartServers);
	bool success = true;
	ROAnything serverModules;
	if ( Lookup("ServerModules", serverModules) ) {
		TraceAny(serverModules, "servers to control");
		AnyExtensions::Iterator<ROAnything> aServersIterator(serverModules);
		ROAnything roaServerConfig;
		long lIdx = 0L;
		while ( aServersIterator.Next(roaServerConfig) && success ) {
			success = fServerThreads[lIdx].CheckState(Thread::eRunning, 5) && fServerThreads[lIdx].SetWorking(roaServerConfig);
			Trace("SetWorking of ServerThread:" << lIdx << "<" << roaServerConfig["ServerName"].AsString() << "> " << ( success ? "successful" : "unsuccessful" ) );
			if ( !success ) {
				SYSERROR("SetWorking of ServerThread:" << lIdx << "<" << roaServerConfig["ServerName"].AsString() << "> unsuccessful");
			}
			++lIdx;
		}
	}
	return success;
}

int MasterServer::DoRun()
{
	StartTrace1(MasterServer.DoRun, "<" << GetName() << ">");
	int iRet = -1;
	if ( StartServers() ) {
		Trace("calling masterservers Run method");
		iRet = Server::DoRun();
		Trace("Server::DoRun returned with code " << (long)iRet);
	}
	return iRet;
}

int MasterServer::DoPrepareShutdown(int retCode)
{
	StartTrace(MasterServer.DoPrepareShutdown);
	for (long i = 0; i < fNumServers; ++i) {
		fServerThreads[i].PrepareShutdown(retCode);
	}
	return Server::DoPrepareShutdown(retCode);
}

// termination of the Server modules
int MasterServer::DoTerminate(int val)
{
	StartTrace(MasterServer.DoTerminate);

	Anything retval(val);
	for (long i = 0; i < fNumServers; ++i) {
		String m(50L);
		m << "\t\tTerminating <" << fServerThreads[i].GetName() << ">\n";
		SystemLog::WriteToStderr(m);
		Thread::EThreadState aState = fServerThreads[i].GetState(false, Thread::eRunning);
		if ( aState < Thread::eTerminationRequested ) {
			fServerThreads[i].Terminate(60, retval);
		}
		fServerThreads[i].CheckState(Thread::eTerminated, 10);
		m.Trim(0);
		m << "\t\tTerminating <" << fServerThreads[i].GetName() << "> done\n";
		SystemLog::WriteToStderr(m);
	}
	Trace("deleting server threads");
	delete [] fServerThreads;
	return Server::DoTerminate(val);
}

ServerThread::ServerThread()
	: Thread("ServerThread")
	, fServer(0)
	, fbServerIsInitialized(false)
	, fTerminationMutex( "ServerThreadTerminationMutex", coast::storage::Global() )
{
}

ServerThread::ServerThread(Server *aServer)
	: Thread("ServerThread")
	, fServer(aServer)
	, fbServerIsInitialized(false)
	, fTerminationMutex( "ServerThreadTerminationMutex", coast::storage::Global() )
{
}

ServerThread::~ServerThread()
{
}

void ServerThread::DoStartedHook(ROAnything config)
{
	StartTrace(ServerThread.DoStartedHook);
	const char *serverName = "Server";
	if ( fServer == NULL ) {
		if ( config.IsDefined("ServerName") ) {
			serverName = config["ServerName"].AsCharPtr(serverName);
		}
		fServer = Server::FindServer(serverName);
	} else {
		serverName = fServer->GetName();
	}
	String strName("ServerThread: ", coast::storage::Global());
	strName.Append(serverName);
	SetName(strName);
	if ( fServer ) {
		fbServerIsInitialized = ( fServer->Init() == 0 );
	}
	Trace("Server <" << GetName() << "> started");
}

int ServerThread::ReInit(const ROAnything config)
{
	StartTrace1(ServerThread.ReInit, "<" << GetName() << ">");
	int iRet = -1;
	if ( fServer && fbServerIsInitialized ) {
		fbServerIsInitialized = ( ( iRet = fServer->ReInit(config) ) == 0 );
	}
	return iRet;
}

void ServerThread::Run()
{
	StartTrace1(ServerThread.Run, "<" << GetName() << ">");
	if ( fbServerIsInitialized && CheckRunningState( eWorking ) ) {
		if (fServer) {
			fServer->Run();
		}
		// synchronize with PrepareShutdown call
		// the caller must have left the method before continuing here
		LockUnlockEntry me(fTerminationMutex);
	}
	SetReady();
}

void ServerThread::DoTerminatedRunMethodHook()
{
	StartTrace1(ServerThread.DoTerminatedRunMethodHook, "<" << GetName() << ">");
	if ( fServer && fbServerIsInitialized ) {
		fServer->Terminate(0L);
		fbServerIsInitialized = false;
	}
}

void ServerThread::PrepareShutdown(long retCode)
{
	StartTrace1(ServerThread.PrepareShutdown, "<" << GetName() << ">");
	if ( fServer && fbServerIsInitialized ) {
		LockUnlockEntry me(fTerminationMutex);
		fServer->PrepareShutdown(retCode);
	}
}

int ServerThread::BlockRequests()
{
	if ( fServer && fbServerIsInitialized ) {
		return fServer->BlockRequests();
	}
	return 0;
}

int ServerThread::UnblockRequests()
{
	if ( fServer && fbServerIsInitialized ) {
		return fServer->UnblockRequests();
	}
	return 0;
}

bool ServerThread::IsReady(bool ready, long timeout)
{
	StartTrace1(ServerThread.IsReady, "ready: [" << ready << "] timeout: [" << timeout << "]");
	if ( fServer && fbServerIsInitialized ) {
		return fServer->IsReady(ready, timeout);
	}
	return (ready == false);
}
