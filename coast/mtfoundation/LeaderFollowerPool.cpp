/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "LeaderFollowerPool.h"
#include "StringStream.h"
#include "Socket.h"
#include "SystemLog.h"
#include "Dbg.h"
#include "SystemBase.h"
#include <iomanip>
#if !defined(WIN32)
#include <sys/poll.h>
#include <sys/select.h>
#else
#define USE_SELECT
#endif

//---- LeaderFollowerPool ----------------------------------------------------------------
const long LeaderFollowerPool::cNoCurrentLeader = -1;
const long LeaderFollowerPool::cBlockPromotion = -2;

LeaderFollowerPool::LeaderFollowerPool(Reactor *reactor)
	: ThreadPoolManager("LeaderFollowerPool")
	, fReactor(reactor)
	, fCurrentLeader(cNoCurrentLeader)
	, fOldLeader(cNoCurrentLeader)
	, fPoolState(Thread::eCreated)
	, fLFMutex("LeaderFollowerPool")
{
	StartTrace(LeaderFollowerPool.Ctor);
	Assert(fReactor);
}

LeaderFollowerPool::~LeaderFollowerPool()
{
	StartTrace(LeaderFollowerPool.Dtor);
}

int LeaderFollowerPool::Terminate(long lWaitToTerminate, long lWaitOnJoin)
{
	StartTrace(LeaderFollowerPool.Terminate);
	int result = DoTerminate(lWaitToTerminate);
	fTerminated = true;
	if ( result == 0 ) {
		LockUnlockEntry me(fLFMutex);
		fPoolState = Thread::eTerminated;
		// need to delete objects before unloading dlls
		if (fReactor) {
			Trace("deleting reactor");
			delete fReactor;
			fReactor = 0;
		}
		Trace("LFPool termination done");
	} else {
		SYSERROR("still " << result << " LeaderFollowerThreads in termination!");
	}
	return result;
}

void LeaderFollowerPool::RequestTermination()
{
	SYSINFO("requesting Termination");
	StartTrace(LeaderFollowerPool.RequestTermination);
	{
		LockUnlockEntry me(fLFMutex);
		fPoolState = Thread::eTerminationRequested;
		fCurrentLeader = cBlockPromotion;
		fFollowersCondition.BroadCast();
	}
	SYSINFO("Termination request done");
}

void LeaderFollowerPool::WaitForRequest(Thread *t, long timeout)
{
	StartTrace1(LeaderFollowerPool.WaitForRequest, "Timeout(" << timeout << ")");

	LockUnlockEntry me(fLFMutex);
	if ( fPoolState < Thread::eRunning ) {
		fPoolState = Thread::eRunning;
	}
	for (; fPoolState == Thread::eRunning;) {
		while ( fCurrentLeader != cNoCurrentLeader ) {
			Trace("Leader: " << fCurrentLeader << " sleeping...");
			if ( timeout > 0 ) {
				fFollowersCondition.TimedWait(fLFMutex, timeout);
			} else {
				fFollowersCondition.Wait(fLFMutex);
			}
			if ( ( fPoolState >= Thread::eTerminationRequested ) || !( t && ( t->CheckState( Thread::eRunning ) ) ) ) {
				Trace("No longer running, terminating");
				PromoteNewLeader();
				return;
			}
		}
		fCurrentLeader = (long)Thread::MyId();
		Trace("New Leader: " << fCurrentLeader << " processing events");
		fLFMutex.Unlock();
		t->SetWorking();
		fReactor->ProcessEvents(this, timeout * 1000);
		t->SetReady();
		fLFMutex.Lock();
	}
	PromoteNewLeader();
}

void LeaderFollowerPool::PromoteNewLeader()
{
	StartTrace(LeaderFollowerPool.PromoteNewLeader);
	{
		LockUnlockEntry me(fLFMutex);
		if (fCurrentLeader != (long)Thread::MyId() && fCurrentLeader != cBlockPromotion ) {
			String msg("inconsistent pool: ");
			msg << (long)Thread::MyId() << " is not leader(" << fCurrentLeader << ")";
			SYSERROR(msg);
			Trace(msg);
		}
		if ( fCurrentLeader != cBlockPromotion ) {
			fCurrentLeader = cNoCurrentLeader;
		} else {
			fOldLeader = cNoCurrentLeader;
		}
		fFollowersCondition.Signal();
	}
}

