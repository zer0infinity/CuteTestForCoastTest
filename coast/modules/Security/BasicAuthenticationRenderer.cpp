/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "BasicAuthenticationRenderer.h"
#include "Base64.h"
//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "SystemLog.h"

//--- c-modules used -----------------------------------------------------------

//---- BasicAuthenticationRenderer ---------------------------------------------------------------
RegisterRenderer(BasicAuthenticationRenderer);

BasicAuthenticationRenderer::BasicAuthenticationRenderer(const char *name) : Renderer(name) { }

BasicAuthenticationRenderer::~BasicAuthenticationRenderer() { }

void BasicAuthenticationRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(BasicAuthenticationRenderer.RenderAll);
	// see RFC 2617 for details
	String user, password, scheme, proxy;
	Renderer::RenderOnString(user, ctx, config["User"]);
	Renderer::RenderOnString(password, ctx, config["Password"]);
	Renderer::RenderOnString(scheme, ctx, config["Scheme"]);
	Renderer::RenderOnString(proxy, ctx, config["Proxy"]);
	if ( (user.Length() == 0L) || (password.Length() == 0L) ) {
		String msg;
		msg.Append("Authorization: Missing userid/password");
		SystemLog::Error(msg);
		Trace(msg);
		reply << msg;
	}
	if ( proxy.AsLong(0L) == false ) {
		proxy = "Authorization";
	} else {
		proxy = "Proxy-Authorization";
	}
	if ( scheme.Length() == 0L ) {
		scheme = "Basic";
	}
	String userPass;
	userPass << user << ":" << password;
	String b64EncodedText;
	Base64Regular(fName).DoEncode(b64EncodedText, userPass);
	String authHeaderLine;
	authHeaderLine << proxy << ": " << scheme << " " << b64EncodedText;
	Trace("AuthHeaderLine: " << authHeaderLine << " User: [" << user << "] Password [" << password << "]");
	reply << authHeaderLine;
}
