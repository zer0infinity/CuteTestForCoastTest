/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CallDataAccessAction_H
#define _CallDataAccessAction_H

//---- Action include -------------------------------------------------
#include "Action.h"

//---- CallDataAccessAction ----------------------------------------------------------
//! Action to perform a DataAccess
//! Preferred (by rum) Alias : CallDA <BR>
//! Configuration has one slot
//! <PRE> { /CallDA {
//!		/DataAccess    		NameOfTheDataAccessToPerform (Renderer spec)
//!		/Parameters {		# Anything that gets pushed on the context
//!			...				# the Dataaccess may access the parameters with context Lookups
//!		}
//! } }</PRE>
//! or maybe one might prefer the short cut form that consists only in the
//! NameOfTheDataAccessToPerform
//! e.g. { /CallDA NameOfTheDataAccessToPerform }
class CallDataAccessAction : public Action
{
public:
	//--- constructors
	CallDataAccessAction(const char *name);
	~CallDataAccessAction();

	//!Calls the DataAccess
	//! \param transitionToken (in/out) the event passed by the caller, can be modified.
	//! \param c the context the action runs within.
	//! \param config the configuration of the action.
	//! \return true if the DataAccess run successfully, false if an error occurred.
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);

};

#endif
