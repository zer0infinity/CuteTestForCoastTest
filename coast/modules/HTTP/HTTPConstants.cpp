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

namespace coast {
	namespace http {
		void putValuesOnSameLine(std::ostream &os, Context &ctx, String const& slotname, ROAnything const &values) {
			StartTrace(Coast.HTTP.putValuesOnSameLine);
			TraceAny(values, "Header[" << slotname << "]");
			AnyExtensions::Iterator<ROAnything> entryIter(values);
			ROAnything entry;
			String valueSlotname;
			bool first = true;
			bool isCookie = (slotname == constants::cookieSlotname || slotname == constants::contentDispositionSlotname);
			char const argumentsDelimiter = ( isCookie ? constants::headerCookieArgumentsDelimiter : constants::headerArgumentsDelimiter );
			os << slotname << constants::headerNameDelimiter << constants::space;
			while (entryIter.Next(entry)) {
				if ( not first ) os << argumentsDelimiter << constants::space;
				if (isCookie && entryIter.SlotName(valueSlotname)) {
					os << valueSlotname << constants::headerCookieValueDelimiter;
				}
				Renderer::Render(os, ctx, entry);
				first = false;
			}
			os << constants::newLine;
		}
		void putValuesOnMultipleLines(std::ostream &os, Context &ctx, String const& slotname, ROAnything const &values) {
			StartTrace(Coast.HTTP.putValuesOnMultipleLines);
			TraceAny(values, "Header[" << slotname << "]");
			AnyExtensions::Iterator<ROAnything> entryIter(values);
			ROAnything entry;
			while (entryIter.Next(entry)) {
				os << slotname << constants::headerNameDelimiter << constants::space;
				Renderer::Render(os, ctx, entry);
				os << constants::newLine;
			}
		}
		void putHeaderFieldToStream(std::ostream &os, Context &ctx, String const &slotname, ROAnything const &values) {
			RE multivalueRE(constants::splitFieldsRegularExpression, RE::MATCH_ICASE);
			if ( slotname.IsEqual(constants::cookieSlotname) || multivalueRE.ContainedIn(slotname) || slotname.IsEqual(constants::contentDispositionSlotname) ) {
				putValuesOnSameLine(os, ctx, slotname, values);
			} else {
				putValuesOnMultipleLines(os, ctx, slotname, values);
			}
		}
	}
}
