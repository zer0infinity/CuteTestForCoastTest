/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FlowControllerPrepareRequestAction_H
#define _FlowControllerPrepareRequestAction_H

//---- Action include -------------------------------------------------
#include "Action.h"

//---- FlowControllerPrepareRequestAction ----------------------------------------------------------
//! comment action
//!	Structure of config:
//!<PRE>	{
//!		/Slot1	....
//!      ...
//!	}</PRE>
class FlowControllerPrepareRequestAction : public Action
{
public:
	//--- constructors
	FlowControllerPrepareRequestAction(const char *name);
	~FlowControllerPrepareRequestAction();

	//!DoSomething method for configured Actions
	//! \param transitionToken (in/out) the event passed by the caller, can be modified.
	//! \param ctx the context the action runs within.
	//! \param config the configuration of the action.
	//! \return true if the action run successfully, false if an error occurred.
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);

};

#endif
