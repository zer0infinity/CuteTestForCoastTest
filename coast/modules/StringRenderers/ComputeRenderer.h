/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ComputeRenderer_H
#define _ComputeRenderer_H

#include "config_StringRenderers.h"
#include "Renderer.h"

//---- ComputeRenderer ----------------------------------------------------------------
//! Compute two values
/*!
\par Configuration
\code
{
	/Number1		Rendererspec	First (left) value to compute with, internally used as long
	/Number2		Rendererspec	Second (right) value to compute with, internally used as long
	/Operator		Rendererspec	Operator of {'+','-','*','/','%','<','<=','>','>=','='}
	/Minimum		RendererSpec	optional, minimum value of result (defines a range)
	/Maximum		RendererSpec	optional, maximum value of result (defines a range)
}
\endcode

*/
class EXPORTDECL_STRINGRENDERERS ComputeRenderer : public Renderer
{
public:
	ComputeRenderer(const char *name);

	void RenderAll(std::ostream &reply, Context &c, const ROAnything &data);
};

#endif		//not defined _ComputeRenderer_H
