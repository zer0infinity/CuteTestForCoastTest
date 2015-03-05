/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "FormattedStringRenderer.h"
#include "Tracer.h"

#include "utf8.h"
#include <iterator>
#include <algorithm>

namespace {
	long getStringLength(String const &str) {
		long len = 0L;
		try {
			len = utf8::distance(str.begin(), str.end());
		} catch (utf8::invalid_utf8& e) {
			len = str.Length();
		}
		StatTrace(FormattedStringRenderer.getStringLength, "len: " << len << " str [" << str << "]", coast::storage::Current());
		return len;
	}
	String& trim(String &str, const long newlen){
		String::iterator it = str.begin();
		try {
			utf8::advance(it, newlen, str.end());
		} catch (utf8::invalid_utf8& e) {
			std::advance(it, newlen);
		}
		String::difference_type len = std::distance(str.begin(), it);
		str.Trim(len);
		StatTrace(FormattedStringRenderer.trim, "trimlen: " << newlen << " effective len: " << len << " str [" << str << "]", coast::storage::Current());
		return str;
	}
	struct whitespaceReplacer {
		String &str;
		String const& filler;
		long const fillersForSpaces;
		whitespaceReplacer(String &result, String const& replacement, long const nSpaces) :
				str(result), filler(replacement), fillersForSpaces(nSpaces) {
		}
		void operator()(String::value_type ch) {
			if (ch == ' ') {
				str << filler;
			} else if (ch == '\t') {
				for (long l = 0; l < fillersForSpaces; ++l) {
					str << filler;
				}
			} else {
				str << ch;
			}
		}
	};
}

RegisterRenderer(FormattedStringRenderer);

void FormattedStringRenderer::RenderAll(std::ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(FormattedStringRenderer.RenderAll);
	TraceAny(config, "config");

	String align, value, filler;
	long nSpaces = 4;
	ROAnything roaSlotConfig;
	if (config.LookupPath(roaSlotConfig, "Value")) {
		RenderOnString(value, c, roaSlotConfig);
	} else {
		reply << "Error in FormattedStringRenderer::RenderAll, Value not defined";
		return;
	}
	Trace("Value: [" << value << "]");
	if (config.LookupPath(roaSlotConfig, "Align")) {
		RenderOnString(align, c, roaSlotConfig);
	} else {
		align = "left";
	}
	Trace("Align: [" << align << "]");
	if (config.LookupPath(roaSlotConfig, "Filler")) {
		RenderOnString(filler, c, roaSlotConfig);
	} else {
		filler = " ";
	}
	Trace("Filler: [" << filler << "]");
	long valueLen = getStringLength(value);
	long width = valueLen;
	if (config.LookupPath(roaSlotConfig, "Width")) {
		String sWidth;
		RenderOnString(sWidth, c, roaSlotConfig);
		width = sWidth.AsLong(valueLen);
	}
	Trace("Width: " << width);
	if (config.LookupPath(roaSlotConfig, "SpacesForTab")) {
		String sWidth;
		RenderOnString(sWidth, c, roaSlotConfig);
		nSpaces = sWidth.AsLong(nSpaces);
	}
	Trace("SpacesForTab: " << nSpaces);

	String result(128);
	long lLeftFill = 0, lRightFill = 0;
	// string is longer than field width, trim string
	if ( valueLen > width ) {
		valueLen = getStringLength(trim(value, width ));
	} else {
		if ( align.IsEqual("right") ) {
			lLeftFill = width - valueLen;
		} else if ( align.IsEqual("center") ) {
			lLeftFill = ( width - valueLen ) / 2L;
			lRightFill = width - valueLen - lLeftFill;
		} else {
			// default: left alignment
			lRightFill = width - valueLen;
		}
	}
	Trace("LeftFills: " << lLeftFill << ", RightFill : " << lRightFill );
	for ( long iL = 0; iL < lLeftFill; ++iL) {
		result << filler;
	}
	std::for_each(value.begin(), value.end(), whitespaceReplacer(result, filler, nSpaces));
	for ( long iR = 0; iR < lRightFill; ++iR ) {
		result << filler;
	}
	Trace("Rendered Value: [" << result << "]");
	reply << result;
}
