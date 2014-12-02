/*
 * Copyright (c) 2011, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 *
 * Author(s): m1huber
 */

#ifndef HTTPCONSTANTS_H_
#define HTTPCONSTANTS_H_
#include <iosfwd>
#include "foundation.h"		//!< for ENDL
class ROAnything;
class String;
class Context;

namespace coast {
	namespace http {
		namespace constants {
			char const * const splitFieldsRegularExpression = "^(accept|allow|cache-control|connection|content-(encoding|language)|expect|If-None-Match|pragma|Proxy-Authenticate|TE$|trailer|Transfer-Encoding|upgrade|vary|via|warning|WWW-Authenticate)";
			char const * const protocolVersionSlotname = "HTTPVersion";
			char const * const protocolCodeSlotname = "ResponseCode";
			char const * const protocolMsgSlotname = "ResponseMsg";
			char const * const newLine = ENDL;
			char const * const contentTypeAnything = "application/x-coast-anything";
			char const * const contentTypeMultipart = "multipart/part";
			char const * const contentDispositionSlotname = "CONTENT-DISPOSITION";
			char const * const contentTypeSlotname = "CONTENT-TYPE";
			char const * const contentLengthSlotname = "CONTENT-LENGTH";
			char const * const boundarySlotname = "BOUNDARY";
			char const * const cookieSlotname = "COOKIE";
			char const * const setCookieSlotname = "SET-COOKIE";

			char const * const getMethodSlotname = "GET";
			char const * const postMethodSlotname = "POST";

			char const headerNameDelimiter = ':';
			char const headerArgumentsDelimiter = ',';
			char const headerCookieArgumentsDelimiter = ';';
			char const headerCookieValueDelimiter = '=';
			char const contentDispositionDelimiter = ';';
			char const keyValueDelimiter = '=';
			char const space = ' ';
		}
		void putValuesOnSameLine(std::ostream &os, Context &ctx, String const& slotname, ROAnything const &values);
		void putValuesOnMultipleLines(std::ostream &os, Context &ctx, String const& slotname, ROAnything const &values);
		void putHeaderFieldToStream(std::ostream &os, Context &ctx, String const &slotname, ROAnything const &values);
	}
}

#endif /* HTTPCONSTANTS_H_ */
