/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "Server.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "System.h"
#include "ServerPoolsManagerInterface.h"
#include "Registry.h"
#include "ServerUtils.h"
#include "RequestProcessor.h"
#include "SessionListManager.h"
#include "ServiceDispatcher.h"
#include "AppBooter.h"
#include "RequestBlocker.h"
#include "Dbg.h"
#include "AnyIterators.h"

//--- c-library modules used ---------------------------------------------------
#if !defined(WIN32)
#include <pwd.h>
#else
#include <io.h>
#endif

#ifdef PURE_LEAK
// use purify api functions to trace memory usage
#include <purify.h>
#endif

//#define LOW_TRACING1

//---- ServerReInitInstaller ------------------------------------------------------
/*! alias installer installs the same object with different names in the registry */
class EXPORTDECL_WDBASE ServerReInitInstaller : public InstallerPolicy
{
public:
	ServerReInitInstaller(const char *category)
		: InstallerPolicy(category)
	{}
	virtual ~ServerReInitInstaller() {};

protected:
	virtual bool DoInstall(const ROAnything installerSpec, Registry *r) {
		StartTrace1(ServerReInitInstaller.DoInstall, "cat <" << GetCategory() << ">");
		// nothing to do, base class does the right thing!
		return true;
	}
};

//---- ServerReInitTerminator ------------------------------------------------------
class EXPORTDECL_WDBASE ServerReInitTerminator : public TerminationPolicy
{
public:
	ServerReInitTerminator(const char *category)
		: TerminationPolicy(category)
	{}
	virtual ~ServerReInitTerminator() {};

protected:
	virtual bool DoTerminate(Registry *r) {
		StartTrace1(ServerReInitTerminator.DoTerminate, "cat <" << GetCategory() << ">");
		// nothing to do, base class does the right thing!
		return true;
	}
};

//---- ServersModule -----------------------------------------------------------
RegisterModule(ServersModule);

Server *ServersModule::fgServerForReInit = 0;

ServersModule::ServersModule(const char *name)
	: WDModule(name)
{
	SetServerForReInit(0);
}

