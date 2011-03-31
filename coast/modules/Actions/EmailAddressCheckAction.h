/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _EmailAddressCheckAction_H
#define _EmailAddressCheckAction_H

#include "NotEmptyCheckAction.h"

//---- EmailAddressCheckAction ----------------------------------------------------------
//! <B>Primitiv check for valid email address.</B><BR>Configuration:
//! <PRE>
//! {
//!		/FieldName		{ Rendererspec }			# mandatory, field that is checked, if missing false is returned
//!		/SearchBase		{ Rendererspec }			# optional, default is query["FieldName"] but can be like "AnySlot.InAStore"
//! }
//! </PRE>
//! Checks if the field contains a least @ and at least one . afterwards and no but trailing blanks
//! If further enhancements are needed, think of a RegExp checking action
class EmailAddressCheckAction : public NotEmptyCheckAction
{
public:
	//--- constructors
	EmailAddressCheckAction(const char *name);
	~EmailAddressCheckAction();

protected:
	//!Checks if the String fieldToCheck contains a formal valid email address
	//! \param fieldToCheck the String to be checked
	//! \param ctx the context the action runs within.
	//! \param config the configuration of the action.
	//! \return true if fieldToCheck contains a formal valid email address
	virtual bool DoCheck(String &fieldToCheck, Context &ctx, const ROAnything &config);
};

#endif
