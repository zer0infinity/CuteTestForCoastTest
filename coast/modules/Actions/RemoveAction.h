/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RemoveAction_H
#define _RemoveAction_H

//---- Action include -------------------------------------------------
#include "config_actions.h"
#include "Action.h"

//---- RemoveAction ----------------------------------------------------------
//! Removes slots from store
/*! @section RemoveActionConfiguration Configuration$
@code
{
	/Store	Role			# optional, default TempStore [Role|Session|Request|TempStore], The Store where the data are copied into
	/Slot	Rendererspec	# mandatory, producing the Slotname in the Store that is removed
	/Delim   				# optional, default ".", first char is taken as delimiter for named slots
	/IndexDelim				# optional, default ":", first char is taken as delimiter for indexed slots
}
@endcode
 */
class EXPORTDECL_ACTIONS RemoveAction : public Action
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	RemoveAction(const char *name) : Action(name) {}

	/*! Removes slots from Stores
	 * @copydetails Action::DoExecAction(String &, Context &, const ROAnything &)
	 * @return true if slot does not exist anymore
	 * @return false when Slot was not defined */
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
