/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPResponseMapper_H
#define _HTTPResponseMapper_H

//---- OutputMapper include -------------------------------------------------
#include "config_stddataaccess.h"
#include "Mapper.h"

//---- HTTPResponseMapper ----------------------------------------------------------
//! <B>Parses a http response from a stream and puts it under Mapper.HTTPResponse</B>
class EXPORTDECL_STDDATAACCESS HTTPResponseMapper : public ResultMapper
{
public:
	//--- constructors
	HTTPResponseMapper(const char *name);
	//--- support for prototype
	IFAObject *Clone() const;
	//! reads a http response from istream and puts it under Mapper.HTTPResponse
	//! \param key not used
	//! \param is the stream to be read
	//! \param ctx the thread context of the invocation
	//! \param conf not used
	//! \return returns true if the mapping was successful otherwise false
	virtual bool DoPutStream(const char *key, istream &is, Context &ctx, ROAnything conf);

private:
	HTTPResponseMapper();
	HTTPResponseMapper(const HTTPResponseMapper &);
	HTTPResponseMapper &operator=(const HTTPResponseMapper &);
};

#endif
