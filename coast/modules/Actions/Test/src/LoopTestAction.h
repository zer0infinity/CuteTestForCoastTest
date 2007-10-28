/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LoopTestAction_H
#define _LoopTestAction_H

//---- Action include -------------------------------------------------
#include "Action.h"

class ROAnything;
class Context;
class String;

//---- LoopTestAction ----------------------------------------------------------
//! just a helper Action to perform Loop- and ListAction Tests
class LoopTestAction : public Action
{
public:
	//--- constructors
	LoopTestAction(const char *name);
	~LoopTestAction();

	//!DoSomething method for configured Actions
	//! \param transitionToken (in/out) the event passed by the caller, can be modified.
	//! \param ctx the context the action runs within.
	//! \param config the configuration of the action.
	//! \return true if the action run successfully, false if an error occurred.
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

//! tests when loop fails
class FailLoopTestAction : public Action
{
public:
	//--- constructors
	FailLoopTestAction(const char *name);
	~FailLoopTestAction();

	//!DoSomething method for configured Actions
	//! \param transitionToken (in/out) the event passed by the caller, can be modified.
	//! \param ctx the context the action runs within.
	//! \param config the configuration of the action.
	//! \return true if the action run successfully, false if an error occurred.
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
