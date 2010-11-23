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
#include "Action.h"

//---- ConditionalAction ----------------------------------------------------------
//! <b>Dispatches on success of a call to another Action</c>
/*! @section ConditionalActionDescription Description
 * The purpose of this Action is to catch the return code of the \c Call Action and
 * dispatch another Action script based on this return code.
 * If there is no configuration for either \c True or \c False slots, it will return true, otherwise it will
 * return the code of the executed Action script.
 * The former might be useful in an Action script when one of its Actions fails from time to time but should
 * not terminate the script.
 * @subsection ConditionalActionConfiguration Configuration
 * @code
{
	/Call 	Actionscript	mandatory, is called and the result is used to decide
	/True	Actionscript	optional, is called if the /Call Action results true
							returns the result of the Actionscript or true if not specified
	/False	Actionscript	optional, is called if the /Call Action results false
							returns the result of the Actionscript or true if not specified
}
 * @endcode
*/
class ConditionalAction : public Action
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	ConditionalAction(const char *name) : Action(name) {}

	/*! Executes the contents of Call slot as Action script and dispatches contents of either \c True or \c False
	 * @copydetails Action::DoExecAction(String &, Context &, const ROAnything &)
	 * @return The result of the executed Actionscript if the renderer produces a String that is found in the Context
	 * @return true otherwise */
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
