/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StressProcessor_H
#define _StressProcessor_H

//---- RequestProcessor include -------------------------------------------------
#include "RequestProcessor.h"

//---- StressProcessor ----------------------------------------------------------
//!Expects a Stresser name in the requests, runs it and replies the result
class StressProcessor : public RequestProcessor
{
public:
	//--- constructors
	StressProcessor(const char *name);
	virtual ~StressProcessor();

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) StressProcessor(fName);
	}

	//--- public api
	//!runs the Stresser given in args and streams the result back
	//! \param args the input should contain /StresserName otherwise the Default Stresser is run.
	//! \param reply the results gets streamed there
	virtual bool DoProcessRequest(std::ostream &reply, Context &ctx);

private:
	StressProcessor();
	// use careful, you inhibit subclass use
	//--- private class api
	//--- private friend
};

#endif
