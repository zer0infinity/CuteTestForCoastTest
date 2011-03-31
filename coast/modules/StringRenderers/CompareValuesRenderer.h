/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CompareValuesRenderer_H
#define _CompareValuesRenderer_H

#include "Renderer.h"

//---- CompareValuesRenderer ----------------------------------------------------------
//: comment Renderer
//	Structure of config:
//<PRE>	{
//		/Value1		Rendererspec	First (left) value to compare with
//		/Value2		Rendererspec	Second (right) value to compare with
//		/Operator	Rendererspec	Operator of {'>','<','>=','<=','==','!=' }
//		/True		{ RendererSpec for TestCondition == True }
//		/False		{ RendererSpec for TestCondition == False }
//		/Error		{ RendererSpec for TestCondition == Error }
//      ...
//	}</PRE>
class CompareValuesRenderer : public Renderer
{
public:
	//--- constructors
	CompareValuesRenderer(const char *name);
	~CompareValuesRenderer();

	//:Renders ?? on <I>reply </I>
	//!param: reply - out - the stream where the rendered output is written on.
	//!param: c - the context the renderer runs within.
	//!param: config - the configuration of the renderer.
	virtual void RenderAll(std::ostream &reply, Context &c, const ROAnything &config);

};

#endif
