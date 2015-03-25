/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DummyHTTPDAImpl_H
#define _DummyHTTPDAImpl_H

#include "DummyDAImpl.h"

//! DummyDataAccess for performing HTTP Requests, uses meta data defined in HTTPMeta.any, difference to base class - uses connector params
class DummyHTTPDAImpl: public DummyDAImpl {
	DummyHTTPDAImpl();
	DummyHTTPDAImpl(const DummyHTTPDAImpl &);
	DummyHTTPDAImpl &operator=(const DummyHTTPDAImpl &);
public:
	DummyHTTPDAImpl(const char *name) :
		DummyDAImpl(name) {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) DummyHTTPDAImpl(fName);
	}
	//! executes the transaction
	//! \param c The context of the transaction
	//! \pre fData2Send contains the HTTP-Request
	//! \post fData2Get contains either an slot /MsgReceived containing the HTTP Response or the slot /Error storing information about the error occured.
	virtual bool Exec(Context &c, ParameterMapper *, ResultMapper *);
};

/* Don't add stuff after this #endif */
#endif		//not defined _DummyHTTPDAImpl_H
