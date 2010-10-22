/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPPROCESSOR_H
#define _HTTPPROCESSOR_H

#include "config_stddataaccess.h"
#include "RequestProcessor.h"

class Context;
class MIMEHeader;

//--- HTTPProcessor ----------------------------------------------------------
//! Policy object to read HTTP Requests unscramble URL Variables
class EXPORTDECL_STDDATAACCESS HTTPProcessor : public RequestProcessor
{
public:
	HTTPProcessor(const char *processorName) :
		RequestProcessor(processorName) {
	}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) HTTPProcessor(fName);
	}

	//!configure request processor with server object
	virtual void Init(Server *server);

	//! checks request headers if zip-encoding is accepted by client
	static bool IsZipEncodingAcceptedByClient(Context &ctx);

protected:
	//!read in the request body from a POST if any
	virtual void ReadRequestBody(std::iostream &ios, Anything &request, MIMEHeader &header, Context &ctx);

	//!read the input arguments from the stream and generate an anything
	virtual void DoReadInput(std::iostream &ios, Context &ctx);

	//!set some client info needed for verification
	virtual void SetWDClientInfo(Context &ctx);
	//!parse the cookie string into an anything
	virtual Anything ParseCookie(const String &line);

	//!process the arguments and generate a reply
	virtual void DoProcessRequest(std::ostream &reply, Context &ctx);

	//! render the protocol specific status
	virtual void DoRenderProtocolStatus(std::ostream &os, Context &ctx);

	//! checks if the connection should keep-alive after the request has processed
	virtual bool DoKeepConnectionAlive(Context &ctx);

	//! Log the error to Security.log
	virtual Anything DoLogError(Context& ctx, long errcode, const String &reason, const String &line, const Anything &clientInfo, const String &msg, Anything &request, const char *who);

	//! render the protocol specific error msg
	virtual void DoError(std::ostream &reply, const String &msg, Context &ctx);

private:
	HTTPProcessor(const HTTPProcessor &);
	HTTPProcessor &operator=(const HTTPProcessor &);
};

#endif
