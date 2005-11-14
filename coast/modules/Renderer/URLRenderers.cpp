/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include ----------------------------------------------------------------
#include "URLRenderers.h"

//--- standard modules used ----------------------------------------------------
#include "SecurityModule.h"
#include "URLUtils.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- LinkRenderer ----------------------------------------------------------------

RegisterRenderer(LinkRenderer);

LinkRenderer::LinkRenderer(const char *name) : Renderer(name)
{
}

void LinkRenderer::RenderAll(ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(LinkRenderer.Render);
	TraceAny(config, "config");

	reply << "<a href=\"";

	// generate URL string
	Renderer *r = Renderer::FindRenderer("URLRenderer");
	if (r) {
		r->RenderAll(reply, c, config);
	}

	// end of quoted URL
	reply << "\" ";
	PrintOptions3(reply, c, config);
	// end of HREF statement
	reply << (">");

	// insert label
	if (config.IsDefined("Label")) {
		Render(reply, c, config["Label"]);
	}

	// end of link
	reply << ("</a>");
}

//---- URLPrinter -------------------------------------------------------
RegisterRenderer(URLPrinter);

URLPrinter::URLPrinter(const char *name) : Renderer(name)
{

}

URLPrinter::URLPrinter(const char *name, char cmdSep, char argSep, char entrySep) : Renderer(name), fArgSep(argSep), fCmdSep(cmdSep), fEntrySep(entrySep)
{
}

void URLPrinter::RenderAll(ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(URLPrinter.Render);
	TraceAny(config, "config");

	// render the state information
	RenderState(reply, c, config);
}

void URLPrinter::RenderState(ostream &reply, Context &c, const ROAnything &config)
{
	Anything state;

	// collect the state
	GetState(state, c);

	// render the public part of the URL
	RenderPublicState(reply, c, config, state);

	// render the private part (encoded) of the URL
	RenderPrivateState(reply, c, config, state);
}

void URLPrinter::RenderScriptName(ostream &reply, Context &context)
/* in: env: The environment of the current request
  out: reply: The name of the script to access the server
 what: Determines the script name based on the environment.
	   The entry /ServicePrefix in the config can overrule this behavior.
*/
{
	ROAnything env(context.GetEnvStore());
	ROAnything scriptName = context.Lookup("ServicePrefix");

	if (scriptName.IsNull()) {
		env.LookupPath(scriptName, "SCRIPT_NAME");
	} // if

	String s;
	RenderOnString(s, context, scriptName);
	if (!s.Length()) {
		s = "wdgateway";
	}

	reply << s << CmdSep();
	// fallback is valid for no base URL's, otherwise we assume the user knows
	// what he is doing.
} // RenderScriptName

void URLPrinter::RenderPublicPartOfURL(ostream &reply, Context &c, const ROAnything &config, Anything &state)
{
	RenderScriptName(reply, c);

	// renders the public information of the URL
	const char *adr = 0;
	const char *port = 0;

	if (state.IsDefined("adr")) {
		adr = state["adr"].AsCharPtr(0);
		if (adr) {
			reply << "adr=" << adr;
			state.Remove("adr");
		}
	}
	if (state.IsDefined("port")) {
		port = state["port"].AsCharPtr(0);
		if (port) {
			if ( adr ) {
				reply <<  ArgSep();
			}
			reply << "port=" << port;
			state.Remove("port");
		}
	}
	//	CR #9
	// 	if no port and adr are present, there is an argument separator too much
	//	but insert the argument separator here if there is either because otherwise
	//	there is always a the CmdSep
	if (port || adr) {
		reply <<  ArgSep();
	}
}

void URLPrinter::RenderPublicState(ostream &reply, Context &c, const ROAnything &config, Anything &state)
{
	StartTrace (URLPrinter.RenderPublicState);

	ROAnything env(c.GetEnvStore());

	ROAnything bAddr;

	if ( config.LookupPath(bAddr, "BaseAddr") ) {
		String baseAddr;
		RenderOnString(baseAddr, c, bAddr);
		reply << baseAddr;
		Trace("BaseAddr :" << baseAddr);
	}

	RenderPublicPartOfURL(reply, c, config, state);
}

