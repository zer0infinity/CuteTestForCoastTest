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

class MIMEHeader;

//--- HTTPProcessor ----------------------------------------------------------
//! Policy object to read HTTP Requests unscramble URL Variables
class EXPORTDECL_STDDATAACCESS HTTPProcessor : public RequestProcessor
{
public:
	HTTPProcessor(const char *processorName)
		: RequestProcessor(processorName),
		  fLineSizeLimit(4096),
		  fRequestSizeLimit(5120),
		  fURISizeLimit(271),	// limit URI size to 255 per default since most of the proxies and some browsers won't handle more
		  fCheckUrlEncodingOverride(),
		  fCheckUrlPathContainsUnsafeCharsOverride(),
		  fCheckUrlPathContainsUnsafeCharsAsciiOverride(),
		  fUrlExhaustiveDecode(0),
		  fFixDirectoryTraversial(0),
		  fURLEncodeExclude("/?") {}

	virtual ~HTTPProcessor()	{ }

	//! support for prototypes is required
	virtual IFAObject *Clone() const {
		return new HTTPProcessor(fName);
	}

	//!configure request processor with server object
	virtual void Init(Server *server);

protected:
	//! checks request headers if zip-encoding is accepted by client
	static bool IsZipEncodingAcceptedByClient(Context &ctx);

	//!read the input arguments from the stream and generate an anything
	virtual void DoReadInput(iostream &ios, Context &ctx);

	//!read in the request body from a POST if any
	virtual void ReadRequestBody(iostream &ios, Anything &request, MIMEHeader &header, Context &ctx);
	//!set some client info needed for verification
	virtual void SetWDClientInfo(Context &ctx);
	//!parse the cookie string into an anything
	virtual Anything ParseCookie(const String &line);

	//!process the arguments and generate a reply
	virtual void DoProcessRequest(ostream &reply, Context &ctx);

	//! render the protocol specific status
	virtual void DoRenderProtocolStatus(ostream &os, Context &ctx);

	//! checks if the connection should keep-alive after the request has processed
	virtual bool DoKeepConnectionAlive(Context &ctx);

	//! render the protocol specific error msg
	virtual void DoError(ostream &reply, const String &msg, Context &ctx);

	long 		fLineSizeLimit;
	long 		fRequestSizeLimit;
	long 		fURISizeLimit;
	String		fCheckUrlEncodingOverride;
	String		fCheckUrlPathContainsUnsafeCharsOverride;
	String		fCheckUrlPathContainsUnsafeCharsAsciiOverride;
	long		fUrlExhaustiveDecode;
	long		fFixDirectoryTraversial;
	String		fURLEncodeExclude;

	friend class HTTPProcessorTest;
	friend class RequestReaderTest;
	friend class RequestReader;
private:
	HTTPProcessor(const HTTPProcessor &);
	HTTPProcessor &operator=(const HTTPProcessor &);
};

#endif
