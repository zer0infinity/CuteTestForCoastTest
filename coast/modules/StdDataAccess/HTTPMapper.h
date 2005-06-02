/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPMAPPER_H
#define _HTTPMAPPER_H

#include "config_stddataaccess.h"
#include "Mapper.h"

// --------------------- HTTPHeaderParameterMapper -------------------------
class EXPORTDECL_STDDATAACCESS HTTPHeaderParameterMapper : public EagerParameterMapper
{
public:
	HTTPHeaderParameterMapper(const char *name) : EagerParameterMapper(name) { }
	~HTTPHeaderParameterMapper()	{ }

	//! support for prototype pattern, implements standard api
	IFAObject *Clone() const {
		return new HTTPHeaderParameterMapper(fName);
	}

	//! prepares headerfield input for http connection
	//! \param key expected HTTPHeader (indicates where to find the headerfield info)
	//! \param os the stream we write the http header fields to
	//! \param ctx the context for this call
	//! \param conf the configuration for this call. may contain a slot
	//              /Suppress that contains names of headers to suppress
	bool DoGetStream(const char *key, ostream &os, Context &ctx,  ROAnything conf);
private:
	HTTPHeaderParameterMapper();
	HTTPHeaderParameterMapper(const HTTPHeaderParameterMapper &);
	HTTPHeaderParameterMapper &operator=(const HTTPHeaderParameterMapper &);
};

// ------------------------- HTTPBodyResultMapper -------------------------
class EXPORTDECL_STDDATAACCESS HTTPBodyResultMapper : public ResultMapper
{
public:
	HTTPBodyResultMapper(const char *name) : ResultMapper(name) { }
	~HTTPBodyResultMapper()	{ }

	IFAObject *Clone() const {
		return new HTTPBodyResultMapper(fName);
	}

	bool DoFinalPutStream(const char *key, istream &is, Context &ctx);

protected:
	virtual void ReadBody(String &body, istream &is, Context &ctx);

private:
	HTTPBodyResultMapper();
	HTTPBodyResultMapper(const HTTPBodyResultMapper &);
	HTTPBodyResultMapper &operator=(const HTTPBodyResultMapper &);
};

// ------------------------- HTTPBodyParameterMapper -------------------------
class EXPORTDECL_STDDATAACCESS HTTPBodyParameterMapper : public ParameterMapper
{
public:
	HTTPBodyParameterMapper(const char *name) : ParameterMapper(name) { }
	~HTTPBodyParameterMapper()	{ }

	IFAObject *Clone() const {
		return new HTTPBodyParameterMapper(fName);
	}

	bool DoFinalGetStream(const char *key, ostream &os, Context &ctx);

private:
	HTTPBodyParameterMapper();
	HTTPBodyParameterMapper(const HTTPBodyParameterMapper &);
	HTTPBodyParameterMapper &operator=(const HTTPBodyParameterMapper &);
};

#endif
