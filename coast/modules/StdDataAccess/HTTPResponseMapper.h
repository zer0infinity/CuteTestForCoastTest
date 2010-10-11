/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPResponseMapper_H
#define _HTTPResponseMapper_H

//---- Mapper include -------------------------------------------------
#include "config_stddataaccess.h"
#include "Mapper.h"

//---- HTTPResponseMapper ----------------------------------------------------------
//! Parses a http response from a stream and puts it under Mapper.HTTPResponse
class EXPORTDECL_STDDATAACCESS HTTPResponseMapper : public ResultMapper
{
public:
	//--- constructors
	HTTPResponseMapper(const char *name);
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const;
	//! reads a http response from istream and puts it under Mapper.HTTPResponse
	//! \param key not used
	//! \param is the stream to be read
	//! \param ctx the thread context of the invocation
	//! \param conf not used
	//! \return returns true if the mapping was successful otherwise false
	virtual bool DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything conf);

private:
	HTTPResponseMapper();
	HTTPResponseMapper(const HTTPResponseMapper &);
	HTTPResponseMapper &operator=(const HTTPResponseMapper &);
};

#endif
