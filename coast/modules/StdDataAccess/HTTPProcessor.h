/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPPROCESSOR_H
#define _HTTPPROCESSOR_H

#include "RequestProcessor.h"

class Context;
class MIMEHeader;
class HTTPRequestReader;
class HTTPPostRequestBodyParser;

namespace coast {
	namespace http {
		void RenderHTTPProtocolStatus(std::ostream &os, Context &ctx);
		Anything GenerateErrorMessageAny(Context& ctx, long const errorcode, String const& msg, String const& content, String const& component);
		void PutErrorMessageIntoContext(Context& ctx, long const errorcode, String const& msg, String const& content, String const& component);
	}
}

//--- HTTPProcessor ----------------------------------------------------------
//! Policy object to read HTTP Requests unscramble URL Variables
class HTTPProcessor : public RequestProcessor
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
	//!get the MIMEHeader used to parse the request
	virtual MIMEHeader GetMIMEHeader() const;

	//!get the RequestReader used to parse and check the request line
	virtual HTTPRequestReader GetRequestReader(MIMEHeader& header) const;

	//!get the RequestBodyParser used to parse and check the request body
	virtual HTTPPostRequestBodyParser GetRequestBodyParser(MIMEHeader& header) const;

	//!read the input arguments from the stream and generate an anything
	virtual bool DoReadInput(std::iostream &ios, Context &ctx);

	virtual bool DoReadRequestHeader(std::iostream &Ios, Context &ctx, HTTPRequestReader &reader);

	virtual bool DoPrepareContextRequest(std::iostream &Ios, Context &ctx, Anything &request, HTTPRequestReader &reader);

	//!read in the request body from a POST if any
	virtual bool DoReadRequestBody(std::iostream &Ios, Context &ctx, Anything &request, HTTPRequestReader &reader);

	//!process the arguments and generate a reply
	virtual bool DoProcessRequest(std::ostream &reply, Context &ctx);

	virtual bool DoVerifyRequest(Context &ctx);

	virtual bool DoHandleVerifyRequestError(std::iostream &Ios, Context &ctx);

	virtual bool DoHandleReadInputError(std::iostream &Ios, Context &ctx);

	virtual bool DoHandleProcessRequestError(std::iostream &Ios, Context &ctx);

	//! render the protocol specific status
	virtual void DoRenderProtocolStatus(std::ostream &os, Context &ctx);

	//! checks if the connection should keep-alive after the request has processed
	virtual bool DoKeepConnectionAlive(Context &ctx);

private:
	HTTPProcessor(const HTTPProcessor &);
	HTTPProcessor &operator=(const HTTPProcessor &);
};

#endif
