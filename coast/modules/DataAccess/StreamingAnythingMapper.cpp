/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "StreamingAnythingMapper.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "Dbg.h"
#include "Timers.h"

//--- c-library modules used ---------------------------------------------------

//---- AnythingToStreamMapper ----------------------------------------------------------------
RegisterInputMapper(AnythingToStreamMapper);

bool AnythingToStreamMapper::DoFinalGetStream(const char *key, ostream &os, Context &ctx)
{
	StartTrace1(AnythingToStreamMapper.DoFinalGetStream, NotNull(key));
	if ( key ) {
		// use the superclass mapper to get the anything
		Anything anyValue;
		DoFinalGetAny(key, anyValue, ctx);

		TraceAny(anyValue, "value fetched from context");

		if ( !anyValue.IsNull() ) {
			DAAccessTimer(AnythingToStreamMapper.DoFinalGetStream, "exporting to stream", ctx);
			os << anyValue;
			os << flush;
			TraceAny(anyValue, "written to stream:");
			return true;
		}
		Trace("Nothing written to stream, value was null.");
	}
	return false;
}

//---- StreamToAnythingMapper ----------------------------------------------------------------
RegisterOutputMapper(StreamToAnythingMapper);

bool StreamToAnythingMapper::DoPutStream(const char *key, istream &is, Context &ctx, ROAnything script)
{
	StartTrace1(StreamToAnythingMapper.DoPutStream, NotNull(key));
	Anything anyResult;
	bool importok;
	{
		DAAccessTimer(StreamToAnythingMapper.DoPutStream, "importing from stream", ctx);
		importok = anyResult.Import(is);
	}
	Assert(importok);
	TraceAny(anyResult, "anything imported from stream:");
	if ( importok ) {
		if (script.IsNull() || script.GetSize() == 0) {
			// no more script to run
			Trace("Script is empty or null...");
			importok = DoFinalPutAny(key, anyResult, ctx);
		} else if (script.GetType() != Anything::eArray) {
			// we found a simple value in script use it as a new key in final put
			Trace("Script is a simple value:" << script.AsCharPtr());
			importok = DoFinalPutAny(script.AsCharPtr(key), anyResult, ctx);
		} else {
			// now for the scripting case, similar to Renderers
			TraceAny(script, "Got a script. Starting interpretation foreach slot...");
			for (long i = 0; importok && i < script.GetSize(); i++) {
				String slotname(script.SlotName(i));
				ResultMapper *m;
				if (slotname.Length() <= 0) {
					Trace("Anonymous slot, call myself again with script[" << i << "]");
					importok = DoPutAny(key, anyResult, ctx, script[i]);
				} else if ((m = ResultMapper::FindResultMapper(slotname))) {
					Trace("Slotname equals mapper: " << slotname);
					if (script[i].IsNull()) {
						// fallback to mappers original config
						Trace("Slotval is null. Calling " << slotname << " with it's default config...");
						importok = m->Put(key, anyResult, ctx);
					} else {
						Trace("Calling " << slotname << " with script[" << i << "][\"" << NotNull(key) << "\"]...");
						importok = m->DoPutAny(key, anyResult, ctx, DoSelectScript(key, script[i]));
					}
				} else {
					Trace("Slotname " << slotname << " is not a mapper (not found).");
					Trace("Using [" << slotname << "] as lookup key in result anything");
					Anything anyValue;
					if ( anyResult.LookupPath(anyValue, slotname) ) {
						TraceAny(anyValue, "Calling myself again with Anything looked up at [" << slotname << "] and with script[" << i << "]");
						importok = DoPutAny(key, anyValue, ctx, script[i]);
					}
				}
			}
		}
	}
	return importok;
}
