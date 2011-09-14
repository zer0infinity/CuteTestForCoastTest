/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "HTTPHeaderRenderer.h"
#include "AnyIterators.h"
#include "RE.h"
#include "HTTPConstants.h"

RegisterRenderer(HTTPHeaderRenderer);

namespace {
    const char *_HeaderSlot = "HeaderSlot";
	char const _headerNameDelimiter = ':';
	char const _headerArgumentsDelimiter = ',';
	char const _whiteSpace = ' ';
	char const *_newLine = ENDL;

	void putValuesOnSameLine(std::ostream &reply, Context &ctx, String const& strKey, ROAnything &values) {
		AnyExtensions::Iterator<ROAnything> entryIter(values);
		ROAnything entry;
		bool first = true;
		reply << strKey << _headerNameDelimiter << _whiteSpace;
		while (entryIter.Next(entry)) {
			if ( not first ) reply << _headerArgumentsDelimiter << _whiteSpace;
			Renderer::Render(reply, ctx, entry);
			first = false;
		}
		reply << _newLine;
	}
	void putValuesOnMultipleLines(std::ostream &reply, Context &ctx, String const& strKey, ROAnything &values) {
		AnyExtensions::Iterator<ROAnything> entryIter(values);
		ROAnything entry;
		while (entryIter.Next(entry)) {
			reply << strKey << _headerNameDelimiter << _whiteSpace;
			Renderer::Render(reply, ctx, entry);
			reply << _newLine;
		}
	}
	void RenderHeader(std::ostream &reply, Context &ctx, const ROAnything &config) {
		StartTrace(HTTPHeaderRenderer.RenderHeader);
		//!@FIXME: use precompiled RE-Program as soon as RE's ctor takes an ROAnything as program
		RE multivalueRE(Coast::HTTP::_httpSplitFieldsRegularExpression, RE::MATCH_ICASE);
		AnyExtensions::Iterator<ROAnything> headerStructureIter(config);
		ROAnything fieldValues;
		String strSlotname;
		while (headerStructureIter.Next(fieldValues)) {
			if ( not headerStructureIter.SlotName(strSlotname) ) {
				//! prepared "header: value" entry or a Renderer specification
				Renderer::Render(reply, ctx, fieldValues);
				reply << _newLine;
			} else {
				if ( multivalueRE.ContainedIn(strSlotname) ) {
					putValuesOnSameLine(reply, ctx, strSlotname, fieldValues);
				} else {
					putValuesOnMultipleLines(reply, ctx, strSlotname, fieldValues);
				}
			}
		}
	}
}

void HTTPHeaderRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config) {
	StartTrace(HTTPHeaderRenderer.RenderAll);
	String slotname;
	if (config.IsDefined(_HeaderSlot)) {
		slotname = Renderer::RenderToString(ctx, config[_HeaderSlot]);
	} else {
		slotname = Renderer::RenderToString(ctx, config);
	}
	Trace("using slotname: " << slotname);
	RenderHeader(reply, ctx, ctx.Lookup(slotname));
}
