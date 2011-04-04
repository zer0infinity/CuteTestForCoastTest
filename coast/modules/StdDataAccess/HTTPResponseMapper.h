/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPResponseMapper_H
#define _HTTPResponseMapper_H

#include "Mapper.h"

//! Parses a http response from a stream and puts it under Mapper.HTTPResponse
class HTTPResponseMapper: public ResultMapper {
	HTTPResponseMapper();
	HTTPResponseMapper(const HTTPResponseMapper &);
	HTTPResponseMapper &operator=(const HTTPResponseMapper &);
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	HTTPResponseMapper(const char *name) :
		ResultMapper(name) {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) HTTPResponseMapper(fName);
	}
protected:
	//! reads a http response from istream and puts it under Mapper.HTTPResponse
	/*! @copydoc ResultMapper::DoPutStream() */
	virtual bool DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything conf);
};

#endif
