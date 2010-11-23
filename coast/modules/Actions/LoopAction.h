/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LoopAction_H
#define _LoopAction_H

//---- Action include -------------------------------------------------
#include "Action.h"

//---- LoopAction ----------------------------------------------------------
/*! <B>Loops from /Start to /End executing the given action for each cycle.</B><BR>Configuration:
<pre>
{
	/Start				Rendererspec	# mandatory, Converted to long - Start Index
	/End				Rendererspec	# mandatory, Converted to long - End Index
	/IndexSlot			Rendererspec	# mandatory, TempStore Slot where the loop index is stored
	/Action				Actionscript	# mandatory, Action that is executed during each loop
	/BreakOutOnFailure	Long			# optional, [0|1], default 0, if set to 1, break out of loop if executed action returns false
}</pre>
Storing the index in tmpStore at value of IndexSlot. If Start is greater than End, the loop counts downward */
class LoopAction : public Action
{
public:
	//--- constructors
	LoopAction(const char *name);
	~LoopAction();

protected:
	/*! Loops from start to end and executes /Action
		\param transitionToken (in/out) the event passed by the caller, can be modified.
		\param ctx the context the action runs within.
		\param config the configuration of the action.
		\return true if the action run successfully, false if an error occurred. */
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
