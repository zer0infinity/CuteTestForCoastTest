/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//#define PAGE_TRACING // output whole rendered page, incl. protocol header

//--- interface include --------------------------------------------------------
#include "Page.h"

//--- standard modules used ----------------------------------------------------
#include "Renderer.h"
#include "Action.h"
#include "Timers.h"
#include "Registry.h"
#include "HTTPStreamStack.h"
#include "RequestProcessor.h"
#include "Dbg.h"
#ifdef PAGE_TRACING
#include "SystemLog.h"
#endif

//---- PagesModule -----------------------------------------------------------
RegisterModule(PagesModule);

PagesModule::PagesModule(const char *name) : WDModule(name)
{
}

PagesModule::~PagesModule()
{
}

bool PagesModule::Init(const ROAnything config)
{
	if (config.IsDefined("Pages")) {
		HierarchyInstaller hi("Page");
		return RegisterableObject::Install(config["Pages"], "Page", &hi);
	}
	return false;
}

bool PagesModule::ResetFinis(const ROAnything config)
{
	AliasTerminator at("Page");
	return RegisterableObject::ResetTerminate("Page", &at);
}

bool PagesModule::Finis()
{
	return StdFinis("Page", "Pages");
}

//---- Page --------------------------------------------------------------------------
RegisterPage(Page);

Page::Page(const char *title) :  HierarchConfNamed(title)
{
	SetName(title);
}

Page::~Page()
{
}

IFAObject *Page::Clone() const
{
	StartTrace(Page.Clone);
	return new Page(fName);
}

void Page::Mime(ostream &reply, Context &c)
{
	reply << "Content-type: text/html" << ENDL;
}

void Page::Header(ostream &reply, Context &)
{
	reply << "<html><head><title>" << fName << "</title></head>\n<body>";
}

void Page::Title(ostream &reply, Context &)
{
	reply << "<h1>" << fName << "</h1>";
}

void Page::Body(ostream &reply, Context &)
{
	reply << "..." ;
}

void Page::Footer(ostream &reply, Context &)
{
	reply << "</body></html>\n";
}

void Page::Start(ostream &reply, Context &context)
{
	context.SetPage(this);
	Render(reply, context);
}

bool Page::Prepare(String &transitionToken, Context &context)
{
	StartTrace1(Page.Prepare, fName << ": <" << transitionToken << ">");
	if (transitionToken == "") {
		return true;
	}
	return ProcessToken(transitionToken, context);
}

bool Page::Finish(String &transitionToken, Context &context)
{
	StartTrace1(Page.Finish, fName << ": <" << transitionToken << ">");
	return ProcessToken(transitionToken, context);
}

bool Page::ProcessToken(String &transitionToken, Context &context)
{
	StartTrace1(Page.ProcessToken, fName << ": <" << transitionToken << ">");
	context.SetPage(this);
	return Action::ExecAction(transitionToken, context, context.Lookup(transitionToken));
}

void Page::Preprocess(Context &c)
{
	StartTrace1(Page.Preprocess, fName << ":");
}

void Page::Render(ostream &reply, Context &c)
{
	StartTrace1(Page.RenderNew, "<" << fName << ">");
	MethodTimer(Page.Render, "", c);
#ifdef PAGE_TRACING
	String dbg;
	OStringStream replyInt(dbg);
	RenderProtocolStatus(replyInt, c);
	RenderProtocolHeader(replyInt, c);
	replyInt << ENDL;
	RenderProtocolBody(replyInt, c);
	RenderProtocolTail(replyInt, c);

	SystemLog::WriteToStderr(String("Page::Render\n") <<
							 "------------ start -----------\n" << dbg <<
							 "------------  end ------------\n");
#endif
	//SOP: try if we can get better responsiveness releasing the session
	SessionReleaser slr(c);
	slr.Use();

	RenderProtocolStatus(reply, c);
	RenderProtocolHeader(reply, c);

	bool zip = false;
	if (c.Lookup("ClientAcceptsGzipEnc").AsBool(false)) {
		ROAnything contentEncoding = c.Lookup("ContentGzipEncoding");
		// FIXME leu: sometimes we have "Content-type: text/html; someotherstuff"
		String contentType = c.Lookup("content-type").AsString("text/html");
		if (contentEncoding[contentType].AsBool(false)) {
			zip = true;
		}
	}

	HTTPStreamStack stackStream(reply, RequestProcessor::KeepConnectionAlive(c), zip);
	ostream &output = stackStream.GetBodyStream();

	RenderProtocolBody(output, c);
	RenderProtocolTail(output, c);

	String token("PostRender");
	ProcessToken(token, c);
}

void Page::RenderProtocolStatus(ostream &reply, Context &c)
{
	StartTrace1(Page.RenderProtocolStatus, "<" << fName << ">");
	RequestProcessor::RenderProtocolStatus(reply, c);
}

void Page::RenderProtocolHeader(ostream &reply, Context &c)
{
	StartTrace1(Page.RenderProtocolStatus, "<" << fName << ">");

	// this part is optional
	ROAnything httpHeader(c.Lookup("HTTPHeader"));

	if (! httpHeader.IsNull()) {
		Renderer::Render(reply, c, httpHeader);
	} else {
		// legacy
		Mime(reply, c);
	}
}

void Page::RenderProtocolBody(ostream &reply, Context &c)
{
	StartTrace1(Page.RenderProtocolBody, "<" << fName << ">");

	ROAnything pagelayout(c.Lookup("PageLayout"));

	if (!pagelayout.IsNull()) {
		Renderer::Render(reply, c, pagelayout);
	} else {
		Header(reply, c);

		reply << "<hr>\n";
		Title(reply, c);

		Body(reply, c);
		reply << "<hr>\n";
		Footer(reply, c);
	}
}

void Page::RenderProtocolTail(ostream &reply, Context &c)
{
	StartTrace1(Page.RenderProtocolTail, "<" << fName << ">");

	// we render optional debug output
	HTMLTraceStores(reply, c);
}

RegCacheImpl(Page);	// FindPage()
