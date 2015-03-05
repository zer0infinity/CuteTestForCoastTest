/*
 * Copyright (c) 2008, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "DirFileListAction.h"
#include "SystemFile.h"
#include "Renderer.h"
#include "Context.h"
#include "Tracer.h"
#include "Timers.h"

using namespace coast;

//---- DirFileListAction ---------------------------------------------------------------
RegisterAction(DirFileListAction);

DirFileListAction::DirFileListAction(const char *name) : Action(name) { }

DirFileListAction::~DirFileListAction() { }

bool DirFileListAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	// this is the new method that also gets a config ( similar to Renderer::RenderAll )
	// write the action code here - you don't have to override DoAction anymore
	StartTrace(DirFileListAction.DoExecAction);
	String sPath, sFilter, sTarget, sSort;

	// Check Config if Path is defined cause mandatory
	if ( config.IsDefined("Path") ) {
		// Render slot
		Renderer::RenderOnString(sPath, ctx, config["Path"]);
		Trace("rendered sPath [" << sPath << "]");
		if ( !sPath.Length() ) {
			Trace("mandatory slot Path is empty after rendering !");
			return false;
		}
	} else {
		Trace("mandatory slot Path is missing in Config !");
		return false;
	}
	// Check Config if Filter is defined
	if ( config.IsDefined("Filter") ) {
		sFilter = config["Filter"].AsString();
		Trace("optional slot Filter is [" << sFilter << "]");
	}
	// Check Config if Target is defined
	if ( config.IsDefined("Target") ) {
		sTarget = config["Target"].AsString();
		Trace("optional slot Target is [" << sTarget << "]");
	}

	Anything aTmpStore(ctx.GetTmpStore());
	// Check if path is a valid path
	if ( !system::IsDirectory(sPath) ) {
		String strMsg("Slot Path contains not a valid path !");
		Trace(strMsg);
		aTmpStore["Error"] = strMsg;
		return false;
	}

	// Do the Action
	Anything aFileList;
	{
		MethodTimerUnit(DirFileListAction.DoExecAction, "GetDirFileList", ctx, TimeLogger::eMicroseconds);
		aFileList = system::DirFileList(sPath, sFilter);
	}
	SubTraceAny(TraceList, aFileList, "Found [" << aFileList.GetSize() << "] files");

	// Check Config if Sort is defined
	if (config.IsDefined("Sort")) {
		sSort = config["Sort"].AsString();
		Trace("optional slot Sort is [" << sSort << "]");
		MethodTimerUnit(DirFileListAction.DoExecAction, "SortList", ctx, TimeLogger::eMicroseconds);
//XXX
		aFileList.SortByStringValues();
	}

	if ( config.IsDefined("DirsOnly") ) {
		bool bDirs( config["DirsOnly"].AsBool(false) );
		Trace("optional slot DirsOnly is [" << bDirs << "]");
		long ii( aFileList.GetSize() - 1 );
		while (ii >= 0) {
			Trace ("File to check [" << aFileList[ii].AsString() << "]");
			String sCurrPath(sPath);
			if ( sPath[sPath.Length()-1] != system::Sep() ) {
				sCurrPath << system::Sep();
			}
			sCurrPath << aFileList[ii].AsString();
			Trace ("Path to check [" << sCurrPath << "]");
			if ( system::IsDirectory(sCurrPath) ) {
				Trace ("File to check [" << aFileList[ii].AsString() << "] is Directory !!");
				if ( !bDirs ) {
					Trace("Removing Directory [" << aFileList[ii].AsString() << "] from list");
					aFileList.Remove(ii);
					++ii;
				}
			} else {
				if ( bDirs ) {
					Trace("Removing File [" << aFileList[ii].AsString() << "] from list");
					aFileList.Remove(ii);
					++ii;
				}
			}
			--ii;
		}
	}

	if ( sTarget.Length() ) {
		aTmpStore[sTarget] = aFileList;
	} else {
		aTmpStore["RetrievedFileList"] = aFileList;
	}
	SubTraceAny(TraceListFinal, aFileList, "Found [" << aFileList.GetSize() << "] files");
	return true;
}
