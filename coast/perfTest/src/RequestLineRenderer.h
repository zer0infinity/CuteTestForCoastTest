/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RequestLineRenderer_H
#define _RequestLineRenderer_H

#include "config_perftest.h"
#include "Renderer.h"

//---- RequestLineRenderer -----------------------------------------------------------
class EXPORTDECL_PERFTEST RequestLineRenderer : public Renderer
{
public:
	RequestLineRenderer(const char *name);
	~RequestLineRenderer();

	virtual void RenderAll(std::ostream &reply, Context &c, const ROAnything &config);
};

#endif		//ifndef _RequestLineRenderer_H
