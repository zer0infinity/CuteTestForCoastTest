/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPProtocolReplyRenderer_H
#define _HTTPProtocolReplyRenderer_H

#include "Renderer.h"

//---- HTTPProtocolReplyRenderer ----------------------------------------------------------
//! generate a valid reply line, like "HTTP/1.1 200 OK" including the ENDL
//! expects the following to be available via context lookup:
//! /HTTPStatus {
//!   /HTTPVersion "HTTP/1.1" optional
//!   /ResponseCode 200     defaults to 200
//!   /ResponseMsg "OK"     might use a lookup from some config to map ResponseMsg to a reason
//! }
class HTTPProtocolReplyRenderer : public Renderer
{
public:
	//--- constructors
	HTTPProtocolReplyRenderer(const char *name) :
		Renderer(name) {
	}

	//! Renders a valid HTTP reply line on <I>reply </I>
	// \param reply the stream where the rendered output is written on.
	// \param ctx the context the renderer runs within.
	// \param config the configuration of the renderer.
	virtual void RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config);

	//! Returns the default reason phrase for a given http status code
	//! If the status code is unknown, the string "Unknown Error" is returned.
	//! \param status http status code
	//! \return default reason phrase for a given http status code
	static const char *DefaultReasonPhrase(long status);
};

#endif
