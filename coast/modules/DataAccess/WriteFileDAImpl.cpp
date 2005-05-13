/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "WriteFileDAImpl.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "System.h"
#include "SysLog.h"
#include "Dbg.h"

//--- WriteFileDAImpl -----------------------------------------------------
RegisterDataAccessImpl(WriteFileDAImpl);

WriteFileDAImpl::WriteFileDAImpl(const char *name)
	: FileDAImpl(name)
{
	StartTrace(WriteFileDAImpl.Ctor);
}

WriteFileDAImpl::~WriteFileDAImpl()
{
	StartTrace(WriteFileDAImpl.Dtor);
}

IFAObject *WriteFileDAImpl::Clone() const
{
	StartTrace(WriteFileDAImpl.Clone);

	return new WriteFileDAImpl(fName);
}

bool WriteFileDAImpl::Exec( Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(WriteFileDAImpl.Exec);

	String filename, extension;
	GetFileName(filename, extension, context, in);
	Trace("Writing file " << filename << "." << extension);

	// we need some special logic for std-ios because there is no more ios::noreplace flag...
	// check if neither trunc nor app flag is set (this is noreplace mode)
	if ( ( GetMode(context, in) & (ios::trunc | ios::app ) ) == 0 ) {
		istream *ifp = System::OpenIStream(filename, extension);
		if (ifp != NULL) {
			delete ifp;
			String strErr("File exists! [");
			strErr << filename << (extension.Length() ? "." : "") << extension << "]";
			SYSWARNING(strErr);
			return false;
		}
	}
	ostream *fp = GetFileStream(context, in);
	if (!fp) {
		String strErr("Open for writing of [");
		SYSERROR(strErr << filename << (extension.Length() ? "." : "") << extension << "] failed!");
		return false;
	}
	in->Get("FileContent", *fp, context);

	delete fp;

	return true;
}

int WriteFileDAImpl::DoGetMode(ROAnything roaModes)
{
	StartTrace(WriteFileDAImpl.DoGetMode);
	int mode = FileDAImpl::DoGetMode(roaModes) | ios::out;
	Trace("mode so far:" << (long)mode);
	if ( roaModes.Contains("truncate") ) {
		mode |= ios::trunc;
		Trace("truncating existing file");
	} else if ( roaModes.Contains("append") ) {
		mode |= ios::app;
		Trace("appending to existing file");
	} else { // ( roaModes.Contains("noreplace") )
		mode &= ~(ios::app | ios::trunc);
		Trace("not overwriting existing file (default)");
	}
	Trace("new mode:" << (long)mode);
	return mode;
}
