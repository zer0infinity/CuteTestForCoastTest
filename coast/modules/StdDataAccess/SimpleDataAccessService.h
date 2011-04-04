/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SimpleDataAccessService_h_
#define _SimpleDataAccessService_h_

#include "ServiceHandler.h"

//---- SimpleDataAccessService ----------------------------------------------------------
//!single line description of the class
//! further explanation of the purpose of the class
//! this may contain <B>HTML-Tags</B>
//! ...
class SimpleDataAccessService : public ServiceHandler
{
public:
	//!standard named object constructor
	SimpleDataAccessService(const char *name) :
		ServiceHandler(name) {
	}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) SimpleDataAccessService(fName);
	}

protected:
	//!handles requested service
	virtual bool DoHandleService(std::ostream &os, Context &ctx);

private:
	// block the following default elements of this class
	// because they're not allowed to be used
	SimpleDataAccessService();
	SimpleDataAccessService(const SimpleDataAccessService &);
	SimpleDataAccessService &operator=(const SimpleDataAccessService &);
	// use careful, you inhibit subclass use
	//--- private class api
	//--- private member variables
};

#endif
