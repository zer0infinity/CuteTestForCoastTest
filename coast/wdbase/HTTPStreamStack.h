/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPStreamStack_H
#define _HTTPStreamStack_H

#include "StringStream.h"

//! Helper class to handle chunked and gzip encoding of http bodys.
class HTTPStreamStack
{
public:
	//! Creates a stack of ostreams to stream out an http body.
	//! The additional header fields followed by the header separator are rendered to the output stream.
	//! \param output http output stream. Header separator has not been rendered yet.
	//! \param chunked true if chunked encoding is needed
	//! \param zipEnc true if gzip encoding is needed
	HTTPStreamStack(std::ostream &output, bool chunked, bool zipEnc);

	//! Returns the ostream where the http body can be rendered to
	std::ostream &GetBodyStream();

	~HTTPStreamStack();

private:
	// use careful, you inhibit subclass use
	//--- private class api
	//--- private member variables
	std::ostream &fOutput;
	std::ostream *fTopOfStack;
	std::ostream *fChunker;
	std::ostream *fZipper;
};

#endif
