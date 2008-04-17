/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPMimeHeaderMapper_H
#define _HTTPMimeHeaderMapper_H

//---- Mapper include -------------------------------------------------
#include "config_stddataaccess.h"
#include "Mapper.h"

//---- HTTPMimeHeaderMapper ----------------------------------------------------------
//! <B>Parses header fields from stream and stores them under Mapper.HTTPHeader</B>
//! config: config.Suppress is an optional list of headers to suppress, config.Add is
//! an optional list of /key "header" entries to add.
//! Example: { /Suppress { "server", "content-type" } /Add { /server "Mozilla" /FD-Info "blah" } }
//! will suppress server and content-type headers and add a new server-header with value
//! "Mozilla" and a new header /fd-info with value "blah". If headers are added that already
//! exists, the old ones are overwritten (thus the above suppressing of /server is not really
//! necessary).
class EXPORTDECL_STDDATAACCESS HTTPMimeHeaderMapper : public EagerResultMapper
{
public:
	//--- constructors
	HTTPMimeHeaderMapper(const char *name);
	//--- support for prototype
	IFAObject *Clone() const;

	//! reads from istream a MIME header storing it under Mapper.HTTPHeader
	//! \param key not used
	//! \param is the stream to be parsed
	//! \param ctx the context of the invocation
	//! \return returns true if the mapping was successful otherwise false
	virtual bool DoPutStream(const char *key, istream &is, Context &ctx,  ROAnything config);
	//fix parsed date fields
	static void CorrectDateFormats(Anything &header);
	static void SuppressHeaders(Anything &header, ROAnything &suppresslist);
	static void AddHeaders(Anything &header, ROAnything &addlist);
	static void Substitute(Anything &header, ROAnything &addlist, Context &ctx);
	static void StoreCookies(Anything &header, Context &ctx);

private:
	HTTPMimeHeaderMapper();
	HTTPMimeHeaderMapper(const HTTPMimeHeaderMapper &);
	HTTPMimeHeaderMapper &operator=(const HTTPMimeHeaderMapper &);
};

#endif
