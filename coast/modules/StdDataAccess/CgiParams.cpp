/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "CgiParams.h"
#include "SystemFile.h"
#include "Dbg.h"
#include "Renderer.h"


//---- CgiParams ------------------------------------------------------------------
RegisterParameterMapper(CgiParams);

CgiParams::CgiParams(const char *name) : URI2FileNameMapper(name)
{
	StartTrace(CgiParams.Ctor);
}

IFAObject *CgiParams::Clone(Allocator *a) const
{
	return new (a) CgiParams(fName);
}

void CgiParams::AddToEnvironment(Context &ctx, Anything &env, ROAnything additionalenv)
{
	StartTrace(CgiParams.AddToEnvironment);

	for (long i = 0, sz = additionalenv.GetSize(); i < sz; ++i) {
		const char *sn = additionalenv.SlotName(i);
		if (sn) {
			env[sn] = Renderer::RenderToString(ctx, additionalenv[sn]);
			Trace("Added: " << sn << "=" << env[sn].AsString(""));
		} // we do not transfer anonymous slots
	}
}

bool CgiParams::DoGetAny(const char *key, Anything &value, Context &ctx,  ROAnything config)
{
	StartTrace1(CgiParams.DoGetAny, "( \"" << NotNull(key) << "\" , Anything &value, Context &ctx, const ROAnything &config)");
	String k(key); // for easier comparison
	if (k == "cgienv") {
		Trace("key found");
		SynthesizeMinimalCGIEnvironment(value, ctx);
		AddToEnvironment(ctx, value, ctx.Lookup("cgienv"));
		AddToEnvironment(ctx, value, Lookup("cgienv")); //!@FIXME double entry with next one?
		AddToEnvironment(ctx, value, config["cgienv"]); // already discriminated when scripted
		return true;
	}
	Trace("unknown key [" << key << "] using ParamterMapper::DoGetAny(key, value, ctx, config)");
	return URI2FileNameMapper::DoGetAny(key, value, ctx, config);
}

bool CgiParams::DoGetStream(const char *key, std::ostream &os, Context &ctx,  ROAnything config)
{
	StartTrace1(CgiParams.DoGetStream, "( \"" << NotNull(key) << "\" , ostream &os, Context &ctx, const ROAnything &config)");
	String k(key); // for easier comparison
	if (k == "stdin") {
		Trace("key found");
		//!@FIXME should use Get("WHOLE_REQUEST_BODY",body,ctx);
		String body = ctx.Lookup("WHOLE_REQUEST_BODY").AsString("");
		if (body.Length() > 0) {
			os << body;
			Trace("Written request body of length: " << body.Length());
		}
		return true;
	}
	Trace("unknown key [" << key << "] using URI2FileNameMapper::DoGetStream(key, os, ctx, config)");
	return URI2FileNameMapper::DoGetStream(key, os, ctx, config);
}

bool CgiParams::ResolveInvalidFile(String &path, String &uri, String &value, Context &ctx)
{
	StartTrace(CgiParams.ResolveInvalidFile);
	Trace("path :" << path << ": uri :" << uri << ":");
	long ixslash = uri.StrChr('/');
	if ( ixslash >= 1) {
		String pgm = path;
		pgm << uri.SubString(0, ixslash - 1);
		if (Coast::System::IsRegularFile(pgm)) {
			value = pgm;
			// should store additional arguments in query....here?
			return true;
		}
	}
	return URI2FileNameMapper::ResolveInvalidFile(path, uri, value, ctx);
}

void CgiParams::SynthesizeMinimalCGIEnvironment(Anything &env, Context &ctx)
{
	StartTrace(CgiParams.SynthesizeMinimalCGIEnvironment);

	env["SERVER_SOFTWARE"] = "Coast/2.0";
	env["SERVER_NAME"] = ctx.Lookup("HostName", "localhost");
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["SERVER_PROTOCOL"] = "HTTP/1.0"; // should be configured somewhere
	env["SERVER_PORT"] = ctx.Lookup("port", 80L);
	env["REQUEST_METHOD"] = ctx.Lookup("REQUEST_METHOD", "GET");

	String p = ctx.Lookup("PATH_INFO", "");
	if (p.Length() > 0) {
		env["PATH_INFO"] = p;
	}

	String name;
	Get("FileName", name, ctx);
	env["SCRIPT_NAME"] = name;
	env["QUERY_STRING"] = ctx.Lookup("QUERY_STRING", "");
	env["header"]["REMOTE_ADDR"] = ctx.Lookup("header.REMOTE_ADDR", "unkown");

	TraceAny(env, "environment set");
}
