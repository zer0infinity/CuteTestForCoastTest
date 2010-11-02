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

class MIMEHeader;
class Context;

//--- HTTPRequestReader ----------------------------------------------------------
//! Policy object to read HTTP Requests unscramble URL Variables
class EXPORTDECL_STDDATAACCESS HTTPRequestReader
{
public:
	//!reads request from ios on behalf of processor
	HTTPRequestReader(MIMEHeader &header);

	//!read a request and handle error through ios
	bool ReadRequest(Context &ctx, std::iostream &Ios);

	//!get the resulting anything if read request was successful
	Anything const& GetRequest();

private:
	//!read the one input line stream and check it against limits,
	//! my generate an error reply in case of "attacks"
	bool ReadLine(Context &ctx, std::iostream &Ios, long const maxLineSz, String &line);

	//!handle the the request line by line
	bool ParseRequest(Context &ctx, String &line);

	//!handle the first line of a request containing GET/POST
	bool HandleFirstLine(Context &ctx, String &line);

	//!check the length of a single request line, handle error if necessary
	bool RequestSizeLimitExceeded(Context &ctx, long const maxReqSz, const String &line) const;

	//!check the size of the request uri
	bool CheckReqURISize(Context &ctx, long lineLength, const String &line) const;

	//!product output a request anything
	Anything fRequest;

	//!subpart of output a header, containing all http headers
	MIMEHeader &fHeader;

	//!contains the current requests size
	long fRequestBufferSize;
};

#endif
