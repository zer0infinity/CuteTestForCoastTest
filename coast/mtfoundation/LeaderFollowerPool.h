/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LeaderFollowerPool_H
#define _LeaderFollowerPool_H

#include "config_mtfoundation.h"

//---- baseclass include -------------------------------------------------
#include "ThreadPools.h"

//---- forward declaration -----------------------------------------------
class Reactor;
class Acceptor;
class Socket;

//---- LeaderFollowerPool ----------------------------------------------------------
//!implements leader follower thread pool; description see POSA2 p.447 ff
class EXPORTDECL_MTFOUNDATION LeaderFollowerPool : public ThreadPoolManager
{
public:
	//--- constructors
	LeaderFollowerPool(Reactor *reactor);
	virtual ~LeaderFollowerPool();

	//!wait for a request and demultiplex the handling of it
	virtual void WaitForRequest(Thread *t, long timeout = 0);

	//!promote a follower thread to become the leader thread
	virtual void PromoteNewLeader();

	bool Init(int maxParallelRequests, ROAnything args);

	//! implementers should stop all pool threads
	virtual int Terminate(long lWaitToTerminate = 5, long lWaitOnJoin = 0);

	//! request for termination of pool
	virtual void RequestTermination();

	Reactor* GetReactor() {
		return fReactor;
	}
protected:
	virtual bool InitReactor(ROAnything args);

	//! create an array of the required workers
	virtual Thread *DoAllocThread(long i, ROAnything args);

	Reactor *fReactor;
	static const long cNoCurrentLeader;
	static const long cBlockPromotion;
	long fCurrentLeader;
	long fOldLeader;
	long fPoolState;

	Condition fFollowersCondition;
	Mutex fLFMutex;
};

//---- LeaderFollowerThread -----------------------------------------------------------
//!Thread that manages a passive connection end point using an Acceptor
class EXPORTDECL_MTFOUNDATION LeaderFollowerThread : public Thread
{
public:
	//!thread configured by a AcceptorCallback that defines the connection to parts processing a request which is sent through the accepted connection
	LeaderFollowerThread(LeaderFollowerPool *lfp, long timeout = 0);
	//!deletes the acceptor
	virtual ~LeaderFollowerThread();

	//! start leader follower cycles
	virtual void Run();

protected:
	LeaderFollowerPool *fPool;
	long fTimeout;

private:
	// block the following default elements of this class
	// because they're not allowed to be used
	LeaderFollowerThread();
	LeaderFollowerThread(const LeaderFollowerThread &);
	LeaderFollowerThread &operator=(const LeaderFollowerThread &);
};

struct pollfd;

//--- HandleSet -----------------------------------------------
//!manages a set of file descriptors as accept points
class  EXPORTDECL_MTFOUNDATION HandleSet
{
public:
	//! does nothing
	HandleSet();

	//!does nothing
	virtual ~HandleSet();

	//!process socket connections
	virtual void HandleEvents(Reactor *reactor, LeaderFollowerPool *lfp, long timeout);

	virtual void RegisterHandle(Acceptor *acceptor);
protected:
	friend class LeaderFollowerPoolTest;

	//!waits for connection requests using select
	Acceptor *WaitForEvents(long timeout);

	Anything fDemuxTable;
	Mutex 	fMutex;
	long	fLastAcceptorUsedIndex;// for handling fairness, index into fDemuxTable

private:
	//! do not use
	HandleSet(const HandleSet &);
	//! do not use
	HandleSet &operator=(const HandleSet &);
};

//---- Reactor ----------------------------------------------------------
//!reactor pattern; description see POSA2 p.179 ff
class EXPORTDECL_MTFOUNDATION Reactor
{
public:
	//!does nothing
	Reactor();

	virtual ~Reactor();

	//!process socket connections
	virtual void ProcessEvents(LeaderFollowerPool *lfp, long timeout);

	//!process a new connection
	virtual void ProcessEvent(Socket *socket, LeaderFollowerPool *lfp);

	//!register an acceptor in the HandleSet
	virtual void RegisterHandle(Acceptor *acceptor);

protected:
	virtual void DoProcessEvent(Socket *) = 0;

private:
	HandleSet fHandleSet;
};

#endif
