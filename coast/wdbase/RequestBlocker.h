/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RequestBlocker_H
#define _RequestBlocker_H

//---- baseclass include -------------------------------------------------
#include "WDModule.h"
#include "Threads.h"

//---- forward declaration -----------------------------------------------
class RWLock;

//---- RequestBlocker ----------------------------------------------------------
//!Maintains a state
//! this is the one and only manager of the servers state, where state may be
//! requests should be blocked by implementors of request processing or
//! requests should be processed.
//! RequestBlocker is used by the server.

class RequestBlocker: public WDModule
{
public:
	//!it exists only one since it is a not cloned
	RequestBlocker(const char *name);
	//!does nothing since everything should be done in Finis
	virtual ~RequestBlocker();

	static RequestBlocker *RB();

	//--- module initialization termination ---
	//!initialize
	virtual bool Init(const ROAnything config);
	//!finalize
	virtual bool Finis();
	//!terminate RequestBlocker for reset
	virtual bool ResetFinis(const ROAnything );
	//!reinitializes
	virtual bool ResetInit(const ROAnything config);

	//!support reinit
	virtual void EnterReInit();
	//!support reinit
	virtual void LeaveReInit();

	//!These methods manipulate the internal state, so we need a Write lock
	//!:Set internal state to block requests
	bool Block();
	//!Set internal state to unblock requests
	bool UnBlock();

	//!That method queries the internal state, so we need only the Read lock
	bool IsBlocked();

protected:
	//--- subclass api
	//!singleton cache
	static RequestBlocker *fgRequestBlocker;

	//!The rw lock that protects the state RB holds
	RWLock fRequestBlockerRWLock;

	//!The state we protect
	static bool fBlocked;

private:
	// use careful, you inhibit subclass use
};

#endif

