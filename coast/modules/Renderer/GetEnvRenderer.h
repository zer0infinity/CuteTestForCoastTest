/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _GetEnvRenderer_H
#define _GetEnvRenderer_H

//---- Renderer include -------------------------------------------------
#include "LookupRenderers.h"

//---- GetEnvRenderer ----------------------------------------------------------
//! <B>use getenv() for lookup</B><BR>Configuration:
//! <PRE>
//! {
//!		/LookupName			Rendererspec	optional, name to lookup
//!		/Default			Rendererspec	optional, default if LookupName name slot was not found
//!		...
//! }
//! </PRE>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class GetEnvRenderer : public LookupRenderer
{
public:
	//--- constructors
	GetEnvRenderer(const char *name);
	~GetEnvRenderer();

protected:
	//! DoLookup to be implemented by subclasses
	//! \param context the context the renderer runs within.
	//! \param name the path to lookup
	//! \param delim a character specifying the named slot delimiter
	//! \param indexdelim a character specifying the unnamed slot delimiter (array indices)
	virtual ROAnything DoLookup(Context &context, const char *name, char delim, char indexdelim);
};

#endif
