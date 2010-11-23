/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ADMINACTIONS_H
#define _ADMINACTIONS_H

#include "Action.h"

//---- ServerManagement ----------------------------------------------------------------
//@{ \deprecated
//! Executes a server command.
//@}
class ServerManagement : public Action
{
public:
	ServerManagement(const char *name);
	bool DoAction(String &action, Context &c);

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) ServerManagement(fName);
	}
};

#endif
