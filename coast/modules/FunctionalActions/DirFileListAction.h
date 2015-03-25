/*
 * Copyright (c) 2008, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DirFileListAction_H
#define _DirFileListAction_H

#include "Action.h"

//---- DirFileListAction ----------------------------------------------------------
//! Retrieve directory entries at given Path and put results in context
/*!
\par Configuration
\code
{
	/Path		Rendererspec	mandatory, path to list files of
	/Filter		String			optional, used to filter the files listed
	/DirsOnly 	Long			optional, [*0|1], 0 (default): files and directories are listed, 1: only directories
	/Target 	Rendererspec	optional, default "RetrievedFileList", puts the list into the renderered slotname in context

}
\endcode

*/
class DirFileListAction : public Action
{
public:
	//--- constructors
	/*! \param name defines the name of the action */
	DirFileListAction(const char *name);
	~DirFileListAction();

	/*! DoSomething method for configured Actions
		\param transitionToken (in/out) the event passed by the caller, can be modified.
		\param ctx the context the action runs within.
		\param config the configuration of the action.
		\return true if the action run successfully, false if an error occurred.
	*/
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
