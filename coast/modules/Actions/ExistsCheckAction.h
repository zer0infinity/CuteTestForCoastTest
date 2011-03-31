/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ExistsCheckAction_H
#define _ExistsCheckAction_H

#include "Action.h"

//---- ExistsCheckAction ----------------------------------------------------------
//! Checks whether a given slot exists in the Context and return true. False otherwise.
/*!
 * @section ecac Action configuration
\code
{
	/LookupName		Rendererspec
	/Delim   		String
	/IndexDelim		String
}
\endcode
or just
\code
{
	"name" "delim" "indexdelim"
}
\endcode
 * @par \c LookupName or first index entry
 * Mandatory\n
 * Render the given specification into a String conforming to Anything::LookupPath syntax
 *
 * @par \c Delim
 * \b optional, default "."\n
 * First character is taken as delimiter for named slots
 *
 * @par \c IndexDelim
 * \b optional, default ":"\n
 * First character is taken as delimiter for indexed slots
 *
 * @par Example1:
 * @code
{
	{
		"LostAndFound.HandBags."
		{ /ContextLookupRenderer "Company.Employees.Fred.WifeName" }
		".Contents"
	}
}
 * @endcode
 * Renders its config into a string and checks whether the rendered path/slot exists in context (e.g. the example checks, whether
 * \em "LostAndFound.Handbags.Cindy.Contents" exists, provided that \em "Company.Employees.Fred.WifeName" is rendered as "Cindy").
 * The action returns true if the rendered slot exists, false otherwise.
 *
 * @par Example2:
 * @code
{
	/LookupName {
		"NeedToTestForASlot_ContainingADefault_Delimiter"
		".Contents"
	}
	/Delim "_"
}
 * @endcode
 * In this example, the LookupPath delimiter will be temporarily changed to \b _ so that \c ".Contents" will be treated a regular string without
 * having the dot being used as a path delimiter.
*/
class ExistsCheckAction: public Action {
public:
	/*! \param name defines the name of the action */
	ExistsCheckAction(const char *name) :
		Action(name) {
	}

protected:
	/*! Checks whether the slot defined in config exists
	 \param transitionToken (in/out) the event passed by the caller, can be modified.
	 \param ctx the context the action runs within.
	 \param config the configuration of the action (see general descr)
	 \return true if the action run successfully, false if an error occurred.
	 */
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
