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
//! <b>Stores a rendered Value into a slot in a store.</b>
/*! @section storeactionconfiguration StoreAction Configuration
 * @par \c Value
 * \b mandatory, Renderer specification\n
 * The value to be stored.
 * @par \c DoRender
 * \b optional, [0|1], default 1\n
 * Defines whether the value should be rendered before storing
 * @par \c Destination
 * See \ref storeputterconfiguration "StorePutter Configuration" to see options.
 */
class EXPORTDECL_ACTIONS StoreAction : public Action
{
public:
	//! constructors
	StoreAction(const char *name);
	~StoreAction();

	/*! Executes the action configuration to store value
	 * \param transitionToken (in/out) the event passed by the caller, can be modified.
	 * \param ctx the context the action runs within.
	 * \param config the configuration of the action.
	 * \return true if the action run successfully, false if an error occurred. */
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
