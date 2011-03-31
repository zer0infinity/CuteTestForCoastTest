/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPDAImpl_H
#define _HTTPDAImpl_H

#include "DataAccessImpl.h"

class ConnectorParams;
class Connector;
class Socket;
class Context;

//! DataAccess for performing HTTP Requests
class HTTPDAImpl: public DataAccessImpl {
	HTTPDAImpl();
	HTTPDAImpl(const HTTPDAImpl &);
	HTTPDAImpl &operator=(const HTTPDAImpl &);
public:
	HTTPDAImpl(const char *name) :
		DataAccessImpl(name) {
	}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) HTTPDAImpl(fName);
	}

	//! executes the transaction
	//! \param c The context of the transaction
	virtual bool Exec(Context &c, ParameterMapper *, ResultMapper *);

protected:
	String GenerateErrorMessage(const char *msg, Context &ctx);

	bool SendInput(std::iostream *ios, Socket *s, long timeout, Context &context, ParameterMapper *in, ResultMapper *out);
	bool DoSendInput(std::iostream *ios, Socket *s, long timeout, Context &context, ParameterMapper *in, ResultMapper *out);
	bool DoExec(Connector *csc, ConnectorParams *cps, Context &context, ParameterMapper *in, ResultMapper *out);

#if defined(RECORD)
	//! simulates a connection to a server, outgoing request is tested, incoming reply is assembled and sent, used when testing ONLY
	//! \param context the context for this call
	//! \param in input Mapper
	//! \param out output Mapper
	bool DoExecRecord(Connector *csc, ConnectorParams *cps, Context &context, ParameterMapper *in, ResultMapper *out);

	bool ReadReply( String &theReply, Context &context, std::iostream *ios );
	bool RenderReply( String &theReply, Context &context, ResultMapper *out );
	//	long CompareRequest( Anything & recording, Context &context, String & request );
	bool BuildRequest( String &request, Context &context, ParameterMapper *in, ResultMapper *out);
	bool SendRequest(String &request, std::iostream *ios, Socket *s, ConnectorParams *cps );
#endif
};

#endif		//not defined _HTTPDAImpl_H
