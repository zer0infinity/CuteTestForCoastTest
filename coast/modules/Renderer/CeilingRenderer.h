/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CeilingRenderer_H
#define _CeilingRenderer_H

#include "ComparingRenderer.h"

//---- CeilingRenderer ----------------------------------------------------------
//! <B>Renderers the first slot in a Anything, that is greater or equal than a given String</B> Precondition: the lists slots are sorted ascending
/*!
\par Configuration
\code
{
	/ListName	Rendererspec		mandatory, Rendered result is looked up, the returned list defines the key-value list
	/Key		Rendererspec		mandatory, Rendered result defines the Key.
}
\endcode
\par Example
\code
/TheList {
	/A01	First
	/B01	Second
	/C01	Last
}
{ /CeilingRenderer {
	/ListName	TheList
	/Key		B99
} }
\endcode

Renders : "Last"
*/
class CeilingRenderer : public ComparingRenderer
{
public:
	//--- constructors
	CeilingRenderer(const char *name);
	~CeilingRenderer();

protected:
	/*! finds the first slot in <I>list</I>, that is greater or equal than <I>key</I>
		\param key the Key to compare with.
		\param list list whose slots are compared.
		\return the index of the matching slot, -1 if no match.
		\pre the lists slots are sorted ascending */
	virtual long FindSlot(String &key, const ROAnything &list);
};

#endif
