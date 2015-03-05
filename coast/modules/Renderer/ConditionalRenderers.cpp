/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ConditionalRenderers.h"
#include "StringStream.h"

//---- ConditionalRenderer ----------------------------------------------------------------
RegisterRenderer( ConditionalRenderer);

static void IdentifyPathAndIndexDelim(char &pathDelim, char &indexDelim, Context &ctx, const ROAnything &config) {
	pathDelim = '.';
	indexDelim = ':';
	if (config.IsDefined("PathDelim")) {
		String renderedPathDelim;
		Renderer::RenderOnString(renderedPathDelim, ctx, config["PathDelim"]);
		if (renderedPathDelim.Length() > 0) {
			if (renderedPathDelim == "Ignore") {
				pathDelim = '\0';
			} else {
				pathDelim = renderedPathDelim[0L];
			}
		}
	}
	if (config.IsDefined("IndexDelim")) {
		String renderedIndexDelim;
		Renderer::RenderOnString(renderedIndexDelim, ctx, config["IndexDelim"]);
		if (renderedIndexDelim.Length() > 0) {
			if (renderedIndexDelim == "Ignore") {
				indexDelim = '\0';
			} else {
				indexDelim = renderedIndexDelim[0L];
			}
		}
	}
}

ConditionalRenderer::ConditionalRenderer(const char *name) :
	Renderer(name) {
}
void ConditionalRenderer::TestCondition(Context &ctx, const ROAnything &config, String &res) {
	StartTrace(ConditionalRenderer.TestCondition);
	ROAnything lookupNameConfig;
	if (config.LookupPath(lookupNameConfig, "ContextCondition")) {
		String condname;
		RenderOnString(condname, ctx, lookupNameConfig);

		if (condname.Length() > 0) {
			char pathDelim;
			char indexDelim;
			IdentifyPathAndIndexDelim(pathDelim, indexDelim, ctx, config);
			Trace("pathDelim: [" << pathDelim << "] indexDelim: [" << indexDelim << "]");
			Trace("condition: [" << condname << "]");

			ROAnything result = ctx.Lookup(condname, pathDelim, indexDelim);

			if (result.IsNull()) {
				res = "Undefined";
			} else if (result.GetType() == AnyLongType) {
				res = result.AsBool() ? "True" : "False";
			} else {
				res = "Defined";
			}
			return; // we are done. anything else is an error.
		} else {
			if (TriggerEnabled(ConditionalRenderer.TestCondition)) {
				String strbuf;
				StringStream stream(strbuf);
				stream << "Error: Invalid lookup name: ";
				lookupNameConfig.PrintOn(stream) << "\n";
				stream.flush();
				SystemLog::WriteToStderr(strbuf);
			}
		}
	} else {
		SystemLog::WriteToStderr("Error: ContextCondition not specified!\n");
	}
	res = "Error";
}

void ConditionalRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config) {
	StartTrace(ConditionalRenderer.Render);
	TraceAny(config, "config");

	String result;
	TestCondition(ctx, config, result);

	Trace("result: <" << result << ">");

	ROAnything renderer;
	if (config.LookupPath(renderer, result) && (!renderer.IsNull())) {
		Render(reply, ctx, renderer);
	} else if (result == "True" || result == "False") {
		// Check if there is a specification for Defined
		if (config.LookupPath(renderer, "Defined") && (!renderer.IsNull())) {
			Render(reply, ctx, renderer);
		}
	}
}

//---- SwitchRenderer ----------------------------------------------------------------

RegisterRenderer( SwitchRenderer);

SwitchRenderer::SwitchRenderer(const char *name) :
	Renderer(name) {
}

void SwitchRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config) {
	StartTrace(SwitchRenderer.Render);
	TraceAny(config, "config");

	ROAnything lookupName;
	char pathDelim;
	char indexDelim;
	IdentifyPathAndIndexDelim(pathDelim, indexDelim, ctx, config);
	Trace("pathDelim: [" << pathDelim << "] indexDelim: [" << indexDelim << "]");
	if (config.LookupPath(lookupName, "ContextLookupName")) {
		// determine lookup slot name
		String condname;
		RenderOnString(condname, ctx, lookupName);
		if (condname.Length() > 0) {
			// lookup data
			ROAnything result = ctx.Lookup(condname, pathDelim, indexDelim);
			if ((result.GetType() == AnyCharPtrType) || (result.GetType() == AnyLongType) || (result.GetType() == AnyDoubleType)
					|| (result.GetType() == AnyNullType)) {
				// PFM: accept empty result
				// get different alternatives for renderer specifications
				String slot = result.AsString("");
				Trace("result of lookup: " << slot);
				ROAnything alternatives;
				if (config.LookupPath(alternatives, "Case")) {
					// choose suitable renderer specification
					ROAnything renderer;
					if (!alternatives.LookupPath(renderer, slot, pathDelim, indexDelim)) { // found slot
						if (renderer.IsNull()) {
							// no matching case statement found.. render default
							config.LookupPath(renderer, "Default", pathDelim, indexDelim);
						}
					}
					if (!renderer.IsNull()) {
						Render(reply, ctx, renderer);
					}
				} else {
					// handle error: no configuration for desired alternatives available
					SystemLog::Error("SwitchRenderer::RenderAll: missing Case structure");
				}
			} else {
				// handle error: looked up data unsuitabe as a switch condition
				String error("SwitchRenderer::RenderAll: data is not a valid switch criteria: ");

				OStringStream os(&error);
				result.PrintOn(os); // append Anything to ease debugging
				os.flush();
				error << '\n' << "ContextLookupName was [" << condname << "]" << '\n';
				SystemLog::Error(error);
			}
		} else {
			// handle error: invalid lookup slot name

			String error("SwitchRenderer::RenderAll: Invalid lookup name: ");

			OStringStream os(&error);
			lookupName.PrintOn(os, false); // append Anything to ease debugging

			SystemLog::Error(error);
		}
	} else {
		SystemLog::Error("SwitchRenderer::RenderAll: ContextCondition not specified!");
	}
}
