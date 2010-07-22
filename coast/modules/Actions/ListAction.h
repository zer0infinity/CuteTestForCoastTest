/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ListAction_H
#define _ListAction_H

//---- Action include -------------------------------------------------
#include "config_actions.h"
#include "Action.h"

//---- ListAction ----------------------------------------------------------
//! <B>Action counterpart of the ListRenderer -> Loops over an Anything and calls /EntryAction for each slot.</B>
/*!
\par Configuration
\code
{
	/ListName			Rendererspec		mandatory, Resulting a String which is used to Lookup the List in the context
	/ListData {			Anything			optional but mandatory if ListName is omitted, inline List to use for executing the action, it is ignored if ListName is also specified and is not the empty list
		...
		...
	}
	/EntryStore			Rendererspec	 	optional, default EntryData, Resulting a String which is used to Store the entry data in the TempStore
	/IndexSlot			String				optional, if not defined the index is not added, denotes the slotname where the index of the actual entry is stored in the TempStore
	/SlotnameSlot		String				optional, if not defined the slotname is not added, denotes the slotname where the slotname of the actual entry is stored in the TempStore
	/EntryAction		Actionscript		mandatory, called for each entry
	/Start				Rendererspec		optional, default 0, list index (zero based) with which to start executing the action, useful when only a portion of the list should be used
	/End				Rendererspec		optional, default size of list, list index (zero based) with which to end executing the action, useful when only a portion of the list should be used
}
\endcode
*/
class EXPORTDECL_ACTIONS ListAction : public Action
{
public:
	//--- constructors
	ListAction(const char *name);
	~ListAction();

	/*! DoSomething method for configured Actions
		\param transitionToken (in/out) the event passed by the caller, can be modified.
		\param ctx the context the action runs within.
		\param config the configuration of the action.
		\return true if each call to /EntryAction returned true, false otherwise. */
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);

protected:
	/*! Gets called for each entry
		\param transitionToken (in/out) the event passed by the caller, can be modified.
		\param ctx the context the action runs within
		\param entryAction the configuration of the action to execute
		\return true if execution of EntryAction returned true, false otherwise. */
	virtual bool ExecEntry(String &transitionToken, Context &ctx, const ROAnything &entryAction);

	/*! gets the List Data from context
		\param ctx the context the action runs within
		\param config the configuration of the action
		\param roaList a reference to the list we work on
		\return true if the list could be found in the context */
	bool GetList(Context &ctx, const ROAnything &config, ROAnything &roaList);
};

#endif
