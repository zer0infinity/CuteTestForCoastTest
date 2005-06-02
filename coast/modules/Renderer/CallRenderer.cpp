/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "CallRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- CallRenderer ---------------------------------------------------------------
RegisterRenderer(CallRenderer);

CallRenderer::CallRenderer(const char *name) : Renderer(name) { }

CallRenderer::~CallRenderer() { }

void CallRenderer::RenderAll(ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(CallRenderer.RenderAll);
	Anything params;
	ROAnything callee;
	if (config.IsDefined("Parameters") && config.IsDefined("Renderer")) {
		params = DoGetParameters(ctx, config["Parameters"]);
		callee = IntGetCallee(ctx, config["Renderer"]);
	} else {
		params = DoGetPositionalParameters(ctx, config);
		callee = IntGetCallee(ctx, config[0L]);
	}
	Context::PushPopEntry aEntry(ctx, "CallRenderer", params);
	Renderer::Render(reply, ctx, callee);
}

ROAnything CallRenderer::IntGetCallee(Context &ctx, ROAnything callee)
{
	StartTrace(CallRenderer.IntGetCallee);
	if (callee.GetType() != Anything::eArray) {
		const char *callname = callee.AsCharPtr(0);
		if (!ctx.Lookup(callname, callee)) {
			String msg;
			msg << "CallRenderer: definition not found: " << callname;
			SysLog::Warning(msg);
			Trace(msg);
		}
	}
	TraceAny(callee, "rendering callee");
	return callee;
}

Anything CallRenderer::DoGetParameters(Context &ctx, const ROAnything &config)
{
	return config.DeepClone();
}

Anything CallRenderer::DoGetPositionalParameters(Context &ctx, const ROAnything &config)
{
	StartTrace(CallRenderer.DoGetPositionalParameters);
	Anything result;
	for (long i = 1; i < config.GetSize(); i ++) {
		result[String("$")<<i] = config[i].DeepClone();
	}
	return result;
}

//---- EagerCallRenderer ---------------------------------------------------------------
RegisterRenderer(EagerCallRenderer);

EagerCallRenderer::EagerCallRenderer(const char *name) : CallRenderer(name) { }

EagerCallRenderer::~EagerCallRenderer() { }

Anything EagerCallRenderer::DoGetParameters(Context &ctx, const ROAnything &config)
{
	Anything result;
	for (long i = 0; i < config.GetSize(); i ++) {
		result[config.SlotName(i)] = Renderer::RenderToString(ctx, config[i]);
	}
	return result;
}

Anything EagerCallRenderer::DoGetPositionalParameters(Context &ctx, const ROAnything &config)
{
	StartTrace(PositionalCallRenderer.DoGetPositionalParameters);
	Anything result;
	for (long i = 1; i < config.GetSize(); i ++) {
		result[String("$")<<i] = Renderer::RenderToString(ctx, config[i]);
	}
	return result;
}
