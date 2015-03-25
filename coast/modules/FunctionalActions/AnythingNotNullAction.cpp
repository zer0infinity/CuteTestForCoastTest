/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AnythingNotNullAction.h"
#include "Context.h"
#include "AnythingUtils.h"
#include "Renderer.h"
#include "Tracer.h"

//---- AnythingNotNullAction ---------------------------------------------------------------
RegisterAction(AnythingNotNullAction);

AnythingNotNullAction::AnythingNotNullAction(const char *name) : Action(name) { }

AnythingNotNullAction::~AnythingNotNullAction() { }

bool AnythingNotNullAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	// this is the new method that also gets a config ( similar to Renderer::RenderAll )
	// write the action code here - you don't have to override DoAction anymore
	StartTrace(AnythingNotNullAction.DoExecAction);
	ROAnything roaTemp;
	String strStore, strSlot;
	strStore = config["Store"].AsString();
	if (config.LookupPath(roaTemp, "Slot")) {
		Renderer::RenderOnString(strSlot, ctx, roaTemp);
	}
	ROAnything roaStore = StoreFinder::FindStore(ctx, strStore);
	bool boRet = roaStore.LookupPath(roaTemp, strSlot) && !roaTemp.IsNull();
	Trace("slot ["  << strSlot << "] in " << (strStore.Length() ? strStore : String("TempStore")) << " is " << (boRet ? "not " : "") << "Null");

	return boRet;
}
