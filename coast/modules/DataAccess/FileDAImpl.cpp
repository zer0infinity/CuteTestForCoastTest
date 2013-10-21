/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "FileDAImpl.h"
#include "Tracer.h"

using namespace coast;

//--- FileDAImpl -----------------------------------------------------
RegisterDataAccessImpl(FileDAImpl);

IFAObject *FileDAImpl::Clone(Allocator *a) const
{
	return new (a) FileDAImpl(fName);
}

bool FileDAImpl::GetFileName(String &filename, String &ext, Context &context, ParameterMapper *in)
{
	StartTrace(FileDAImpl.GetFileName);

	String name;
	bool ret = true;

	// do not use return codes since it might not be defined
	in->Get("DocumentRoot", filename, context);
	SubTrace(DocumentRoot, "DocumentRoot:<" << filename << ">");

	ret = in->Get("Filename", name, context);
	Trace("filename from context [" << name << "]");
	system::ResolvePath(name);
	Trace("cleaned filename [" << name << "]");
	if (ret) {
		filename << name;
		SubTrace(DocumentRoot, "Filename:<" << name << ">");
		// as extension is optional, do not use its return value
		if (in->Get("Extension", ext, context)) {
			SubTrace(Extension, "Extension:<" << ext << ">");
		}
	}
	Trace("Filename:<" << filename << ">");
	return ret;
}

system::openmode FileDAImpl::GetMode(Context &context, ParameterMapper *in)
{
	StartTrace(FileDAImpl.GetMode);
	system::openmode mode = (system::openmode)0;
	Anything anyModes;
	// do not fail when Get did not find a Mode...
	in->Get("Mode", anyModes, context);
	TraceAny(anyModes, "mapped modes" );
	mode = DoGetMode(anyModes);
	Trace("mode to return:" << (long)mode);
	return mode;
}

system::openmode FileDAImpl::DoGetMode(ROAnything roaModes)
{
	StartTrace(FileDAImpl.DoGetMode);
	system::openmode mode = (system::openmode)0;
	if ( roaModes.Contains("text") ) {
		Trace("text mode");
	} else if ( roaModes.Contains("binary") ) {
		mode |= std::ios::binary;
		Trace("binary mode");
	}
	return mode;
}

std::iostream *FileDAImpl::GetFileStream(Context &context, ParameterMapper *in)
{
	StartTrace(FileDAImpl.GetFileStream);
	std::iostream *pStream = NULL;
	String filename, ext;
	if (GetFileName(filename, ext, context, in)) {
		system::openmode mode = GetMode(context, in);
		pStream = system::OpenStream(filename, ext, mode);
		if (pStream) {
			Trace("Stream opened ok");
		}
	}
	return pStream;
}
