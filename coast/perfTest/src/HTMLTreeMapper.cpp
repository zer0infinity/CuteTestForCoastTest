/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "HTMLTreeMapper.h"

//--- standard modules used ----------------------------------------------------
#include "MultiWriterParser.h"
#include "HTMLTreeWriter.h"
#include "StringStream.h"
#include "Dbg.h"

//---- HTMLTreeMapper -----------------------------------------------------------
RegisterOutputMapper(HTMLTreeMapper);

bool HTMLTreeMapper::DoPutStream(const char *, istream &is, Context &ctx, ROAnything)
{
	// ignore key and config
	StartTrace(HTMLTreeMapper.DoPutStream);

	AAT_HTMLReader mr(&is);
	HTMLTreeWriter mw;
	AAT_StdHTMLParser p( mr, mw );

	// now we may read the HTML-Tags
	p.IntParse();
	mw.Flush();
	Anything fAllReq = mw.PublishResult();
	TraceAny(fAllReq, "Parsed HTML Tree");

	// store under Mapper.HTTPBody
	return DoFinalPutAny("HTTPBody", fAllReq, ctx);
}

//---- HTMLTreeAndSortedTagsMapper -----------------------------------------------------------
RegisterOutputMapper(HTMLTreeAndSortedTagsMapper);

bool HTMLTreeAndSortedTagsMapper::DoPutStream(const char *, istream &is, Context &ctx, ROAnything)
{

	// ignore key and config
	StartTrace(HTMLTreeAndSortedTagsMapper.DoPutStream);

	AAT_HTMLReader mr(&is);
	HTMLTreeWriter mw1;
	Anything reqLinks;
	MyHTMLWriter mw2(reqLinks);
	MultiWriterParser p( mr, mw1, mw2 );

	// now we may read the HTML-Tags
	p.IntParse();
	p.Flush();
	TraceAny(reqLinks, "Parsed HTML Links ");
	Anything fAllReq = mw1.PublishResult();
	TraceAny(fAllReq, "Parsed HTML Tree");
	return DoFinalPutAny("Output", reqLinks, ctx) && DoFinalPutAny("HTTPBody", fAllReq, ctx);
}
