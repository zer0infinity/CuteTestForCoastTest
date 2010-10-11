/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FormattedStringRenderer_H
#define _FormattedStringRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_StringRenderers.h"
#include "Renderer.h"

//---- FormattedStringRenderer ----------------------------------------------------------
//: comment Renderer
//	Structure of config:
//<PRE>	{
//		/Value			Rendererspec	String to be rendered
//		/Width			Rendererspec	Overall length of the rendered String, if the string is shorter than the field
//										length the rest of the string will be filled with the String given in the slot /Filler
//		/Filler			Rendererspec	string used for filling the string until reaching Length, Default " "
//		/Align			Rendererspec	{ left, center, right }
//		/SpacesForTab	Rendererspec	how many fillers should be inserted for a tab (\t) in value
//      ...
//	}</PRE>
class EXPORTDECL_STRINGRENDERERS FormattedStringRenderer : public Renderer
{
public:
	//--- constructors
	FormattedStringRenderer(const char *name);
	~FormattedStringRenderer();

	//:Renders ?? on <I>reply </I>
	//!param: reply - out - the stream where the rendered output is written on.
	//!param: c - the context the renderer runs within.
	//!param: config - the configuration of the renderer.
	virtual void RenderAll(std::ostream &reply, Context &c, const ROAnything &config);

};

#endif
