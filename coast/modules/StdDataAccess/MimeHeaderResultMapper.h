/*
 * Copyright (c) 2010, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MimeHeaderResultMapper_H
#define _MimeHeaderResultMapper_H

//---- Mapper include -------------------------------------------------
#include "Mapper.h"

//---- MimeHeaderResultMapper ----------------------------------------------------------
//! Parses header fields from stream and stores them under Mapper.HTTPHeader
//! config: config.Suppress is an optional list of headers to suppress, config.Add is
//! an optional list of /key "header" entries to add.
//! Example: { /Suppress { "server", "content-type" } /Add { /server "Mozilla" /FD-Info "blah" } }
//! will suppress server and content-type headers and add a new server-header with value
//! "Mozilla" and a new header /fd-info with value "blah". If headers are added that already
//! exists, the old ones are overwritten (thus the above suppressing of /server is not really
//! necessary).
class MimeHeaderResultMapper: public EagerResultMapper {
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	MimeHeaderResultMapper(const char *name) :
		EagerResultMapper(name) {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) MimeHeaderResultMapper(fName);
	}

protected:
	//! reads from istream a MIME header and putting it with some key
	/*! @copydoc ResultMapper::DoPutStream() */
	virtual bool DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything script);
};

#endif
