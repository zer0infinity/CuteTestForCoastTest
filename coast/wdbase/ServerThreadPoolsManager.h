/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ServerThreadPoolsManager_h_
#define _ServerThreadPoolsManager_h_

#include "config_wdbase.h"
#include "ServerPoolsManagerInterface.h"

class RequestThreadsManager;
class ListenerPool;

class EXPORTDECL_WDBASE ServerThreadPoolsManager: public ServerPoolsManagerInterface
{
public:
	ServerThreadPoolsManager(const char *ServerThreadPoolsManagerName);
	virtual ~ServerThreadPoolsManager();

	//!cloning interface
	IFAObject *Clone() const {
		return new ServerThreadPoolsManager(fName);
	}

	//! initialize the managed thread pools
	virtual int Init(Server *server);

	//! reinitialize the managed thread pools if possible
	virtual int ReInit(Server *server);

	//! run the the thread pools
	virtual int Run(Server *server);

	//!block the request handling in the managed pools
	virtual bool BlockRequests(Server *server);

	//!unblock the request handling in the managed pools
	virtual void UnblockRequests();

	//!terminate thread pool operation
	virtual int RequestTermination();

	//!terminate thread pool operation
	virtual void Terminate();

	//!access to configuration data
	virtual long GetThreadPoolSize();

protected:
	/*! allocates a new ListenerPool and initializes it
		\param server Server to use for the context and to retrieve params from
		\return true in case the pool init was successful */
	bool SetupSocket(Server *server);
	int SetupThreadPool(bool reinit, Server *server);

	//!pool of threads that handle incoming requests
	RequestThreadsManager *fActiveRequests;

	//!acceptor objects running the accept loops and calling back the RequestManager via callback object
	ListenerPool *fAcceptors;

private:
	// block the following default elements of this class
	// because they're not allowed to be used
	ServerThreadPoolsManager();
	ServerThreadPoolsManager(const ServerThreadPoolsManager &);
	ServerThreadPoolsManager &operator=(const ServerThreadPoolsManager &);
};

#endif
