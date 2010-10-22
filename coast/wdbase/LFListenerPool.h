/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LFListenerPool_h_
#define _LFListenerPool_h_

#include "config_wdbase.h"
#include "LeaderFollowerPool.h"

class Acceptor;
class AcceptorCallBack;
class RequestProcessor;
class RequestReactor;
class Socket;
class WPMStatHandler;

//---- LFListenerPool -----------------------------------------------------------
//! Leader Follower implementation for socket connection dispatching and request processing
class EXPORTDECL_WDBASE LFListenerPool : public LeaderFollowerPool
{
public:
	//!configure LeaderFollowerPool with request reactor
	LFListenerPool(RequestReactor *reactor);

	//!terminates lfpool anyway
	~LFListenerPool();
	/*! adapts ListenerPool configuration to new LF requirements by creating acceptors as arguments to ListenerPool::Init
		\param maxParallelRequests size of the pool
		\param args names of the acceptor factories to be used
		\param useThreadLocalStorage set to true if Acceptor threads should have their own memory
		\return returns true if bind succeeds */
	virtual bool Init(int maxParallelRequests, ROAnything args, bool useThreadLocalStorage);

	//!initialize reactor
	virtual bool InitReactor(ROAnything args);

	//!support reinitialization of server by blocking request handling
	virtual void BlockRequests();

	//!support reinitialization of server by unblocking request handling after the fact
	virtual void UnblockRequests();

	//!blocks the caller waiting for the running requests to terminate. It waits at most 'secs' seconds
	virtual bool AwaitEmpty(long secs);
};

//--- RequestReactor -------------
//!dispatcher of socket connection requests
class EXPORTDECL_WDBASE RequestReactor: public Reactor, public StatGatherer
{
public:
	//!takes Request processor and WPMStatHandler as strategies for request processing with regard to protocol (e.g. http) and statistics
	RequestReactor(RequestProcessor *rp, WPMStatHandler *stat);

	//!deletes processor and stat handler
	virtual ~RequestReactor();

	//! process one accepted socket connections
	virtual void DoProcessEvent(Socket *);

	//! implementation of AwaitEmpty needs fStatHandler since it is the only one knowing how many requests are being processed right now
	virtual bool AwaitEmpty(long sec);

	virtual bool ValidInit();

	//!register an acceptor in the HandleSet and set it to nonblocking
	virtual void RegisterHandle(Acceptor *acceptor);

	RequestProcessor* GetRequestProcessor() {
		return fProcessor;
	}

protected:
	//!get the statistics gathered so far in item
	void DoGetStatistic(Anything &item);

private:
	//! strategy for processing requests with regard to protocol (e.g. http)
	RequestProcessor *fProcessor;

	//! gathering statistics about request handling
	WPMStatHandler *fStatHandler;
};

#endif
