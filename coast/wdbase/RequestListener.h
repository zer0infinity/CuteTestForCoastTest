/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RequestListener_h_
#define _RequestListener_h_

#include "config_wdbase.h"
#include "ThreadPools.h"
#include "WDModule.h"
#include "Socket.h"

//---- AcceptorFactoriesModule -----------------------------------------------------------
class EXPORTDECL_WDBASE AcceptorFactoriesModule : public WDModule
{
public:
	AcceptorFactoriesModule(const char *);
	~AcceptorFactoriesModule();

	virtual bool Init(const ROAnything config);
	virtual bool Finis();
	virtual bool ResetInit(const ROAnything config);
	virtual bool ResetFinis(const ROAnything config);
};

//---- AcceptorFactory -----------------------------------------------------------
//!Factory that produces Acceptor; allows configuration of ListenerPool with different Acceptors
class EXPORTDECL_WDBASE AcceptorFactory : public HierarchConfNamed
{
public:
	//!standard named object constructor
	AcceptorFactory(const char *AcceptorFactoryName);
	//!does nothing
	~AcceptorFactory();

	//!registry interface
	RegCacheDef(AcceptorFactory);	// FindAcceptorFactory()
	//! Cloning interface
	IFAObject *Clone() const {
		return new AcceptorFactory(fName);
	}
	//!hook to provide factory functionality
	virtual Acceptor *MakeAcceptor(AcceptorCallBack *ac);

protected:
	//!special impelementation of configuration loading
	bool DoLoadConfig(const char *category);
	//!overriden impelementation of config file handling
	bool DoGetConfigName(const char *category, const char *objName, String &configFileName);

private:
	// block the following default elements of this class
	// because they're not allowed to be used
	AcceptorFactory();
	AcceptorFactory(const AcceptorFactory &);
	AcceptorFactory &operator=(const AcceptorFactory &);
};

#define RegisterAcceptorFactory(name) RegisterObject(name, AcceptorFactory)

//---- ListenerThread -----------------------------------------------------------
//!Thread that manages a passive connection end point using an Acceptor
class EXPORTDECL_WDBASE ListenerThread : public Thread
{
public:
	//!thread configured by a AcceptorCallback that defines the connection to parts processing a request which is sent through the accepted connection
	ListenerThread(AcceptorCallBack *ac);
	//!deletes the acceptor
	~ListenerThread();

	//!allocates callback and acceptor; deprecated will be replaced by DoStartRequestedHook
	//! \param args information about the server socket to create
	//! \return returns 0 if bind succeeds; otherwise -1
	virtual int Init(ROAnything args);

	//! start listening on acceptor
	virtual void Run();

protected:
	//!the name of this listening end point used for creating the correct Acceptor in Init
	String fAcceptorName;
	//!object that parametrizes the acceptor
	AcceptorCallBack *fCallBack;
	//!Acceptor created by the acceptor factory
	Acceptor *fAcceptor;

	//!allocates callback and acceptor
	//! \param args information about the server socket to create
	//! \return returns true if bind succeeds; otherwise false
	bool DoStartRequestedHook(ROAnything args);

	//!stops accept loop
	virtual void DoTerminationRequestHook(ROAnything args);
private:
	// block the following default elements of this class
	// because they're not allowed to be used
	ListenerThread();
	ListenerThread(const ListenerThread &);
	ListenerThread &operator=(const ListenerThread &);
};

//---- ListenerPool -----------------------------------------------------------
//! Thread pool that manages ListenerThreads
class EXPORTDECL_WDBASE ListenerPool: public ThreadPoolManager
{
public:
	//!constructor is configured by a CallBackFactory that allows to allocate ListenerThreads with correct CallBack objects
	ListenerPool(CallBackFactory *callBackFactory);
	//!Terminates the ListenerPool and deletes the CallBackFactory
	~ListenerPool();

protected:
	//!bottleneck routine; allocates ListenerThreads and configures them with the correct callback
	virtual Thread *DoAllocThread(ROAnything args);

	//!the factory object to create callback objects that are used to handle incoming connection requests
	CallBackFactory *fCallBackFactory;
};

#endif
