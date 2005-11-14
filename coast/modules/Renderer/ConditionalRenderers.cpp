/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ConditionalRenderers.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "Dbg.h"

//---- ConditionalRenderer ----------------------------------------------------------------
RegisterRenderer(ConditionalRenderer);

ConditionalRenderer::ConditionalRenderer(const char *name) : Renderer(name)
{
}
void ConditionalRenderer::TestCondition(Context &context, const ROAnything &config, String &res)
{
	ROAnything lookupNameConfig;
	if (config.LookupPath(lookupNameConfig, "ContextCondition")) {
		String condname;
		RenderOnString(condname, context, lookupNameConfig);

		if (condname.Length() > 0) {
			StartTrace1(ConditionalRenderer.Condition, "condition: " <<  condname);

			ROAnything result = context.Lookup(condname);

			if (result.IsNull()) {
				res = "Undefined";
			} else if (result.GetType() == AnyLongType) {
				res =  result.AsBool() ? "True" : "False";
			} else {
				res = "Defined";
			}
			return; // we are done. anything else is an error.
		}
#ifdef DEBUG
		else {
			String strbuf;
			StringStream stream(strbuf);
			stream << "Error: Invalid lookup name: ";
			lookupNameConfig.PrintOn(stream) << "\n";
			stream.flush();
			SysLog::WriteToStderr(strbuf);
		}
	} else {
		SysLog::WriteToStderr("Error: ContextCondition not specified!\n");
#endif
	}
	res = "Error";
}

void ConditionalRenderer::RenderAll(ostream &reply, Context &context, const ROAnything &config)
{
	StartTrace(ConditionalRenderer.Render);
	TraceAny(config, "config");

	String result;
	TestCondition(context, config, result);

	Trace("result: <" << result << ">");

	ROAnything renderer;
	if (config.LookupPath(renderer, result) && (!renderer.IsNull()) ) {
		Render(reply, context, renderer);
	} else if ( result == "True" || result == "False" ) {
		// Check if there is a specification for Defined
		if (config.LookupPath(renderer, "Defined") && (!renderer.IsNull()) ) {
			Render(reply, context, renderer);
		}
	}
}

//---- SwitchRenderer ----------------------------------------------------------------

RegisterRenderer(SwitchRenderer);

SwitchRenderer::SwitchRenderer(const char *name) : Renderer(name)
{
}

void SwitchRenderer::RenderAll(ostream &reply, Context &context, const ROAnything &config)
{
	StartTrace(SwitchRenderer.Render);
	TraceAny(config, "config");

	// check context condition

	ROAnything lookupName;
	if (config.LookupPath(lookupName, "ContextLookupName")) {
		// determine lookup slot name
		String condname;
		RenderOnString(condname, context, lookupName);
		if (condname.Length() > 0) {
			// lookup data
			ROAnything result = context.Lookup(condname);
			if ((result.GetType() == AnyCharPtrType) || // accept only these simple types
				(result.GetType() == AnyLongType) ||
				(result.GetType() == AnyDoubleType) ||
				(result.GetType() == AnyNullType) ) {	// PFM: accept empty result
				// get different alternatives for renderer specifications

				String slot = result.AsString("");

				Trace("result of lookup: " << slot);

				ROAnything alternatives;
				if (config.LookupPath(alternatives, "Case")) {
					// choose suitable renderer specification

					ROAnything renderer;

					if (!alternatives.LookupPath(renderer, slot)) {	// found slot
						if (renderer.IsNull()) {
							// no matching case statement found.. render default
							config.LookupPath(renderer, "Default");
						}
					}
					if (!renderer.IsNull()) {
						Render(reply, context, renderer);
					}
				} else {
					// handle error: no configuration for desired alternatives available

					SysLog::Error("SwitchRenderer::RenderAll: missing Case structure");
				}
			} else {
				// handle error: looked up data unsuitabe as a switch condition
				String error("SwitchRenderer::RenderAll: data is not a valid switch criteria: ");

				OStringStream os(&error);
				result.PrintOn(os);	// append Anything to ease debugging
				os.flush();
				error << '\n' << "ContextLookupName was [" << condname << "]" << '\n';
				SysLog::Error(error);
			}
		} else {
			// handle error: invalid lookup slot name

			String error("SwitchRenderer::RenderAll: Invalid lookup name: ");

			OStringStream os(&error);
			lookupName.PrintOn(os, false);	// append Anything to ease debugging

			SysLog::Error(error);
		}
	} else {
		SysLog::Error("SwitchRenderer::RenderAll: ContextCondition not specified!");
	}
}