void URLPrinter::RenderPrivateState(ostream &, Context &, const ROAnything &, Anything &)
{
}

void URLPrinter::BuildPrivateState(ostream &, Context &c, const ROAnything &config,  Anything &state)
{
	StartTrace(URLPrinter.BuildPrivateState);
	TraceAny(config, "config");
	// append action from config
	ROAnything a(config["Action"]);
	String action;
	RenderOnString(action, c, a);

	if (action.Length() > 0) {
		state["action"] = action;
	}

	// parameter list
	ROAnything params;
	if (config.LookupPath(params, "Params")) {
		for (long i = 0, sz = params.GetSize(); i < sz; ++i) {
			String name = params.SlotName(i);
			if (name.IsEqual("")) {
				name = "P";
				name.Append(long(i));
			}

			// allow renderer specification
			String value;
			RenderOnString(value, c, params[i]);

			Trace("name = " << name << " value = " << value);

			state[name] = value;
		}
	}
}

void URLPrinter::AppendEncodedState(ostream &reply, Context &c, const Anything &state, const char *argName)
{
	StartTrace(URLPrinter.AppendEncodedState);
	// encode state string
	String s(argName);
	s.Append("=");
	SecurityModule::ScrambleState(s, state);
	Trace("scrambled state [" << s << "]");
	if ( URLUtils::CheckUrlEncoding(s) ) {
		reply << s;
	} else {
		// now we need to encode the string according to RFC1738/1808
		String strEncoded = URLUtils::urlEncode(s);
		Trace("state after additional encoding [" << strEncoded << "]");
		reply << strEncoded;
	}
}

void URLPrinter::GetState(Anything &state, Context &c)
{
	c.CollectLinkState(state);
}

//---- FullURLPrinter -------------------------------------------------------
RegisterRenderer(FullURLPrinter);

FullURLPrinter::FullURLPrinter(const char *name) : URLPrinter(name, '?', '&', ' ')
{
}

void FullURLPrinter::RenderPrivateState(ostream &reply, Context &c, const ROAnything &config, Anything &state)
{
	StartTrace(FullURLPrinter.RenderPrivateState);
	TraceAny(config, "config");

	// first append a seperator
	// reply << ArgSep(); CR #9
	// don't use an ArgSep() here, since you don't know what's done upstream

	// append action etc specific to this URL
	BuildPrivateState(reply, c, config, state);

	// append encoded arguments under a single name
	AppendEncodedState(reply, c, state, "X");
}

//---- BaseURLRenderer -------------------------------------------------------
RegisterRenderer(BaseURLRenderer);

BaseURLRenderer::BaseURLRenderer(const char *name) : BaseURLPrinter(name)
{
}

void BaseURLRenderer::RenderAll(ostream &reply, Context &context, const ROAnything &config)
{
	if (context.Lookup("UseBaseURL").AsLong(0L) != 0L) {
		reply << "<base href=\"";
		BaseURLPrinter::RenderAll(reply, context, config);
		reply << "\"/>\n";
	}
}

//---- BaseURLPrinter -------------------------------------------------------
RegisterRenderer(BaseURLPrinter);

BaseURLPrinter::BaseURLPrinter(const char *name) : URLPrinter(name, '/', '/', '/')
{
}

void BaseURLPrinter::RenderPrivateState(ostream &reply, Context &c, const ROAnything &config, Anything &state)
{
	StartTrace(BaseURLPrinter.RenderPrivateState);
	// append encoded arguments under a single name
	AppendEncodedState(reply, c, state, "X1");
	reply << ArgSep();
}

