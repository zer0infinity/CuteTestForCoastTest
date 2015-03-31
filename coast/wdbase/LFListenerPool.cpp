/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "LFListenerPool.h"
#include "StringStream.h"
#include "Timers.h"
#include "RequestProcessor.h"
#include "RequestListener.h"
#include "WPMStatHandler.h"
#include "Socket.h"
#include <iomanip>

LFListenerPool::LFListenerPool(RequestReactor *reactor)
	: LeaderFollowerPool(reactor)
{
	StartTrace(LFListenerPool.Ctor);
}

LFListenerPool::~LFListenerPool()
{
	StartTrace(LFListenerPool.Dtor);
	Terminate();
}

bool LFListenerPool::InitReactor(ROAnything args)
{
	StartTrace(LFListenerPool.InitReactor);
	if (fReactor && ((RequestReactor *)fReactor)->ValidInit()) {
		return LeaderFollowerPool::InitReactor(args);
	}
	return false;
}

bool LFListenerPool::Init(int maxParallelRequests, ROAnything args, bool useThreadLocalStorage)
{
	StartTrace(LFListenerPool.Init);
	Anything leaderFollowerConfig;
	TraceAny(args, "LFListenerPool config");
	for (long i = 0; i < args.GetSize(); ++i) {
		const char *acceptorName = args[i].AsCharPtr("AcceptorFactory");

		Trace("AcceptorFactory: " << acceptorName);

		AcceptorFactory *acf = AcceptorFactory::FindAcceptorFactory(acceptorName);
		if (!acf) {
			String msg("AcceptorFactory: ");
			msg << acceptorName << " not found!";
			Trace(msg);
			SYSERROR(msg);
			return false;
		}

		Acceptor *acceptor = acf->MakeAcceptor(0);
		if (!acceptor) {
			const char *logMsg = "no acceptor created";
			SYSERROR(logMsg);
			Trace(logMsg);
			return false;
		}
		acceptor->SetThreadLocal(useThreadLocalStorage);
		leaderFollowerConfig[acceptorName] = (IFAObject *)acceptor;
	}
	return LeaderFollowerPool::Init(maxParallelRequests, leaderFollowerConfig);
}

void LFListenerPool::BlockRequests()
{
	StartTrace(LFListenerPool.BlockRequests);
	LockUnlockEntry me(fLFMutex);
	fOldLeader = fCurrentLeader;
	fCurrentLeader = cBlockPromotion;
}

//:support reinitialization of server by unblocking request handling after the fact
void LFListenerPool::UnblockRequests()
{
	StartTrace(LFListenerPool.UnblockRequests);
	{
		LockUnlockEntry me(fLFMutex);
		fCurrentLeader = fOldLeader;
		fFollowersCondition.Signal();
	}
}

bool LFListenerPool::AwaitEmpty(long sec)
{
	return ((RequestReactor *)fReactor)->AwaitEmpty(sec);
}

RequestReactor::RequestReactor(RequestProcessor *rp, WPMStatHandler *stat)
	: Reactor()
	, fProcessor(rp)
	, fStatHandler(stat)
{
	StartTrace(RequestReactor.Ctor);
	Assert(ValidInit());
}

bool RequestReactor::ValidInit()
{
	StartTrace(RequestReactor.ValidInit);
	return (fProcessor && fStatHandler);
}

RequestReactor::~RequestReactor()
{
	StartTrace(RequestReactor.Dtor);
	// must be done before dll's get unloaded
	delete fProcessor;

	if (fStatHandler) {
		Anything statistic;
		fStatHandler->Statistic(statistic);
		String strbuf;
		StringStream stream(strbuf);
		statistic.PrintOn(stream) << "\n";
		stream.flush();
		SystemLog::WriteToStderr(strbuf);
		delete fStatHandler;
	}
}

void RequestReactor::DoGetStatistic(Anything &item)
{
	StartTrace(RequestReactor.Statistic);
	if (fStatHandler) {
		fStatHandler->Statistic(item);
	}
}

void RequestReactor::RegisterHandle(Acceptor *acceptor)
{
	StartTrace(RequestReactor.RegisterHandle);
	if ( acceptor ) {
		Reactor::RegisterHandle(acceptor);
	}
}

class StatEntry
{
public:
	StatEntry(WPMStatHandler *stat);
	~StatEntry();

protected:
	WPMStatHandler *fStat;
};

void RequestReactor::DoProcessEvent(Socket *sock)
{
	StartTrace(RequestReactor.DoProcessEvent);
	if ( fProcessor && sock ) {
		bool keepConnection;
		do {
			StatEntry se(fStatHandler);

			// **the** one and only context for this request
			Context ctx(sock);
			{
				// set the request timer to time
				// the duration of this request
				RequestTimer(Cycle, "Nr: " << fStatHandler->GetTotalRequests(), ctx);
				fProcessor->ProcessRequest(ctx);
			}
			// log all time related items
			RequestTimeLogger(ctx);

			if (!sock->HadTimeout()) { // short timeout, is some request still waiting...
				keepConnection = fProcessor->KeepConnectionAlive(ctx);
			} else {
				keepConnection = false;
				Trace("Close connection");
			}
		} while (keepConnection);
	}
}

bool RequestReactor::AwaitEmpty(long sec)
{
	StartTrace1(RequestReactor.AwaitEmpty, "sec:[" << sec << "]");
	if ( fStatHandler ) {
		// check for active requests running
		// but wait at most sec seconds
		long tstart = time(0);
		long tnow = tstart;
		int msgCount = 0;
		long parallelRequests = 0;
		while (((parallelRequests = fStatHandler->GetCurrentParallelRequests()) > 0) && 	// check for active requests
			   (tstart + sec > tnow)) {	// check for timeout
			Thread::Wait(1);				// wait for 1 second
			tnow = time(0);					// calculate new time
			++msgCount;						// message interval count
			if ((msgCount % 5) == 0) {      // tell us the story.
				OStringStream os;
				os << "MaxSecToWait: " << std::setw(4) << sec <<
				   "  SecsWaited: "	<< std::setw(4) << msgCount <<
				   "  Pending requests: " << std::setw(6) << parallelRequests << std::endl;
				SystemLog::WriteToStderr(os.str());
			}
		}
		Trace("CurrentParallelRequests:[" << parallelRequests << "]");
		return (parallelRequests <= 0);
	}
	Trace("StatHandler not set!!");
	return false;
}

StatEntry::StatEntry(WPMStatHandler *stat)
	: fStat(stat)
{
	if (fStat) {
		fStat->HandleStatEvt(WPMStatHandler::eEnter);
	}
}

StatEntry::~StatEntry()
{
	if (fStat) {
		fStat->HandleStatEvt(WPMStatHandler::eLeave);
	}
}
