/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FloorRenderer_H
#define _FloorRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_renderer.h"
#include "ComparingRenderer.h"

//---- FloorRenderer ----------------------------------------------------------
//! <B>Renders the configuration of the slot in an Anything that is lower than a given Key</B>
/*!
<B>Configuration:</B><PRE>
{
	/ListData	Rendererspec		mandatory, lookup key to an Anything with a list of key-value pairs which must be sorted lexically ascending
	/Key		Rendererspec		mandatory, value to test Slotnames of List-Anything against
}
</PRE>
<B>Example:</B>
<PRE>
/TheList {
	/A01	First
	/B01	Second
	/C01	Last
}
{ /FloorRenderer {
	/ListData	TheList
	/Key		B99
} }
</PRE>
Renders : "Second"
 */
class EXPORTDECL_RENDERER FloorRenderer : public ComparingRenderer
{
public:
	//--- constructors
	FloorRenderer(const char *name);
	~FloorRenderer();

protected:
	//!finds the last slot in <I>list</I>, that is lower than <I>key</I>
	//! \param key the Key to compare with.
	//! \param list list whose slots are compared.
	//! \return the index of the matching slot, -1 if no match.
	//! \pre the lists slots are sorted ascending
	virtual long FindSlot(String &key, const ROAnything &list);
};

#endif
