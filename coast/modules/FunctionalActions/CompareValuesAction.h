/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CompareValuesAction_H
#define _CompareValuesAction_H

//---- Action include -------------------------------------------------
#include "config_FunctionalActions.h"
#include "Action.h"

class String;
class ROAnything;
class Context;

//---- CompareValuesAction ----------------------------------------------------------
//: comment action
//	Structure of config:
//<PRE>	{
//		/Slot1	....
//      ...
//	}</PRE>
class EXPORTDECL_FUNCTIONALACTIONS CompareValuesAction : public Action
{
public:
	//--- constructors
	CompareValuesAction(const char *name);
	~CompareValuesAction();

	//:DoSomething method for configured Actions
	//!param: transitionToken - (in/out) the event passed by the caller, can be modified.
	//!param: ctx - the context the action runs within.
	//!param: config - the configuration of the action.
	//!retv: true if the action run successfully, false if an error occurred.
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
