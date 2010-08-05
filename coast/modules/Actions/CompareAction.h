/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CompareAction_H
#define _CompareAction_H

//---- Action include -------------------------------------------------
#include "Action.h"

//---- CompareAction ----------------------------------------------------------
//! Compares two literal (rendered) values or two locations.
/*!
\par Configuration
\code
{
	/RenderVal1		mandatory	RendererSpec
	/RenderVal2		mandatory	RendererSpec

	/LookupPath1	mandatory	RendererSpec
	/LookupPath2	mandatory	RendererSpec

	/CompareAs		optional	one of "Long", "String", "Bool", "Double" or "Anything" (defaults to String)
}
\endcode

CompareAction will first check if "RenderVal"-Slots are defined. If this is
the case, the two values are rendered and compared as strings. If the
"RenderVal"-Slots are not defined, the action checks for the "LookupPath"-Slots.
If they are present, their values are rendered and then looked up. The
results are then compared as defined (by default as string).
*/
class CompareAction : public Action
{
public:
	//--- constructors
	/*! \param name defines the name of the action */
	CompareAction(const char *name);
	~CompareAction();

	/*! Compare two values, according to config
		\param transitionToken (in/out) the event passed by the caller, can be modified.
		\param ctx the context the action runs within.
		\param config the configuration of the action (see general descr).
		\return true if the action run successfully, false if an error occurred.
	*/
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
