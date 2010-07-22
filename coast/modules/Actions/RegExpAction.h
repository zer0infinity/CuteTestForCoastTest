/*
 * Copyright (c) 2008, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RegExpAction_H
#define _RegExpAction_H

//---- Action include -------------------------------------------------
#include "Action.h"

//---- RegExpAction ----------------------------------------------------------
//! <b>Search for regular expression in a string</b>
/*!
 * Searches a regular expression pattern in a string and stores the matched string or groups (if any) into the defined Destination.
 * When no group expression is present, the matched string will be stored at index 0 within the destination slot.
 * If groups are present, the whole match is stored at index 0 and each subsequent group will be stored at its index beginning at 1.
 * @section regexpactionconfig Action Configuration
 * @par \c Text
 * \b mandatory, the string to which the regular expression pattern will be applied, can also be a Renderer specification\n
 * To optimize for non-rendered slots, set \c DoRender flag to 0
 * @par \c Pattern
 * \b mandatory, the regular expression pattern to search within string of \c Text slot.
 * @par \c MatchFlags
 * \b optional, default 0 (RE::MATCH_NORMAL), check \ref RE::eMatchFlags for valid values
 * @par \c Destination
 * See \ref storeputterconfiguration "StorePutter configuration" to see options. \todo Use \b copydoc instead of \b ref
 *
 * @section regexpactionexample Example
 * @subsection regexpactionexampleconfig Configuration (ActionScript)
 * The following example uses a regular expression to extract the last 3 characters of a string in slot \c Text and saves the result in into slot UserInfo.short_uid_db of Role store.
 * @code
/RegExpAction {
	/Text		{ /Lookup UserInfo.uid }
	/Pattern    "\\w{3}$"
	/Destination {
		/Store  Role
		/Slot   UserInfo.short_uid_db
	}
}
 * @endcode */
class RegExpAction : public Action
{
public:
	//--- constructors
	/*! \param name defines the name of the action */
	RegExpAction(const char *name);
	~RegExpAction();

	/*! DoSomething method for configured Actions
		\param transitionToken (in/out) the event passed by the caller, can be modified.
		\param ctx the context the action runs within.
		\param config the configuration of the action.
		\return true if the action run successfully, false if an error occurred.
	*/
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
