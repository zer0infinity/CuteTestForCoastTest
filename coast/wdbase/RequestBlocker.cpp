/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//#define TRACE_LOCKS
#include "RequestBlocker.h"
#include "TraceLocks.h"
#include "SystemLog.h"

RequestBlocker *RequestBlocker::fgRequestBlocker = 0;
bool RequestBlocker::fBlocked = false;

RequestBlocker *RequestBlocker::RB()
{
	fgRequestBlocker = SafeCast(WDModule::FindWDModule("RequestBlocker"), RequestBlocker);
	return fgRequestBlocker;
}

RegisterModule(RequestBlocker);
RequestBlocker::RequestBlocker(const char *name)
	: WDModule(name)
	, fRequestBlockerRWLock("RequestBlockerRWLock")
{
	StartTrace1(RequestBlocker.RequestBlocker, "Name:<" << NotNull(name) << ">");
	SystemLog::Info("RequestBlocker: <unblocked>");
}

RequestBlocker::~RequestBlocker()
{
	StartTrace(RequestBlocker.~RequestBlocker);
	Finis();
}

bool RequestBlocker::Block()
{
	StartTrace(RequestBlocker.Block);
	TRACE_LOCK_START("Block");
	if ( IsBlocked() == true) {
		return true;
	}
	{
		LockUnlockEntry me(fRequestBlockerRWLock, RWLock::eWriting);
		fBlocked = true;
	}
	SystemLog::Info("RequestBlocker: <  blocked>");
	return true;
}

bool RequestBlocker::UnBlock()
{
	StartTrace(RequestBlocker.UnBlock);

	TRACE_LOCK_START("UnBlock");

	if ( IsBlocked() == false) {
		return true;
	}
	{
		LockUnlockEntry me(fRequestBlockerRWLock, RWLock::eWriting);
		fBlocked = false;
	}
	SystemLog::Info("RequestBlocker: <unblocked>");
	return true;
}

bool RequestBlocker::IsBlocked()
{
	StartTrace(RequestBlocker.IsBlocked);
	TRACE_LOCK_START("IsBlocked");

	bool theState = false;
	{
		LockUnlockEntry me(fRequestBlockerRWLock, RWLock::eReading);
		theState = fBlocked;
	}
	return theState;
}

bool RequestBlocker::Init(const ROAnything config)
{
	StartTrace(RequestBlocker.Init);
	return ResetInit(config);
}

bool RequestBlocker::Finis()
{
	StartTrace(RequestBlocker.Finis);
	// might set state to blocked here?
	return true;
}

bool RequestBlocker::ResetFinis(const ROAnything )
{
	StartTrace(RequestBlocker.ResetFinis);
	return true;
}

bool RequestBlocker::ResetInit(const ROAnything config)
{
	StartTrace(RequestBlocker.ResetInit);
	return true;
}

void RequestBlocker::EnterReInit()
{
	StartTrace(RequestBlocker.EnterReInit);
}

void RequestBlocker::LeaveReInit()
{
	StartTrace(RequestBlocker.LeaveReInit);
}