void BaseURLPrinter::RenderPublicState(ostream &reply, Context &c, const ROAnything &config, Anything &state)
{
	StartTrace (URLPrinter.RenderPublicState);
	TraceAny(config, "config");

	ROAnything bAddr;

	if ( config.LookupPath(bAddr, "BaseAddr") ) {
		String baseAddr;
		RenderOnString(baseAddr, c, bAddr);
		reply << baseAddr;
		Trace("BaseAddr :" << baseAddr);
	} else {
		bAddr = c.Lookup("BaseAddress");
		String baseAddr;
		RenderOnString(baseAddr, c, bAddr);
		Trace("BaseAddr :" << baseAddr);

		if ( baseAddr.Length() ) {	// use base address defined in Config.any
			reply << baseAddr;
		} else {
			// generate an absolute address from environment info
			ROAnything env(c.GetEnvStore());
			reply << "http" << ((c.Lookup("header.HTTPS", 0L)) ? "s" : "") << "://" << env["header"]["HOST"].AsCharPtr("");
		}
	}
	RenderPublicPartOfURL(reply, c, config, state);
}

//---- SimpleURLPrinter -------------------------------------------------------
RegisterRenderer(SimpleURLPrinter);

SimpleURLPrinter::SimpleURLPrinter(const char *name) : URLPrinter(name, '/', '/', '/')
{
}

void SimpleURLPrinter::RenderState(ostream &reply, Context &c, const ROAnything &config)
{
	Anything state;
	// render only the private state for a simple URL
	RenderPrivateState(reply, c, config, state);
}

void SimpleURLPrinter::RenderPrivateState(ostream &reply, Context &c, const ROAnything &config, Anything &state)
{
	StartTrace(SimpleURLPrinter.RenderPrivateState);
	TraceAny(config, "config");

	// append action etc specific to this URL
	BuildPrivateState(reply, c, config, state);

	// append encoded arguments under a single name
	AppendEncodedState(reply, c, state, "X2");
}

//---- URLRenderer ----------------------------------------------------------------
RegisterRenderer(URLRenderer);

URLRenderer::URLRenderer(const char *name) : Renderer(name)
{
}

void URLRenderer::RenderAll(ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(URLRenderer.Render);
	TraceAny(config, "config");

	bool useBaseURL = (c.Lookup("UseBaseURL", 0L) != 0);
	Renderer *r;

	if (useBaseURL && !config.IsDefined("BaseAddr")) {
		r = FindRenderer("SimpleURLPrinter");
		r->RenderAll(reply, c, config);
	} else {
		r = FindRenderer("FullURLPrinter");
		r->RenderAll(reply, c, config);
	}
}

String URLRenderer::CreateIntraPageLink(Context &c, String id)
{
	// creates intra page links that also work with the
	// base URL feature.. (In the presence of a base URL absolute URLs
	// need to be generated. Otherwise the link will not be recognized
	// as such by the browser.)
	// Note: As to improve the performance for the duration of one request
	// a necessary intermediary URL is cached in tmpStore at slot 'ABSOLUTE_URL'

	bool useBaseURL = (c.Lookup("UseBaseURL", 0L) != 0);
	String path;
	if ( useBaseURL ) {	// use BaseURL if defined in Config.any
		Anything env = c.GetEnvStore();
		Anything tmpStore = c.GetTmpStore();

		// create the absolute URL of the current document
		Anything absURL;
		if (!tmpStore.LookupPath(absURL, "ABSOLUTE_URL")) {

			// reconstruct actual referer URL

			const char *baseAddr = c.Lookup("BaseAddress", (const char *)0);

			path = baseAddr;
			path << env["SCRIPT_NAME"].AsCharPtr("/wdgateway");
			path << env["PATH_INFO"].AsCharPtr("");

			tmpStore["ABSOLUTE_URL"] = path;	// cache it in tmpStore

		} else {
			path = absURL.AsString();
		}

		// append actual target
		path << "#" << id;

		// append query (this is necessary for the browser to recognize
		// that its an intra page link..)

		if (env.IsDefined("QUERY_STRING")) {
			String queryString = env["QUERY_STRING"].AsString();
			if (queryString.Length()) {
				path << "?" << queryString;
			}
		}
		return path;
	}

	// without base URLs the intra page link is trivial:
	path << "#" << id;

	return path;
}