ServersModule::~ServersModule()
{
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

//---- Server -------------------------------------------------------------------------
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

Server::Server(const char *name) :
	Application(name),
	fRetVal(0),
	fPoolManager(0),
	fPidFileNameMutex("PIDFile"),
	fPidFileName("pid"),
	fPid(System::getpid()),	 // on linux this pid can't be used to stop the server
	fStoreMutex("Store"),
	fStore(Storage::Global()),
	fStatisticObserver(0)
{
}

Server::~Server()
{
	Assert(this != ServersModule::GetServerForReInit());
	if (fPoolManager) {
		delete fPoolManager;
	}
}

// intialization of the Server and its modules
int Server::GlobalInit(int argc, char *argv[], const ROAnything config)
{
	StartTrace(Server.GlobalInit);

	// make sure we have a session list manager initialized
	SessionListManager::SLM()->Init(config);

	int ret = Application::GlobalInit(argc, argv, config);
	String msg;
	msg << "Global init: " << (ret == 0 ? "suceeded" : "failed");
	SYSINFO(msg);
	SetUid();
	return (ret);
}

int Server::GlobalRun()
{
	StartTrace(Server.GlobalRun);

	InterruptHandler ir(this);

	// let the server do the main loop
	int retVal = Run();

	return retVal;
}

bool Server::IsInReInit()
{
	StartTrace(Server.IsInReInit);
	MutexEntry me(fgReInitMutex);
	me.Use();
	return fgInReInit;
}

int Server::GlobalReinit()
{
	StartTrace(Server.GlobalReinit);
	MutexEntry me(fgReInitMutex);
	me.Use();
	int retCode = 0;
	if ( BlockRequests() != 0 ) {
		UnblockRequests();
		return -1;
	}

	fgInReInit = true;
	fDispatcher = 0; // SOP test it
	ServersModule::SetServerForReInit(this);
	// assume there are no threads running with pending requests
	// reinitialization should take place in global storage
	// otherwise config anys are allocated in thread local storage
	Storage::ForceGlobalStorage(true);
	{
		// block cleaner thread since CheckTimeout of Session
		// makes Lookup calls to shared configs
		SessionListManager::SLM()->EnterReInit();
		retCode = DoGlobalReinit();
		SessionListManager::SLM()->LeaveReInit();
	}
	Storage::ForceGlobalStorage(false);
	ServersModule::SetServerForReInit(0);
	Assert(fDispatcher); // should be re-established
	retCode = UnblockRequests();
	fgInReInit = false;
	String msg;
	msg << "Global reinit: " << (retCode == 0 ? "suceeded" : "failed");
	Trace(msg);
	SYSINFO(msg);
	return retCode;
}

// reintialization of the Server and its modules
int Server::DoGlobalReinit()
{
	StartTrace(Server.DoGlobalReinit);

	Anything config;
	const char *bootfilename = ROAnything(fgConfig)["WD_BOOTFILE"].AsCharPtr("Config");
	if (AppBooter().ReadFromFile(config, bootfilename)) {
		// config file may redefine root directory
		System::SetRootDir( Lookup("Root", System::GetRootDir()), true);
		// fgConfig file may redefine path list
		System::SetPathList( Lookup("PathList", System::GetPathList()), true);
		SysLog::WriteToStderr("Environment set\nResetting Components\n");

		TraceAny(fgConfig, "Old Config");
		TraceAny(config, "New Config");

		int retCode = WDModule::Reset(fgConfig, config);
		fgConfig = config;
		return retCode;
	}
	return -1;
}

// intialization of the Server and its modules
int Server::Init()
{
	StartTrace(Server.Init);
	// Although already done on modules init, do it
	// explicitly here for clarity: Unblock requests
	RequestBlocker::RB()->UnBlock();
	TraceAny(fConfig, "Server config");

	String poolManagerName(Lookup("PoolManager", "ServerThreadPoolsManager"));
	String strServerName;
	GetName(strServerName);
	fPoolManager = ServerPoolsManagerInterface::FindServerPoolsManagerInterface(poolManagerName);
	Trace("PoolManager <" << poolManagerName << "> " << (long)fPoolManager << ((fPoolManager) ? " found" : " not found"));
	if (fPoolManager) {
		// make unique name different of clone-base name to avoid problems
		poolManagerName << "_of_" << strServerName;
		fPoolManager = (ServerPoolsManagerInterface *)fPoolManager->ConfiguredClone("ServerPoolsManagerInterface", poolManagerName, true);
		if ( fPoolManager ) {
			if ( (fPoolManager->Init(this) == 0) && (SetupDispatcher() == 0) ) {
				SysLog::Info(String("Server init of [") << strServerName << "] OK.");
				return 0;
			}
		}
	}
	SYSERROR("Server init of [" << strServerName << "] FAILED.");
	return -1;
}

int Server::SetupDispatcher()
{
	StartTrace(Server.SetupDispatcher);

	fDispatcher = ServiceDispatcher::FindServiceDispatcher(Lookup("Dispatcher", "ServiceDispatcher"));
	if (!fDispatcher) {
		String msg;
		msg << "ServiceDispatcher not found";
		SYSERROR(msg);
		Trace(msg);
		return -1;
	}
	return 0;
}

long Server::GetThreadPoolSize()
{
	if (fPoolManager) {
		return fPoolManager->GetThreadPoolSize();
	}

	return Lookup("ThreadPoolSize", 25L);
}

int Server::ReInit(const ROAnything )
{
	StartTrace(Server.ReInit);
	// hidden dependency in module init/termination sequence!
	long ret = SetupDispatcher();

	if (ret == 0 && fPoolManager) {
		// re-initialize configuration of pool
		fPoolManager->Finalize();
		fPoolManager->Initialize("ServerPoolsManagerInterface");
		ret = fPoolManager->ReInit(this);
	}
	return ret;
}

// termination of the Server modules
int Server::Terminate(int val)
{
	String m;
	m << "Terminating: <" << fName << ">" << "\n";
	SysLog::WriteToStderr(m);
	SysLog::Info("killed");

	if (fPoolManager) {
		fPoolManager->Terminate();
		fPoolManager->Finalize();
	}

	String n;
	this->GetName(n);
	m = "";
	m    << "Cleaning up Server: " << n << "\n\tTerminating running requests";
	SysLog::WriteToStderr(m);
	if (fPoolManager) {
		delete fPoolManager;
		fPoolManager = 0;
	}
	m = " done\n\tTerminating sessions\n";
	SysLog::WriteToStderr(m);
	SessionListManager *pSLM = SessionListManager::SLM();
	if (pSLM) {
		pSLM->Finis();
	}
	m = " done\nServer Cleanup completed\nLeaving shutdown\n";
	SysLog::WriteToStderr(m);
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
	if (ROAnything(Application::fgConfig).LookupPath(result, key, delim, indexdelim)) {
		Trace("found in Master Config");
		return true;
	}

	Trace("failed");
	return false;
}

int Server::Run()
{
	StartTrace(Server.Run);
	int iRet = -1;
	if (fPoolManager) {
		iRet = fPoolManager->Run(this);
		Trace("PoolManager::Run returned with code " << (long)iRet);
	}
	return iRet;
}

bool Server::IsReady(bool ready, long timeout)
{
	StartTrace1(Server.IsReady, "ready: [" << ready << "] timeout: [" << timeout << "]");
	if (fPoolManager) {
		return fPoolManager->IsReady(ready, timeout);
	}
	return false;
}

//---- process a request ---------------------------------------------

void Server::ProcessRequest(ostream &reply, Context &ctx)
{
	StartTrace(Server.ProcessRequest);
	if ( fDispatcher ) {
		fDispatcher->Dispatch2Service(reply, ctx);
	} else {
		RequestProcessor::Error(reply, "Access denied. Lookuptoken: NDA", ctx);
	}
}

RequestProcessor *Server::DoMakeProcessor()
{
	StartTrace(Server.DoMakeProcessor);
	RequestProcessor *rp;
	const char *rpn =  Lookup("RequestProcessor", "RequestProcessor") ;

	Trace("Processor: <" << NotNull(rpn) << ">");
	rp = RequestProcessor::FindRequestProcessor(rpn);
	if (rp == 0) {
		SysLog::WriteToStderr(String("RequestProcessor ") << rpn << " not found\n");
		PrepareShutdown(-1);
		// will shut down the server
	} // if
	rp = (RequestProcessor *)rp->Clone(); // create processor that is connected to this server
	rp->Init(this);
	return rp;
}

void Server::PrepareShutdown(long retCode)
{
	StartTrace(Server.PrepareShutdown);
	fRetVal = retCode;
	RequestBlocker::RB()->Block();
	QuitRunLoop();
}

void Server::QuitRunLoop()
{
	// shutdown the listening socket and terminate the server threads
	if (fPoolManager) {
		fRetVal = fPoolManager->RequestTermination();
	}
}

int Server::BlockRequests()
{
	StartTrace1(Server.BlockRequests, "Server [" << fName << "]");
	Trace("Blocking requests");
	RequestBlocker::RB()->Block();
	if ( fPoolManager && fPoolManager->BlockRequests(this) ) {
		return 0;
	}
	return  -1;
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
int Server::WritePIDFile()
{
	if ( Lookup("UsePIDFile", 0L) ) {
		MutexEntry me(fPidFileNameMutex);
		me.Use();

		PIDFileName(fPidFileName);
		if ( DoWritePIDFile(fPidFileName) != 0 ) {
			return -1;
		}
	}
	return 0;
}

int Server::RemovePIDFile()
{
	MutexEntry me(fPidFileNameMutex);
	me.Use();
	return DoDeletePIDFile(fPidFileName); // ignore retCode
}

void Server::PIDFileName(String &pidFilePath)
{
	StartTrace(Server.PIDFileName);

	String pidFileName(Lookup("PIDFileName", "pid"));

	// reset path to root dir
	pidFilePath = System::GetRootDir();

	// add log directory and file name
	pidFilePath << System::Sep() << pidFileName;
	Trace("PID File<" << pidFilePath << ">");
}

int Server::DoWritePIDFile(const String &pidFilePath)
{
	StartTrace(Server.WritePIDFile);

	ostream *os = System::OpenOStream(pidFilePath, 0);

	if ( os ) {
		Trace("PID File<" << pidFilePath << "> opened");
		fPid = System::getpid();
		(*os) << fPid;
		os->flush();
		delete os;
	} else {
		SYSERROR("couldn't write pid file to " << pidFilePath);
		return -1;
	}

	return 0;
}

int Server::DoDeletePIDFile(const String &pidFilePath)
{
	StartTrace(Server.DoDeletePIDFile);

	if ( System::IO::unlink(pidFilePath) != 0 ) {
		SYSERROR("couldn't delete pid file " << pidFilePath << ": " << SysLog::LastSysError());
		return -1;
	}
	return 0;
}

int Server::GetPid()
{
	MutexEntry me(fPidFileNameMutex);
	me.Use();
	return fPid;
}

int Server::SetUid()
{
	ROAnything serverModules;
	int ret = 1;
#if !defined(WIN32)
	String lookupedUser;
	lookupedUser = Lookup("UserName", "");

	// from uname2id in util.c in apache/src
	struct passwd *ent;

	String m;
	if (lookupedUser != "") {
		const char *username = lookupedUser;
		/* Only try to switch if we're running as root */
		// get real id
		if (!(ent = getpwnam(username))) {
			m << "bad user name %s\n" << username << "\n" ;
			SysLog::WriteToStderr(m);
		}

		if ((geteuid() != ent->pw_uid) && (setuid(ent->pw_uid) == -1)) {
			m = "";
			m << "WARNING setuid - unable to change uid to " << lookupedUser << "\n";
			SysLog::WriteToStderr(m);
			ret = 0;
		}
	}

	ent = getpwuid(geteuid());
	m = "";
	m << (Server::Lookup("ServerModules", serverModules) == 0 ? "Server" : "MasterServer") << " handling requests as:  " << ent->pw_name << "(";
#if defined(__linux__)
	m << long(geteuid());
#else
	m << geteuid();
#endif
	m << ")" << "\n" << "Have fun :-)" << "\n";
	SysLog::WriteToStderr(m);
#endif
	return ret;
}

//---- MasterServer -------------------------------------------------------------------
RegisterServer(MasterServer);
MasterServer::MasterServer(const char *name) : Server(name), fNumServers(0), fServerThreads(0)
{
}
MasterServer::~MasterServer()
{
}

int MasterServer::Init()
{
	StartTrace(MasterServer.Init);
	long retCode = 0;

	if ( !fgInReInit ) {
		retCode = Server::Init();
	}

	ROAnything serverModules;
	if ( (retCode == 0) && (Server::Lookup("ServerModules", serverModules)) ) {
		TraceAny(serverModules, "Server Modules");
		fNumServers = serverModules.GetSize();
		if ( fNumServers > 0 ) {
			fServerThreads = new ServerThread[fNumServers];
			for (long i = 0; (retCode == 0) && (i < fNumServers); ++i) {
				TraceAny(serverModules[i], "initializing server");
				retCode = fServerThreads[i].Init(serverModules[i]);
			}
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
	fgConfig = config.DeepClone();
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
	for (long i = 0; i < fNumServers; ++i) {
		success = fServerThreads[i].Start() && success;
	}
	return success;
}

int MasterServer::Run()
{
	StartTrace(MasterServer.Run);
	int iRet = -1;
	if ( StartServers() ) {
		iRet = Server::Run();
		Trace("Server::Run returned with code " << (long)iRet);
	}
	return iRet;
}

void MasterServer::PrepareShutdown(long retCode)
{
	StartTrace(MasterServer.PrepareShutdown);
	for (long i = 0; i < fNumServers; ++i) {
		fServerThreads[i].PrepareShutdown(retCode);
	}
	Server::PrepareShutdown(retCode);
}

// termination of the Server modules
int MasterServer::Terminate(int val)
{
	String m;
	m << "\tTerminating Master: <" << fName << ">\n";
	SysLog::WriteToStderr(m);
	Anything retval(val);
	for (long i = 0; i < fNumServers; ++i) {
		fServerThreads[i].Terminate(2, retval);
	}
	delete [] fServerThreads;
	long ret = Server::Terminate(val);
	m = "";
	m << "\tTerminating Master: <" << fName << "> done\n";
	SysLog::WriteToStderr(m);
	return ret;
}

//---- ServerThread ------------------------------------------------------------
ServerThread::ServerThread()
	: Thread("ServerThread")
	, fServer(0)
{
}

ServerThread::ServerThread(Server *aServer)
	: Thread("ServerThread")
	, fServer(aServer)
{
}

ServerThread::~ServerThread()
{
	if (fServer) {
		fServer->Terminate(0L);
		fServer = 0;
	}
}

int ServerThread::Init(ROAnything config)
{
	const char *serverName = "Server";
	if ( config.IsDefined("ServerName") ) {
		serverName = config["ServerName"].AsCharPtr(serverName);
	}
	fServer = Server::FindServer(serverName);
	String strName("ServerThread: ");
	strName << serverName;
	SetName(strName);
	if (fServer) {
		return fServer->Init();
	}
	return -1;
}

int ServerThread::ReInit(const ROAnything config)
{
	if (fServer) {
		return fServer->ReInit(config);
	}
	return -1;
}

void ServerThread::Run()
{
	if (fServer) {
		fServer->Run();
	}
}

void ServerThread::PrepareShutdown(long retCode)
{
	if (fServer) {
		fServer->PrepareShutdown(retCode);
	}
}

int ServerThread::BlockRequests()
{
	if (fServer) {
		return fServer->BlockRequests();
	}
	return 0;
}

int ServerThread::UnblockRequests()
{
	if (fServer) {
		return fServer->UnblockRequests();
	}
	return 0;
}

bool ServerThread::IsReady(bool ready, long timeout)
{
	StartTrace1(ServerThread.IsReady, "ready: [" << ready << "] timeout: [" << timeout << "]");
	if (fServer) {
		return fServer->IsReady(ready, timeout);
	}
	return false;
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
