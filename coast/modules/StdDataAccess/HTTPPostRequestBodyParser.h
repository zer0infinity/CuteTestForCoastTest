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

namespace Coast {
	namespace HTTP {
		String const contentTypeAnything("application/x-coast-anything", Coast::Storage::Global());
	}
}

class MIMEHeader;

//!decode a mime body, even multipart. special treatment for POST requests
//! where content-disposition gives us a hint for decoding
//! decodes bodies according to normal browser POST requests
//! only works for multipart-form data
class HTTPPostRequestBodyParser
{
public:
	//! ctor requires a header for parameters on length and decoding
	HTTPPostRequestBodyParser(MIMEHeader &mainheader);

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
	bool ParseMultiPart(std::istream &input, const String &bound);
	//! parse the mime body, usually xxx-form-urlencoded
	//! \return indicates whether body was successfully read
	bool ParseBody(std::istream &input);
	//! auxiliary for ParseMultiPart
	//! \return indicates whether body was successfully read
	bool ReadToBoundary(std::istream &input, const String &bound, String &body);
	//! do the url-decoding of str
	void Decode(String str, Anything &result);

private:
	HTTPPostRequestBodyParser();
	HTTPPostRequestBodyParser(const HTTPPostRequestBodyParser &);
	MIMEHeader &fHeader;
	Anything fContent;
	String fUnparsedContent;

	friend class HTTPPostRequestBodyParserTest;
};

#endif
