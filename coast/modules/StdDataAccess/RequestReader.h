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
#include "HTTPProcessor.h"
#include "MIMEHeader.h"

//--- RequestReader ----------------------------------------------------------
//! Policy object to read HTTP Requests unscramble URL Variables
class EXPORTDECL_STDDATAACCESS RequestReader
{
public:
	//!reads request from ios on behalf of processor
	RequestReader(HTTPProcessor *p, MIMEHeader &header);

	//!read a request and handle error throug ios
	bool ReadRequest(iostream &Ios, const Anything &clientInfo = Anything());

	//!get the resulting anything if read request was successful
	Anything GetRequest();

	//!get the resulting anything if read request was successful
	Anything GetErrors() {
		return fErrors;
	};
	bool	 HasErrors()	{
		return fErrors.GetSize() > 0 ? true : false;
	};

protected:
	//!read the one input line stream and check it against limits,
	//! my generate an error reply in case of "attacks"
	bool ReadLine(iostream &Ios, String &line, const Anything &clientInfo, bool &hadError);
	//!handle the the request line by line
	bool ParseRequest(iostream &Ios, String &line, const Anything &clientInfo);

	//!handle the first line of a request containing GET/POST
	bool HandleFirstLine(iostream &Ios, String &line, const Anything &clientInfo);

	//!check the length of the request, handle error if necessary
	bool CheckReqLineSize(iostream &Ios, long lineLength, const String &line, const Anything &clientInfo);

	//!check the length of a single request line, handle error if necessary
	bool CheckReqBufferSize(iostream &Ios, long lineLength, const String &line, const Anything &clientInfo);

	//!check the size of the request uri
	bool CheckReqURISize(iostream &Ios, long lineLength, const String &line, const Anything &clientInfo);

	//!Verify t RFC 1738 compliance
	bool VerifyUrlPath(iostream &Ios, String &urlPath, const Anything &clientInfo);

	//!Verify t RFC 1738 compliance
	bool VerifyUrlArgs(String &urlArgs);

	//!writes back http error codes with html msg
	bool DoHandleError(iostream &Ios, long errcode, const String &reason, const String &line, const Anything &clientInfo, const String & = String("Page not found."));

	//!Logs  the error if SecurityLog is defined in AppLog config
	void DoLogError(long errcode, const String &reason, const String &line, const Anything &clientInfo, const String &msg);

	//!the processor we are working for
	HTTPProcessor *fProc;

	//!product output a request anything
	Anything fRequest;

	//!subpart of output a header, containing all http headers
	MIMEHeader &fHeader;

	//!contains the current request only really used when tracing
	String fRequestBuffer;

	//!contains the current requests size
	long fRequestBufferSize;

	//!flag defining the first line
	bool fFirstLine;

	Anything fErrors;
};

#endif
