/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _REQUESTREADER_H
#define _REQUESTREADER_H

#include "config_stddataaccess.h"
#include "Anything.h"
#include <iosfwd>

class RequestProcessor;
class MIMEHeader;
class Context;

//--- HTTPRequestReader ----------------------------------------------------------
//! Policy object to read HTTP Requests unscramble URL Variables
class EXPORTDECL_STDDATAACCESS HTTPRequestReader
{
public:
	//!reads request from ios on behalf of processor
	HTTPRequestReader(RequestProcessor *p, MIMEHeader &header);

	//!read a request and handle error through ios
	bool ReadRequest(Context &ctx, std::iostream &Ios);

	//!get the resulting anything if read request was successful
	Anything const& GetRequest();

private:
	//!read the one input line stream and check it against limits,
	//! my generate an error reply in case of "attacks"
	bool ReadLine(Context &ctx, std::iostream &Ios, long const maxLineSz, String &line);
	//!handle the the request line by line
	bool ParseRequest(Context &ctx, std::iostream &Ios, String &line);

	//!handle the first line of a request containing GET/POST
	bool HandleFirstLine(Context &ctx, std::iostream &Ios, String &line);

	//!check the length of a single request line, handle error if necessary
	bool RequestSizeLimitExceeded(Context &ctx, std::iostream &Ios, long const maxReqSz, const String &line);

	//!check the size of the request uri
	bool CheckReqURISize(Context &ctx, std::iostream &Ios, long lineLength, const String &line);

	//!Verify t RFC 1738 compliance
	bool VerifyUrlPath(Context &ctx, std::iostream &Ios, String &urlPath);

	//!Verify t RFC 1738 compliance
	bool VerifyUrlArgs(Context &ctx, String &urlArgs);

	//!writes back http error codes with html msg
	bool DoHandleError(Context &ctx, std::iostream &Ios, long errcode, const String &reason, const String &line, bool reject = true, const String & = String("Page not found."));

	//!Logs  the error if SecurityLog is defined in AppLog config
	void LogError(Context &ctx, long errcode, const String &reason, const String &line, const String &msg);

	//!the processor we are working for
	RequestProcessor *fProc;

	//!product output a request anything
	Anything fRequest;

	//!subpart of output a header, containing all http headers
	MIMEHeader &fHeader;

	//!contains the current requests size
	long fRequestBufferSize;

	//!flag defining the first line
	bool fFirstLine;

};

#endif
