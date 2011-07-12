/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Anything.h"
#include "Context.h"
#include "Renderer.h"
#include "Tracer.h"
#include "TitlesOnceColsAction.h"

//---- TitlesOnceColsAction ---------------------------------------------------------------
RegisterAction(TitlesOnceColsAction);

TitlesOnceColsAction::TitlesOnceColsAction(const char *name) : Action(name) { }

TitlesOnceColsAction::~TitlesOnceColsAction() { }

bool TitlesOnceColsAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	// this is the new method that also gets a config ( similar to Renderer::RenderAll )
	// write the action code here - you don't have to override DoAction anymore
	StartTrace(TitlesOnceColsAction.DoExecAction);

	TraceAny(config, "config");

	String listName, titlesList;

	ROAnything roaSlotConfig;

	if (config.LookupPath(roaSlotConfig, "BoxColumns")) {
		Renderer::RenderOnString(listName, ctx, roaSlotConfig);
	} else {
		Trace("Error in TitlesOnceColsAction::DoExecAction, BoxColumns not defined");
		return false;
	}
	Trace("listName: [" << listName << "]");

	if (config.LookupPath(roaSlotConfig, "BoxQuery")) {
		Renderer::RenderOnString(titlesList, ctx, roaSlotConfig);
	} else {
		Trace("Error in TitlesOnceColsAction::DoExecAction, BoxQuery not defined");
		return false;
	}
	Trace("titlesList: [" << titlesList << "]");

	ROAnything listNameAny, titlesListAny;

	listNameAny = ctx.Lookup( listName );
	titlesListAny = ctx.Lookup( titlesList );

	TraceAny( listNameAny  , "List to prepare BoxColumns");
	TraceAny( titlesListAny, "List get titles info");

	Anything list, tempStore = ctx.GetTmpStore();
	long idxMax = listNameAny.GetSize(), seqNr = 0;
	for ( long idx = 0L; idx < idxMax; ++idx ) {
		Anything any;
		String intName = listNameAny[idx]["IntName"].AsString();
		any[intName]["Lookup"] = ( String("SelectBoxOption:") <<  titlesListAny[ intName ].AsString() );
		TraceAny( any, "ANY RESULT" );
		list[ intName ] =  any[0L];

		bool boHide = false;

		if (listNameAny[idx].IsDefined("Hide")) {
			String strHide;
			Renderer::RenderOnString(strHide, ctx, listNameAny[idx]["Hide"]);
			boHide = strHide.AsLong(0L);
			Trace("Column is " << (boHide ? "hidden" : "visible"));
		}

		if ( !listNameAny[idx].IsDefined("Hide") ||
			 (listNameAny[idx].IsDefined("Hide") && (boHide != 1)) ) {
			tempStore["TempSlot"][0L] = listNameAny[idx].DeepClone();
			tempStore["TempSlot"][0L].Remove("Value");
			tempStore["TempSlot"][0L]["Value"]["Lookup"] = ( String("SelectBoxOption:") <<  titlesListAny[ intName ].AsString() );
			tempStore["TempSlot"][0L].Remove("Hide");
			tempStore[ String(listName) << "Opt"][String() << seqNr] = tempStore["TempSlot"][0L];
			seqNr += 1;
		}
	}
	tempStore.Remove("TempSlot");
	tempStore[listName << "Lookups"] = list;
	TraceAny( tempStore, "Definitives Resultat" );
	return true;
}
