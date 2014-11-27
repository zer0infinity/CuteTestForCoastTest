/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RequestBodyParser_H
#define _RequestBodyParser_H

#include "Anything.h"

namespace coast {
	namespace http {
		String const contentTypeAnything("application/x-coast-anything", coast::storage::Global());
	}
}

class MIMEHeader;

//!decode a mime body, even multipart. special treatment for POST requests
//! where content-disposition gives us a hint for decoding
//! decodes bodies according to normal browser POST requests
//! only works for multipart-form data
class HTTPPostRequestBodyParser {
	HTTPPostRequestBodyParser();
	MIMEHeader &fHeader;
	Anything fContent;
	String fUnparsedContent;
public:
	//! ctor requires a header for parameters on length and decoding
	HTTPPostRequestBodyParser(MIMEHeader &mainheader) :
		fHeader(mainheader) {
	}
	virtual ~HTTPPostRequestBodyParser() {}
	//! do the parsing, read everything
	bool Parse(std::istream &input);
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
	virtual bool DoParseMultiPart(std::istream &input, const String &bound);
	//! parse the mime body, usually xxx-form-urlencoded
	//! \return indicates whether body was successfully read
	virtual bool DoParseBody(std::istream &input);
	//! auxiliary for ParseMultiPart
	//! \return indicates whether body was successfully read
	virtual bool DoReadToBoundary(std::istream &input, const String &bound, String &body);
};

#endif
