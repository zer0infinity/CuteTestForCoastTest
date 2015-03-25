/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ReadFileDAImpl.h"
#include "Tracer.h"

//--- ReadFileDAImpl -----------------------------------------------------
RegisterDataAccessImpl(ReadFileDAImpl);

ReadFileDAImpl::ReadFileDAImpl(const char *name)
	: FileDAImpl(name)
{
}

ReadFileDAImpl::~ReadFileDAImpl()
{
}

IFAObject *ReadFileDAImpl::Clone(Allocator *a) const
{
	return new (a) ReadFileDAImpl(fName);
}

bool ReadFileDAImpl::Exec( Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(ReadFileDAImpl.Exec);
	bool retVal = false;
	std::istream *Ios = GetFileStream(context, in);
	if (Ios) {
		retVal = out->Put("FileContent", *Ios, context);
		delete Ios;
	}
	return retVal;
}
