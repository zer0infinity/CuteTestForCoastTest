/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "UTF8Renderer.h"
#include "utf8.h"
#include <algorithm>
#include <iterator>

namespace {
	struct appendFunctor {
		typedef std::back_insert_iterator<String> stringInserter;
		stringInserter backinsert;
		appendFunctor(String& strRet): backinsert(strRet) {}
		void operator()(unsigned char const c) {
			StatTrace(UTF8Renderer.append, "character to add:" << static_cast<long>(c), Coast::Storage::Current());
			utf8::append(static_cast<utf8::uint32_t>(c), backinsert);
		}
	};
	String AsUTF8(String const &strInput) {
		StartTrace(UTF8Renderer.AsUTF8);
		String strRet(strInput.Length());
		std::for_each(strInput.begin(), strInput.end(), appendFunctor(strRet));
		return strRet;
	}
}
//---- UTF8Renderer ---------------------------------------------------------------
RegisterRenderer(UTF8Renderer);

void UTF8Renderer::RenderAll(std::ostream &reply, Context &c, const ROAnything &config) {
	StartTrace(UTF8Renderer.RenderAll);
	ROAnything lookupName;
	if (!config.LookupPath(lookupName, "String")) {
		lookupName = config[0L];
	}
	TraceAny(lookupName, "rendering content to convert");
	reply << AsUTF8(RenderToString(c, lookupName));
}
