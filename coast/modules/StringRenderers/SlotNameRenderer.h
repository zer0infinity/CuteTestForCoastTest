/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SlotNameRenderer_H
#define _SlotNameRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_hikups.h"
#include "Renderer.h"

//---- SlotNameRenderer ----------------------------------------------------------
class EXPORTDECL_HIKUPS SlotNameRenderer : public Renderer
{
public:
	//--- constructors
	SlotNameRenderer(const char *name);
	~SlotNameRenderer();

	virtual void RenderAll(ostream &reply, Context &c, const ROAnything &config);
};

#endif
