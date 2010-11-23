/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CreateAnythingFromStringAction_H
#define _CreateAnythingFromStringAction_H

//---- Action include -------------------------------------------------
#include "Action.h"

//---- CreateAnythingFromStringAction ----------------------------------------------------------
//! <B>Stores a rendered Value into a slot in a store</B><BR>Configuration:
//! <PRE>
//! {
//!		/Destination {
//!			/Store		String			optional, default Role ["Role"|"TempStore"], the Store where the data are stored in
//!			/Slot		Rendererspec	mandatory, producing the Slotname in Destination where the data is stored to
//!			/Delim	 	character		optional, default ".", first char is taken as delimiter for named slots
//!			/IndexDelim	character		optional, default ":", first char is taken as delimiter for indexed slots
//!		}
//!		/String			Rendererspec	mandatory, the string to be converted  into an anything
//!	}</PRE>
/*!
*/
class CreateAnythingFromStringAction : public Action
{
public:
	//--- constructors
	CreateAnythingFromStringAction(const char *name);
	~CreateAnythingFromStringAction();

	//:DoSomething method for configured Actions
	//!param: transitionToken - (in/out) the event passed by the caller, can be modified.
	//!param: ctx - the context the action runs within.
	//!param: config - the configuration of the action.
	//!retv: true if the action run successfully, false if an error occurred.
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);

};

#endif
