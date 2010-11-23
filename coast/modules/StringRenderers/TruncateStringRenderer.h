/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TruncateStringRenderer_H
#define _TruncateStringRenderer_H

//---- Renderer include -------------------------------------------------
#include "Renderer.h"

//---- TruncateStringRenderer ----------------------------------------------------------
//!Structure of config
//!<PRE> {
//!		/String		Rendererspec	mandatory, string to be truncated according to given token
//!		/Token		Rendererspec	mandatory, token to split the given string into two pieces
//!		/Direction	String			optional ["LR"|"RL"], default "LR", specify the token search direction
//!		/PartToUse	String			optional ["L"|"R"], default "L", specify which part of the splitted string should be taken
//!		/Default	Rendererspec	optional, default is empty string, specify what should be returned when the token was not found
//! }
//!</PRE>
/*!
*/
class TruncateStringRenderer : public Renderer
{
public:
	//--- constructors
	TruncateStringRenderer(const char *name);
	~TruncateStringRenderer();

	virtual void RenderAll(std::ostream &reply, Context &c, const ROAnything &config);
};

#endif
