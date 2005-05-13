/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPDAImpl_H
#define _HTTPDAImpl_H

//--- superclass interface ---
#include "config_stddataaccess.h"
#include "DataAccessImpl.h"

class ConnectorParams;
class Connector;

//! DataAccess for performing HTTP Requests, uses meta data defined in HTTPMeta.any
class EXPORTDECL_STDDATAACCESS HTTPDAImpl: public DataAccessImpl
{
public:
	HTTPDAImpl(const char *name);
	~HTTPDAImpl();

	// returns a new TRX object
	IFAObject *Clone() const;

	//! executes the transaction
	//! \param c The context of the transaction
	virtual bool Exec(Context &c, ParameterMapper *, ResultMapper *);

protected:
	String GenerateErrorMessage(const char *msg, Context &ctx);

	bool SendInput(iostream *ios, Socket *s, long timeout, Context &context, ParameterMapper *in, ResultMapper *out);
	bool DoSendInput(iostream *ios, Socket *s, long timeout, Context &context, ParameterMapper *in, ResultMapper *out);
	bool DoExec(Connector *csc, ConnectorParams *cps, Context &context, ParameterMapper *in, ResultMapper *out);

#if defined(RECORD)
	//! simulates a connection to a server, outgoing request is tested, incoming reply is assembled and sent, used when testing ONLY
	//! \param context the context for this call
	//! \param in input Mapper
	//! \param out output Mapper
	bool DoExecRecord(Connector *csc, ConnectorParams *cps, Context &context, ParameterMapper *in, ResultMapper *out);

	bool ReadReply( String &theReply, Context &context, iostream *ios );
	bool RenderReply( String &theReply, Context &context, ResultMapper *out  );
//	long CompareRequest( Anything & recording, Context &context, String & request );
	bool BuildRequest( String &request, Context &context, ParameterMapper *in, ResultMapper *out);
	bool SendRequest(String &request, iostream *ios, Socket *s, ConnectorParams *cps );
#endif

private:
	//constructor
	HTTPDAImpl();
	HTTPDAImpl(const HTTPDAImpl &);
	//assignement
	HTTPDAImpl &operator=(const HTTPDAImpl &);
};

/* Don't add stuff after this #endif */
#endif		//not defined _HTTPDAImpl_H
