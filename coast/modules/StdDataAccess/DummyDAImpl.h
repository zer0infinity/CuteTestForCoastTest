/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DummyDAImpl_H
#define _DummyDAImpl_H

class String;
class Context;

#include "DataAccessImpl.h"

//! DataAccess for performing HTTP Requests, uses meta data defined in HTTPMeta.any
class DummyDAImpl: public DataAccessImpl
{
public:
	DummyDAImpl(const char *name);
	~DummyDAImpl();

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const;

	//! executes the transaction
	//! \param c The context of the transaction
	//! \pre fData2Send contains the HTTP-Request
	//! \post fData2Get contains either an slot /MsgReceived containing the HTTP Response or the slot /Error storing information about the error occured.
	virtual bool Exec(Context &c, ParameterMapper *, ResultMapper *);

protected:
	virtual bool HandleError(const char *msg, Context &ctx);

	//! simulates a connection to a server, outgoing request is tested, incoming reply is assembled and sent, used when testing ONLY
	//! author - Mike egoless
	//! Mappers are configured as in usual case but with this DataAccess object rather than the usual one, e.g. HTTPDAImpl.
	//! everything is then tested down to the mappers themselves. The file to be used as config must supply all the expected Request/Reply pairs expected
	//! as an anything. The current logic expects Requests to be unique and uses them thus directly as slotnames. If this does not suit your requirements change
	//! the CompareRequest method here to supply your desired behaviour. Same is true for other methods, although they should need to be
	//! overridden since they are config from Mappers anyway.
	//! \param context the context for this call
	//! \param in ParameterMapper
	//! \param out ResultMapper
	virtual bool DoExec( Context &context, ParameterMapper *in, ResultMapper *out);

	virtual bool RenderReply( String &theReply, Context &context, ResultMapper *out  );
	virtual String GetReplyMatchingRequest( Anything &recording, Context &context, String &request );
	virtual bool BuildRequest( String &request, Context &context, ParameterMapper *in );

private:
	//constructor
	DummyDAImpl();
	DummyDAImpl(const DummyDAImpl &);
	//assignement
	DummyDAImpl &operator=(const DummyDAImpl &);
};

/* Don't add stuff after this #endif */
#endif		//not defined _DummyDAImpl_H
