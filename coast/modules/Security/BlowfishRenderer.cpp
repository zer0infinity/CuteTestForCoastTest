/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "BlowfishRenderer.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "SecurityModule.h"
#include "Base64.h"
#include "SysLog.h"

//--- c-modules used -----------------------------------------------------------

//---- BlowfishRenderer ---------------------------------------------------------------
RegisterRenderer(BlowfishRenderer);

BlowfishRenderer::BlowfishRenderer(const char *name) : Renderer(name) { }

BlowfishRenderer::~BlowfishRenderer() { }

void BlowfishRenderer::RenderAll(ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(BlowfishRenderer.RenderAll);

	String blowfishSecurityItem, base64ArmouredKey, armouredKey, key;
	RenderOnString(base64ArmouredKey, ctx, config["Base64ArmouredKey"]);
	RenderOnString(blowfishSecurityItem, ctx, config["BlowfishSecurityItem"]);
	Trace("BlowfishSecurityItem: " << blowfishSecurityItem << " Base64ArmouredKey: " << base64ArmouredKey);
	Base64("DecryptAppKey").DoDecode(armouredKey, base64ArmouredKey);
	Scrambler *scrambler = Scrambler::FindScrambler(blowfishSecurityItem);
	String msg;
	msg << "BlowfishRenderer: BlowfishSecurityItem: " << blowfishSecurityItem << " Base64ArmouredKey: " << base64ArmouredKey;
	if (scrambler) {
		if (!scrambler->DoDecode(key, armouredKey)) {
			msg << " Couldn't decode armoured key";
			Trace(msg);
			SysLog::Error(msg);
			reply << msg;
		} else {
			Trace("decoded appKey =" << key);
			reply << key;
		}
	} else {
		msg << " BlowfishSecurityItem not configured";
		Trace(msg);
		SysLog::Error(msg);
		reply << msg;
	}
}
