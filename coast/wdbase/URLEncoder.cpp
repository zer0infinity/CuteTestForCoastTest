/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "URLUtils.h"
#include "Tracer.h"
#include "URLEncoder.h"

// ------------------- URLEncoder ---------------------------------------------
RegisterEncoder(URLEncoder);

URLEncoder::URLEncoder(const char *name) : Encoder(name)
{
}

URLEncoder::~URLEncoder()
{
}

void URLEncoder::DoEncode(String &encStr, const String &str) const
{
	StartTrace(URLEncoder.DoEncode);
	encStr << coast::urlutils::urlEncode(str);
}

bool URLEncoder::DoDecode(String &str, const String &encStr) const
{
	StartTrace(URLEncoder.DoDecode);
	str = coast::urlutils::urlDecode(encStr);
	return true;
}
