/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ReadXMLFileDAImpl.h"

//--- project modules used -----------------------------------------------------
#include "System.h"
#include "GenericXMLParser.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//--- ReadXMLFileDAImpl -----------------------------------------------------
RegisterDataAccessImpl(ReadXMLFileDAImpl);

ReadXMLFileDAImpl::ReadXMLFileDAImpl(const char *name)
	: ReadFileDAImpl(name)
{
}

ReadXMLFileDAImpl::~ReadXMLFileDAImpl()
{
}

IFAObject *ReadXMLFileDAImpl::Clone() const
{
	return new ReadXMLFileDAImpl(fName);
}

bool ReadXMLFileDAImpl::Exec( Context &context, InputMapper *in, OutputMapper *out)
{
	StartTrace(ReadXMLFileDAImpl.Exec);
	bool retVal = false;
	istream *Ios = GetFileStream(context, in);
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
