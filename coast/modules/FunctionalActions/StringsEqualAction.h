/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StringsEqualAction_H
#define _StringsEqualAction_H

//---- Action include -------------------------------------------------
#include "config_FunctionalActions.h"
#include "Action.h"

class Context;

//---- StringsEqualAction ----------------------------------------------------------
//! <B>Compares if the two strings are equal</B>
/*!
\par Configuration
\code
{
	/String1	Rendererspec	mandatory
	/String2	Rendererspec	mandatory
}
\endcode

Result is True (1) or False (0)
*/
class EXPORTDECL_FUNCTIONALACTIONS StringsEqualAction : public Action
{
public:
	//--- constructors
	StringsEqualAction(const char *name);
	~StringsEqualAction();

	//:DoSomething method for configured Actions
	//!param: transitionToken - (in/out) the event passed by the caller, can be modified.
	//!param: ctx - the context the action runs within.
	//!param: config - the configuration of the action.
	//!retv: true if the action run successfully, false if an error occurred.
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
