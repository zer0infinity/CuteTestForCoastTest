/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SERVERUTILS_H
#define _SERVERUTILS_H

#include "config_wdbase.h"
#include "ThreadPools.h"

class Server;
class Socket;
class RequestProcessor;
class RequestThreadsManager;
class Anything;
class ROAnything;

//---- InterruptHandler ------------------------------------------------------------------
#if defined(WIN32)
// implementation for the WIN32 interrupthandler
// which is quite different from the solaris one

class EXPORTDECL_WDBASE WIN32InterruptHandler
{
public:
	WIN32InterruptHandler(Server *server);
	virtual ~WIN32InterruptHandler() { }
private:
	WIN32InterruptHandler();
	WIN32InterruptHandler &operator=(const WIN32InterruptHandler &);

	Server *fServer;
public:
	static void ShutDown();
	static Server *fgServer;
};

#else
// the solaris implementation of the InterruptHandler
// is based on the signal handling API's of the
// solaris thread library

class  EXPORTDECL_WDBASE InterruptHandler: public Thread
{
public:
	InterruptHandler(Server *server);
	~InterruptHandler();

	void Run();
private:
	Server *fServer;
};
#endif

//---- HandleRequest -----------------------------------------------
/*!handles request execution in its own thread; it handles at most one request at a time
wraps RequestProcessor with a thread of control; it implements the worker metapher
with the setup; work and ready cycle */
class EXPORTDECL_WDBASE HandleRequest : public WorkerThread
{
public:
	//! default constructor used for array allocation
	HandleRequest(const char *name = "HandleRequest");

protected:
	//!setup request processor as callback object
	virtual void DoInit(ROAnything workerInit);
	//!prepare for next working run; sets up socket
	virtual void DoWorkingHook(ROAnything workloadArgs);
	//!calls request processor to handle the request
	virtual void DoProcessWorkload();
	//!close the socket stream if any is activ
	virtual void DoTerminationRequestHook(ROAnything args);

private:
	// block the following default elements of this class
	// because they're not allowed to be used
	HandleRequest(const HandleRequest &);
	HandleRequest &operator=(const HandleRequest &);

protected:
	Socket *fClientSocket;	// the socket file descriptor for this request
	RequestProcessor *fProcessor;		// the processor i'm working with
	long fRequestNumber;	// the sequence number of this request
};

//---- RequestThreadsManager ------------------------------------------------
class EXPORTDECL_WDBASE RequestThreadsManager : public WorkerPoolManager
{
public:
	RequestThreadsManager(String name);
	~RequestThreadsManager();

protected:
	//!check if the presently pool may be re-initalized
	virtual bool CanReInitPool();

	//!allocate the handle request thread pool
	virtual void DoAllocPool(ROAnything args);

	//!cleanup hook for re-initialization of pool
	virtual void DoDeletePool(ROAnything args);

	virtual WorkerThread *DoGetWorker(long i);
private:
	// block the following default elements of this class
	// because they're not allowed to be used
	RequestThreadsManager(const RequestThreadsManager &);
	RequestThreadsManager &operator=(const RequestThreadsManager &);

protected:
	HandleRequest *fRequests;				// vector storing all request thread objects
	RequestProcessor *fProcessor;			// shared processor for this pool (I am the owner)
};

#endif		//ifndef _SERVERUTILS_H
