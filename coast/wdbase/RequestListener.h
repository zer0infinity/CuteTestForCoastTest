/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RequestListener_h_
#define _RequestListener_h_

#include "WDModule.h"
#include "ThreadPools.h"

class AcceptorFactoriesModule: public WDModule {
public:
	AcceptorFactoriesModule(const char *);
	virtual bool Init(const ROAnything config);
	virtual bool Finis();
	virtual bool ResetInit(const ROAnything config);
	virtual bool ResetFinis(const ROAnything config);
};

class AcceptorCallBack;
class Acceptor;

//!Factory that produces Acceptor; allows configuration of ListenerPool with different Acceptors
class AcceptorFactory: public HierarchConfNamed {
	AcceptorFactory();
	AcceptorFactory(const AcceptorFactory &);
	AcceptorFactory &operator=(const AcceptorFactory &);
public:
	//!standard named object constructor
	AcceptorFactory(const char *AcceptorFactoryName) :
		HierarchConfNamed(AcceptorFactoryName) {
	}
	//!registry interface
	RegCacheDef(AcceptorFactory); // FindAcceptorFactory()
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) AcceptorFactory(fName);
	}
	//!hook to provide factory functionality
	virtual Acceptor *MakeAcceptor(AcceptorCallBack *ac);

protected:
	//!special impelementation of configuration loading
	bool DoLoadConfig(const char *category);
	//!overridden implementation of config file handling
	bool DoGetConfigName(const char *category, const char *objName, String &configFileName) const;
};

#define RegisterAcceptorFactory(name) RegisterObject(name, AcceptorFactory)

//!Thread that manages a passive connection end point using an Acceptor
class ListenerThread: public Thread {
	ListenerThread();
	ListenerThread(const ListenerThread &);
	ListenerThread &operator=(const ListenerThread &);
	//!the name of this listening end point used for creating the correct Acceptor in Init
	String fAcceptorName;
	//!object that parametrizes the acceptor
	AcceptorCallBack *fCallBack;
	//!Acceptor created by the acceptor factory
	Acceptor *fAcceptor;
public:
	//!thread configured by a AcceptorCallback that defines the connection to parts processing a request which is sent through the accepted connection
	ListenerThread(AcceptorCallBack *ac) :
		Thread("ListenerThread"), fCallBack(ac), fAcceptor(0) {
	}
	//!deletes the acceptor
	virtual ~ListenerThread();

	//!allocates callback and acceptor; deprecated will be replaced by DoStartRequestedHook
	//! \param args information about the server socket to create
	//! \return returns 0 if bind succeeds; otherwise -1
	virtual int Init(ROAnything args);

	//! start listening on acceptor
	virtual void Run();

protected:
	//!allocates callback and acceptor
	//! \param args information about the server socket to create
	//! \return returns true if bind succeeds; otherwise false
	bool DoStartRequestedHook(ROAnything args);

	//!stops accept loop
	virtual void DoTerminationRequestHook(ROAnything args);
};

class CallBackFactory;
//! Thread pool that manages ListenerThreads
class ListenerPool: public ThreadPoolManager {
	//!the factory object to create callback objects that are used to handle incoming connection requests
	CallBackFactory *fCallBackFactory;
public:
	//!constructor is configured by a CallBackFactory that allows to allocate ListenerThreads with correct CallBack objects
	ListenerPool(CallBackFactory *callBackFactory);
	//!Terminates the ListenerPool and deletes the CallBackFactory
	virtual ~ListenerPool();

protected:
	//!bottleneck routine; allocates ListenerThreads and configures them with the correct callback
	virtual Thread *DoAllocThread(long i, ROAnything args);
};

#endif
