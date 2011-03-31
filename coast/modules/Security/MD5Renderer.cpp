/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "MD5Renderer.h"
#include "Dbg.h"
#include "MD5.h"
#include "Base64.h"

//---- MD5Renderer ---------------------------------------------------------------
RegisterRenderer(MD5Renderer);

MD5Renderer::MD5Renderer(const char *name) : Renderer(name) { }

MD5Renderer::~MD5Renderer() { }

void MD5Renderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(MD5Renderer.RenderAll);
	String textToHash, resultingHash, b64EncodedText;
	TraceAny(config, "config");
	Renderer::RenderOnString(textToHash, ctx, config["ToSign"]);
	MD5Signer::DoHash(textToHash, resultingHash);
	if ( config["Mode"].AsString("Base64") == "Base64" ) {
		Base64Regular(fName).DoEncode(b64EncodedText, resultingHash);
		reply <<  b64EncodedText;
	}
	if ( config["Mode"].AsString() == "Hex" ) {
		String asHex;
		asHex.AppendAsHex((const unsigned char *)(const char *)resultingHash, resultingHash.Length());
		reply << asHex;
	}
}
