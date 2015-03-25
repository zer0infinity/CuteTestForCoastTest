/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _PeriodicAction_H
#define _PeriodicAction_H

#include "Threads.h"

//!performs periodic execution of action associated with string fAction
//! this thread wakes up periodically and tries to cleanup sessions that have
//! timed out
class PeriodicAction : public Thread
{
public:
	//!construct thread with action which is executed periodically after waitTime seconds
	//! \param action the action name which is used to executed a periodic action
	//! \param waitTime the waitTime between two action executions
	PeriodicAction(const String &action, long waitTime);
	~PeriodicAction();

protected:
	//!implements periodic action loop
	void Run();

private:
	//! blocks the following default elements of this class because they're not allowed to be used
	PeriodicAction();
	//! blocks the following default elements of this class because they're not allowed to be used
	PeriodicAction(const PeriodicAction &);
	//! blocks the following default elements of this class because they're not allowed to be used
	PeriodicAction &operator=(const PeriodicAction &);

	//!action name that associates the function to be executed with the thread
	String fAction;
	//!the wait time between to invocation of the action
	long fWaitTime;

	friend class PeriodicActionTest;
};

#endif
