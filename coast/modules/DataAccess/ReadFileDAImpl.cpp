/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ReadFileDAImpl.h"

//--- project modules used -----------------------------------------------------
#include "System.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//--- ReadFileDAImpl -----------------------------------------------------
RegisterDataAccessImpl(ReadFileDAImpl);

ReadFileDAImpl::ReadFileDAImpl(const char *name)
	: FileDAImpl(name)
{
}

ReadFileDAImpl::~ReadFileDAImpl()
{
}

IFAObject *ReadFileDAImpl::Clone() const
{
	return new ReadFileDAImpl(fName);
}

bool ReadFileDAImpl::Exec( Context &context, InputMapper *in, OutputMapper *out)
{
	StartTrace(ReadFileDAImpl.Exec);
	bool retVal = false;
	istream *Ios = GetFileStream(context, in);
	if (Ios) {
		retVal = out->Put("FileContent", *Ios, context);
		delete Ios;
	}
	return retVal;
}
