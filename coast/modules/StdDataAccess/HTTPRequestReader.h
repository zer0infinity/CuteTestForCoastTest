/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _REQUESTREADER_H
#define _REQUESTREADER_H

#include "Anything.h"
#include <iosfwd>

class MIMEHeader;
class Context;

//! Policy object to read HTTP Requests unscramble URL Variables
class HTTPRequestReader {
	//!product output a request anything
	Anything fRequest;

	//!subpart of output a header, containing all http headers
	MIMEHeader &fHeader;

	//!contains the current requests size
	long fRequestBufferSize;
public:
	//!reads request from ios on behalf of processor
	HTTPRequestReader(MIMEHeader &header) :
		fHeader(header), fRequestBufferSize(0) {
	}
	//!read a request and handle error through ios
	bool ReadRequest(Context & ctx, std::iostream & Ios);
	//!get the resulting anything if read request was successful
	const Anything & GetRequest();
};

#endif
