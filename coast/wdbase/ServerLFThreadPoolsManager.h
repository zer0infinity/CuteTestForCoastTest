/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ServerLFThreadPoolsManager_h_
#define _ServerLFThreadPoolsManager_h_

#include "config_wdbase.h"
#include "ServerPoolsManagerInterface.h"

class LFListenerPool;
//---- ServerLFThreadPoolsManager -----------------------------------------------------------

class EXPORTDECL_WDBASE ServerLFThreadPoolsManager: public ServerPoolsManagerInterface
{
public:
	ServerLFThreadPoolsManager(const char *ServerThreadPoolsManagerName);
	virtual ~ServerLFThreadPoolsManager();

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) ServerLFThreadPoolsManager(fName);
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
	bool SetupLFPool(Server *server);

	//!leader follower
	LFListenerPool *fLFPool;
	long fThreadPoolSz;

private:
	virtual RequestProcessor* DoGetRequestProcessor();
	// block the following default elements of this class
	// because they're not allowed to be used
	ServerLFThreadPoolsManager();
	ServerLFThreadPoolsManager(const ServerLFThreadPoolsManager &);
	ServerLFThreadPoolsManager &operator=(const ServerLFThreadPoolsManager &);
};
#endif
