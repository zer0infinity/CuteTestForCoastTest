/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ServerPoolsManagerInterface_h_
#define _ServerPoolsManagerInterface_h_

#include "Threads.h"
#include "WDModule.h"

class Server;
class RequestProcessor;

//---- ServerPoolsManagerInterfacesModule -----------------------------------------------------------
class ServerPoolsManagerInterfacesModule : public WDModule
{
public:
	ServerPoolsManagerInterfacesModule(const char *);
	virtual ~ServerPoolsManagerInterfacesModule();

	virtual bool Init(const ROAnything config);
	virtual bool Finis();
	virtual bool ResetFinis(const ROAnything config);
};

//---- ServerThreadPoolsManager -----------------------------------------------------------
class ServerPoolsManagerInterface: public ConfNamedObject
{
public:
	ServerPoolsManagerInterface(const char *ServerThreadPoolsManagerName);
	virtual ~ServerPoolsManagerInterface();

	//!registry interface
	RegCacheDef(ServerPoolsManagerInterface);	// FindServerThreadPoolsManager()

	RequestProcessor* GetRequestProcessor() {
		return DoGetRequestProcessor();
	}

	//! initialize the managed thread pools
	virtual int Init(Server *server) = 0;

	//! reinitialize the managed thread pools if possible
	virtual int ReInit(Server *server) = 0;

	//! run the the thread pools
	virtual int Run(Server *server) = 0;

	//!block the request handling in the managed pools
	virtual bool BlockRequests(Server *server) = 0;

	//!unblock the request handling in the managed pools
	virtual void UnblockRequests() = 0;

	//!terminate thread pool operation
	virtual int RequestTermination() = 0;

	//!terminate thread pool operation
	virtual void Terminate() = 0;

	//!access to configuration data
	virtual long GetThreadPoolSize() = 0;

	//!check if pool already started or terminated
	bool IsReady(bool ready, long timeout);

protected:
	//!set flag if pool ready or terminated
	void SetReady(bool ready);

	//!keep ready state
	bool fReady, fbInTermination;

private:
	virtual RequestProcessor* DoGetRequestProcessor() = 0;

	//!guard ready flag
	Mutex fMutex;
	//!synchronize ready changes
	Mutex::ConditionType fCond;
	long fCount;

	// block the following default elements of this class
	// because they're not allowed to be used
	ServerPoolsManagerInterface();
	ServerPoolsManagerInterface(const ServerPoolsManagerInterface &);
	ServerPoolsManagerInterface &operator=(const ServerPoolsManagerInterface &);
};

#define RegisterServerPoolsManagerInterface(name) RegisterObject(name, ServerPoolsManagerInterface)

#endif