bool LeaderFollowerPool::Init(int maxParallelRequests, ROAnything args)
{
	StartTrace(LeaderFollowerPool.Init);

	if ( (maxParallelRequests > 0) && InitReactor(args) ) {
		return ThreadPoolManager::Init(maxParallelRequests, args);
	}
	return false;
}

bool LeaderFollowerPool::InitReactor(ROAnything args)
{
	StartTrace(LeaderFollowerPool.InitReactor);
	TraceAny(args, "Init arguments:");

	long argSz = args.GetSize();

	if ( argSz <= 0 ) {
		Trace("argSz:[" << argSz << "]");
		SYSERROR("no acceptors: argument <= 0");
		return false;
	}
	if ( !fReactor ) {
		Trace("Reactor not set");
		SYSERROR("Reactor not set");
		return false;
	}

	for (long i = 0; i < argSz; ++i) {
		Acceptor *acceptor = (Acceptor *)args[i].AsIFAObject(0);
		if ( acceptor ) {
			int retVal;
			if ( (retVal = acceptor->PrepareAcceptLoop()) != 0) {
				String logMsg;
				logMsg << "server (" << args.SlotName(i) << ")  prepare accept failed with retVal " << (long)retVal;
				SYSERROR(logMsg);
				Trace(logMsg);
				return false;
			}
			// start the accept loop
			OStringStream os;
			os << std::setw(20) <<  args.SlotName(i) << " Accepting requests from: " <<
			   acceptor->GetAddress() << " port: " << acceptor->GetPort() << " backlog: " <<
			   acceptor->GetBacklog() << std::endl;
			SystemLog::WriteToStderr(os.str());
			fReactor->RegisterHandle(acceptor);
		} else {
			return false;
		}
	}
	return true;
}

Thread *LeaderFollowerPool::DoAllocThread(long i, ROAnything args)
{
	StartTrace(LeaderFollowerThread.DoAllocThread);
	return new (Coast::Storage::Global()) LeaderFollowerThread(this);
}

//--- LeaderFollowerThread -----------------------------------------

LeaderFollowerThread::LeaderFollowerThread(LeaderFollowerPool *lfp, long timeout)
	: Thread("LeaderFollowerThread")
	, fPool(lfp)
	, fTimeout(timeout)
{
	StartTrace(LeaderFollowerThread.Ctor);
}

LeaderFollowerThread::~LeaderFollowerThread()
{
	StartTrace(LeaderFollowerThread.Dtor);
}

void LeaderFollowerThread::Run()
{
	StartTrace(LeaderFollowerThread.Run);
	if (fPool) {
		fPool->WaitForRequest(this, fTimeout);
	}
}

//--- Reactor ---
Reactor::Reactor()
{
	StartTrace(Reactor.Ctor);
}

Reactor::~Reactor()
{
	StartTrace(Reactor.Dtor);
}

void Reactor::ProcessEvents(LeaderFollowerPool *lfp, long timeout)
{
	StartTrace(Reactor.ProcessEvents);
	fHandleSet.HandleEvents(this, lfp, timeout);
}

void Reactor::ProcessEvent(Socket *socket, LeaderFollowerPool *lfp)
{
	StartTrace(Reactor.ProcessEvent);
	// We have to promote a new leader even if the socket we got from
	// the acceptor is invalid. Otherwise nobody sits at the phone to
	// wait for incoming calls :-)
	if ( lfp ) {
		lfp->PromoteNewLeader();
	}
	if (socket) {
		DoProcessEvent(socket);
		delete socket;
	}
}

void Reactor::RegisterHandle(Acceptor *acceptor)
{
	StartTrace(Reactor.RegisterHandle);
	if ( acceptor ) {
		fHandleSet.RegisterHandle(acceptor);
	}
}

//--- HandleSet --------------------------------------------------

HandleSet::HandleSet()
	: fMutex("HandleSet")
	, fLastAcceptorUsedIndex(0)
{
	StartTrace(HandleSet.Ctor);
}

