/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _NotEmptyCheckAction_H
#define _NotEmptyCheckAction_H

//---- Action include -------------------------------------------------
#include "config_actions.h"
#include "Action.h"

//---- NotEmptyCheckAction ----------------------------------------------------------
//! <B>Checks if a field in the Query does contain something else than blanks.</B><BR>Configuration:
//! <PRE>
//! {
//!		/FieldName		{ Rendererspec }			# mandatory, field that is checked, if missing false is returned
//!		/SearchBase		{ Rendererspec }			# optional, default is query["FieldName"] but can be like "AnySlot.InAStore"
//! }
//! </PRE>
class EXPORTDECL_ACTIONS NotEmptyCheckAction : public Action
{
public:
	//--- constructors
	NotEmptyCheckAction(const char *name);
	~NotEmptyCheckAction();

	//!Checks if a field in the Query contains something else than blanks
	//! \param transitionToken (in/out) the event passed by the caller, can be modified.
	//! \param ctx the context the action runs within.
	//! \param config the configuration of the action.
	//! \return true if the specified field is not empty.
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);

protected:
	//!Checks if the String fieldToCheck contains something else than blanks
	//! \param fieldToCheck the String to be checked
	//! \param ctx the context the action runs within.
	//! \param config the configuration of the action.
	//! \return true if fieldToCheck is not empty.
	virtual bool DoCheck(String &fieldToCheck, Context &ctx, const ROAnything &config);
};

#endif
