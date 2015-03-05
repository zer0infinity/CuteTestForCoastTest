/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "RendererDispatchAction.h"
#include "Renderer.h"
//---- RendererDispatchAction ---------------------------------------------------------------
RegisterAction(RendererDispatchAction);

bool RendererDispatchAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config) {
	StartTrace(RendererDispatchAction.DoExecAction);
	TraceAny(config, "Configuration")
	ROAnything rendererSpec;
	if (!config.LookupPath(rendererSpec, "Renderer", '\000')) {
		rendererSpec = config;
	}
	String nextAction(64L);
	Renderer::RenderOnString(nextAction, ctx, rendererSpec);
	TraceAny(rendererSpec, "RendererSpec resulted in Action <" << nextAction << ">");

	if ( nextAction.Length() ) {
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
