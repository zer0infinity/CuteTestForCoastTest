/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "HTMLParser.h"
#include "StringStream.h"
#include "Context.h"
#include "Dbg.h"

//--- interface include --------------------------------------------------------
#include "SortedTagsHTMLMapper.h"

//---- SortedTagsHTMLMapper -----------------------------------------------------------
RegisterResultMapper(SortedTagsHTMLMapper);

bool SortedTagsHTMLMapper::DoPutStream(const char *key, istream &is, Context &ctx, ROAnything)
{
	// config is ignored
	StartTrace1(SortedTagsHTMLMapper.PutStream, NotNull(key));

	bool analyseReply = ctx.Lookup("SortedTagsHTMLMapperAnalyseReply", 0L);

	if ( !analyseReply ) {
		analyseReply = (ctx.Lookup("IsAbsPath", 0L) == 0);
	}

	Trace("analyseReply is: " << analyseReply);
	if ( key && analyseReply) {
		AAT_HTMLReader mr(&is);
		Anything fAllReq;
		MyHTMLWriter mw(fAllReq);
		AAT_StdHTMLParser p( mr, mw );

		// now we may read the HTML-Tags
		p.IntParse();
		mw.Flush();

		TraceAny(fAllReq, "Parsed HTML");
		return (  DoFinalPutAny(key, fAllReq, ctx) &&  !mw.GetReqFailed() );
	}

	return (!analyseReply ? DoFinalPutStream("Output", is, ctx) : false);
}

bool SortedTagsHTMLMapper::Put(const char *key, const String &value, Context &ctx)
{
	StartTrace1(SortedTagsHTMLMapper.PutString, NotNull(key));

	IStringStream is(value);
	return ResultMapper::Put(key, is, ctx);
}
