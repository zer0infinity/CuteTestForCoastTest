/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TransitionDispatchAction_H
#define _TransitionDispatchAction_H

//---- Action include -------------------------------------------------
#include "config_actions.h"
#include "Action.h"

//---- TransitionDispatchAction ----------------------------------------------------------
//! <b>Lookups the transitionToken in context and executes the configuration as actionscript</b>
/*!
<b>Configuration: None</b><br>
This Action is useful if anyone wants to dispatch an action(script) based on the value of the transitionToken returned from a previously called Action(script).

Example:<pre>
/SomeActionScript {
	/LoginAction {
		/...
	}
	# assume that LoginAction changed transitionToken to <b>AuthenticateSuccess</b>
	/TransitionDispatchAction *
}
/AuthenticateSuccess {
	/NextActionToExecute {}
	/AndSoOn ...
}
/AuthenticateFailed {
	/WhateverWeMustDoHere {}
}
</pre>
In this case the action specification given in slot <b>AuthenticateSuccess</b> gets executed.
*/
class EXPORTDECL_ACTIONS TransitionDispatchAction : public Action
{
public:
	//! constructor
	TransitionDispatchAction(const char *name);
	//! destructor
	~TransitionDispatchAction();

	/*! DoSomething method for configured Actions
		\param transitionToken (in/out) the event passed by the caller, can be modified.
		\param ctx the context the action runs within.
		\param config the configuration of the action.
		\return true if the action run successfully, false if an error occurred. */
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
