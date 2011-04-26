/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FlowControllerPerformAnalyzeAction_H
#define _FlowControllerPerformAnalyzeAction_H

#include "Action.h"

//! comment action
//!	Structure of config:
//!<PRE>	{
//!		/Slot1	....
//!      ...
//!	}</PRE>
class FlowControllerPerformAnalyzeAction: public Action {
public:
	FlowControllerPerformAnalyzeAction(const char *name) :
		Action(name) {
	}
	//!DoSomething method for configured Actions
	//! \param transitionToken (in/out) the event passed by the caller, can be modified.
	//! \param ctx the context the action runs within.
	//! \param config the configuration of the action.
	//! \return true if the action run successfully, false if an error occurred.
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
