/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StresserThread_h_
#define _StresserThread_h_

#include "Threads.h"

class Stresser;

//! Wraps a Stresser in a thread.
class StresserThread: public Thread {
	StresserThread(const StresserThread &);
	StresserThread &operator=(const StresserThread &);
public:
	StresserThread() :
		Thread("StresserThread"), fCond(0), fMutex(0), fMyId(-1), fResult(coast::storage::Global()) {
	}
	//! Initializing routine
	//! Typical clients want to construct vast arrays of StresserThreads
	//! so they have to use the default constructor. Then they should loop over
	//! the array and initialize each stresser.
	virtual void Init(const String &stresserName, long id, Condition *cond, Mutex *mtx, long *Counter);
	//! Runs the Stresser and signals the client upon completion.
	virtual void Run();

	//! passes the results from the Stresser to the client
	virtual Anything GetResult();

protected:
	virtual void DoTerminatedHook();
	String fStresserName;
	Condition *fCond;
	Mutex *fMutex;
	long *fPending;
	long fMyId;
	Anything fResult;
};

#endif
