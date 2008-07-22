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
//! <B>Searches a pattern in a string (specified by a regular expression) and stores the matched string (if any) into the defined Destination.
/*!
\par Configuration
\code
{
	/Text 		(Rendererspec)	mandatory, the string to which a regular expression pattern will be applied to
	/Pattern	String			mandatory, the regular expression patter to search in string /Text
	/DoRender	bool			optional, default 1 (if /Text contains /Lookup... and not a String, then must render value)
	/Destination {		mandatory,
		/Store			String,			optional, default TempStore ["Role"|"TempStore"], the Store where the matched string is stored
		/Slot			Rendererspec,	mandatory, is the Slot name in Destination where the matched string is stored
	}
}
\endcode
\par Example
\code
\note Use a regular expression to extract the last 4 characters from string in slot /Text, then save result in  into Role.UserInfo.short_uid_db
{	/RegExpAction {
		/Text		"tkchgil"
		/Pattern    "[\\w]{4}$"
		/Destination {
			/Store  Role
			/Slot   UserInfo.short_uid_db
		}
	}
}
{	/RegExpAction {
		/Text		{ /Lookup UserInfo.uid }
		/Pattern    "[\\w]{4}$"
		/Destination {
			/Store  Role
			/Slot   UserInfo.short_uid_db
		}
	}
}
\endcode
*/

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
