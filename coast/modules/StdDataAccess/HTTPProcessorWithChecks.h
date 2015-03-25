/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPProcessorWithChecks_H
#define _HTTPProcessorWithChecks_H

#include "HTTPProcessor.h"

class Context;

//! Policy object to read HTTP Requests unscramble URL Variables
class HTTPProcessorWithChecks : public HTTPProcessor
{
public:
	HTTPProcessorWithChecks(const char *processorName) :
		HTTPProcessor(processorName) {
	}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) HTTPProcessorWithChecks(fName);
	}

protected:
	virtual bool DoPrepareContextRequest(std::iostream &Ios, Context &ctx, Anything &request, HTTPRequestReader &reader);

	virtual bool DoVerifyRequest(Context &ctx);

	virtual bool DoHandleVerifyRequestError(std::iostream &Ios, Context &ctx);

	virtual bool DoHandleReadInputError(std::iostream &Ios, Context &ctx);

private:
	HTTPProcessorWithChecks(const HTTPProcessorWithChecks &);
	HTTPProcessorWithChecks &operator=(const HTTPProcessorWithChecks &);
};

#endif
