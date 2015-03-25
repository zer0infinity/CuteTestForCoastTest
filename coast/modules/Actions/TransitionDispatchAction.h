/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TransitionDispatchAction_H
#define _TransitionDispatchAction_H

#include "Action.h"

//---- TransitionDispatchAction ----------------------------------------------------------
//! Lookups the transitionToken in context and executes the configuration as actionscript
/*! @section TransitionDispatchActionDescription Description
 * This Action is useful if anyone wants to dispatch an action(script) based on the value of the transitionToken returned from a previously called Action(script).
 * @subsection TransitionDispatchActionConfiguration Configuration
 * There is nothing to configure for this Action.
 * @subsection TransitionDispatchActionExamples Examples
 * In the following example, the action specification given in slot \b AuthenticateSuccess gets executed as it is assumed to be the result of the LoginAction.
@code
/SomeActionScript {
	/LoginAction {
		/...
	}
	# assume that LoginAction changed transitionToken to AuthenticateSuccess
	/TransitionDispatchAction *
}
/AuthenticateSuccess {
	/NextActionToExecute {}
	/AndSoOn ...
}
/AuthenticateFailed {
	/WhateverWeMustDoHere {}
}
@endcode
*/
class TransitionDispatchAction : public Action
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	TransitionDispatchAction(const char *name) : Action(name) {}

	/*! Uses a Renderer script to decide what to execute next
	 * @copydetails Action::DoExecAction(String &, Context &, const ROAnything &)
	 * @return The result of the executed Actionscript if the renderer produces a String that is found in the Context
	 * @return true otherwise */
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
