/*
 * Copyright (c) 2011, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 *
 * Author(s): m1huber
 */

#include "HTTPConstants.h"
#include "AnyIterators.h"
#include "Renderer.h"
#include "RE.h"

namespace Coast {
	namespace HTTP {
		void putValuesOnSameLine(std::ostream &os, Context &ctx, String const& slotname, ROAnything const &values) {
			StartTrace(Coast.HTTP.putValuesOnSameLine);
			TraceAny(values, "Header[" << slotname << "]");
			AnyExtensions::Iterator<ROAnything> entryIter(values);
			ROAnything entry;
			String valueSlotname;
			bool first = true;
			bool isCookie = (slotname == _COOKIE || slotname == _CONTENTDISPOSITIONSLOTNAME);
			char const argumentsDelimiter = ( isCookie ? _headerCookieArgumentsDelimiter : _headerArgumentsDelimiter );
			os << slotname << _headerNameDelimiter << _whiteSpace;
			while (entryIter.Next(entry)) {
				if ( not first ) os << argumentsDelimiter << _whiteSpace;
				if (isCookie && entryIter.SlotName(valueSlotname)) {
					os << valueSlotname << _headerCookieValueDelimiter;
				}
				Renderer::Render(os, ctx, entry);
				first = false;
			}
			os << _newLine;
		}
		void putValuesOnMultipleLines(std::ostream &os, Context &ctx, String const& slotname, ROAnything const &values) {
			StartTrace(Coast.HTTP.putValuesOnMultipleLines);
			TraceAny(values, "Header[" << slotname << "]");
			AnyExtensions::Iterator<ROAnything> entryIter(values);
			ROAnything entry;
			while (entryIter.Next(entry)) {
				os << slotname << _headerNameDelimiter << _whiteSpace;
				Renderer::Render(os, ctx, entry);
				os << _newLine;
			}
		}
		void putHeaderFieldToStream(std::ostream &os, Context &ctx, String const &slotname, ROAnything const &values) {
			RE multivalueRE(_httpSplitFieldsRegularExpression, RE::MATCH_ICASE);
			if ( slotname.IsEqual(_COOKIE) || multivalueRE.ContainedIn(slotname) || slotname.IsEqual(_CONTENTDISPOSITIONSLOTNAME) ) {
				putValuesOnSameLine(os, ctx, slotname, values);
			} else {
				putValuesOnMultipleLines(os, ctx, slotname, values);
			}
		}
	}
}
