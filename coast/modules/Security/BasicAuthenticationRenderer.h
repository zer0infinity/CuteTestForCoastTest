/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _BasicAuthenticationRenderer_H
#define _BasicAuthenticationRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_security.h"
#include "Renderer.h"

//---- BasicAuthenticationRenderer ----------------------------------------------------------
/*!
<B>Configuration:</B><PRE>
{
	/User								Rendererspec
	/Password							Rendererspec
	/Scheme								Rendererspec, default is "Basic"
	/BasicAuthorizationHeaderFieldName	Rendererspec, default is "Authorization"
}</PRE>
 Purpose_Begin
	Generate a basic authorization request header field. (RFC 2617)
  Purpose_End
*/
class EXPORTDECL_SECURITY BasicAuthenticationRenderer : public Renderer
{
public:
	//--- constructors
	BasicAuthenticationRenderer(const char *name);
	~BasicAuthenticationRenderer();

	//! Renders a given rendererspec as base64
	//! \param reply out - the stream where the rendered output is written on.
	//! \param ctx the context the renderer runs within.
	//! \param config the configuration of the renderer.
	virtual void RenderAll(ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
