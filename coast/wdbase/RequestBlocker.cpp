/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- c-library modules used ---------------------------------------------------
#include <stdlib.h>

//#define TRACE_LOCKS

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "TraceLocks.h"
#include "Threads.h"
#include "SysLog.h"
#include "Context.h"
#include "Dbg.h"
#include "Registry.h"

//--- interface include --------------------------------------------------------
#include "RequestBlocker.h"

RequestBlocker *RequestBlocker::fgRequestBlocker = 0;
bool RequestBlocker::fBlocked = false;

RequestBlocker *RequestBlocker::RB()
{
	fgRequestBlocker = SafeCast(WDModule::FindWDModule("RequestBlocker"), RequestBlocker);
	return fgRequestBlocker;
}

RegisterModule(RequestBlocker);
//---- RequestBlocker ----------------------------------------------------------------
RequestBlocker::RequestBlocker(const char *name)
	: WDModule(name)
	, fRequestBlockerRWLock("RequestBlockerRWLock")
{
	StartTrace1(RequestBlocker.RequestBlocker, "Name:<" << NotNull(name) << ">");
	SysLog::Info("RequestBlocker: <unblocked>");
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
		RWLockEntry me(fRequestBlockerRWLock, false);
		me.Use();
		fBlocked = true;
	}
	SysLog::Info("RequestBlocker: <  blocked>");
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
		RWLockEntry me(fRequestBlockerRWLock, false);
		me.Use();
		fBlocked = false;
	}
	SysLog::Info("RequestBlocker: <unblocked>");
	return true;
}

bool RequestBlocker::IsBlocked()
{
	StartTrace(RequestBlocker.IsBlocked);
	TRACE_LOCK_START("IsBlocked");

	bool theState = false;
	{
		RWLockEntry me(fRequestBlockerRWLock, true);
		me.Use();
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
