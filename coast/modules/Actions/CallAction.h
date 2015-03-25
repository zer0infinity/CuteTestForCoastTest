/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CallAction_H
#define _CallAction_H

#include "Action.h"

//! Call a Action or Action script with parameters (lazy evaluation)
/*!
\par Configuration
\code
{
	/Action		Action name or script		mandatory, Action script to execute
	/Parameters {	Anything	optional, slots or parameters to pass to the called Action
		/Key		Value
		...
	}
}
\endcode
or
\code
{
	<Action name>		Name of Action to be rendered
	<arg 1>				Argument made available as $1
	<arg 2>				Argument made available as $2
	....
}
\endcode

The CallAction allows to reuse Action specifications with multiple calls
of a specification on a single page with passing "local" parameters at each call.
*/
class CallAction: public Action {
public:
	CallAction(const char *name) :
		Action(name) {
	}
protected:
	/*! Executes the specified Action, looked up in Context, with the given Parameters
	 * @copydetails Action::DoExecAction(String &, Context &, const ROAnything &) */
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
