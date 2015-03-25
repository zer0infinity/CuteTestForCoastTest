/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SortedTagsHTMLMapper.h"
#include "HTMLParser.h"
#include "StringStream.h"
#include "Context.h"

RegisterResultMapper(SortedTagsHTMLMapper);

bool SortedTagsHTMLMapper::DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything script) {
	StartTrace1(SortedTagsHTMLMapper.DoPutStream, NotNull(key));

	bool analyseReply = ctx.Lookup("SortedTagsHTMLMapperAnalyseReply", 0L);

	if (!analyseReply) {
		analyseReply = (ctx.Lookup("IsAbsPath", 0L) == 0);
	}

	Trace("analyseReply is: " << analyseReply);
	if (key && analyseReply) {
		AAT_HTMLReader mr(&is);
		Anything fAllReq;
		MyHTMLWriter mw(fAllReq);
		AAT_StdHTMLParser p(mr, mw);

		// now we may read the HTML-Tags
		p.IntParse();
		mw.Flush();

		TraceAny(fAllReq, "Parsed HTML");
		return (ResultMapper::DoPutAny(key, fAllReq, ctx, script) && !mw.GetReqFailed());
	}
	return (!analyseReply ? DoFinalPutStream("Output", is, ctx) : false);
}

bool SortedTagsHTMLMapper::DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script) {
	StartTrace1(SortedTagsHTMLMapper.DoPutAny, NotNull(key));
	if (value.GetType() != AnyArrayType) {
		String sText = value.AsString();
		IStringStream is(sText);
		return DoPutStream(key, is, ctx, script);
	}
	return ResultMapper::DoPutAny(key, value, ctx, script);
}
