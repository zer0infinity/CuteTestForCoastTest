/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TruncateStringRenderer.h"
#include "SystemLog.h"
#include "Dbg.h"

//---- TruncateStringRenderer ---------------------------------------------------------------
RegisterRenderer(TruncateStringRenderer);

TruncateStringRenderer::TruncateStringRenderer(const char *name) : Renderer(name) { }

TruncateStringRenderer::~TruncateStringRenderer() { }

void TruncateStringRenderer::RenderAll(std::ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(TruncateStringRenderer.RenderAll);

	TraceAny(config, "config");
	String value, prefix;
	ROAnything roaSlotConfig;

	if (config.LookupPath(roaSlotConfig, "String")) {
		RenderOnString(value, c, roaSlotConfig);
	} else {
		SystemLog::Error("TruncateStringRenderer::RenderAll: String not defined");
		return;
	}
	Trace("String: [" << value << "]");

	if (config.LookupPath(roaSlotConfig, "Token")) {
		RenderOnString(prefix, c, roaSlotConfig);
	} else {
		SystemLog::Error("TruncateStringRenderer::RenderAll: Token not defined");
		return;
	}
	Trace("Token: [" << prefix << "]");

	String direction = config["Direction"].AsString("LR");
	Trace("Direction: [" << direction << "]");

	if ( !direction.IsEqual("LR") &&  !direction.IsEqual("RL") ) {
		SystemLog::Error("TruncateStringRenderer::RenderAll: Wrong direction given");
		return;
	}

	String strPart2Use = config["PartToUse"].AsString("L");
	Trace("PartToUse: [" << strPart2Use << "]");
	if ( !strPart2Use.IsEqual("L") &&  !strPart2Use.IsEqual("R") ) {
		SystemLog::Error("TruncateStringRenderer::RenderAll: Wrong PartToUse given");
		return;
	}

	String strL, strR, temp;
	if (prefix.Length() == 1L) {
		Trace("got char prefix [" << prefix << "]");
		Trace("direction is " << direction);
		long lIdx = -1L;
		if (direction.IsEqual("LR")) {
			lIdx = value.StrChr(prefix[0L]);
		} else {
			lIdx = value.StrRChr(prefix[0L]);
		}
		if (lIdx >= 0L) {
			strL = value.SubString(0, lIdx);
			strR = value.SubString(lIdx + 1L);
		}
	} else {
		Trace("got string prefix [" << prefix << "]");
		Trace("direction is " << direction);
		if (direction.IsEqual("LR")) {
			long lIdx = value.Contains(prefix);
			if (lIdx >= 0L) {
				Trace("token [" << prefix << "] contained in string [" << value << "]");
				strL = value.SubString(0, lIdx);
				strR = value.SubString(lIdx + prefix.Length());
			}
		} else {
			String strTmp = value;
			long lIdx = strTmp.Contains(prefix);
			while (lIdx >= 0L) {
				Trace("token [" << prefix << "] contained in string [" << strTmp << "]");
				strL.Append(strTmp.SubString(0, lIdx));
				strTmp.TrimFront(lIdx + prefix.Length());
				lIdx = strTmp.Contains(prefix);
				if (lIdx >= 0L) {
					strL.Append(prefix);
				} else {
					// now we have got the right hand side part
					strR = strTmp;
				}
				Trace("left  string so far [" << strL << "]");
				Trace("right string so far [" << strR << "]");
			}
		}
	}

	Trace("left  string [" << strL << "]");
	Trace("right string [" << strR << "]");
	if ( strPart2Use == "L" ) {
		Trace("using left part");
		temp = strL;
	} else {
		Trace("using right part");
		temp = strR;
	}

	if (!temp.Length()) {
		ROAnything roaDefaultConfig;
		String strDefault;
		if (config.LookupPath(roaDefaultConfig, "Default")) {
			RenderOnString(strDefault, c, roaDefaultConfig);
		}
		temp = strDefault;
	}
	Trace("string to return [" << temp << "]");
	reply << temp;
}
