/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ReplaceExtChrRenderer_H
#define _ReplaceExtChrRenderer_H

//---- Renderer include -------------------------------------------------
#include "Renderer.h"

class Context;
class ROAnything;
class ostream;

//---- ReplaceExtChrRenderer ----------------------------------------------------------
class ReplaceExtChrRenderer : public Renderer
{
public:
	//--- constructors
	ReplaceExtChrRenderer(const char *name);
	~ReplaceExtChrRenderer();

	virtual void RenderAll(ostream &reply, Context &c, const ROAnything &config);
};

#endif
