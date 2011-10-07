/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Page.h"
#include "Renderer.h"
#include "Action.h"
#include "Timers.h"
#include "HTTPStreamStack.h"
#include "RequestProcessor.h"
#include "Policy.h"

const char* Page::gpcCategory = "Page";
const char* Page::gpcConfigPath = "Pages";

RegCacheImpl(Page);
RegisterModule(PagesModule);

bool PagesModule::Init(const ROAnything config) {
	ROAnything roaConfig;
	if (config.LookupPath(roaConfig, Page::gpcConfigPath)) {
		HierarchyInstaller ai(Page::gpcCategory);
		return RegisterableObject::Install(roaConfig, Page::gpcCategory, &ai);
	}
	return false;
}

bool PagesModule::ResetFinis(const ROAnything config) {
	AliasTerminator at(Page::gpcCategory);
	return RegisterableObject::ResetTerminate(Page::gpcCategory, &at);
}

bool PagesModule::Finis() {
	return StdFinis(Page::gpcCategory, Page::gpcConfigPath);
}
RegisterPage(Page);

Page::Page(const char *title) :
	HierarchConfNamed(title) {
	SetName(title);
}

IFAObject *Page::Clone(Allocator *a) const {
	StartTrace(Page.Clone);
	return new (a) Page(fName);
}

void Page::Mime(std::ostream &reply, Context &c) {
	reply << "Content-type: text/html" << ENDL;
}

void Page::Header(std::ostream &reply, Context &) {
	reply << "<html><head><title>" << fName << "</title></head>\n<body>";
}

void Page::Title(std::ostream &reply, Context &) {
	reply << "<h1>" << fName << "</h1>";
}

void Page::Body(std::ostream &reply, Context &) {
	reply << "...";
}

void Page::Footer(std::ostream &reply, Context &) {
	reply << "</body></html>\n";
}

void Page::Start(std::ostream &reply, Context &context) {
	StartTrace1(Page.Start, fName << ":");
	context.SetPage(this);
	Render(reply, context);
}

bool Page::Prepare(String &transitionToken, Context &context) {
	StartTrace1(Page.Prepare, fName << ": <" << transitionToken << ">");
	if (!transitionToken.Length()) {
		return true;
	}
	return ProcessToken(transitionToken, context);
}

bool Page::Finish(String &transitionToken, Context &context) {
	StartTrace1(Page.Finish, fName << ": <" << transitionToken << ">");
	return ProcessToken(transitionToken, context);
}

bool Page::ProcessToken(String &transitionToken, Context &context) {
	StartTrace1(Page.ProcessToken, fName << ": <" << transitionToken << ">");
	context.SetPage(this);
	ROAnything roaTransitionConfig = context.Lookup(transitionToken);
	TraceAny(roaTransitionConfig, "context config for <" << transitionToken << ">");
	return Action::ExecAction(transitionToken, context, roaTransitionConfig);
}

void Page::Preprocess(Context &c) {
	StatTrace(Page.Preprocess, fName << ":", Coast::Storage::Current());
}

void Page::Render(std::ostream &reply, Context &ctx) {
	StartTrace1(Page.Render, "<" << fName << ">");
	MethodTimer(Page.Render, "", ctx);
	if (TriggerEnabled(Page.Render.TracePage)) {
		String dbg;
		OStringStream replyInt(dbg);
		RenderProtocolStatus(replyInt, ctx);
		RenderProtocolHeader(replyInt, ctx);
		replyInt << ENDL;
		RenderProtocolBody(replyInt, ctx);
		RenderProtocolTail(replyInt, ctx);
		replyInt << std::flush;
		SystemLog::WriteToStderr(
				String("Page::Render\n") << "------------ start -----------\n" << dbg << "------------  end ------------\n");
	}
	//SOP: try if we can get better responsiveness releasing the session
	SessionReleaser slr(ctx);
	slr.Use();

	RenderProtocolStatus(reply, ctx);
	RenderProtocolHeader(reply, ctx);

	bool zip = false;
	if (ctx.Lookup("ClientAcceptsGzipEnc").AsBool(false)) {
		ROAnything contentEncoding = ctx.Lookup("ContentGzipEncoding");
		//!@FIXME leu: sometimes we have "Content-type: text/html; someotherstuff"
		String contentType = ctx.Lookup("content-type").AsString("text/html");
		if (contentEncoding[contentType].AsBool(false)) {
			zip = true;
		}
	}

	HTTPStreamStack stackStream(reply, RequestProcessor::KeepConnectionAlive(ctx), zip);
	std::ostream &output = stackStream.GetBodyStream();

	RenderProtocolBody(output, ctx);
	RenderProtocolTail(output, ctx);

	String token("PostRender");
	ProcessToken(token, ctx);
}

void Page::RenderProtocolStatus(std::ostream &reply, Context &ctx) {
	StartTrace1(Page.RenderProtocolStatus, "<" << fName << ">");
	RequestProcessor::RenderProtocolStatus(reply, ctx);
}

void Page::RenderProtocolHeader(std::ostream &reply, Context &ctx) {
	StartTrace1(Page.RenderProtocolHeader, "<" << fName << ">");
	ROAnything httpHeader(ctx.Lookup("HTTPHeader"));
	if (!httpHeader.IsNull()) {
		Renderer::Render(reply, ctx, httpHeader);
	} else {
		// legacy
		Mime(reply, ctx);
	}
}

void Page::RenderProtocolBody(std::ostream &reply, Context &ctx) {
	StartTrace1(Page.RenderProtocolBody, "<" << fName << ">");

	ROAnything pagelayout(ctx.Lookup("PageLayout"));

	if (!pagelayout.IsNull()) {
		Renderer::Render(reply, ctx, pagelayout);
	} else {
		Header(reply, ctx);

		reply << "<hr>\n";
		Title(reply, ctx);

		Body(reply, ctx);
		reply << "<hr>\n";
		Footer(reply, ctx);
	}
}

void Page::RenderProtocolTail(std::ostream &reply, Context &ctx) {
	StartTrace1(Page.RenderProtocolTail, "<" << fName << ">");
	//!@FIXME: this is a temporary workaround to only render Debug output onto html pages
	OStringStream ostr;
	RenderProtocolHeader(ostr, ctx);
	if (ostr.str().Contains("text/html") >= 0L) {
		// we render optional debug output
		ctx.HTMLDebugStores(reply);
	}
}
