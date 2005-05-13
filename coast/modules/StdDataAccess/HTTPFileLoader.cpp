/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "HTTPFileLoader.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "System.h"
#include "MmapStream.h"
#include "Renderer.h"
#include "Dbg.h"

//--- HTTPFileLoader -----------------------------------------------------
RegisterDataAccessImpl(HTTPFileLoader);

HTTPFileLoader::HTTPFileLoader(const char *name) : ReadFileDAImpl(name)
{
}

HTTPFileLoader::~HTTPFileLoader()
{
}

IFAObject *HTTPFileLoader::Clone() const
{
	return new HTTPFileLoader(fName);
}

bool HTTPFileLoader::GenReplyStatus(Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(HTTPFileLoader.GenReplyHeader);

	Anything statusSpec;

	Anything verSpec;
	verSpec[0L]["ContextLookupRenderer"] = "Mapper.HTTPVersion";
	statusSpec["HTTPVersion"] = verSpec;

	Anything resCodeSpec;
	resCodeSpec[0L]["ContextLookupRenderer"] = "Mapper.ResponseCode";
	statusSpec["ResponseCode"] = resCodeSpec;

	Anything resMsgSpec;
	resMsgSpec[0L]["ContextLookupRenderer"] = "Mapper.ResponseMsg";
	statusSpec["ResponseMsg"] = resMsgSpec;

	return out->Put("HTTPStatus", statusSpec, context);
}

bool HTTPFileLoader::GenReplyHeader(Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(HTTPFileLoader.GenReplyHeader);

	GenReplyStatus(context, in, out);

	Anything contentLengthSpec;
	contentLengthSpec[0L] = "Content-Length: ";
	contentLengthSpec[1L]["ContextLookupRenderer"] = "Mapper.content-length";
	contentLengthSpec[2L] = ENDL;

	Anything condSpec;
	condSpec["ContextCondition"] = "Mapper.content-length";
	condSpec["Defined"]   = contentLengthSpec;

//FIXME: allow for content-compression...

	Anything headerSpec;
	headerSpec[0L] = "Content-Type: ";
	headerSpec[1L]["ContextLookupRenderer"] = "Mapper.content-type";
	headerSpec[2L] = ENDL;
	headerSpec[3L]["ConditionalRenderer"] = condSpec;
	SubTraceAny(HTTPHeader, headerSpec, "HTTPHeader:");
	return out->Put("HTTPHeader", headerSpec, context);
}

bool HTTPFileLoader::Exec( Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(HTTPFileLoader.Exec);
	bool retVal = true;
	String filename;

	context.Push("HTTPFileLoader", this);

	retVal = in->Get("FileName", filename, context);
	SubTrace(FileName, "FileName:<" << filename << ">");

	retVal = GenReplyHeader(context, in, out) && retVal;
	retVal = out->Put("HTTPVersion", String("HTTP/1.1"), context) && retVal; // PS Fix binary &

	if ( retVal ) {
		retVal = ProcessFile(filename, context, in, out);
	}

	if (!retVal) {
		ProduceErrorReply(filename, context, in, out);
	}
	context.Remove("HTTPFileLoader");
	return retVal;
}

void HTTPFileLoader::ProduceErrorReply(const String &filename, Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace1(HTTPFileLoader.ProduceErrorReply, "Filename: >" << filename << "<");

	long errorCode(context.Lookup("HTTPError", 400L));
	String errormsg(context.Lookup("HTTPResponse", String("Bad Request")));
	String errorReply;

	errorReply << "<html><head>\n";
	errorReply << "<title>" << errorCode << " " << errormsg << "</title>\n";
	errorReply << "</head><body>\n";
	errorReply << "<h1>" << errormsg << "</h1>\n";

	String taintedRequest(context.Lookup("REQUEST_URI", "/"));
	Anything unTaintRConf;
	unTaintRConf["UnTaintRenderer"]["ToClean"] = taintedRequest;
	errorReply << "<p>The requested URL <b>" << Renderer::RenderToString(context, unTaintRConf) << "</b> is invalid.</p>\n";
	errorReply << "<hr />\n";
	errorReply << "<address>Coast 2.0 Server</address>\n";
	errorReply << "</body></html>\n";

	Trace("errorCode :" << errorCode );
	Trace("errorMsg :" << errormsg );

	out->Put("ResponseCode", errorCode, context);
	out->Put("ResponseMsg", errormsg, context);
	out->Put("content-type", String("text/html"), context);
	IStringStream is(errorReply);
	out->Put("HTTPBody", is, context);

	TraceAny(context.GetTmpStore()["Mapper"], "Error handling");
}

bool HTTPFileLoader::ProcessFile(const String &filename, Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace1(HTTPFileLoader.ProcessFile, "Filename: >" << filename << "<");

	bool retVal = true;
	iostream *Ios = 0;
	String ext;
	Ios = System::OpenStream(filename, ext, ios::in | ios::binary);
	if (Ios) {
		Trace("Stream opened ok");
		retVal = out->Put("ResponseCode", 200L, context) && retVal;
		retVal = out->Put("ResponseMsg", String("Ok"), context) && retVal;

		long posDot = filename.StrRChr('.');
		if ( posDot != -1 ) {
			ext = filename.SubString(posDot + 1, filename.Length());
		}
		String ctquery("Ext2MIMETypeMap");
		ctquery << '.' << ext;
		retVal = out->Put("content-type", String(context.Lookup(ctquery, "text/plain")), context) && retVal;

#if !defined(WIN32)
		MmapStream *fs = (MmapStream *)Ios;
		long sz = fs->rdbuf()->Length();
		retVal = out->Put("content-length", sz, context) && retVal;
#endif
		retVal = out->Put("HTTPBody", (*(istream *)Ios), context) && retVal;
		delete Ios;
	} else {
		retVal = false;
		Anything tmpStore(context.GetTmpStore());
		tmpStore["HTTPError"] = 403L;
		tmpStore["HTTPResponse"] = "Forbidden";
	}
	return retVal;
}
