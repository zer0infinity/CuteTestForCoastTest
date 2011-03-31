/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ReadXMLFileDAImpl.h"
#include "GenericXMLParser.h"
#include "Dbg.h"

//--- ReadXMLFileDAImpl -----------------------------------------------------
RegisterDataAccessImpl(ReadXMLFileDAImpl);

ReadXMLFileDAImpl::ReadXMLFileDAImpl(const char *name)
	: ReadFileDAImpl(name)
{
}

ReadXMLFileDAImpl::~ReadXMLFileDAImpl()
{
}

IFAObject *ReadXMLFileDAImpl::Clone(Allocator *a) const
{
	return new (a) ReadXMLFileDAImpl(fName);
}

bool ReadXMLFileDAImpl::Exec( Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(ReadXMLFileDAImpl.Exec);
	bool retVal = false;
	std::istream *Ios = GetFileStream(context, in);
	if (Ios) {
		GenericXMLParser p;
		Anything result = p.Parse(*Ios);
		TraceAny(result, "Parsed XML");
		String targetLoc("ParsedXMLAsAny");
		in->Get("ResultSlotName", targetLoc, context);
		retVal = out->Put(targetLoc, result, context);
		delete Ios;
	}
	return retVal;
}
