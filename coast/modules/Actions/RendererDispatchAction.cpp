/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "RendererDispatchAction.h"

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Context.h"
#include "StringStream.h"
#include "Renderer.h"
#include "Dbg.h"

//---- RendererDispatchAction ---------------------------------------------------------------
RegisterAction(RendererDispatchAction);

RendererDispatchAction::RendererDispatchAction(const char *name) : Action(name) { }

RendererDispatchAction::~RendererDispatchAction() { }

bool RendererDispatchAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(RendererDispatchAction.DoExecAction);

	TraceAny(config, "Configuration")
	ROAnything rendererSpec;
	if (!config.LookupPath(rendererSpec, "Renderer")) {
		rendererSpec = config[0L];
	}
	String nextAction(64L);
	Renderer::RenderOnString(nextAction, ctx, rendererSpec);
	TraceAny(rendererSpec, "RendererSpec resulted in Action <" << nextAction << ">");

	if (nextAction != "") {
		ROAnything nextActionConfig = ctx.Lookup(nextAction);
		TraceAny(nextActionConfig, "nextActionConfig");
		if (nextActionConfig.IsNull()) {
			Trace("config was Null, setting transitionToken to nextAction [" << nextAction << "]");
			transitionToken = nextAction;
		}
		return Action::ExecAction(transitionToken, ctx, nextActionConfig);
	}
	return true;
}
