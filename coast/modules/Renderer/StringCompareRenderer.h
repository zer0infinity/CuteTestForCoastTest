/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _STRINGCOMPARERENDERER_H
#define _STRINGCOMPARERENDERER_H

#include "config_renderer.h"
#include "Renderer.h"

//---- StringCompareRenderer -----------------------------------------------------------
//! <B>Renderer compares two strings and choses renderer depending if they are equal or not</B>
/*!
<B>Configuration:</B><PRE>
{
	/String1	Rendererspec	mandatory, First string
	/String2	Rendererspec	mandatory, Second string
	/IgnoreCase	Boolean			optional,  default is case sensitive comparision
	/Equal		Rendererspec	optional, Renderer used if String1 is equal to String2
	/Unequal	Rendererspec	optional, Renderer used if String1 is not equal to String2
}
</PRE>
*/
class EXPORTDECL_RENDERER StringCompareRenderer : public Renderer
{
public:
	StringCompareRenderer(const char *name);
	~StringCompareRenderer();

	virtual void RenderAll(ostream &reply, Context &c, const ROAnything &data);
};

#endif		//ifndef _STRINGCOMPARERENDERER_H
