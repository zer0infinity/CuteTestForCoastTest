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

namespace Coast {
	namespace HTTP {
		static const char *_httpSplitFieldsRegularExpression = "^(accept|allow|cache-control|connection|content-(encoding|language)|expect|If-None-Match|pragma|Proxy-Authenticate|TE$|trailer|Transfer-Encoding|upgrade|vary|via|warning|WWW-Authenticate)";
		static const char *_httpProtocolVersionSlotname = "HTTPVersion";
		static const char *_httpProtocolCodeSlotname = "ResponseCode";
		static const char *_httpProtocolMsgSlotname = "ResponseMsg";
	}
}

#endif /* HTTPCONSTANTS_H_ */
