/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "FileLoadRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "Dbg.h"

//--- project modules used -----------------------------------------------------

//--- c-modules used -----------------------------------------------------------

//---- FileLoadRenderer ---------------------------------------------------------------
RegisterRenderer(FileLoadRenderer);

FileLoadRenderer::FileLoadRenderer(const char *name) : Renderer(name) { }

FileLoadRenderer::~FileLoadRenderer() { }

void FileLoadRenderer::RenderAll(ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(FileLoadRenderer.RenderAll);

	String fileName;
	String quietRendered;
	bool quiet = false;
	RenderOnString(fileName, ctx, config["File2Load"]);
	RenderOnString(quietRendered, ctx, config["Quiet"]);
	quiet = (quietRendered == "yes") ? true : false;

	if (fileName.Length() == 0) {
		if ( !quiet ) {
			reply << "FileLoadRenderer: File2Load Param missing.";
		}
		return;
	}
	iostream *is = System::OpenStream(fileName, String(), ios::in | ios::binary);
	if (!is ) {
		if ( !quiet ) {
			reply << "FileLoadRenderer: File " << fileName << " not found.";
		}
	} else {
		reply << is->rdbuf();
		delete(is);
	}
}
