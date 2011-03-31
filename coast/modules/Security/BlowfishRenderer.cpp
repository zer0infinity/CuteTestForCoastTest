/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "BlowfishRenderer.h"
#include "Dbg.h"
#include "SecurityModule.h"
#include "Base64.h"
#include "SystemLog.h"

//---- BlowfishRenderer ---------------------------------------------------------------
RegisterRenderer(BlowfishRenderer);

BlowfishRenderer::BlowfishRenderer(const char *name) : Renderer(name) { }

BlowfishRenderer::~BlowfishRenderer() { }

void BlowfishRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(BlowfishRenderer.RenderAll);

	String blowfishSecurityItem, base64ArmouredInput, deArmouredInput, decodedInput;
	RenderOnString(base64ArmouredInput, ctx, config["Base64ArmouredInput"]);
	RenderOnString(blowfishSecurityItem, ctx, config["BlowfishSecurityItem"]);
	Trace("BlowfishSecurityItem: " << blowfishSecurityItem << " Base64ArmouredInput: " << base64ArmouredInput);
	Base64("DecryptAppdecodedInput").DoDecode(deArmouredInput, base64ArmouredInput);
	Scrambler *scrambler = Scrambler::FindScrambler(blowfishSecurityItem);
	String msg;
	msg << "BlowfishRenderer: BlowfishSecurityItem: " << blowfishSecurityItem <<
		" Base64ArmouredInput: " << base64ArmouredInput <<
		" DeArmouredInput: " << deArmouredInput;
	if (scrambler) {
		if (!scrambler->DoDecode(decodedInput, deArmouredInput)) {
			msg << " Couldn't decode Base64ArmouredInput";
			Trace(msg);
			SystemLog::Error(msg);
			reply << msg;
		} else {
			Trace("decoded Input: [" << decodedInput << "]");
			reply << decodedInput;
		}
	} else {
		msg << " BlowfishSecurityItem not configured";
		Trace(msg);
		SystemLog::Error(msg);
		reply << msg;
	}
}
