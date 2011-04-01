/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "RequestListener.h"
#include "SystemLog.h"
#include "Registry.h"
#include "Policy.h"
#include "Socket.h"

//---- AcceptorFactoriesReInitInstaller ------------------------------------------------------
/*! alias installer installs the same object with different names in the registry */
class AcceptorFactoriesReInitInstaller: public InstallerPolicy {
public:
	AcceptorFactoriesReInitInstaller(const char *category) :
		InstallerPolicy(category) {
	}

protected:
	virtual bool DoInstall(const ROAnything installerSpec, Registry *r) {
		StartTrace1(AcceptorFactoriesReInitInstaller.DoInstall, "cat <" << GetCategory() << ">");
		// nothing to do, base class does the right thing!
		return true;
	}
};

//---- AcceptorFactoriesReInitTerminator ------------------------------------------------------
class AcceptorFactoriesReInitTerminator: public TerminationPolicy {
public:
	AcceptorFactoriesReInitTerminator(const char *category) :
		TerminationPolicy(category) {
	}

protected:
	virtual bool DoTerminate(Registry *r) {
		StartTrace1(AcceptorFactoriesReInitTerminator.DoTerminate, "cat <" << GetCategory() << ">");
		// nothing to do, base class does the right thing!
		return true;
	}
};
//---- AcceptorFactoriesModule -----------------------------------------------------------
RegisterModule(AcceptorFactoriesModule);

AcceptorFactoriesModule::AcceptorFactoriesModule(const char *name) :
	WDModule(name) {
	StartTrace1(AcceptorFactoriesModule.Ctor, "Name: <" << NotNull(name) << ">");
}

bool AcceptorFactoriesModule::Init(const ROAnything config) {
	StartTrace(AcceptorFactoriesModule.Init);
	if (config.IsDefined("AcceptorFactories")) {
		HierarchyInstaller ai("AcceptorFactory");
		return RegisterableObject::Install(config["AcceptorFactories"], "AcceptorFactory", &ai);
	}
	return false;
}

bool AcceptorFactoriesModule::Finis() {
	StartTrace(AcceptorFactoriesModule.Finis);
	return StdFinis("AcceptorFactory", "AcceptorFactories");
}

bool AcceptorFactoriesModule::ResetInit(const ROAnything config) {
	AcceptorFactoriesReInitInstaller hi("AcceptorFactory");
	return RegisterableObject::Install(config["AcceptorFactories"], "AcceptorFactory", &hi);
}

bool AcceptorFactoriesModule::ResetFinis(const ROAnything) {
	AcceptorFactoriesReInitTerminator at("AcceptorFactory");
	return RegisterableObject::ResetTerminate("AcceptorFactory", &at);
}

Acceptor *AcceptorFactory::MakeAcceptor(AcceptorCallBack *ac) {
	StartTrace(AcceptorFactory.MakeAcceptor);
	Trace("fName: " << fName);
	TraceAny(fConfig, "fConfig: ");

	const char *address = Lookup("Address", (const char *) 0);
	long port = Lookup("Port", 80L);
	long backlog = Lookup("Backlog", 50L);

	Trace("Configuring: address<" << NotNull(address) << "> port: " << port << " backlog: " << backlog);
	return new Acceptor(address, port, backlog, ac);
}

bool AcceptorFactory::DoLoadConfig(const char *category) {
	StartTrace1(AcceptorFactory.DoLoadConfig, "category: <" << NotNull(category) << "> object: <" << fName << ">");

	if (HierarchConfNamed::DoLoadConfig(category) && fConfig.IsDefined(fName)) {
		// AcceptorFactories use only a subset of the whole configuration file
		fConfig = fConfig[fName];
		Assert(!fConfig.IsNull());
		TraceAny(fConfig, "fConfig: ");
		return (!fConfig.IsNull());
	}
	String cfgFilename = GetConfigName();
	SystemLog::Info(
			String("AcceptorFactory::DoLoadConfig: no specific config entry for <") << fName << "> found in " << cfgFilename << ".any");
	fConfig = Anything();
	// because these object are hierarchical, it can be that some of them do not have their own config
	// so we must not fail here
	return true;
}

bool AcceptorFactory::DoGetConfigName(const char *category, const char *objName, String &configFileName) const {
	StartTrace1(AcceptorFactory.DoGetConfigName, "category: <" << NotNull(category) << "> object: <" << NotNull(objName) << ">");
	configFileName = "Config";
	Trace("returning <" << configFileName << "> true");
	return true;
}

RegCacheImpl(AcceptorFactory); // FindAcceptorFactory()

RegisterAcceptorFactory(AcceptorFactory);

ListenerThread::~ListenerThread() {
	StartTrace(ListenerThread.Dtor);
	//	Terminate();
	delete fAcceptor;
}

int ListenerThread::Init(ROAnything args) {
	StartTrace(ListenerThread.Init);

	fAcceptorName = args.AsCharPtr("AcceptorFactory");

	Trace("Acceptor: " << fAcceptorName);

	AcceptorFactory *acf = AcceptorFactory::FindAcceptorFactory(fAcceptorName);
	if (!acf) {
		String msg("AcceptorFactory: ");
		msg << fAcceptorName << " not found!";
		Trace(msg);
		SystemLog::Error(msg);
		return -1;
	}

	fAcceptor = acf->MakeAcceptor(fCallBack);
	if (!fAcceptor) {
		String logMsg;
		SystemLog::Error(logMsg << "no acceptor created");
		Trace(logMsg);
		return -1;
	}
	int retVal;
	if ((retVal = fAcceptor->PrepareAcceptLoop()) != 0) {
		String logMsg;
		SystemLog::Error(logMsg << "server (" << fAcceptorName << ")  prepare accept failed");
		Trace(logMsg << " with retVal " << (long)retVal );
		return retVal;
	}
	// start the accept loop
	String m;
	m << GetName() << " ready on ip: " << fAcceptor->GetAddress() << " port: " << fAcceptor->GetPort() << "\n";
	SystemLog::WriteToStderr(m);
	return 0;
}

//:this method gets called from the threads callback function
void ListenerThread::Run() {
	StartTrace(ListenerThread.Run);
	if (fAcceptor) {
		fAcceptor->RunAcceptLoop();
	}
}

bool ListenerThread::DoStartRequestedHook(ROAnything args) {
	return true;
}

void ListenerThread::DoTerminationRequestHook(ROAnything args) {
	StartTrace(ListenerThread.DoTerminationRequestHook);
	if (fAcceptor && fAcceptor->StopAcceptLoop()) {
		String m;
		m << "\tQuitting Server::Run loop <" << fAcceptorName << ">" << "\n";
		SystemLog::WriteToStderr(m);
	}
}

ListenerPool::ListenerPool(CallBackFactory *callBackFactory) :
	ThreadPoolManager("ListenerPool"), fCallBackFactory(callBackFactory) {
	StartTrace(ListenerPool.Ctor);
	Assert(fCallBackFactory);
}

ListenerPool::~ListenerPool() {
	StartTrace(ListenerPool.Dtor);
	Terminate(1, GetPoolSize() + 5);
	delete fCallBackFactory;
}

Thread *ListenerPool::DoAllocThread(long i, ROAnything args) {
	StartTrace(ListenerPool.DoAllocThread);
	TraceAny(args, "threads arguments");
	if (!fCallBackFactory) {
		return 0;
	}
	return new (Coast::Storage::Global()) ListenerThread(fCallBackFactory->MakeCallBack());
}
