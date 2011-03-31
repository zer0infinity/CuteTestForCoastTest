/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DummyHTTPDAImpl_H
#define _DummyHTTPDAImpl_H

class Context;

#include "DummyDAImpl.h"

//! DummyDataAccess for performing HTTP Requests, uses meta data defined in HTTPMeta.any, difference to base class - uses connector params
class DummyHTTPDAImpl : public DummyDAImpl
{
public:
	DummyHTTPDAImpl(const char *name);
	~DummyHTTPDAImpl();

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const;

	//! executes the transaction
	//! \param c The context of the transaction
	//! \pre fData2Send contains the HTTP-Request
	//! \post fData2Get contains either an slot /MsgReceived containing the HTTP Response or the slot /Error storing information about the error occured.
	virtual bool Exec(Context &c, ParameterMapper *, ResultMapper *);

private:
	//constructor
	DummyHTTPDAImpl();
	DummyHTTPDAImpl(const DummyHTTPDAImpl &);
	//assignement
	DummyHTTPDAImpl &operator=(const DummyHTTPDAImpl &);
};

/* Don't add stuff after this #endif */
#endif		//not defined _DummyHTTPDAImpl_H
