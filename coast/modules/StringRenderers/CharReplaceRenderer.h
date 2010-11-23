/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CharReplaceRenderer_H
#define _CharReplaceRenderer_H

//---- Renderer include -------------------------------------------------
#include "Renderer.h"

//---- CharReplaceRenderer ----------------------------------------------------------
//: comment Renderer
//	Structure of config:
//<PRE>	{
//		/CharsToReplace	String			string which contains all characters to be replaced with ReplaceChar
//		/ReplaceChar	Character		replacement character, default is '_'
//		/String			Rendererspec	String which will be affected
//	}</PRE>
class CharReplaceRenderer : public Renderer
{
public:
	//--- constructors
	CharReplaceRenderer(const char *name);
	~CharReplaceRenderer();

	//:Renders ?? on <I>reply </I>
	//!param: reply - out - the stream where the rendered output is written on.
	//!param: c - the context the renderer runs within.
	//!param: config - the configuration of the renderer.
	virtual void RenderAll(std::ostream &reply, Context &c, const ROAnything &config);
};

#endif