HandleSet::~HandleSet()
{
	StartTrace1(HandleSet.Dtor, "fDemuxTableSz: [" << fDemuxTable.GetSize() << "]");
	TraceAny(fDemuxTable, "fDemuxTable: ");
	for (long i = fDemuxTable.GetSize() - 1; i >= 0; --i) {
		Acceptor *acceptor = (Acceptor *)fDemuxTable[i].AsIFAObject();
		delete acceptor;
	}
}

void HandleSet::HandleEvents(Reactor *reactor, LeaderFollowerPool *lfp, long timeout)
{
	StartTrace(HandleSet.HandleEvents);

	Acceptor *acceptor = WaitForEvents(timeout);
	if ( acceptor ) {
		reactor->ProcessEvent(acceptor->DoAccept(), lfp);
	} else {
		// timeout case
		lfp->PromoteNewLeader();
	}
}

Acceptor *HandleSet::WaitForEvents(long timeout)
{
	StartTrace1(HandleSet.WaitForEvents, "Timeout: " << timeout);
	const long NOFDS = fDemuxTable.GetSize();
#if defined(USE_SELECT)
	fd_set rfds;
	FD_ZERO(&rfds);
	long maxfd = 0;
#else
	pollfd fds[NOFDS];
#endif
	long i = 0L, sz = 0;
	for (i = 0; i < NOFDS; ++i) {
#if !defined(USE_SELECT)
		fds[i].events = POLLIN;
		fds[i].fd = -1;
#endif
		Acceptor *a = (Acceptor *)fDemuxTable[i].AsIFAObject(0);
		if (a) {
			int fd = a->GetFd();
			if (fd >= 0) {
#if defined(USE_SELECT)
				FD_SET(fd, &rfds);
				if (fd > maxfd) {
					maxfd = fd;
				}
#else
				fds[i].fd = fd;
#endif
			}
		}
	}
	if ( 0 == timeout ) {
		timeout = 200;
	}
#if defined(USE_SELECT)
	struct timeval tv;
	tv.tv_sec = timeout / 1000;
	tv.tv_usec = 1000 * (timeout % 1000);
	long retCode = select(maxfd + 1, &rfds, 0, 0, &tv);
#else
	long retCode = poll(fds, NOFDS, timeout);
#endif
	if (0 == retCode ) {
		return 0;    // timeout, no error
	}
	while ( retCode < 0 && Coast::System::SyscallWasInterrupted()) {
#if defined(USE_SELECT)
		retCode = select(maxfd + 1, &rfds, 0, 0, &tv);
#else
		retCode = poll(fds, NOFDS, timeout);
#endif
		SYSERROR("select/poll call interrupted. I do a restart. Socket error number: " <<
				 (long) SOCKET_ERROR << " return code " << retCode << " LastSyError: " <<  SystemLog::LastSysError());
	}
	if ( retCode > 0 ) {
		for (i = 0, sz = fDemuxTable.GetSize(); i < sz; ++i) {
			long idx = (fLastAcceptorUsedIndex + 1 + i) % NOFDS;
			Acceptor *a = (Acceptor *)fDemuxTable[idx].AsIFAObject(0);
			if (a) {
#if defined(USE_SELECT)
				int fd = a->GetFd();
				if (fd >= 0 && FD_ISSET(fd, &rfds))
#else
				Assert(a->GetFd() == fds[idx].fd);
				if (fds[idx].revents & POLLIN)
#endif
				{
					fLastAcceptorUsedIndex = idx;
					return a;
				}
			}
		}
	}
	SYSERROR("select/poll call with acceptors failed");
	SYSERROR("Socket error number: " << (long) SOCKET_ERROR << " return code " << retCode <<
			 " LastSyError: " <<  SystemLog::LastSysError());
	return 0;
}

void HandleSet::RegisterHandle(Acceptor *acceptor)
{
	StartTrace(HandleSet.RegisterHandle);
	LockUnlockEntry me(fMutex);
	if ( acceptor && acceptor->GetFd() > 0 ) {
		fDemuxTable.Append((IFAObject *)acceptor);
	}
}
