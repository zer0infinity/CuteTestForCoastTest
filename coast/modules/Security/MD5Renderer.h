/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MD5Renderer_H
#define _MD5Renderer_H

#include "Renderer.h"

//---- MD5Renderer ----------------------------------------------------------
//! <B>MD5 </B><BR>Configuration:
//! <PRE>
//! {
//!		/Mode	[Base64 (default)|Hex]
//!		/ToSign [Renderer Spec to provide input]
//! }
//! </PRE>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class MD5Renderer : public Renderer
{
public:
	//--- constructors
	MD5Renderer(const char *name);
	~MD5Renderer();
	virtual void RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
