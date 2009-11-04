/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _BPLDAImpl_H
#define _BPLDAImpl_H

//--- superclass interface ---
#include "config_stddataaccess.h"
#include "DataAccessImpl.h"
#include "HTTPDAImpl.h"

class ConnectorParams;
class Connector;

//! DataAccess for performing HTTP Requests, uses meta data defined in HTTPMeta.any
class EXPORTDECL_STDDATAACCESS BPLDAImpl: public HTTPDAImpl
{
public:
	BPLDAImpl(const char *name);
	~BPLDAImpl();

	// returns a new TRX object
	IFAObject *Clone() const;

	//! executes the transaction
	//! \param c The context of the transaction
	virtual bool Exec(Context &c, ParameterMapper *, ResultMapper *);

protected:
//	String GenerateErrorMessage(const char *msg, Context &ctx);

//	bool SendInput(iostream *ios, Socket *s, long timeout, Context &context, ParameterMapper *in, ResultMapper *out);
//	bool DoSendInput(iostream *ios, Socket *s, long timeout, Context &context, ParameterMapper *in, ResultMapper *out);
	bool DoExec(Connector *csc, ConnectorParams *cps, Context &context, ParameterMapper *in, ResultMapper *out);

private:
	//constructor
	BPLDAImpl();
	BPLDAImpl(const BPLDAImpl &);
	//assignement
	BPLDAImpl &operator=(const BPLDAImpl &);
};

/* Don't add stuff after this #endif */
#endif		//not defined _BPLDAImpl_H
