/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TESTACTION_H
#define _TESTACTION_H

#include "Action.h"

//---- TestAction ----------------------------------------------------------
//!Action for testing purposes.
//! The only thing this action does is: it creates a slot with the actions name
//! in the roleStore and stores 'OK' in it.
class TestAction : public Action
{
public:
	//--- constructors
	TestAction(const char *name);
	~TestAction();

	//--- public api
	//! changes the actionToken and returns true or false according to name
	//! this Action is only usefull for testing purposes
	//! \param name name of the transition
	//! \param context a context (unused)
	//! \return true or false according to name
	virtual bool DoAction(String &name, Context &context);
};

class ConfiguredTestAction: public TestAction
{
public:
	//--- constructors
	ConfiguredTestAction(const char *name) : TestAction(name) {}
	~ConfiguredTestAction() {}

	//! changes the actionToken and returns true or false according to config
	//! this Action is only usefull for testing purposes
	//! \param name name of the transition,
	//! \param context a context (unused)
	//! \param config Configuration for Action performing
	//! \return true or false according to config
	//!config : <PRE> {
	//!		/ActionToken	"NewValue"	# String - unchanged if not defined
	//!		/RetValue		0			# Bool
	//!	}</PRE>
	virtual bool DoExecAction(String &name, Context &context, const ROAnything &config);
};

#endif
