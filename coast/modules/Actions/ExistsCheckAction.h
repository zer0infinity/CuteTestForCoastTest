/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ExistsCheckAction_H
#define _ExistsCheckAction_H

//---- Action include -------------------------------------------------
#include "Action.h"

//---- ExistsCheckAction ----------------------------------------------------------
//! <B>Checks whether a given Slot exists.</B>
/*!
\par Configuration
\code
{
	A render spec
}
\endcode
\parExample:
\code
{
	"LostAndFound.HandBags."
	{ /ContextLookupRenderer "Company.Employees.Fred.WifeName" }
	".Contents"
}
\endcode

Renders its config onto a string and checks whether the rendered path/slot
exists in context (e.g. the example checks, whether
"LostAndFound.Handbags.Cindy.Contents" exists, provided that
"Company.Employees.Fred.WifeName" is rendered as "Cindy").
The action returns true if the rendered slot exists, false otherwise.
*/
class ExistsCheckAction : public Action
{
public:
	//--- constructors
	/*! \param name defines the name of the action */
	ExistsCheckAction(const char *name);
	~ExistsCheckAction();

	/*! Checks whether the slot defined in config exists
		\param transitionToken (in/out) the event passed by the caller, can be modified.
		\param ctx the context the action runs within.
		\param config the configuration of the action (see general descr)
		\return true if the action run successfully, false if an error occurred.
	*/
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
