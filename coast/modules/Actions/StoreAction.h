/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StoreAction_H
#define _StoreAction_H

//---- Action include -------------------------------------------------
#include "config_actions.h"
#include "Action.h"

//---- StoreAction ----------------------------------------------------------
//! <B>Stores a rendered Value into a slot in a store.</B><BR>Configuration:
//! <PRE>
//! {
//!		/Destination {
//!			/Store						optional, default TempStore [Role|Session|Request|TempStore], Store where the data will be stored in
//!			/Slot		Rendererspec	mandatory, producing the Slotname in Destination where the data are stored in
//!			/Delim   					optional, default ".", first char is taken as delimiter for named slots
//!			/IndexDelim					optional, default ":", first char is taken as delimiter for indexed slots
//!		}
//!		/Value			(Rendererspec)	the value to be stored
//!		/DoRender		bool			optional, default 1: defines whether the value should be rendered before storing
//! }
//! </PRE>
class EXPORTDECL_ACTIONS StoreAction : public Action
{
public:
	//--- constructors
	StoreAction(const char *name);
	~StoreAction();

	//! Executes the action
	//! \param transitionToken (in/out) the event passed by the caller, can be modified.
	//! \param ctx the context the action runs within.
	//! \param config the configuration of the action.
	//! \return true if the action run successfully, false if an error occurred.
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
