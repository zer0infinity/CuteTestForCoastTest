/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Context.h"

//--- interface include --------------------------------------------------------
#include "CallRenderer.h"
#include "SysLog.h"

//--- c-modules used -----------------------------------------------------------
#include <iostream.h>

#include "Dbg.h"

//--- project modules used -----------------------------------------------------

//---- CallRenderer ---------------------------------------------------------------
RegisterRenderer(CallRenderer);

CallRenderer::CallRenderer(const char *name) : Renderer(name) { }

CallRenderer::~CallRenderer() { }

class ROAnyLookupAdapter : public virtual LookupInterface
{
public:
	ROAnyLookupAdapter(ROAnything ro): fRoAny(ro) {}
	~ROAnyLookupAdapter() {}
protected:
	ROAnything fRoAny;
	//!subclass api to search for key in objects context
	//! method to implement in subclasses
	virtual bool DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const {
		return fRoAny.LookupPath(result, key, delim, indexdelim);
	}
};

void CallRenderer::RenderAll(ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(CallRenderer.RenderAll);
	ROAnything callee;
	const char *callname = config["Renderer"].AsCharPtr(0);
	if (callname && ctx.Lookup(callname, callee)) {
		ROAnyLookupAdapter params(config["Parameters"]);
		ctx.Push(&params);
		Renderer::Render(reply, ctx, callee);
		ctx.Pop();
	} else {
		String msg("CallRenderer: definition not found:");
		msg << callname ? callname : "(null)";
		Trace(msg);
		// should we render the error message?
		SysLog::Warning(msg);
	}
}
