/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FormattedDateRenderer_H
#define _FormattedDateRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_StringRenderers.h"
#include "Renderer.h"

//---- FormattedDateRenderer ----------------------------------------------------------
//: comment Renderer
//	Structure of config:
//<PRE>	{
//		/Value		Rendererspec	String to be rendered
//	}</PRE>
class EXPORTDECL_STRINGRENDERERS FormattedDateRenderer : public Renderer
{
public:
	//--- constructors
	FormattedDateRenderer(const char *name);
	~FormattedDateRenderer();
	virtual void RenderAll(std::ostream &reply, Context &c, const ROAnything &config);

};

#endif
