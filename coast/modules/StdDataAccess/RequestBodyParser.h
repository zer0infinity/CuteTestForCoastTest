/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RequestBodyParser_H
#define _RequestBodyParser_H

#include "config_stddataaccess.h"
#include "Anything.h"

class MIMEHeader;

//!decode a mime body, even multipart. special treatment for POST requests
//! where content-disposition gives us a hint for decoding
//! decodes bodies according to normal browser POST requests
//! only works for multipart-form data
class EXPORTDECL_STDDATAACCESS RequestBodyParser
{
public:
	//! ctor requires a header for parameters on length and decoding
	RequestBodyParser(MIMEHeader &mainheader, istream &input);

	//! do the parsing, read everything
	bool Parse();
	//! return the decoded result after parsing
	Anything &GetContent() {
		return fContent;
	}
	//! return the unparsed body of the request
	String &GetUnparsedContent() {
		return fUnparsedContent;
	}

protected:
	// operational methods
	//! do the multi-part mime parsing, using boundary-string bound
	//! \return whether multipart body was read successfully
	bool ParseMultiPart(istream *is, const String &bound);
	//! parse the mime body, usually xxx-form-urlencoded
	//! \return indicates whether body was successfully read
	bool ParseBody();
	//! auxiliary for ParseMultiPart
	//! \return indicates whether body was successfully read
	bool ReadToBoundary(istream *is, const String &bound, String &body);
	//! do the url-decoding of str
	void Decode(String str, Anything &result);

private:
	RequestBodyParser();
	RequestBodyParser(const RequestBodyParser &);
	istream	&fInput;	// to parse from
	istream	*fIn;	// to parse from
	MIMEHeader &fHeader;
	Anything fContent;
	String fUnparsedContent;

	friend class RequestBodyParserTest;
};

#endif
