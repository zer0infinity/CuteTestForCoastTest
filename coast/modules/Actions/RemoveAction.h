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
//! <B>Removes slots from Stores.</B><BR>Configuration:
//! <PRE>
//! {
//!		/Store	Role			# optional, default TempStore [Role|Session|Request|TempStore], The Store where the data are copied into
//!		/Slot	Rendererspec	# mandatory, producing the Slotname in the Store that is removed
//!		/Delim   				# optional, default ".", first char is taken as delimiter for named slots
//!		/IndexDelim				# optional, default ":", first char is taken as delimiter for indexed slots
//! }
//! </PRE>
class EXPORTDECL_ACTIONS RemoveAction : public Action
{
public:
	//--- constructors
	RemoveAction(const char *name);
	~RemoveAction();

	//!Removes slots from Stores
	//! \param transitionToken (in/out) the event passed by the caller, can be modified.
	//! \param ctx the context the action runs within.
	//! \param config the configuration of the action.
	//! \return true if slot does not exist anymore, false when Slot was not defined.
	//! config looks like this
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
