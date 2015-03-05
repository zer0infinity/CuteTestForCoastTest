/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CGIParams_H
#define _CGIParams_H

#include "URI2FileNameMapper.h"

//---- CgiParams ----------------------------------------------------------
//! obtain CGI parameters
class CgiParams : public URI2FileNameMapper
{
public:
	//--- constructors
	CgiParams(const char *name);

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const;

	//! fills value with a CGI environment on key "cgienv" otherwise delegates up
	//! environment values are rendered per slot and thus can be renderer configs
	//! \param key the name defines kind of value to get
	//! \param value the value to be mapped
	//! \param ctx the thread context of the invocation
	//! \param info current mapper configuration as ROAnything
	//! \return returns true if the mapping was successful otherwise false
	virtual bool DoGetAny(const char *key, Anything &value, Context &ctx,  ROAnything config);

	//! streams parameter "WHOLE_REQUEST_BODY" on key "stdin"
	//! \param key the name defines kind of values to write
	//! \param os the stream to be mapped
	//! \param ctx the thread context of the invocation
	//! \param config current mapper configuration as ROAnything
	//! \return returns true if the mapping was successful otherwise false
	virtual bool DoGetStream(const char *key, std::ostream &os, Context &ctx,  ROAnything config);

protected:
	virtual bool ResolveInvalidFile(String &path, String &uri, String &value, Context &ctx);

private:
	CgiParams();
	CgiParams(const CgiParams &);
	CgiParams &operator=(const CgiParams &);

	//! create synthetic cgi env for later use
	//! fields created are: SERVER_SOFTWARE SERVER_NAME GATEWAY_INTERFACE
	void SynthesizeMinimalCGIEnvironment(Anything &env, Context &ctx);
	void AddToEnvironment(Context &ctx, Anything &env, ROAnything additionalenv);
};

#endif
