/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ConditionalAction_H
#define _ConditionalAction_H

//---- Action include -------------------------------------------------
#include "config_actions.h"
#include "Action.h"

//---- ConditionalAction ----------------------------------------------------------
//! <B>Dispatches on success of a call to another Action</B><BR>Configuration:
//! <PRE>
//! {
//!		/Call 	Actionscript	mandatory, is called and the result is used to decide
//!		/True	Actionscript	optional, is called if the /Call Action results true
//!								returns the result of the Actionscript or true if not specified
//!		/False	Actionscript	optional, is called if the /Call Action results false
//!								returns the result of the Actionscript or false if not specified
//! }
//! </PRE>
class EXPORTDECL_ACTIONS ConditionalAction : public Action
{
public:
	//--- constructors
	ConditionalAction(const char *name);
	~ConditionalAction();

	//! \param transitionToken (in/out) the event passed by the caller, can be modified.
	//! \param ctx the context the action runs within.
	//! \param config the configuration of the action.
	//! \return true or false, depending on the success of the configured actions.
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
