/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StringUpperLowerRenderer_H
#define _StringUpperLowerRenderer_H

#include "Renderer.h"

//---- StringUpperLowerRenderer ----------------------------------------------------------
//: comment Renderer
//	Structure of config:
//<PRE>	{
//		/Value 	String to convert
//      /Mode	upper or lower
//	}</PRE>
class StringUpperLowerRenderer : public Renderer
{
public:
	//--- constructors
	StringUpperLowerRenderer(const char *name);
	~StringUpperLowerRenderer();

	//:Renders ?? on <I>reply </I>
	//!param: reply - out - the stream where the rendered output is written on.
	//!param: c - the context the renderer runs within.
	//!param: config - the configuration of the renderer.
	virtual void RenderAll(std::ostream &reply, Context &c, const ROAnything &config);

};

#endif
