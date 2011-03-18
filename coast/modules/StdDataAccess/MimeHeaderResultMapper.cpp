/*
 * Copyright (c) 2010, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "MimeHeaderResultMapper.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "MIMEHeader.h"

//---- MimeHeaderResultMapper ------------------------------------------------------------------
RegisterResultMapper(MimeHeaderResultMapper);

bool MimeHeaderResultMapper::DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything script) {
	StartTrace1(MimeHeaderResultMapper.DoPutStream, "key [" << NotNull(key) << "]");
	MIMEHeader mh(Coast::URLUtils::eUpshift);
	try {
		if (mh.ParseHeaders(is) && is.good()) {
			Anything header(mh.GetInfo());
			TraceAny(header, "header");
			return DoPutAny(key, header, ctx, script);
		}
	} catch (MIMEHeader::StreamNotGoodException &e) {
		;
	}
	return false;
}
