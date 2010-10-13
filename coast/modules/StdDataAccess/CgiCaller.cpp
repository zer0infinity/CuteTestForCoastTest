/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "StringStream.h"
#include "PipeExecutor.h"
#include "Context.h"
#include "Dbg.h"

//--- interface include --------------------------------------------------------
#include "CgiCaller.h"

//--- CgiCaller -----------------------------------------------------
RegisterDataAccessImpl(CgiCaller);

CgiCaller::CgiCaller(const char *name) : HTTPFileLoader(name)
{

}

CgiCaller::~CgiCaller()
{

}

IFAObject *CgiCaller::Clone(Allocator *a) const
{
	return new (a) CgiCaller(fName);
}

bool CgiCaller::GenReplyHeader(Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(HTTPFileLoader.GenReplyHeader);

	GenReplyStatus(context, in, out);

	Anything headerSpec;
	headerSpec[0L] = ""; // cgi provides its own header and ENDL ENDL
	SubTraceAny(HTTPHeader, headerSpec, "HTTPHeader:");
	return out->Put("HTTPHeader", headerSpec, context);
}

void CgiCaller::SplitRChar(const String &full, char sep, String &before, String &after)
{
	StartTrace(CgiCaller.SplitRChar);

	long pos = full.StrRChr(sep);
	if (pos < 0) {
		before = "";
		after = full;
	} else if (pos >= 0) {
		before = full.SubString(0, pos + 1);
		after = full.SubString(pos + 1);
	}
}

void CgiCaller::SplitPath(const String &fullPath, String &path, String &file)
{
	StartTrace1(CgiCaller.SplitPath, "Filename: >" << fullPath << "<");
	SplitRChar(fullPath, '/', path, file);
}

String CgiCaller::GetFileExtension(const String &file)
{
	StartTrace(CgiCaller.GetFileExtension);
	String dummy, ext;
	SplitRChar(file, '.', dummy, ext);
	return ext;
}

bool CgiCaller::ProcessFile(const String &filename, Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace1(CgiCaller.ProcessFile, "Filename: >" << filename << "<");

	String path, file;
	SplitPath(filename, path, file);
	String fileExt(GetFileExtension(file));
	Anything cgiextensions;
	in->Get("cgiextensions", cgiextensions, context);
	if (!cgiextensions.Contains(fileExt)) {
		return HTTPFileLoader::ProcessFile(filename, context, in, out);
	}

	long timeout = 1000L;
	in->Get("CgiTimeout", timeout, context);

	Anything cgienviron;
	bool retVal = in->Get("cgienv", cgienviron, context);
	if (retVal) {
		Trace("calling in path [" << path << "] program [" << file << "]");
		PipeExecutor cgi(filename, cgienviron, path, timeout);
		iostream *ioStream = 0;
		if (cgi.Start() && (ioStream = cgi.GetStream())) {
			// the following is tricky, because of potential pipe blocking
			// if something large is passed, we do not check for now.
			in->Get("stdin", *(ostream *)ioStream, context); // provide cgi's stdin
			cgi.ShutDownWriting();
			retVal = out->Put("ResponseCode", 200L, context) && retVal;
			retVal = out->Put("ResponseMsg", String("Ok"), context) && retVal;
			// call HTTPHeader rendering ?
			retVal = out->Put("HTTPBody", *(istream *)ioStream, context) && retVal; // return cgi's output
			long exitStatus = (long)cgi.TerminateChild();
			Trace("cgi terminated exit status was " << exitStatus);
			// we ignore the childs exit status as the response code has already been set
		} else {
			retVal = false;
			Anything tmpStore(context.GetTmpStore());
			tmpStore["HTTPError"] = 403L;
			tmpStore["HTTPResponse"] = "Forbidden";
			String errorMsg;
			errorMsg << "Content-Type: text/html\n\n<HTML>" << filename << " not allowed</HTML>";
			tmpStore["HTTPErrorHTMLReply"] = errorMsg;
			Trace("failed to call CGI");
		}
	}
	return retVal;
}
