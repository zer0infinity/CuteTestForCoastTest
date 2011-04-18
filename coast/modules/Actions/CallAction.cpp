/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "CallAction.h"
#include "Context.h"
RegisterAction(CallAction);

namespace {
	char const *_CalleeSlotname = "Action";
	char const *_ParametersSlotname = "Parameters";
	long const _CalleeIndex = 0L;
	long const _ParametersStartIndex = _CalleeIndex + 1L;

	ROAnything IntGetCallee(Context &ctx, ROAnything callee, char const* className) {
		StartTrace(CallAction.IntGetCallee);
		if (callee.GetType() != AnyArrayType) {
			const char *callname = callee.AsCharPtr(0);
			if (!ctx.Lookup(callname, callee)) {
				String msg;
				msg << className << ": definition not found: " << callname;
				SystemLog::Warning(msg);
				Trace(msg);
			}
		}
		TraceAny(callee, "rendering callee");
		return callee;
	}
	Anything DoGetParameters(Context &ctx, const ROAnything &config) {
		return config.DeepClone();
	}

	Anything DoGetPositionalParameters(Context &ctx, const ROAnything &config) {
		StartTrace(CallAction.DoGetPositionalParameters);
		Anything result;
		for (long i = _ParametersStartIndex, sz = config.GetSize(); i < sz; ++i) {
			result[String("$") << i] = config[i].DeepClone();
		}
		return result;
	}
}

bool CallAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config) {
	StartTrace(CallAction.DoExecAction);
	ROAnything callee;
	if (!config.LookupPath(callee, _CalleeSlotname, '\000')) {
		callee = config[_CalleeIndex];
	}
	callee = IntGetCallee(ctx, callee, GetName());
	ROAnything configparams;
	Anything params;
	if (config.LookupPath(configparams, _ParametersSlotname, '\000')) {
		params = DoGetParameters(ctx, configparams);
	} else {
		params = DoGetPositionalParameters(ctx, config);
	}
	Context::PushPopEntry<Anything> aEntry(ctx, GetName(), params);
	return Action::ExecAction(transitionToken, ctx, callee);
}
