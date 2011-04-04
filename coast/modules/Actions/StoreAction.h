/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StoreAction_H
#define _StoreAction_H

#include "Action.h"

//! Stores a rendered Value into a slot in a store.
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
class StoreAction: public Action {
public:
	StoreAction(const char *name) :
		Action(name) {
	}
protected:
	/*! copy a optionally rendered value into a slot in a store
	 * @copydetails Action::DoExecAction(String &, Context &, const ROAnything &)
	 * @return true if slot does not exist anymore
	 * @return false when Slot was not defined */
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
